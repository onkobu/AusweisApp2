/*
 * \copyright Copyright (c) 2018-2022 Governikus GmbH & Co. KG, Germany
 */

import QtQuick 2.12
import QtQuick.Controls 2.12

import Governikus.TitleBar 1.0
import Governikus.View 1.0

Controller {
	id: controller

	property bool isAbstract: false
	property TitleBarAction titleBarAction: null
	readonly property bool sectionPageTypeMarker: true

	function setActive() {
		if (visible && !isAbstract) {
			forceActiveFocus()
		}
	}

	anchors.fill: parent

	Accessible.role: Accessible.Grouping
	activeFocusOnTab: !isAbstract

	onVisibleChanged: setActive()

	FocusPoint {}
}
