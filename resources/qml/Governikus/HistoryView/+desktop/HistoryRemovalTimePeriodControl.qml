/*
 * \copyright Copyright (c) 2019-2022 Governikus GmbH & Co. KG, Germany
*/

import QtQuick 2.12
import QtQuick.Layouts 1.12

import Governikus.Global 1.0
import Governikus.Style 1.0
import Governikus.View 1.0
import Governikus.Type.ApplicationModel 1.0


RowLayout {
	id: root

	property string period
	property alias description: description
	property alias removalPeriod: removalPeriod

	Accessible.role: Accessible.Grouping
	Accessible.name: description.text

	spacing: Constants.groupbox_spacing

	GText {
		id: description

		activeFocusOnTab: true

		//: LABEL DESKTOP
		text: qsTr("Time period")
		textStyle: Style.text.normal_inverse

		FocusFrame {}
	}

	GComboBox {
		id: removalPeriod

		Layout.fillWidth: true
		height: ApplicationModel.scaleFactor * 50

		activeFocusOnTab: true
		model: ListModel {
			id: timePeriods

			//: LABEL DESKTOP
			ListElement { modelData: qsTr("Past hour"); value: "PAST_HOUR" }
			//: LABEL DESKTOP
			ListElement { modelData: qsTr("Past day"); value: "PAST_DAY" }
			//: LABEL DESKTOP
			ListElement { modelData: qsTr("Past week"); value: "PAST_WEEK" }
			//: LABEL DESKTOP
			ListElement { modelData: qsTr("Last four weeks"); value: "LAST_FOUR_WEEKS" }
			//: LABEL DESKTOP
			ListElement { modelData: qsTr("All history"); value: "ALL_HISTORY" }
		}

		onCurrentIndexChanged: root.period = timePeriods.get(currentIndex).value
	}
}
