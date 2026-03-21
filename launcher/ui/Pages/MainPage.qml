// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import zone.xiv.umbra

Kirigami.Page {
    id: page

    title: i18n("Home")

    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0

    actions: [
        Kirigami.Action {
            text: i18nc("@action:menu", "Tools")
            icon.name: "tools"

            Kirigami.Action {
                text: i18nc("@action:inmenu", "Official Launcher")
                icon.name: "application-x-executable"
                onTriggered: LauncherCore.openOfficialLauncher(LauncherCore.currentProfile)
            }
            Kirigami.Action {
                text: i18nc("@action:inmenu", "Config Tool")
                icon.name: "application-x-executable"
                onTriggered: LauncherCore.openConfigTool(LauncherCore.currentProfile)
            }
        },
        Kirigami.Action {
            text: i18nc("@action:button", "Settings")
            icon.name: "configure"
            onTriggered: Qt.createComponent("zone.xiv.umbra", "SettingsPage").createObject(page, { window: applicationWindow() }).open()
        }
    ]

    RowLayout {
        anchors.fill: parent

        spacing: 0

        LoginPage {
            id: loginPage

            Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
            Layout.minimumWidth: Kirigami.Units.gridUnit * 26
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillHeight: true
        }
    }
}
