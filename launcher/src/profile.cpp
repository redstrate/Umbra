// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "profile.h"

#include <KLocalizedString>
#include <QFile>
#include <QJsonDocument>
#include <QProcess>

#include "account.h"
#include "umbra_log.h"
#include "launchercore.h"
#include "profileconfig.h"
#include "utility.h"

using namespace Qt::StringLiterals;

Profile::Profile(const QString &key, QObject *parent)
    : QObject(parent)
    , m_uuid(key)
    , m_config(new ProfileConfig(key, this))
{
    readWineInfo();
    readGameVersion();

    connect(m_config, &ProfileConfig::WineTypeChanged, this, &Profile::readWineInfo);
    connect(m_config, &ProfileConfig::GamePathChanged, this, &Profile::readGameVersion);
}

Profile::~Profile()
{
    m_config->save();
}

void Profile::readWineInfo()
{
    const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    const QDir compatibilityToolDir = dataDir.absoluteFilePath(QStringLiteral("tool"));
    const QDir wineDir = compatibilityToolDir.absoluteFilePath(QStringLiteral("wine"));
    if (wineDir.exists()) {
        const QString wineVer = wineDir.absoluteFilePath(QStringLiteral("wine.ver"));
        if (QFile::exists(wineVer)) {
            m_compatibilityToolVersion = Utility::readVersion(wineVer);
            qInfo(UMBRA_LOG) << "Compatibility tool version:" << m_compatibilityToolVersion;
        }
    }

    auto wineProcess = new QProcess(this);

    connect(wineProcess, &QProcess::readyReadStandardOutput, this, [wineProcess, this] {
        m_wineVersion = QString::fromUtf8(wineProcess->readAllStandardOutput().trimmed());
        Q_EMIT wineChanged();
    });

    wineProcess->start(winePath(), {QStringLiteral("--version")});
    wineProcess->waitForFinished();
}

void Profile::readGameVersion()
{
    m_bootVersion = Utility::readVersion(config()->gamePath() + QStringLiteral("/boot.ver"));
    m_gameVersion = Utility::readVersion(config()->gamePath() + QStringLiteral("/game.ver"));
    Q_EMIT versionTextChanged();
}

QString Profile::winePath() const
{
    switch (config()->wineType()) {
    case WineType::BuiltIn: {
        const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        const QDir compatibilityToolDir = dataDir.absoluteFilePath(QStringLiteral("tool"));
        const QDir wineDir = compatibilityToolDir.absoluteFilePath(QStringLiteral("wine"));
        const QDir wineBinDir = wineDir.absoluteFilePath(QStringLiteral("bin"));

        return wineBinDir.absoluteFilePath(QStringLiteral("wine64"));
    }
    case WineType::Custom: // custom pth
        return m_config->winePath();
    default:
        return {};
    }
}

void Profile::setWinePath(const QString &path)
{
    if (m_config->winePath() != path) {
        m_config->setWinePath(path);
        m_config->save();
        Q_EMIT winePathChanged();
    }
}

Account *Profile::account() const
{
    return m_account;
}

void Profile::setAccount(Account *account)
{
    m_account = account;
    m_config->setAccount(account->uuid());
    m_config->save();
    Q_EMIT accountChanged();
}

QString Profile::uuid() const
{
    return m_uuid;
}

QString Profile::wineVersionText() const
{
    if (!isWineInstalled()) {
        if (config()->wineType() == Profile::WineType::BuiltIn) {
            return i18n("Wine will be installed when you launch the game.");
        } else {
            return i18n("Wine is not installed.");
        }
    } else {
        return m_wineVersion;
    }
}

[[nodiscard]] bool Profile::isGameInstalled() const
{
    return !m_bootVersion.isEmpty() & !m_gameVersion.isEmpty();
}

[[nodiscard]] bool Profile::isWineInstalled() const
{
    return !m_wineVersion.isEmpty();
}

QString Profile::bootVersion() const
{
    return m_bootVersion;
}

QString Profile::baseGameVersion() const
{
    return m_gameVersion;
}

QString Profile::compatibilityToolVersion() const
{
    return m_compatibilityToolVersion;
}

void Profile::setCompatibilityToolVersion(const QString &version)
{
    m_compatibilityToolVersion = version;
}

QString Profile::subtitle() const
{
    if (m_gameVersion.isEmpty()) {
        return i18n("Unknown");
    } else {
        return m_gameVersion;
    }
}

ProfileConfig *Profile::config() const
{
    return m_config;
}

bool Profile::isGamePathDefault() const
{
    return m_config->gamePath() == m_config->defaultGamePathValue();
}

QString Profile::versionText() const
{
    QString versionText;
    if (m_bootVersion.isEmpty()) {
        versionText += i18n("Boot: Unknown");
    } else {
        versionText += i18n("Boot: %1", m_bootVersion);
    }

    if (m_gameVersion.isEmpty()) {
        versionText += i18n("\nGame: Unknown");
    } else {
        versionText += i18n("\nGame: %1", m_gameVersion);
    }

    return versionText;
}

#include "moc_profile.cpp"
