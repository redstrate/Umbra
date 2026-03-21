// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <qcorotask.h>

#include "launchercore.h"

class LauncherCore;
class QNetworkReply;

class AssetUpdater : public QObject
{
    Q_OBJECT

public:
    explicit AssetUpdater(Profile &profile, LauncherCore &launcher, QObject *parent = nullptr);

    /// Checks for any asset updates.
    /// \return False if the asset update failed, which can be fatal.
    QCoro::Task<bool> update();

private:
    QCoro::Task<bool> checkRemoteCompatibilityToolVersion();
    QCoro::Task<bool> checkRemoteDxvkVersion();

    QCoro::Task<bool> installCompatibilityTool() const;
    QCoro::Task<bool> installDxvkTool() const;

    [[nodiscard]] QUrl dxvkReleasesUrl() const;
    [[nodiscard]] QUrl wineReleasesUrl() const;

    static bool extractZip(const QString &filePath, const QString &directory);

    LauncherCore &launcher;

    QTemporaryDir m_tempDir;
    QDir m_wineDir;
    QDir m_dxvkDir;
    QDir m_dataDir;

    QString m_remoteCompatibilityToolVersion;
    QString m_remoteCompatibilityToolUrl;
    QString m_remoteDxvkToolVersion;
    QString m_remoteDxvkToolUrl;

    Profile &m_profile;
};
