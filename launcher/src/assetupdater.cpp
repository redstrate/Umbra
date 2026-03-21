// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "assetupdater.h"
#include "umbra_log.h"
#include "profileconfig.h"
#include "utility.h"

#include <KLocalizedString>
#include <KTar>
#include <KZip>
#include <QCoroSignal>
#include <QFile>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QStandardPaths>
#include <qcorofuture.h>
#include <qcoronetworkreply.h>

using namespace Qt::StringLiterals;

AssetUpdater::AssetUpdater(Profile &profile, LauncherCore &launcher, QObject *parent)
    : QObject(parent)
    , launcher(launcher)
    , m_profile(profile)
{
}

QCoro::Task<bool> AssetUpdater::update()
{
    qInfo(UMBRA_LOG) << "Checking for compatibility tool updates...";

    m_dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    if (LauncherCore::needsCompatibilityTool()) {
        const QDir compatibilityToolDir = m_dataDir.absoluteFilePath(QStringLiteral("tool"));
        m_wineDir = compatibilityToolDir.absoluteFilePath(QStringLiteral("wine"));
        m_dxvkDir = compatibilityToolDir.absoluteFilePath(QStringLiteral("dxvk"));

        Utility::createPathIfNeeded(m_wineDir);
        Utility::createPathIfNeeded(m_dxvkDir);

        if (m_profile.config()->wineType() == Profile::WineType::BuiltIn && !co_await checkRemoteCompatibilityToolVersion()) {
            co_return false;
        }

        // TODO: should DXVK be tied to this setting...?
        if (m_profile.config()->wineType() == Profile::WineType::BuiltIn && !co_await checkRemoteDxvkVersion()) {
            co_return false;
        }
    }

    m_dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    co_return true;
}

