// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "account.h"

#include <qcorocore.h>
#include <qt6keychain/keychain.h>

#include "accountconfig.h"
#include "configsys.h"
#include "launchercore.h"
#include "umbra_log.h"
#include "utility.h"

using namespace Qt::StringLiterals;

Account::Account(const QString &key, LauncherCore *core, QObject *parent)
    : QObject(parent)
    , m_config(new AccountConfig(key, this))
    , m_key(key)
    , m_configSys(new ConfigSys())
    , m_core(core)
{
    fetchPassword();
    readConfigSys();

    connect(m_config, &AccountConfig::WinePrefixPathChanged, this, &Account::winePrefixChanged);
    connect(m_config, &AccountConfig::WinePrefixPathChanged, this, &Account::readConfigSys);
}

Account::~Account()
{
    m_config->save();
}

void Account::readConfigSys()
{
    const auto path = configSysPath();
    const auto buffer = krasis_read_file(path.toStdString().c_str());
    if (buffer.size > 0) {
        auto config = krasis_config_sys_parse(buffer);
        m_configSys->loadFromConfigSys(config);
    } else {
        qCWarning(UMBRA_LOG) << "Config.sys not found! Creating a new default one...";

        // Load defaults if file not found
        auto config = krasis_config_sys_default();
        m_configSys->loadFromConfigSys(config);
    }
}

void Account::writeConfigSys()
{
    const auto path = configSysPath();
    m_configSys->writeToConfigSys(Utility::toWindowsPath(m_core->config()->screenshotDir()), path);
}

void Account::writeConfigLng()
{
    const auto buffer = krasis_config_lng_write_to_buffer(static_cast<ClientLanguage>(config()->language()));
    const auto path = configLngPath();
    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(reinterpret_cast<const char *>(buffer.data), buffer.size);
    } else {
        qCWarning(UMBRA_LOG) << "Failed to write config.sys to" << path;
    }
}

QString Account::configSysPath() const
{
    const QDir ffxivDir = ffxivPath();
    return ffxivDir.absoluteFilePath(QStringLiteral("config.sys"));
}

QString Account::configLngPath() const
{
    const QDir ffxivDir = ffxivPath();
    return ffxivDir.absoluteFilePath(QStringLiteral("config.lng"));
}

QDir Account::ffxivPath() const
{
    QDir documentsDir;

    // TODO: think about how multiple profiles would work on Windows. Maybe we should store multiple config.sys in a dedicated folder, and then copy them at
    // runtime?
#if defined(Q_OS_WIN)
    documentsDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
#else
    const QDir winePrefixDir = config()->winePrefixPath();
    const QDir driveDir = winePrefixDir.absoluteFilePath(QStringLiteral("drive_c"));
    const QDir usersDir = driveDir.absoluteFilePath(QStringLiteral("users"));
    const QDir userDir = usersDir.absoluteFilePath(QString::fromUtf8(qgetenv("USER")));
    documentsDir = userDir.absoluteFilePath(QStringLiteral("Documents"));
#endif

    const QDir myGamesDir = documentsDir.absoluteFilePath(QStringLiteral("My Games"));
    return myGamesDir.absoluteFilePath(QStringLiteral("FINAL FANTASY XIV"));
}

ConfigSys *Account::configSys() const
{
    return m_configSys;
}

bool Account::isWinePrefixDefault() const
{
    return m_config->winePrefixPath() == m_config->defaultWinePrefixPathValue();
}

QString Account::uuid() const
{
    return m_key;
}

QString Account::getPassword()
{
    return QCoro::waitFor(getKeychainValue(QStringLiteral("password")));
}

void Account::setPassword(const QString &password)
{
    setKeychainValue(QStringLiteral("password"), password);

    if (m_needsPassword) {
        m_needsPassword = false;
        Q_EMIT needsPasswordChanged();
    }
}

void Account::setKeychainValue(const QString &key, const QString &value)
{
    auto job = new QKeychain::WritePasswordJob(QStringLiteral("Umbra"), this);
    job->setTextData(value);
    job->setKey(m_key + QStringLiteral("-") + key);
    job->start();

    connect(job, &QKeychain::WritePasswordJob::finished, this, [job] {
        if (job->error() != QKeychain::NoError) {
            qWarning(UMBRA_LOG) << "Error when writing" << job->errorString();
        }
    });
}

QCoro::Task<QString> Account::getKeychainValue(const QString &key)
{
    auto job = new QKeychain::ReadPasswordJob(QStringLiteral("Umbra"), this);
    job->setKey(m_key + QStringLiteral("-") + key);
    job->start();

    co_await qCoro(job, &QKeychain::ReadPasswordJob::finished);

    if (job->error() != QKeychain::NoError) {
        qWarning(UMBRA_LOG) << "Error when reading" << key << job->errorString() << "for account" << uuid();
    }

    co_return job->textData();
}

bool Account::needsPassword() const
{
    return m_needsPassword;
}

QCoro::Task<> Account::fetchPassword()
{
    const QString password = co_await getKeychainValue(QStringLiteral("password"));
    m_needsPassword = password.isEmpty();
    Q_EMIT needsPasswordChanged();

    co_return;
}

AccountConfig *Account::config() const
{
    return m_config;
}

#include "moc_account.cpp"
