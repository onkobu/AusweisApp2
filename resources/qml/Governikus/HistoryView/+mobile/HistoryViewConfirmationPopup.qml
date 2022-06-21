/*
 * \copyright Copyright (c) 2018-2022 Governikus GmbH & Co. KG, Germany
 */

import QtQuick 2.12
import QtQuick.Controls 2.12

import Governikus.Global 1.0
import Governikus.Style 1.0
import Governikus.Type.ApplicationModel 1.0
import Governikus.Type.SettingsModel 1.0

ConfirmationPopup {
	//: LABEL ANDROID IOS
	title: qsTr("Delete history")
	//: LABEL ANDROID IOS Confirmaton popup to clear all history entries.
	text: qsTr("All history entries will be deleted.")
	//: LABEL ANDROID IOS
	okButtonText: qsTr("Delete")

	onConfirmed: SettingsModel.removeHistory("ALL_HISTORY")
}