QCoro::Task<bool> AssetUpdater::checkRemoteCompatibilityToolVersion()
{
    const QNetworkRequest request(wineReleasesUrl());
    Utility::printRequest(QStringLiteral("GET"), request);

    const auto reply = co_await launcher.mgr()->get(request);
    if (reply->error() != QNetworkReply::NetworkError::NoError) {
        Q_EMIT launcher.assetError(i18n("Could not check for the latest compatibility tool version.\n\n%1", reply->errorString()));

        const bool shouldContinue = co_await qCoro(&launcher, &LauncherCore::assetDecided);
        co_return shouldContinue;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    const QJsonArray releases = doc.array();
    const QJsonObject latestRelease = releases.first().toObject();

    if (latestRelease.contains("tag_name"_L1)) {
        m_remoteCompatibilityToolVersion = latestRelease["tag_name"_L1].toString();
    } else if (latestRelease.contains("name"_L1)) {
        m_remoteCompatibilityToolVersion = latestRelease["name"_L1].toString();
    } else {
        m_remoteCompatibilityToolVersion = latestRelease["commit"_L1].toObject()["sha"_L1].toString();
    }

    for (auto asset : latestRelease["assets"_L1].toArray()) {
        if (asset["name"_L1].toString().contains("ubuntu"_L1)) {
            m_remoteCompatibilityToolUrl = asset["browser_download_url"_L1].toString();
        }
    }

    // fallback to first asset just in case
    if (m_remoteCompatibilityToolUrl.isEmpty()) {
        m_remoteCompatibilityToolUrl = latestRelease["assets"_L1].toArray().first()["browser_download_url"_L1].toString();
    }

    qInfo(UMBRA_LOG) << "Compatibility tool remote version" << m_remoteCompatibilityToolVersion;

    // TODO: this version should not be profile specific
    if (m_remoteCompatibilityToolVersion != m_profile.compatibilityToolVersion()) {
        qInfo(UMBRA_LOG) << "Compatibility tool out of date";

        co_return co_await installCompatibilityTool();
    }

    co_return true;
}

QCoro::Task<bool> AssetUpdater::checkRemoteDxvkVersion()
{
    const QNetworkRequest request(dxvkReleasesUrl());
    Utility::printRequest(QStringLiteral("GET"), request);

    const auto reply = co_await launcher.mgr()->get(request);
    if (reply->error() != QNetworkReply::NetworkError::NoError) {
        Q_EMIT launcher.assetError(i18n("Could not check for the latest DXVK version.\n\n%1", reply->errorString()));

        const bool shouldContinue = co_await qCoro(&launcher, &LauncherCore::assetDecided);
        co_return shouldContinue;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    const QJsonArray releases = doc.array();
    const QJsonObject latestRelease = releases.first().toObject();

    if (latestRelease.contains("tag_name"_L1)) {
        m_remoteDxvkToolVersion = latestRelease["tag_name"_L1].toString();
    } else if (latestRelease.contains("name"_L1)) {
        m_remoteDxvkToolVersion = latestRelease["name"_L1].toString();
    } else {
        m_remoteDxvkToolVersion = latestRelease["commit"_L1].toObject()["sha"_L1].toString();
    }
    m_remoteDxvkToolUrl = latestRelease["assets"_L1].toArray().first()["browser_download_url"_L1].toString();

    qInfo(UMBRA_LOG) << "DXVK remote version" << m_remoteDxvkToolVersion;

    QString localDxvkVersion;
    const QString dxvkVer = m_dxvkDir.absoluteFilePath(QStringLiteral("dxvk.ver"));
    if (QFile::exists(dxvkVer)) {
        localDxvkVersion = Utility::readVersion(dxvkVer);
        qInfo(UMBRA_LOG) << "Local DXVK version:" << localDxvkVersion;
    }

    // TODO: this version should not be profile specific
    if (m_remoteDxvkToolVersion != localDxvkVersion) {
        qInfo(UMBRA_LOG) << "DXVK tool out of date";

        co_return co_await installDxvkTool();
    }

    co_return true;
}

QCoro::Task<bool> AssetUpdater::installCompatibilityTool() const
{
    Q_EMIT launcher.stageChanged(i18n("Updating compatibility tool..."));

    const auto request = QNetworkRequest(QUrl(m_remoteCompatibilityToolUrl));
    Utility::printRequest(QStringLiteral("GET"), request);

    const auto reply = launcher.mgr()->get(request);
    co_await reply;

    if (reply->error() != QNetworkReply::NetworkError::NoError) {
        Q_EMIT launcher.miscError(i18n("Could not update compatibility tool:\n\n%1", reply->errorString()));
        co_return false;
    }

    qInfo(UMBRA_LOG) << "Finished downloading compatibility tool";

    QFile file(m_tempDir.filePath(QStringLiteral("wine.tar.xz")));
    file.open(QIODevice::WriteOnly);
    file.write(reply->readAll());
    file.close();

    KTar archive(m_tempDir.filePath(QStringLiteral("wine.tar.xz")));
    if (!archive.open(QIODevice::ReadOnly)) {
        qCritical(UMBRA_LOG) << "Failed to install compatibility tool";
        Q_EMIT launcher.miscError(i18n("Failed to install compatibility tool."));
        co_return false;
    }

    const auto *root = dynamic_cast<const KArchiveDirectory *>(archive.directory()->entry(archive.directory()->entries().first()));
    Q_UNUSED(root->copyTo(m_wineDir.absolutePath(), true))

    archive.close();

    Utility::writeVersion(m_wineDir.absoluteFilePath(QStringLiteral("wine.ver")), m_remoteCompatibilityToolVersion);

    m_profile.setCompatibilityToolVersion(m_remoteCompatibilityToolVersion);

    co_return true;
}

QCoro::Task<bool> AssetUpdater::installDxvkTool() const
{
    Q_EMIT launcher.stageChanged(i18n("Updating DXVK..."));

    const auto request = QNetworkRequest(QUrl(m_remoteDxvkToolUrl));
    Utility::printRequest(QStringLiteral("GET"), request);

    const auto reply = co_await launcher.mgr()->get(request);

    qInfo(UMBRA_LOG) << "Finished downloading DXVK";

    if (reply->error() != QNetworkReply::NetworkError::NoError) {
        Q_EMIT launcher.miscError(i18n("Could not update DXVK:\n\n%1", reply->errorString()));
        co_return false;
    }

    QFile file(m_tempDir.filePath(QStringLiteral("dxvk.tar.xz")));
    file.open(QIODevice::WriteOnly);
    file.write(reply->readAll());
    file.close();

    KTar archive(m_tempDir.filePath(QStringLiteral("dxvk.tar.xz")));
    if (!archive.open(QIODevice::ReadOnly)) {
        qCritical(UMBRA_LOG) << "Failed to install DXVK";
        Q_EMIT launcher.miscError(i18n("Failed to install DXVK."));
        co_return false;
    }

    const auto *root = dynamic_cast<const KArchiveDirectory *>(archive.directory()->entry(archive.directory()->entries().first()));
    Q_UNUSED(root->copyTo(m_dxvkDir.absolutePath(), true))

    archive.close();

    Utility::writeVersion(m_dxvkDir.absoluteFilePath(QStringLiteral("dxvk.ver")), m_remoteDxvkToolVersion);

    co_return true;
}

QUrl AssetUpdater::dxvkReleasesUrl() const
{
    QUrl url;
    url.setScheme(launcher.config()->preferredProtocol());
    url.setHost(launcher.config()->githubApi());
    url.setPath(QStringLiteral("/repos/%1/releases").arg(launcher.config()->dXVKRepository()));

    return url;
}

QUrl AssetUpdater::wineReleasesUrl() const
{
    QUrl url;
    url.setScheme(launcher.config()->preferredProtocol());
    url.setHost(launcher.config()->githubApi());
    url.setPath(QStringLiteral("/repos/%1/releases").arg(launcher.config()->wineRepository()));

    return url;
}

bool AssetUpdater::extractZip(const QString &filePath, const QString &directory)
{
    KZip archive(filePath);
    if (!archive.open(QIODevice::ReadOnly)) {
        return false;
    }

    const KArchiveDirectory *root = archive.directory();
    if (!root->copyTo(directory, true)) {
        archive.close();
        return false;
    }

    archive.close();

    return true;
}

#include "moc_assetupdater.cpp"
