// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <KLocalizedString>
#include <QDir>
#include <QImage>
#include <QNetworkAccessManager>
#include <QStandardPaths>
#include <algorithm>
#include <qcoronetworkreply.h>

#include "account.h"
#include "accountconfig.h"
#include "assetupdater.h"
#include "umbra_log.h"
#include "gamerunner.h"
#include "launchercore.h"
#include "profileconfig.h"
#include "squareenixlogin.h"
#include "utility.h"

#ifdef HAS_DBUS
#include <QDBusConnection>
#include <QDBusReply>
#include <QGuiApplication>
#endif

using namespace Qt::StringLiterals;

LauncherCore::LauncherCore()
    : QObject()
{
    m_config = new Config(KSharedConfig::openConfig(QStringLiteral("umbrarc"), KConfig::SimpleConfig, QStandardPaths::AppConfigLocation), this);
    m_mgr = new QNetworkAccessManager(this);
    m_squareEnixLogin = new SquareEnixLogin(*this, this);
    m_profileManager = new ProfileManager(this);
    m_accountManager = new AccountManager(this);
    m_runner = new GameRunner(*this, this);

    connect(this, &LauncherCore::gameClosed, this, &LauncherCore::handleGameExit);

    m_profileManager->load();
    m_accountManager->load();

    // restore profile -> account connections
    for (const auto profile : m_profileManager->profiles()) {
        if (const auto account = m_accountManager->getByUuid(profile->config()->account())) {
            profile->setAccount(account);
        }
    }

    // set default profile, if found
    if (const auto profile = m_profileManager->getProfileByUUID(currentProfileId())) {
        setCurrentProfile(profile);
    }

    m_loadingFinished = true;
    Q_EMIT loadingFinished();
}

LauncherCore::~LauncherCore()
{
    m_config->save();
}

void LauncherCore::login(Profile *profile, const QString &username, const QString &password)
{
    Q_ASSERT(profile != nullptr);

    inhibitSleep();

    const auto loginInformation = new LoginInformation(this);
    loginInformation->profile = profile;

    loginInformation->username = username;
    loginInformation->password = password;

    if (profile->account()->config()->rememberPassword()) {
        profile->account()->setPassword(password);
    }

    beginLogin(*loginInformation);
}

bool LauncherCore::autoLogin(Profile *profile)
{
    Q_ASSERT(profile != nullptr);

    login(profile, profile->account()->config()->name(), profile->account()->getPassword());
    return true;
}

void LauncherCore::immediatelyLaunch(Profile *profile)
{
    Q_ASSERT(profile != nullptr);

    m_runner->beginGameExecutable(*profile, std::nullopt);
}

Profile *LauncherCore::currentProfile() const
{
    return m_profileManager->getProfile(m_currentProfileIndex);
}

void LauncherCore::setCurrentProfile(const Profile *profile)
{
    Q_ASSERT(profile != nullptr);

    const int newIndex = m_profileManager->getProfileIndex(profile->uuid());
    if (newIndex != m_currentProfileIndex) {
        m_currentProfileIndex = newIndex;

        auto stateConfig = KSharedConfig::openStateConfig();
        stateConfig->group(QStringLiteral("General")).writeEntry(QStringLiteral("CurrentProfile"), profile->uuid());
        stateConfig->sync();

        Q_EMIT currentProfileChanged();
    }
}

[[nodiscard]] QString LauncherCore::autoLoginProfileName() const
{
    return config()->autoLoginProfile();
}

[[nodiscard]] Profile *LauncherCore::autoLoginProfile() const
{
    if (config()->autoLoginProfile().isEmpty()) {
        return nullptr;
    }
    return m_profileManager->getProfileByUUID(config()->autoLoginProfile());
}

void LauncherCore::setAutoLoginProfile(const Profile *profile)
{
    if (profile != nullptr) {
        auto uuid = profile->uuid();
        if (uuid != config()->autoLoginProfile()) {
            config()->setAutoLoginProfile(uuid);
        }
    } else {
        config()->setAutoLoginProfile({});
    }

    config()->save();
    Q_EMIT autoLoginProfileChanged();
}

void LauncherCore::buildRequest(const Profile &settings, QNetworkRequest &request)
{
    Utility::setSSL(request);

    const auto hashString = QSysInfo::bootUniqueId();

    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(hashString);

    QByteArray bytes = hash.result();
    bytes.resize(4);

    auto checkSum = (uint8_t)-(bytes[0] + bytes[1] + bytes[2] + bytes[3]);
    bytes.prepend(checkSum);

    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("SQEXAuthor/2.0.0(Windows 6.2; ja-jp; %1)").arg(QString::fromUtf8(bytes.toHex())));

    request.setRawHeader(QByteArrayLiteral("Accept"),
                         QByteArrayLiteral("image/gif, image/jpeg, image/pjpeg, application/x-ms-application, application/xaml+xml, "
                                           "application/x-ms-xbap, */*"));
    request.setRawHeader(QByteArrayLiteral("Accept-Encoding"), QByteArrayLiteral("gzip, deflate"));
    request.setRawHeader(QByteArrayLiteral("Accept-Language"), QByteArrayLiteral("en-us"));
    request.setRawHeader(QByteArrayLiteral("Connection"), QByteArrayLiteral("Keep-Alive"));
    request.setRawHeader(QByteArrayLiteral("Cookie"), QByteArrayLiteral("_rsid=\"\""));
}

