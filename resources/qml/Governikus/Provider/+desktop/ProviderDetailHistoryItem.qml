/*
 * \copyright Copyright (c) 2016-2022 Governikus GmbH & Co. KG, Germany
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12

import Governikus.Global 1.0
import Governikus.Style 1.0
import Governikus.View 1.0

Item {
	id: baseItem

	property string providerName: ""
	property string purposeText: ""

	Accessible.role: Accessible.ListItem
	Accessible.name: date.text + ". " + providerName + ". " + purposeText

	height: columnLayout.height

	ColumnLayout {
		id: columnLayout

		anchors.left: parent.left
		anchors.right: parent.right
		anchors.top: parent.top

		spacing: Constants.text_spacing

		GText {
			id: date

			textStyle: Style.text.normal
			font.capitalization: Font.AllUppercase
			text: ( Utils.isToday(dateTime) ? qsTr("today") :
				  Utils.isYesterday(dateTime) ? qsTr("yesterday") :
				  Utils.isThisWeek(dateTime) ? dateTime.toLocaleString(Qt.locale(), qsTr("dddd")) :
				  dateTime.toLocaleString(Qt.locale(), qsTr("dd.MM.yyyy"))
				  )
		}

		GridLayout {
			Layout.fillWidth: true

			columnSpacing: Constants.text_spacing
			columns: 2

			GText {
				textStyle: Style.text.normal
				//: LABEL DESKTOP
				text: qsTr("Service:")
				font.weight: Font.Bold
				maximumLineCount: 1
			}

			GText {
				Layout.fillWidth: true

				textStyle: Style.text.normal
				text: purposeText
				maximumLineCount: 1
				elide: Text.ElideRight
			}

			GText {
				textStyle: Style.text.normal
				//: LABEL DESKTOP
				text: qsTr("Provider:")
				font.weight: Font.Bold
				maximumLineCount: 1
			}

			GText {
				Layout.fillWidth: true

				textStyle: Style.text.normal
				text: providerName
				maximumLineCount: 1
				elide: Text.ElideRight
			}
		}
	}

	FocusFrame {
		borderColor: Style.color.focus_indicator
	}
}
