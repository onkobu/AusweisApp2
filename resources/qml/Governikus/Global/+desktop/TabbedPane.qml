/*
 * \copyright Copyright (c) 2019-2022 Governikus GmbH & Co. KG, Germany
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Governikus.Global 1.0
import Governikus.Style 1.0
import Governikus.Type.ApplicationModel 1.0
import Governikus.View 1.0


Item {
	id: root

	property var sectionsModel: undefined
	property var contentObjectModel: undefined
	property Component contentDelegate: null
	property Component footerItem: null
	property Component sectionDelegate: TabbedPaneDelegateOneLineText {
		sectionName: model ? (model.display ? model.display : model.modelData) : ""
	}
	property alias currentIndex: sectionNameList.currentIndex
	property alias sectionCount: sectionNameList.count

	readonly property var currentItemModel: sectionNameList.currentItem ? sectionNameList.currentItem.itemModel : null
	readonly property var currentContentItem: contentLoader.item
	readonly property real relativeListViewWidth: 0.3
	readonly property int availableHeight: height - 2 * contentItemPadding - 2 * Style.dimens.high_contrast_item_border
	property int contentPadding: Constants.pane_padding
	readonly property int contentItemPadding: currentContentItem && currentContentItem.contentPadding !== undefined ? currentContentItem.contentPadding : contentPadding

	function scrollYPositionIntoView(pYposition) {
		var dy = pYposition - flickable.contentY - flickable.height

		if (dy > 0 || flickable.contentY > 0) {
			flickable.contentY += dy

			if (flickable.contentY < 0)
				flickable.contentY = 0
			else if (flickable.contentY + flickable.height > flickable.contentHeight)
				flickable.contentY = flickable.contentHeight - flickable.height
		}
	}

	Item {
		anchors.fill: parent

		ColumnLayout {
			height: parent.height
			width: parent.width * relativeListViewWidth + 2 * Style.dimens.high_contrast_item_border

			anchors {
				left: parent.left
				top: parent.top
				bottom: parent.bottom
			}

			spacing: Constants.component_spacing
			z: 1

			GListView {
				id: sectionNameList

				Layout.fillWidth: true
				Layout.fillHeight: true

				activeFocusOnTab: true

				clip: true
				boundsBehavior: Flickable.StopAtBounds
				model: sectionsModel
				highlightFollowsCurrentItem: true
				highlight: null
				delegate: sectionNameDelegate
				scrollBarTopPadding: Constants.text_spacing
				scrollBarBottomPadding: Constants.text_spacing
				scrollBarAutohide: true
				onCurrentIndexChanged: flickable.positionViewAtBeginning()
			}

			Loader {
				Layout.preferredHeight: status === Loader.Ready ? item.height : 0
				Layout.fillWidth: true
				Layout.alignment: Qt.AlignLeft

				sourceComponent: footerItem
			}
		}

		RoundedRectangle {
			height: parent.height
			width: parent.width * (1.0 - relativeListViewWidth)

			anchors.right: parent.right

			radius: Style.dimens.corner_radius
			color: Style.color.background_pane
			topLeftCorner: false
			bottomLeftCorner: sectionNameList.contentHeight < height

			borderWidth: Style.dimens.high_contrast_item_border
			borderColor: Style.color.high_contrast_item_border

			Item {
				anchors.fill: parent
				anchors.margins: Style.dimens.high_contrast_item_border

				clip: true

				GFlickable {
					id: flickable

					anchors {
						fill: parent
						bottomMargin: contentItemPadding
						topMargin: contentItemPadding
					}

					contentHeight: contentLoader.height

					Loader {
						id: contentLoader

						anchors {
							top: parent.top
							left: parent.left
							right: parent.right
							leftMargin: contentItemPadding
							rightMargin: contentItemPadding
						}

						sourceComponent: {
							if (contentDelegate !== null) {
								return contentDelegate
							}

							if (contentObjectModel === undefined) {
								return null
							}

							if (sectionNameList.currentIndex < contentObjectModel.count){
								return contentObjectModel.get(sectionNameList.currentIndex)
							}
						}
					}
				}
			}

			ScrollGradients {
				anchors.fill: parent
				anchors.margins: Style.dimens.high_contrast_item_border
				color: parent.color
			}
		}
	}


	Component {
		id: sectionNameDelegate

		Control {
			id: delegateItem

			readonly property bool isFirstItem: index === 0
			readonly property bool isLastItem: index === ListView.view.count - 1
			readonly property bool isCurrentItem: ListView.isCurrentItem
			readonly property bool isPreviousToCurrentItem: index === ListView.view.currentIndex - 1
			readonly property var itemModel: model

			activeFocusOnTab: false
			Accessible.role: Accessible.PageTab
			Accessible.name: delegateLoader.item ? delegateLoader.item.sectionName : ""

			width: sectionNameList.width
			height: delegateLoader.height + 2 * Constants.pane_padding

			RoundedRectangle {
				id: background

				anchors {
					fill: parent
					topMargin: isCurrentItem && !isFirstItem ? Style.dimens.tabbed_pane_separator_size : 0
					bottomMargin: isCurrentItem && !isLastItem ? Style.dimens.tabbed_pane_separator_size : 0
					rightMargin: !isCurrentItem ? (Style.dimens.tabbed_pane_separator_size + 2 * Style.dimens.high_contrast_item_border) : 0
				}

				radius: Style.dimens.corner_radius
				color: isCurrentItem ? Style.color.background_pane : Style.color.background_pane_inactive

				topLeftCorner: isFirstItem
				topRightCorner: false
				bottomRightCorner: false
				bottomLeftCorner: isLastItem
				borderWidth: Style.dimens.high_contrast_item_border
				borderColor: Style.color.high_contrast_item_border

				Rectangle {
					width: isCurrentItem ? Style.dimens.high_contrast_item_border : 0
					anchors {
						right: parent.right
						top: parent.top
						bottom: parent.bottom
						topMargin: isFirstItem ? Style.dimens.high_contrast_item_border : 0
						bottomMargin: isLastItem ? Style.dimens.high_contrast_item_border : 0
					}

					color: background.color
				}
			}

			GSeparator {
				id: horizontalSeparator

				visible: !isLastItem && !isCurrentItem && !isPreviousToCurrentItem

				anchors {
					left: parent.left
					right: parent.right
					bottom: parent.bottom
					rightMargin: (!isCurrentItem ? Style.dimens.tabbed_pane_separator_size : 0) + 2 * Style.dimens.high_contrast_item_border
					leftMargin: Style.dimens.high_contrast_item_border
				}
			}

			FocusFrame {
				visible: delegateItem.focusReason !== Qt.MouseFocusReason

				framee: delegateLoader
				borderColor: Style.color.focus_indicator
			}

			Loader {
				id: delegateLoader

				property var model: itemModel

				anchors {
					verticalCenter: parent.verticalCenter
					left: parent.left
					right: parent.right
					leftMargin: Constants.pane_padding
					rightMargin: Constants.pane_padding + 2 * Style.dimens.high_contrast_item_border
				}

				sourceComponent: sectionDelegate
			}

			MouseArea {
				anchors.fill: parent

				cursorShape: index === currentIndex ? Qt.ArrowCursor : Qt.PointingHandCursor
				onClicked: {
					delegateItem.ListView.view.currentIndex = index
				}
			}
		}
	}
}