void LauncherCore::setupIgnoreSSL(QNetworkReply *reply)
{
    Q_ASSERT(reply != nullptr);

    if (reply->request().url().scheme() == QStringLiteral("http")) {
        connect(reply, &QNetworkReply::sslErrors, this, [reply](const QList<QSslError> &errors) {
            reply->ignoreSslErrors(errors);
        });
    }
}

bool LauncherCore::isLoadingFinished() const
{
    return m_loadingFinished;
}

bool LauncherCore::isWindows()
{
#if defined(Q_OS_WIN)
    return true;
#else
    return false;
#endif
}

bool LauncherCore::needsCompatibilityTool()
{
    return !isWindows();
}

bool LauncherCore::isPatching() const
{
    return m_isPatching;
}

QNetworkAccessManager *LauncherCore::mgr()
{
    return m_mgr;
}

Config *LauncherCore::config() const
{
    return m_config;
}

ProfileManager *LauncherCore::profileManager()
{
    return m_profileManager;
}

AccountManager *LauncherCore::accountManager()
{
    return m_accountManager;
}

QCoro::Task<> LauncherCore::beginLogin(LoginInformation &info)
{
    std::optional<LoginAuth> auth = co_await m_squareEnixLogin->login(&info);

    const auto assetUpdater = new AssetUpdater(*info.profile, *this, this);
    if (co_await assetUpdater->update()) {
        // If we expect an auth ticket, don't continue if missing
        if (auth == std::nullopt) {
            co_return;
        }

        Q_EMIT stageChanged(i18n("Launching game..."));

        m_runner->beginGameExecutable(*info.profile, auth);
    }

    assetUpdater->deleteLater();
}

QCoro::Task<> LauncherCore::handleGameExit(const Profile *profile)
{
    qCDebug(UMBRA_LOG) << "Game has closed.";

    uninhibitSleep();

    // Otherwise, quit when everything is finished.
    if (config()->closeWhenLaunched()) {
        QCoreApplication::exit();
    }

    co_return;
}

QCoro::Task<> LauncherCore::beginAutoConfiguration(Account *account, QString url)
{
    // NOTE: url is intentionally copied so it doesn't deference during coroutine execution

    QUrl requestUrl = QUrl::fromUserInput(url);
    requestUrl.setPath(QStringLiteral("/.well-known/legacyxiv"));

    auto reply = m_mgr->get(QNetworkRequest(requestUrl));
    co_await reply;

    if (reply->error() != QNetworkReply::NoError) {
        Q_EMIT account->autoConfigurationResult(i18n("Configuration Error"), i18n("Failed to fetch configuration from %1:\n\n%2", url, reply->errorString()));
        co_return;
    }

    const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    const QJsonObject obj = document.object();

    account->config()->setGamePatchServer(obj["game_patch_server"_L1].toString());
    account->config()->setBootPatchServer(obj["boot_patch_server"_L1].toString());
    account->config()->setLobbyServer(obj["lobby_server"_L1].toString());
    account->config()->setLoginServer(obj["login_server"_L1].toString());

    Q_EMIT account->autoConfigurationResult(i18n("Configuration Successful"), i18n("Configuration from %1 applied! You can now log into this server.", url));

    co_return;
}

void LauncherCore::inhibitSleep()
{
#ifdef HAS_DBUS
    if (screenSaverDbusCookie != 0)
        return;

    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.ScreenSaver"),
                                                          QStringLiteral("/ScreenSaver"),
                                                          QStringLiteral("org.freedesktop.ScreenSaver"),
                                                          QStringLiteral("Inhibit"));
    message << QGuiApplication::desktopFileName();
    message << i18n("Playing FFXIV");

    const QDBusReply<uint> reply = QDBusConnection::sessionBus().call(message);
    if (reply.isValid()) {
        screenSaverDbusCookie = reply.value();
    }
#endif
}

void LauncherCore::uninhibitSleep()
{
#ifdef HAS_DBUS
    if (screenSaverDbusCookie == 0)
        return;

    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.ScreenSaver"),
                                                          QStringLiteral("/ScreenSaver"),
                                                          QStringLiteral("org.freedesktop.ScreenSaver"),
                                                          QStringLiteral("UnInhibit"));
    message << static_cast<uint>(screenSaverDbusCookie);
    screenSaverDbusCookie = 0;
    QDBusConnection::sessionBus().send(message);
#endif
}

QString LauncherCore::currentProfileId() const
{
    return KSharedConfig::openStateConfig()->group(QStringLiteral("General")).readEntry(QStringLiteral("CurrentProfile"));
}

void LauncherCore::openOfficialLauncher(Profile *profile)
{
    Q_ASSERT(profile != nullptr);
    m_runner->openOfficialLauncher(*profile);
}

void LauncherCore::openConfigTool(Profile *profile)
{
    Q_ASSERT(profile != nullptr);
    m_runner->openConfigTool(*profile);
}

void LauncherCore::downloadServerConfiguration(Account *account, const QString &url)
{
    beginAutoConfiguration(account, url);
}

#include "moc_launchercore.cpp"
