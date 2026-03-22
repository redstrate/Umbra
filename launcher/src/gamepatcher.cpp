// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gamepatcher.h"

#include "account.h"
#include "accountconfig.h"
#include "profile.h"
#include "profileconfig.h"

#include <QDir>

// The patching process happens statically (for now) as I couldn't figure out how to adapt LauncherTweaks to inject into a 32-bit process.
// It also does this weird backup system because the client is *very* specific about where files are located and what the executable is named as.
// So yes, I did try other methods and none of them worked as reliably at the time, unfortunately.

void patchGameExecutable(const Profile &profile)
{
    const QString originalGameExecutable = QDir(profile.config()->gamePath()).absoluteFilePath(QStringLiteral("ffxivgame.exe"));
    const QString backupExePath = QDir(profile.config()->gamePath()).absoluteFilePath(QStringLiteral("ffxivgame.retail.exe"));

    if (!QFile::exists(backupExePath)) {
        QFile::copy(originalGameExecutable, backupExePath);
    }

    QFile originalExe(backupExePath);
    Q_ASSERT(originalExe.open(QIODevice::ReadOnly));

    QByteArray exeData = originalExe.readAll();

    if (!profile.account()->config()->lobbyServer().isEmpty()) {
        // Patch the  default lobby config option
        const QByteArray existingLobbyUrl(QByteArrayLiteral("lobby01.ffxiv.com\0\0"));

        QByteArray newLobbyUrl(profile.account()->config()->lobbyServer().toLatin1());
        newLobbyUrl.resize(existingLobbyUrl.length(), '\0');

        exeData.replace(existingLobbyUrl, newLobbyUrl);
    }

    // Patch out the JNZ instruction with a JMP one so the tick count always passes
    exeData.replace(0x374b, 1, QByteArrayLiteral("\xEB"));

    // TODO: patch out the lobby port?
    // TODO: patch out the location of the user directory?

    QFile newExe(originalGameExecutable);
    Q_ASSERT(newExe.open(QIODevice::WriteOnly));
    newExe.write(exeData);
}
