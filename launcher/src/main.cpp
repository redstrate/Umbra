// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <KAboutData>
#include <KIconTheme>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <KirigamiApp>
#include <QApplication> // NOTE: do not remove this, if your IDE suggests to do so
#include <QQuickStyle>
#include <kdsingleapplication.h>
#include <qcoroqml.h>

#ifdef Q_OS_WINDOWS
#include <QStyleFactory>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "umbra_log.h"
#endif

#include "umbra-version.h"
#include "launchercore.h"
#include "logger.h"
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

    KIconTheme::initTheme(); // TODO: KirigamiApp doesn't call this before app construction so it doesn't work on Windows

    KirigamiApp::App app(argc, argv);
    KirigamiApp kapp;

    // TODO: remove once https://invent.kde.org/libraries/kirigami-addons/-/merge_requests/399 is merged
#ifdef Q_OS_WINDOWS
    app.setStyle(QStyleFactory::create(QStringLiteral("Breeze")));
#endif

    const KDSingleApplication singleApplication;
    if (!singleApplication.isPrimaryInstance()) {
        return 0;
    }

    // Default to a sensible message pattern
    if (qEnvironmentVariableIsEmpty("QT_MESSAGE_PATTERN")) {
        qputenv("QT_MESSAGE_PATTERN", "[%{time yyyy-MM-dd h:mm:ss.zzz}] %{if-category}[%{category}] %{endif}[%{type}] %{message}");
    }

    KLocalizedString::setApplicationDomain("umbra");

    KAboutData about(QStringLiteral("umbra"),
                     i18n("Umbra"),
                     QStringLiteral(UMBRA_VERSION_STRING),
                     i18n("FFXIV 1.x Launcher"),
                     KAboutLicense::GPL_V3,
                     i18n("© 2026 Joshua Goins"));
    about.setOtherText(
        i18n("FINAL FANTASY, FINAL FANTASY XIV, FFXIV, SQUARE ENIX, and the SQUARE ENIX logo are registered trademarks or "
             "trademarks of Square Enix Holdings Co., Ltd."));
    about.addAuthor(i18n("Joshua Goins"),
                    i18n("Maintainer"),
                    QStringLiteral("josh@redstrate.com"),
                    QStringLiteral("https://redstrate.com/"),
                    QUrl(QStringLiteral("https://redstrate.com/rss-image.png")));
    about.setHomepage(QStringLiteral("https://xiv.zone/umbra"));
    about.addComponent(QStringLiteral("KDSingleApplication"),
                       i18n("Helper class for single-instance policy applications."),
                       QStringLiteral("1.1.1"),
                       QStringLiteral("https://github.com/KDAB/KDSingleApplication"),
                       KAboutLicense::MIT);
    about.setDesktopFileName(QStringLiteral("zone.xiv.umbra"));
    about.setBugAddress(QByteArrayLiteral("https://github.com/redstrate/umbra/issues"));
    about.setComponentName(QStringLiteral("umbra"));
    about.setProgramLogo(QStringLiteral("zone.xiv.umbra"));
    about.setOrganizationDomain(QByteArrayLiteral("xiv.zone"));

    KAboutData::setApplicationData(about);

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

    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    QObject::connect(&engine, &QQmlApplicationEngine::quit, &app, &QCoreApplication::quit);

    engine.loadFromModule(QStringLiteral("zone.xiv.umbra"), QStringLiteral("Main"));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return QCoreApplication::exec();
}
