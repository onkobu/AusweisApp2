/*
 * \copyright Copyright (c) 2020-2022 Governikus GmbH & Co. KG, Germany
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.12

import Governikus.Global 1.0
import Governikus.Style 1.0

GPaneBackground {
	id: root

	property alias title: text.text
	property alias image: image.source

	signal clicked()

	Accessible.role: Accessible.Button
	Accessible.name: title
	Accessible.onPressAction: if (Qt.platform.os === "ios") clicked()

	color: Qt.darker(Style.color.accent, mouseArea.pressed ? Constants.highlightDarkerFactor : 1)

	MouseArea {
		id: mouseArea

		anchors.fill: root

		onClicked: root.clicked()
	}

	TintableIcon {
		id: image

		anchors {
			left: root.left
			verticalCenter: root.verticalCenter
			leftMargin: Constants.component_spacing
		}
		height: root.height - Constants.pane_padding

		Accessible.ignored: true

		sourceSize.height: Style.dimens.large_icon_size
		tintColor: text.color
	}

	GText {
		id: text

		anchors {
			right: root.right
			left: image.right
			verticalCenter: root.verticalCenter
			leftMargin: Constants.text_spacing
			rightMargin: Constants.text_spacing
		}

		Accessible.ignored: true

		textStyle: Style.text.title_inverse
		color: Qt.darker(textStyle.textColor, mouseArea.pressed ? Constants.highlightDarkerFactor : 1)
		horizontalAlignment: Text.AlignHCenter
		font.bold: true
	}
}
