// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QProcess>

class LauncherCore;
class Profile;
struct LoginAuth;

class GameRunner : public QObject
{
public:
    explicit GameRunner(LauncherCore &launcher, QObject *parent = nullptr);

    /// Begins the game executable.
    void beginGameExecutable(Profile &profile, const std::optional<LoginAuth> &auth);
    void openOfficialLauncher(Profile &profile);
    void openConfigTool(Profile &profile);

private:
    /// Starts a vanilla game session.
    void beginVanillaGame(const QString &gameExecutablePath, Profile &profile, const std::optional<LoginAuth> &auth);

    /// Returns the game arguments needed to properly launch the game. This encrypts it too if needed, and it's already joined!
    QString getGameArgs(const Profile &profile, const std::optional<LoginAuth> &auth) const;

    /// This wraps it in wine if needed.
    void launchExecutable(const Profile &profile, QProcess *process, const QStringList &args, bool isGame, bool needsRegistrySetup);

    void setWindowsVersion(const Profile &settings, const QString &version);

    LauncherCore &m_launcher;
};
