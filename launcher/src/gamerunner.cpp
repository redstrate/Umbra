// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gamerunner.h"

#include "accountconfig.h"
#include "encryptedarg.h"
#include "gamepatcher.h"
#include "launchercore.h"
#include "processlogger.h"
#include "processwatcher.h"
#include "profileconfig.h"
#include "umbra_log.h"
#include "utility.h"

#include <KProcessList>
#include <QGuiApplication>

using namespace Qt::StringLiterals;

GameRunner::GameRunner(LauncherCore &launcher, QObject *parent)
    : QObject(parent)
    , m_launcher(launcher)
{
}

void GameRunner::beginGameExecutable(Profile &profile, const std::optional<LoginAuth> &auth)
{
    patchGameExecutable(profile);

    const QString gameExecutable = profile.config()->gamePath() + QStringLiteral("/ffxivgame.exe");
    beginVanillaGame(gameExecutable, profile, auth);

    Q_EMIT m_launcher.successfulLaunch();
}

void GameRunner::openOfficialLauncher(Profile &profile)
{
    const auto process = new QProcess(this);
    process->setProcessEnvironment(QProcessEnvironment::systemEnvironment());

    new ProcessLogger(QStringLiteral("ffxivlauncher"), process);

    launchExecutable(profile, process, {profile.config()->gamePath() + QStringLiteral("/ffxivboot.exe")}, false, true);
}

void GameRunner::openConfigTool(Profile &profile)
{
    const auto process = new QProcess(this);
    process->setProcessEnvironment(QProcessEnvironment::systemEnvironment());

    new ProcessLogger(QStringLiteral("ffxivconfig"), process);

    launchExecutable(profile, process, {profile.config()->gamePath() + QStringLiteral("/ffxivconfig.exe")}, false, true);
}

void GameRunner::beginVanillaGame(const QString &gameExecutablePath, Profile &profile, const std::optional<LoginAuth> &auth)
{
    const auto gameProcess = new QProcess(this);
    gameProcess->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    connect(gameProcess, &QProcess::finished, this, [this, &profile](const int exitCode) {
        qCInfo(UMBRA_LOG) << "Game process exited:" << exitCode;
        Q_EMIT m_launcher.gameClosed(&profile);
    });

    auto args = getGameArgs(profile, auth);

    new ProcessLogger(QStringLiteral("ffxivgame"), gameProcess);

    launchExecutable(profile, gameProcess, {gameExecutablePath, args}, true, true);
}

QString GameRunner::getGameArgs(const Profile &profile, const std::optional<LoginAuth> &auth) const
{
    QList<std::pair<QString, QString>> gameArgs;

    if (auth.has_value()) {
        gameArgs.push_back({QStringLiteral("SESSION_ID"), auth->SID});
    } else {
        gameArgs.push_back({QStringLiteral("SESSION_ID"), QString::number(1)});
    }

    // FIXME: this should belong somewhere else...
    if (LauncherCore::needsCompatibilityTool())
        Utility::createPathIfNeeded(profile.account()->config()->winePrefixPath());

    const QString argFormat = m_launcher.config()->encryptArguments() ? QStringLiteral(" /%1 =%2") : QStringLiteral(" %1=%2");

    QString argJoined;
    for (const auto &[key, value] : gameArgs) {
        argJoined += argFormat.arg(key, value);
    }

    return m_launcher.config()->encryptArguments() ? encryptGameArg(argJoined) : argJoined;
}

