// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <qcorotask.h>

#include "launchercore.h"

class Patcher;

class SquareEnixLogin : public QObject
{
    Q_OBJECT

public:
    explicit SquareEnixLogin(LauncherCore &window, QObject *parent = nullptr);

    /// Begins the login process for official Square Enix servers
    /// \param info The required login information
    /// \return Arguments used for logging into the game, if successful
    QCoro::Task<std::optional<LoginAuth>> login(LoginInformation *info);

private:
    /// Check for updates to the boot components. Even though we don't use these, it's checked by later login steps.
    QCoro::Task<bool> checkBootUpdates();

    using StoredInfo = std::pair<QString, QUrl>;

    /// Get the _STORED_ value used in the oauth step
    QCoro::Task<std::optional<StoredInfo>> getStoredValue();

    /// Logs into the server
    /// \return Returns false if the oauth call failed for some reason
    QCoro::Task<bool> loginOAuth();

    /// Registers a new session with the login server and patches the game if necessary
    /// \return Returns false if the session registration failed for some reason
    QCoro::Task<bool> registerSession();

    QString m_SID, m_username;
    LoginAuth m_auth;
    LoginInformation *m_info = nullptr;
    bool m_lastRunHasPatched = true;
    Patcher *m_patcher = nullptr;

    LauncherCore &m_launcher;
};
