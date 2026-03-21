// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtCore
import QtQuick
import QtQuick.Dialogs
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.umbra

FormCard.FormCardPage {
    id: page

    property var profile
    readonly property bool isInitialSetup: !LauncherCore.profileManager.hasAnyExistingInstallations()

    title: isInitialSetup ? i18n("Initial Setup") : i18n("Profile Setup")
    globalToolBarStyle: isInitialSetup ? Kirigami.ApplicationHeaderStyle.None : Kirigami.ApplicationHeaderStyle.ToolBar

    readonly property Kirigami.Separator prettySeparator: Kirigami.Separator {
        width: page.width
    }
    header: isInitialSetup ? prettySeparator : null

    data: FolderDialog {
        id: installFolderDialog

        onAccepted: {
            page.profile.config.gamePath = decodeURIComponent(selectedFolder.toString().replace("file://", ""));
            applicationWindow().checkSetup();
        }
    }

    Image {
        source: "qrc:/zone.xiv.umbra.svg"

        fillMode: Image.PreserveAspectFit
        visible: !LauncherCore.profileManager.hasAnyExistingInstallations()

        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.margins: Kirigami.Units.largeSpacing * 3
    }

    FormCard.FormCard {
        Layout.fillWidth: true
        Layout.topMargin: LauncherCore.profileManager.hasAnyExistingInstallations() ? Kirigami.Units.largeSpacing : 0

        FormCard.FormTextDelegate {
            id: helpText

            text: {
                if (page.isInitialSetup) {
                    return i18n("You have to setup the game to continue.");
                } else {
                    return i18n("You need to select a game installation for '%1'.", page.profile.config.name);
                }
            }
        }
    }


    FormCard.FormCard {
        visible: LauncherCore.profileManager.hasAnyExistingInstallations()

        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.fillWidth: true

        FormCard.FormTextDelegate {
            id: existingHelpDelegate

            text: i18n("You can select an existing game installation from another profile.")
        }

        FormCard.FormDelegateSeparator {
            above: existingHelpDelegate
        }

        Repeater {
            model: LauncherCore.profileManager

            FormCard.FormButtonDelegate {
                required property var profile

                text: profile.config.name
                description: profile.subtitle
                visible: profile.isGameInstalled

                onClicked: {
                    LauncherCore.currentProfile.config.gamePath = profile.config.gamePath;
                    applicationWindow().checkSetup();
                }
            }
        }
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing

        FormCard.FormButtonDelegate {
            id: useExistingDelegate

            text: i18n("Select an Existing Installation")
            icon.name: "document-import-symbolic"
            onClicked: installFolderDialog.open()
        }
    }

    FormCard.FormCard {
        visible: page.isInitialSetup

        Layout.topMargin: Kirigami.Units.largeSpacing

        FormCard.FormButtonDelegate {
            id: settingsButton

            text: i18nc("@action:button Application settings", "Settings")
            icon.name: "settings-configure"

            onClicked: Qt.createComponent("zone.xiv.umbra", "SettingsPage").createObject(page, { window: applicationWindow() }).open()
        }
    }
}