void GameRunner::launchExecutable(const Profile &profile, QProcess *process, const QStringList &args, bool isGame, bool needsRegistrySetup)
{
    if (m_launcher.config()->launchHack()) {
        QDir(profile.account()->config()->winePrefixPath()).removeRecursively();
    }

    QList<QString> arguments;
    auto env = process->processEnvironment();

    if (needsRegistrySetup) {
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
        setWindowsVersion(profile, QStringLiteral("win7"));

        // copy DXVK
        const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        const QDir compatibilityToolDir = dataDir.absoluteFilePath(QStringLiteral("tool"));
        const QDir dxvkDir = compatibilityToolDir.absoluteFilePath(QStringLiteral("dxvk"));
        const QDir dxvk64Dir = dxvkDir.absoluteFilePath(QStringLiteral("x32"));

        const QDir winePrefix = profile.account()->config()->winePrefixPath();
        const QDir driveC = winePrefix.absoluteFilePath(QStringLiteral("drive_c"));
        const QDir windows = driveC.absoluteFilePath(QStringLiteral("windows"));
        const QDir system32 = windows.absoluteFilePath(QStringLiteral("syswow64"));

        for (const auto &entry : dxvk64Dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot)) {
            if (QFile::exists(system32.absoluteFilePath(entry.fileName()))) {
                QFile::remove(system32.absoluteFilePath(entry.fileName()));
            }
            QFile::copy(entry.absoluteFilePath(), system32.absoluteFilePath(entry.fileName()));
        }
#endif
    }

    if (m_launcher.config()->enableRenderDocCapture()) {
        env.insert(QStringLiteral("VK_LAYER_RENDERDOC_Capture"), QStringLiteral("VK_LAYER_RENDERDOC_Capture"));
        env.insert(QStringLiteral("ENABLE_VULKAN_RENDERDOC_CAPTURE"), QString::number(1));
    }

#if defined(Q_OS_LINUX)
    env.insert(QStringLiteral("WINEESYNC"), QString::number(1));
    env.insert(QStringLiteral("WINEFSYNC"), QString::number(1));
    env.insert(QStringLiteral("WINEFSYNC_FUTEX2"), QString::number(1));

    const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString logDir = dataDir.absoluteFilePath(QStringLiteral("log"));

    env.insert(QStringLiteral("DXVK_LOG_PATH"), logDir);
    if (!m_launcher.config()->gPUDeviceFilter().isEmpty()) {
        env.insert(QStringLiteral("DXVK_FILTER_DEVICE_NAME"), m_launcher.config()->gPUDeviceFilter());
    }

    // Enable the Wayland backend if we detect we're running on Wayland, otherwise fallback to X11.
    if (QGuiApplication::platformName() == QStringLiteral("wayland") && m_launcher.config()->enableWayland()) {
        // We have to unset the DISPLAY variable for Wine to pick it up.
        env.remove(QStringLiteral("DISPLAY"));
    }
#endif

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    env.insert(QStringLiteral("WINEPREFIX"), profile.account()->config()->winePrefixPath());

    if (profile.config()->wineType() == Profile::WineType::BuiltIn) {
        env.insert(QStringLiteral("WINEDLLOVERRIDES"), QStringLiteral("msquic=,mscoree=n,b;d3d9,d3d11,d3d10core,dxgi=n,b"));

        // The game nor its tools like anything more than 4 cores.
        // I based this off the recommended system requirements of an Intel i7, which in 2010 would've had 4 cores.
        env.insert(QStringLiteral("WINE_CPU_TOPOLOGY"), QStringLiteral("4:0,1,2,3"));
    }

    arguments.append({QStringLiteral("taskset"), QStringLiteral("-a"), QStringLiteral("-c"), QStringLiteral("0-4")});

    arguments.push_back(profile.winePath());
#endif
    arguments.append(args);

    const QString executable = arguments.takeFirst();

    if (isGame) {
        process->setWorkingDirectory(profile.config()->gamePath());
    }

    process->setProcessEnvironment(env);

    process->setProgram(executable);
    process->setArguments(arguments);

    process->start();
}

void GameRunner::setWindowsVersion(const Profile &settings, const QString &version)
{
    const auto process = new QProcess(this);
    process->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    launchExecutable(settings, process, {QStringLiteral("winecfg"), QStringLiteral("/v"), version}, false, false);
    process->waitForFinished();
}
