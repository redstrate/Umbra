// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMutex>
#include <QNetworkAccessManager>
#include <QStorageInfo>
#include <QString>
#include <qcorotask.h>

#include <physis.hpp>

class LauncherCore;

// General-purpose patcher routine. It opens a nice dialog box, handles downloading
// and processing patches.
class Patcher : public QObject
{
    Q_OBJECT

public:
    Patcher(LauncherCore &launcher, const QString &baseDirectory, bool isBoot, QObject *parent = nullptr);
    ~Patcher() override;

    QCoro::Task<bool> patch(const physis_PatchList &patchList);

private:
    void setupDirectories();
    [[nodiscard]] QString getBaseString() const;

    struct QueuedPatch {
        QString name, version, path;
        QStringList hashes;
        long hashBlockSize;
        long length;
        bool isBoot;

        long bytesDownloaded = 0;
        bool downloaded = false;

        [[nodiscard]] QString getVersion() const
        {
            if (isBoot) {
                return QStringLiteral("ffxivboot - %1").arg(name);
            }
            return QStringLiteral("ffxivgame - %1").arg(name);
        }
    };

    void processPatch(const QueuedPatch &patch);

    QList<QueuedPatch> m_patchQueue;

    QDir m_patchesDir;
    QString m_baseDirectory;
    QStorageInfo m_patchesDirStorageInfo;
    QStorageInfo m_baseDirStorageInfo;

    int m_remainingPatches = -1;

    LauncherCore &m_launcher;

    QMutex m_finishedPatchesMutex;
    int m_finishedPatches = 0;
    bool m_isBoot;

    void updateDownloadProgress(int index, int received);
    void updateMessage();
};
