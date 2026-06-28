// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Window
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.umbra

FormCard.FormCardPage {
    id: page

    property var account

    title: i18nc("@title:window", "Edit Account")

    Connections {
        target: page.account

        function onAutoConfigurationResult(title: string, subtitle: string): void {
            configurationPrompt.title = title;
            configurationPrompt.subtitle = subtitle;
            configurationPrompt.visible = true;
            page.reloadServerConfiguration();
        }

        function onResetConfiguration(): void {
            page.reloadServerConfiguration();
        }
    }

    // Needed because Kirigami Add-ons overwrites our bindings with static values
    function reloadServerConfiguration(): void {
        loginServerDelegate.text = page.account.config.loginServer;
        bootPatchServerDelegate.text = page.account.config.bootPatchServer;
        gamePatchServerDelegate.text = page.account.config.gamePatchServer;
        gameServerDelegate.text = page.account.config.lobbyServer;
    }

    Component.onCompleted: reloadServerConfiguration()

    actions: [
        Kirigami.Action {
            text: i18n("Delete Account…")
            tooltip: !enabled ? i18n("Cannot delete the only account.") : ""
            icon.name: "delete"
            enabled: LauncherCore.accountManager.canDelete(page.account)

            onTriggered: deletePrompt.open()
        }
    ]

    header: Kirigami.NavigationTabBar {
        width: parent.width

        actions: [
            Kirigami.Action {
                id: generalAction
                text: i18n("General")
                icon.name: "configure-symbolic"
            },
            Kirigami.Action {
                id: optionsAction
                text: i18n("Options")
                icon.name: "configure-symbolic"
            },
            Kirigami.Action {
                id: loginAction
                text: i18n("Login")
                icon.name: "password-copy-symbolic"
            },
            Kirigami.Action {
                id: serverAction
                text: i18n("Servers")
                icon.name: "network-server-symbolic"
            }
        ]

        Component.onCompleted: actions[0].checked = true
    }

    FormCard.FormCard {
        visible: generalAction.checked

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormTextFieldDelegate {
            id: usernameDelegate

            label: i18n("Username")
            text: page.account.config.name
            onTextChanged: {
                page.account.config.name = text;
                page.account.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: usernameDelegate
            below: winePrefixPathDelegate
            visible: LauncherCore.config.showDevTools
        }

        FormFolderDelegate {
            id: winePrefixPathDelegate

            text: i18n("Wine Prefix Folder")
            folder: page.account.config.winePrefixPath
            displayText: page.account.isWinePrefixDefault ? i18n("Default Location") : LauncherCore.readHostPath(folder)
            visible: LauncherCore.config.showDevTools

            onAccepted: (path) => {
                page.account.config.winePrefixPath = path;
                page.account.config.save();
            }
        }
    }

    FormCard.FormCard {
        visible: optionsAction.checked

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormComboBoxDelegate {
            id: languageDelegate

            text: i18n("Language")
            description: i18n("The language used in the game client.")
            model: ["Japanese", "English", "German", "French"]
            currentIndex: page.account.config.language
            onCurrentIndexChanged: {
                page.account.config.language = currentIndex;
                page.account.config.save();
                page.account.writeConfigLng();
            }
        }
    }

    FormCard.FormHeader {
        title: i18n("Network")
        visible: optionsAction.checked
    }

    FormCard.FormCard {
        visible: optionsAction.checked

        Layout.fillWidth: true

        FormCard.FormComboBoxDelegate {
            id: upnpPortMappingDelegate

            text: i18n("UPnP Port Mapping")
            model: ["Automatic", "Use", "Do not use"]
            currentIndex: page.account.configSys.upnpPortMapping
            onCurrentIndexChanged: {
                page.account.configSys.upnpPortMapping = currentIndex;
                page.account.writeConfigSys();
            }
        }

        FormCard.FormSpinBoxDelegate {
            id: upnpPortDelegate

            label: i18n("Port (55296 - 55551)")
            value: page.account.configSys.upnpPort
            onValueChanged: {
                page.account.configSys.upnpPort = value;
                page.account.writeConfigSys();
            }
            from: 55296
            to: 55551
        }
    }

    FormCard.FormHeader {
        title: i18n("Video Settings")
        visible: optionsAction.checked
    }

    FormCard.FormCard {
        visible: optionsAction.checked

        Layout.fillWidth: true

        FormCard.FormSpinBoxDelegate {
            id: devicesDelegate

            label: i18n("Devices")
            value: page.account.configSys.devices
            onValueChanged: {
                page.account.configSys.devices = value;
                page.account.writeConfigSys();
            }
        }

        FormCard.FormComboBoxDelegate {
            id: displayModeDelegate

            text: i18n("UPnP Port Mapping")
            model: ["Windowed", "Full Screen"]
            currentIndex: page.account.configSys.displayMode
            onCurrentIndexChanged: {
                page.account.configSys.displayMode = currentIndex;
                page.account.writeConfigSys();
            }
        }

        FormCard.FormSpinBoxDelegate {
            id: windowWidthDelegate

            label: i18n("Window Size Width")
            value: page.account.configSys.windowWidth
            onValueChanged: {
                page.account.configSys.windowWidth = value;
                page.account.writeConfigSys();
            }
            from: 1
            to: 99999
        }

        FormCard.FormSpinBoxDelegate {
            id: windowHeightDelegate

            label: i18n("Window Size Height")
            value: page.account.configSys.windowHeight
            onValueChanged: {
                page.account.configSys.windowHeight = value;
                page.account.writeConfigSys();
            }
            from: 1
            to: 99999
        }

        FormCard.FormComboBoxDelegate {
            id: multisamplingDelegate

            // TODO: lol, use list model here eventually
            property list <int> msaaValues: [0, 1, 3, 7]

            text: i18n("Multisampling")
            model: ["No AA", "2x MSAA", "4x MSAA", "8xQ MSAA"]
            currentIndex: page.account.configSys.multisampling
            onCurrentIndexChanged: {
                page.account.configSys.multisampling = msaaValues[currentIndex];
                page.account.writeConfigSys();
            }
        }

        FormCard.FormComboBoxDelegate {
            id: generalDrawingQualityDelegate

            text: i18n("General Drawing Quality")
            model: ["1 (Lowest)", "2", "3", "4", "5", "6", "7", "8 (Standard)", "9", "10 (Highest)"]
            currentIndex: page.account.configSys.generalDrawingQuality
            onCurrentIndexChanged: {
                page.account.configSys.generalDrawingQuality = currentIndex;
                page.account.writeConfigSys();
            }
        }

        FormCard.FormComboBoxDelegate {
            id: backgroundDrawingQualityDelegate

            text: i18n("Background Drawing Quality")
            model: ["1 (Low)", "2", "3 (Standard)", "4", "5 (High)"]
            currentIndex: page.account.configSys.backgroundDrawingQuality
            onCurrentIndexChanged: {
                page.account.configSys.backgroundDrawingQuality = currentIndex;
                page.account.writeConfigSys();
            }
        }

        FormCard.FormComboBoxDelegate {
            id: shadowDetailDelegate

            text: i18n("Background Drawing Quality")
            model: ["Lowest", "Low", "Standard", "High", "Highest"]
            currentIndex: page.account.configSys.shadowDetail
            onCurrentIndexChanged: {
                page.account.configSys.shadowDetail = currentIndex;
                page.account.writeConfigSys();
            }
        }
    }

    FormCard.FormHeader {
        title: i18n("Graphics")
        visible: optionsAction.checked
    }

    FormCard.FormCard {
        visible: optionsAction.checked

        Layout.fillWidth: true

        FormCard.FormCheckDelegate {
            id: ambientOcclusionDelegate

            text: i18n("Ambient Occlusion")
            checked: page.account.configSys.ambientOcclusion
            onCheckedChanged: {
                page.account.configSys.ambientOcclusion = checked;
                page.account.writeConfigSys();
            }
        }

        FormCard.FormCheckDelegate {
            id: depthOfFieldDelegate

            text: i18n("Depth of Field")
            checked: page.account.configSys.depthOfField
            onCheckedChanged: {
                page.account.configSys.depthOfField = checked;
                page.account.writeConfigSys();
            }
        }

        FormCard.FormCheckDelegate {
            id: disableCutsceneEffectsDelegate

            text: i18n("Disable cutscene effects")
            checked: page.account.configSys.disableCutsceneEffects
            onCheckedChanged: {
                page.account.configSys.disableCutsceneEffects = checked;
                page.account.writeConfigSys();
            }
        }

        FormCard.FormCheckDelegate {
            id: enableHardwareMouseCursorDelegate

            text: i18n("Enable hardware mouse cursor")
            checked: page.account.configSys.enableHardwareCursor
            onCheckedChanged: {
                page.account.configSys.enableHardwareCursor = checked;
                page.account.writeConfigSys();
            }
        }

        FormCard.FormComboBoxDelegate {
            id: textureQualityDelegate

            text: i18n("Texture Quality")
            model: ["High", "Standard", "Low"]
            currentIndex: page.account.configSys.textureQuality
            onCurrentIndexChanged: {
                page.account.configSys.textureQuality = currentIndex;
                page.account.writeConfigSys();
            }
        }

        FormCard.FormComboBoxDelegate {
            id: textureFilteringDelegate

            text: i18n("Texture Filtering")
            model: ["Highest", "High", "Standard", "Low"]
            currentIndex: page.account.configSys.textureFiltering
            onCurrentIndexChanged: {
                page.account.configSys.textureFiltering = currentIndex;
                page.account.writeConfigSys();
            }
        }
    }

    FormCard.FormHeader {
        title: i18n("Sound")
        visible: optionsAction.checked
    }

    FormCard.FormCard {
        visible: optionsAction.checked

        Layout.fillWidth: true

        FormCard.FormCheckDelegate {
            id: disableSoundDeviceDelegate

            text: i18n("Disable Sound Device")
            checked: page.account.configSys.disableSoundDevice
            onCheckedChanged: {
                page.account.configSys.disableSoundDevice = checked;
                page.account.writeConfigSys();
            }
        }

        FormCard.FormCheckDelegate {
            id: playSoundDelegate

            text: i18n("Play sound when application is in background")
            checked: page.account.configSys.playSoundWhenApplicationIsInBackground
            onCheckedChanged: {
                page.account.configSys.playSoundWhenApplicationIsInBackground = checked;
                page.account.writeConfigSys();
            }
        }
    }

    FormCard.FormCard {
        visible: loginAction.checked

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormCheckDelegate {
            id: rememberPasswordDelegate

            text: i18n("Remember password")
            description: i18n("Stores the password on the device, using it's existing secure credential storage.")
            checked: page.account.config.rememberPassword
            onCheckedChanged: {
                page.account.config.rememberPassword = checked;
                page.account.config.save();
            }
        }
    }

    FormCard.FormCard {
        visible: serverAction.checked

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormTextFieldDelegate {
            id: serverUrlDelegate

            label: i18n("Autoconfig Server URL")
        }

        FormCard.FormDelegateSeparator {
            above: serverUrlDelegate
            below: downloadConfigDelegate
        }

        FormCard.FormButtonDelegate {
            id: downloadConfigDelegate

            icon.name: "download-symbolic"
            text: i18nc("@action:button", "Download Configuration")
            enabled: serverUrlDelegate.text.length > 0

            onClicked: LauncherCore.downloadServerConfiguration(page.account, serverUrlDelegate.text)
        }

        FormCard.FormDelegateSeparator {
            above: downloadConfigDelegate
            below: resetConfigDelegate
        }

        FormCard.FormButtonDelegate {
            id: resetConfigDelegate

            icon.name: "kt-restore-defaults-symbolic"
            text: i18nc("@action:button", "Reset to Defaults")

            onClicked: LauncherCore.resetServerConfiguration(page.account)
        }
    }

    FormCard.FormCard {
        visible: serverAction.checked

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing

        FormCard.FormTextFieldDelegate {
            id: loginServerDelegate

            label: i18n("Login Server")
            onTextChanged: {
                page.account.config.loginServer = text;
                page.account.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: loginServerDelegate
            below: bootPatchServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: bootPatchServerDelegate

            label: i18n("Boot Patch Server")
            onTextChanged: {
                page.account.config.bootPatchServer = text;
                page.account.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: bootPatchServerDelegate
            below: gamePatchServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: gamePatchServerDelegate

            label: i18n("Game Patch Server")
            onTextChanged: {
                page.account.config.gamePatchServer = text;
                page.account.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: gamePatchServerDelegate
            below: loginServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: gameServerDelegate

            label: i18n("Lobby Server")
            onTextChanged: {
                page.account.config.lobbyServer = text;
                page.account.config.save();
            }
        }
    }

    Kirigami.PromptDialog {
        id: deletePrompt

        title: i18nc("@title", "Delete Account")
        subtitle: i18nc("@label", "Are you sure you want to delete this account?")
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        showCloseButton: false
        parent: page.QQC2.Overlay.overlay

        onAccepted: {
            LauncherCore.accountManager.deleteAccount(page.account);
            page.Window.window.pageStack.layers.pop();
        }
    }

    Kirigami.PromptDialog {
        id: configurationPrompt

        standardButtons: Kirigami.Dialog.Ok
        showCloseButton: false
        parent: page.QQC2.Overlay.overlay
    }
}
