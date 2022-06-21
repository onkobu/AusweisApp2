/*
 * \copyright Copyright (c) 2019-2022 Governikus GmbH & Co. KG, Germany
 */

import QtQuick 2.12
import QtGraphicalEffects 1.12

import Governikus.Global 1.0
import Governikus.Type.ProviderCategoryFilterModel 1.0


ListItem {
	readonly property bool isProvider: itemType === "provider"
	readonly property string providerDisplayName: !!display ? display : ""

	Accessible.description: (isProvider ? qsTr("Open provider details for %1").arg(display) : qsTr("Click to set category filter to %1").arg(text))

	text: (isProvider ? providerDisplayName : Category.displayString(providerCategory))
	icon: isProvider ? "" : Category.imageSource(providerCategory)
}
