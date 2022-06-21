/*
 * \copyright Copyright (c) 2019-2022 Governikus GmbH & Co. KG, Germany
 */

import QtQuick 2.12

QtObject {
	readonly property var tutorial_title: TextStyle {
		textSize: Style.dimens.tutorial_title_font_size
	}
	readonly property var tutorial_header: TextStyle {
		textSize: Style.dimens.tutorial_header_font_size
		textFamily: "Noto Serif"
	}
	readonly property var tutorial_header_secondary: TextStyle {
		textSize: Style.dimens.header_font_size
		textFamily: "Noto Serif"
	}
	readonly property var tutorial_content: TextStyle {
		textSize: Style.dimens.normal_font_size
		textFamily: "Noto Serif"
	}
}
