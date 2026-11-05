// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <KAboutData>
#include <KLocalizedQmlContext>
#include <KLocalizedString>
#include <KirigamiAppDefaults>
#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <kdsingleapplication.h>
#include <kdsingleapplication_version.h>
#include <physis.hpp>
#include <qcoroqml.h>

#ifdef Q_OS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "launchercore.h"
#include "logger.h"
#include "umbra-version.h"
#include "umbra_log.h"
#include "utility.h"

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    // Restrict to first 14 threads on Windows as the game will crash when running on 16 or more threads.
    // This is handled via taskset on Linux, but on Windows that doesn't exist. If we set our own affinity, that's then passed to our children.
#if defined(Q_OS_WIN)
    HANDLE hProcess = GetCurrentProcess();

    unsigned long lProcessAffinityMask = 0;
    unsigned long lSystemAffinityMask = 0;
    GetProcessAffinityMask(hProcess, reinterpret_cast<PDWORD_PTR>(&lProcessAffinityMask), reinterpret_cast<PDWORD_PTR>(&lSystemAffinityMask));

    lProcessAffinityMask &= 0x3FFF;

    if (!SetProcessAffinityMask(hProcess, lProcessAffinityMask)) {
        qCWarning(UMBRA_LOG) << "Could not set process affinity, the game will NOT launch correctly on certain systems!";
    }
#endif

    QApplication app(argc, argv);
    KirigamiAppDefaults::apply(&app);

    const KDSingleApplication singleApplication;
    if (!singleApplication.isPrimaryInstance()) {
        qCWarning(UMBRA_LOG) << "Umbra is already running";
        return 0;
    }

    // Default to a sensible message pattern
    if (qEnvironmentVariableIsEmpty("QT_MESSAGE_PATTERN")) {
        qputenv("QT_MESSAGE_PATTERN", "[%{time yyyy-MM-dd h:mm:ss.zzz}] %{if-category}[%{category}] %{endif}[%{type}] %{message}");
    }

    KLocalizedString::setApplicationDomain("umbra");

    auto about = KAboutData::fromAppStreamId(u"zone.xiv.umbra"_s);
    about.setVersion(QByteArrayLiteral(UMBRA_VERSION_STRING));
    about.setCopyrightStatement(i18n("© 2026 Joshua Goins"));
    about.setOtherText(
        i18n("FINAL FANTASY, FINAL FANTASY XIV, FFXIV, SQUARE ENIX, and the SQUARE ENIX logo are registered trademarks or "
             "trademarks of Square Enix Holdings Co., Ltd."));
    about.addAuthor(i18n("Joshua Goins"),
                    i18n("Maintainer"),
                    QStringLiteral("josh@redstrate.com"),
                    QStringLiteral("https://redstrate.com/"),
                    QUrl(QStringLiteral("https://redstrate.com/rss-image.png")));
    about.addComponent(QStringLiteral("physis"),
                       i18n("Library for reading and writing FFXIV data."),
                       QString::fromLatin1(physis_get_physis_version()),
                       QStringLiteral("https://xiv.zone/physis"),
                       KAboutLicense::GPL_V3);
    about.addComponent(QStringLiteral("libphysis"),
                       i18n("C/C++ bindings for Physis."),
                       QString::fromLatin1(physis_get_libphysis_version()),
                       QStringLiteral("https://github.com/redstrate/libphysis"),
                       KAboutLicense::GPL_V3);
    about.addComponent(QStringLiteral("KDSingleApplication"),
                       i18n("Helper class for single-instance policy applications."),
                       QString::fromLatin1(KDSINGLEAPPLICATION_VERSION_STRING),
                       QStringLiteral("https://github.com/KDAB/KDSingleApplication"),
                       KAboutLicense::MIT);
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    about.addComponent(QStringLiteral("Wine"),
                       i18n("Used to run the Windows version of FFXIV."),
                       {},
                       QStringLiteral("https://winehq.org/"),
                       KAboutLicense::GPL_V2);
    about.addComponent(QStringLiteral("DXVK"),
                       i18n("Vulkan-based implementation of D3D8, 9, 10 and 11 for Linux / Wine"),
                       {},
                       QStringLiteral("https://github.com/doitsujin/dxvk"),
                       KAboutLicense::Unknown);
#endif

    KAboutData::setApplicationData(about);
    QGuiApplication::setWindowIcon(QIcon::fromTheme(u"zone.xiv.umbra"_s));

    initializeLogging();

    QCommandLineParser parser;
    about.setupCommandLine(&parser);

    parser.parse(QCoreApplication::arguments());
    about.processCommandLine(&parser);

    // We must handle these manually, since we use parse() and not process()
    if (parser.isSet(QStringLiteral("help")) || parser.isSet(QStringLiteral("help-all"))) {
        parser.showHelp();
    }

    if (parser.isSet(QStringLiteral("version"))) {
        parser.showVersion();
    }

    QCoro::Qml::registerTypes();

    QQmlApplicationEngine engine;
    KLocalization::setupLocalizedContext(&engine);

    QObject::connect(&engine, &QQmlApplicationEngine::quit, &app, &QCoreApplication::quit);

    engine.loadFromModule(QStringLiteral("zone.xiv.umbra"), QStringLiteral("Main"));
    if (engine.rootObjects().isEmpty()) {
        qCWarning(UMBRA_LOG) << "Failed to start application due to QML failing to load!";
        return -1;
    }

    return QCoreApplication::exec();
}
