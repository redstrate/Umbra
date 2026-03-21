// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QNetworkAccessManager>
#include <QtQml>
#include <qcorotask.h>

#include "accountmanager.h"
#include "config.h"
#include "profile.h"
#include "profilemanager.h"

class SquareEnixLogin;
class AssetUpdater;
class GameRunner;

class LoginInformation : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString username MEMBER username)
    Q_PROPERTY(QString password MEMBER password)
    Q_PROPERTY(Profile *profile MEMBER profile)

public:
    explicit LoginInformation(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    Profile *profile = nullptr;

    QString username, password;
};

struct LoginAuth {
    QString SID;
    int region = 2; // america?
    Account *account = nullptr;
};

class LauncherCore : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool loadingFinished READ isLoadingFinished NOTIFY loadingFinished)
    Q_PROPERTY(bool isWindows READ isWindows CONSTANT)
    Q_PROPERTY(Config *config READ config CONSTANT)
    Q_PROPERTY(ProfileManager *profileManager READ profileManager CONSTANT)
    Q_PROPERTY(AccountManager *accountManager READ accountManager CONSTANT)
    Q_PROPERTY(Profile *currentProfile READ currentProfile WRITE setCurrentProfile NOTIFY currentProfileChanged)
    Q_PROPERTY(Profile *autoLoginProfile READ autoLoginProfile WRITE setAutoLoginProfile NOTIFY autoLoginProfileChanged)

public:
    LauncherCore();
    ~LauncherCore() override;

    /// Begins the login process.
    /// It's designed to be opaque as possible to the caller.
    /// \note The login process is asynchronous.
    Q_INVOKABLE void login(Profile *profile, const QString &username, const QString &password);

    /// Attempts to log into a profile without LoginInformation, which may or may not work depending on a combination of the password failing, OTP not being
    /// available to auto-generate, among other things. The launcher will still warn the user about any possible errors, however the call site will need to
    /// check the result to see whether they need to "reset" or show a failed state or not. \note The login process is asynchronous.
    Q_INVOKABLE bool autoLogin(Profile *profile);

    /// Launches the game without patching, or logging in.
    /// Meant to test if we can get to the title screen and is intended to fail to do anything else.
    Q_INVOKABLE void immediatelyLaunch(Profile *profile);

    [[nodiscard]] Profile *currentProfile() const;
    void setCurrentProfile(const Profile *profile);

    [[nodiscard]] QString autoLoginProfileName() const;
    [[nodiscard]] Profile *autoLoginProfile() const;
    void setAutoLoginProfile(const Profile *value);

    // Networking misc.
    void buildRequest(const Profile &settings, QNetworkRequest &request);
    void setupIgnoreSSL(QNetworkReply *reply);

    [[nodiscard]] bool isLoadingFinished() const;
    [[nodiscard]] static bool isWindows();
    [[nodiscard]] static bool needsCompatibilityTool();
    [[nodiscard]] Q_INVOKABLE bool isPatching() const;

    [[nodiscard]] QNetworkAccessManager *mgr();
    [[nodiscard]] Config *config() const;
    [[nodiscard]] ProfileManager *profileManager();
    [[nodiscard]] AccountManager *accountManager();

    /**
     * @brief Opens the official launcher. Useful if Umbra decides not to work that day!
     */
    Q_INVOKABLE void openOfficialLauncher(Profile *profile);

    /**
     * @brief Opens the config tool.
     */
    Q_INVOKABLE void openConfigTool(Profile *profile);

    Q_INVOKABLE void downloadServerConfiguration(Account *account, const QString &url);

Q_SIGNALS:
    void loadingFinished();
    void successfulLaunch();
    void gameClosed(Profile *profile);
    void loginError(QString message);
    void miscError(QString message);
    void assetError(QString message);
    void stageChanged(QString message, QString explanation = {});
    void stageIndeterminate();
    void stageDeterminate(int min, int max, int value);
    void currentProfileChanged();
    void autoLoginProfileChanged();
    void showWindow();
    void requiresUpdate(QString message);
    void updateDecided(bool allowUpdate);
    void assetDecided(bool shouldContinue);

protected:
    friend class Patcher;

    bool m_isPatching = false;

private:
    QCoro::Task<> beginLogin(LoginInformation &info);

    QCoro::Task<> handleGameExit(const Profile *profile);

    QCoro::Task<> beginAutoConfiguration(Account *account, QString url);

    /// Tell the system to keep the screen on and don't go to sleep
    void inhibitSleep();

    /// Tell the system we can allow the screen to turn off
    void uninhibitSleep();

    QString currentProfileId() const;

    bool m_loadingFinished = false;

    ProfileManager *m_profileManager = nullptr;
    AccountManager *m_accountManager = nullptr;

    SquareEnixLogin *m_squareEnixLogin = nullptr;

    QNetworkAccessManager *m_mgr = nullptr;
    Config *m_config = nullptr;
    GameRunner *m_runner = nullptr;

    int m_currentProfileIndex = 0;

    unsigned int screenSaverDbusCookie = 0;
};
