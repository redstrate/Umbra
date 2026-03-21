// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQml

import org.kde.kirigamiaddons.settings as KirigamiSettings
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents

import zone.xiv.umbra

KirigamiSettings.ConfigurationView {
    id: settingsPage

    readonly property bool isInitialSetup: !LauncherCore.profileManager.hasAnyExistingInstallations()

    modules: [
        KirigamiSettings.ConfigurationModule {
            moduleId: "general"
            text: i18n("General")
            icon.name: "zone.xiv.umbra"
            page: () => Qt.createComponent("zone.xiv.umbra", "GeneralSettings")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "profiles"
            text: i18n("Profiles")
            icon.name: "applications-games-symbolic"
            page: () => Qt.createComponent("zone.xiv.umbra", "ProfilesPage")
            visible: !settingsPage.isInitialSetup
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "accounts"
            text: i18n("Accounts")
            icon.name: "preferences-system-users-symbolic"
            page: () => Qt.createComponent("zone.xiv.umbra", "AccountsPage")
            visible: !settingsPage.isInitialSetup
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "devtool"
            text: i18n("Developer Settings")
            icon.name: "applications-development-symbolic"
            page: () => Qt.createComponent("zone.xiv.umbra", "DeveloperSettings")
            visible: LauncherCore.config.showDevTools
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "about"
            text: i18n("About Umbra")
            icon.name: "help-about-symbolic"
            page: () => Qt.createComponent("zone.xiv.umbra", "AboutPage")
        }
    ]
}
