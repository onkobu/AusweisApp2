/*
 * \copyright Copyright (c) 2020-2022 Governikus GmbH & Co. KG, Germany
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12

import Governikus.Global 1.0
import Governikus.Style 1.0
import Governikus.TitleBar 1.0
import Governikus.View 1.0


SectionPage {
	id: root

	signal cancel()
	signal pinUnknown()
	signal pinKnown()

	navigationAction: NavigationAction { state: "cancel"; onClicked: cancel() }

	//: LABEL ANDROID IOS
	title: qsTr("Identify")

	content: Item {
		width: root.width
		height: root.height

		StatusIcon {
			id: image

			height: Math.min(Style.dimens.large_icon_size, parent.width * 0.4)

			anchors {
				top: parent.top
				left: parent.left
				margins: Constants.component_spacing
			}

			source: "qrc:///images/status_info.svg"
		}

		GText {
			anchors {
				left: image.right
				right: parent.right
				verticalCenter: image.verticalCenter
				margins: Constants.component_spacing
			}

			//: LABEL ANDROID IOS
			text: qsTr("Online identification with Transport PIN is not possible.")
		}

		GPane {
			anchors {
				left: parent.left
				right: parent.right
				top: image.bottom
				margins: Constants.component_spacing
			}

			//: LABEL ANDROID IOS
			title: qsTr("Do you know your six-digit PIN?")

			GText {
				width: parent.width

				//: LABEL ANDROID IOS
				text: qsTr("The personal, six-digit PIN is mandatory to use the online identification function.")
			}
		}

		RowLayout {
			anchors {
				left: parent.left
				right: parent.right
				bottom: parent.bottom
				margins: Constants.component_spacing
			}

			spacing: Constants.component_spacing

			GButton {
				Layout.fillWidth: true
				Layout.maximumWidth: Style.dimens.max_text_width / 2
				Layout.preferredWidth: parent.width / 2
				Layout.alignment: Qt.AlignCenter

				//: LABEL ANDROID IOS
				text: qsTr("No")

				onClicked: pinUnknown()
			}

			GButton {
				Layout.fillWidth: true
				Layout.maximumWidth: Style.dimens.max_text_width / 2
				Layout.preferredWidth: parent.width / 2
				Layout.alignment: Qt.AlignCenter

				//: LABEL ANDROID IOS
				text: qsTr("Yes")

				onClicked: pinKnown()
			}
		}
	}
}
