/*
 * \copyright Copyright (c) 2016-2022 Governikus GmbH & Co. KG, Germany
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12

import Governikus.Global 1.0
import Governikus.Style 1.0
import Governikus.Type.UiModule 1.0

Item {
	id: content

	ListModel {
		id: navModel

		ListElement {
			image: "qrc:///images/mobile/material_home.svg"
			desc: QT_TR_NOOP("Start")
			module: UiModule.DEFAULT
		}

		ListElement {
			image: "qrc:///images/material_history.svg"
			desc: QT_TR_NOOP("History")
			module: UiModule.HISTORY
		}

		ListElement {
			image: "qrc:///images/mobile/platform_specific_phone.svg"
			desc: QT_TR_NOOP("Remote")
			module: UiModule.REMOTE_SERVICE
		}

		ListElement {
			image: "qrc:///images/material_settings.svg"
			desc: QT_TR_NOOP("Settings")
			module: UiModule.SETTINGS
		}

		ListElement {
			image: "qrc:///images/material_help.svg"
			desc: QT_TR_NOOP("Help")
			module: UiModule.HELP
		}
	}

	GSeparator {
		id: topBorderLine
		width: parent.width
	}

	RowLayout {
		anchors {
			left: parent.left
			right: parent.right
			bottom: parent.bottom
			top: topBorderLine.bottom
			leftMargin: Style.dimens.navigation_bar_padding
			rightMargin: Style.dimens.navigation_bar_padding
			bottomMargin: Constants.is_layout_android ? Style.dimens.navigation_bar_padding : Style.dimens.navigation_bar_text_padding
			topMargin: Style.dimens.navigation_bar_padding
		}

		Repeater {
			id: repeater

			model: navModel

			delegate: NavigationItem {
				Layout.fillWidth: true
				Layout.fillHeight: true

				Accessible.ignored: content.Accessible.ignored

				source: image
				text: qsTr(desc)
				selected: baseItem.activeModule === module ||
							module === UiModule.DEFAULT && (
								baseItem.activeModule === UiModule.IDENTIFY ||
								baseItem.activeModule === UiModule.PINMANAGEMENT ||
								baseItem.activeModule === UiModule.PROVIDER ||
								baseItem.activeModule === UiModule.CHECK_ID_CARD
							)
				onClicked: {
					baseItem.resetContentArea()
					baseItem.show(module)
				}
			}
		}
	}
}
