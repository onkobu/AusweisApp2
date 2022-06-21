/*
 * \copyright Copyright (c) 2015-2022 Governikus GmbH & Co. KG, Germany
 */

import QtQuick 2.12

import Governikus.View 1.0
import Governikus.Type.ApplicationModel 1.0
import Governikus.Type.AuthModel 1.0
import Governikus.Type.NumberModel 1.0
import Governikus.Type.SettingsModel 1.0
import Governikus.Type.ChatModel 1.0
import Governikus.Type.ConnectivityManager 1.0

Controller {
	enum WorkflowStates {
		Initial,
		Reader,
		Card,
		Update,
		Password,
		Processing
	}

	id: controller
	readonly property string currentState: AuthModel.currentState

	property bool connectedToCard: false
	property int workflowState: 0
	property bool workflowProgressVisible: false

	states: [
		State {
			when: AuthModel.currentState === "StateGetTcToken" && !ConnectivityManager.networkInterfaceActive
			StateChangeScript {
				script: controller.nextView(IdentifyView.SubViews.Connectivity)
			}
		},
		State {
			when: AuthModel.currentState === "StateGetTcToken" && ConnectivityManager.networkInterfaceActive
			StateChangeScript {
				script: {
					controller.nextView(IdentifyView.SubViews.Progress)
					AuthModel.continueWorkflow()
				}
			}
		}
	]

	Component.onCompleted: if (AuthModel.currentState === "StateProcessing") processStateChange()

	Connections {
		target: AuthModel
		onFireCurrentStateChanged: processStateChange()
		// This is necessary because onCurrentStateChanged is not
		// working, when we need to process a state a second time.
	}

	function showRemoveCardFeedback(success) {
		if (controller.connectedToCard) {
			controller.connectedToCard = false
			if (success) {
				//: INFO DESKTOP The authentication process finished successfully, the ID card may be removed from the card reader.
				ApplicationModel.showFeedback(qsTr("Process finished successfully. You may now remove your ID card from the device."))
			} else {
				//: INFO DESKTOP The authentication process is completed, the ID card may be removed from the card reader.
				ApplicationModel.showFeedback(qsTr("You may now remove your ID card from the device."))
			}
		}
	}

	function processStateChange() {
		switch (AuthModel.currentState) {
			case "Initial":
				break
			case "StateGetTcToken":
				controller.workflowState = IdentifyController.WorkflowStates.Initial
				break
			case "StatePreVerification":
				if (!NumberModel.isCanAllowedMode && SettingsModel.transportPinReminder) {
					SettingsModel.transportPinReminder = false
					controller.nextView(IdentifyView.SubViews.TransportPinReminder)
				} else {
					AuthModel.continueWorkflow()
				}
				break
			case "StateEditAccessRights":
				if (NumberModel.isCanAllowedMode && SettingsModel.skipRightsOnCanAllowed) {
					ChatModel.transferAccessRights()
					AuthModel.continueWorkflow()
				} else {
					controller.nextView(IdentifyView.SubViews.AccessRights)
				}
				AuthModel.setInitialPluginType()
				break
			case "StateSelectReader":
				controller.nextView(IdentifyView.SubViews.Workflow)
				setIdentifyWorkflowStateAndContinue(IdentifyController.WorkflowStates.Reader)
				break
			case "StateConnectCard":
				setIdentifyWorkflowStateAndContinue(IdentifyController.WorkflowStates.Card)
				break
			case "StateHandleRetryCounter":
				setIdentifyWorkflowStateAndContinue(IdentifyController.WorkflowStates.Update)
				break
			case "StateEnterPacePassword":
				controller.workflowState = IdentifyController.WorkflowStates.Password
				if (AuthModel.isBasicReader) {
					controller.nextView(IdentifyView.SubViews.Password)
				} else {
					AuthModel.continueWorkflow()
				}
				break
			case "StateUnfortunateCardPosition":
				controller.nextView(IdentifyView.SubViews.CardPosition)
				break
			case "StateSendDIDAuthenticateResponseEAC1":
				if (AuthModel.isCancellationByUser()) {
					controller.nextView(IdentifyView.SubViews.Aborting)
				}
				else {
					controller.workflowProgressVisible = true
					controller.nextView(IdentifyView.SubViews.Progress)
				}
				setIdentifyWorkflowStateAndContinue(IdentifyController.WorkflowStates.Processing)
				break
			case "StateCleanUpReaderManager":
				controller.connectedToCard = AuthModel.selectedReaderHasCard() && !AuthModel.isError;
				AuthModel.continueWorkflow()
				break;
			case "StateWriteHistory":
				showRemoveCardFeedback(true)
				AuthModel.continueWorkflow()
				break
			case "FinalState":
				if (AuthModel.error) {
					if (AuthModel.shouldSkipResultView()) {
						controller.nextView(IdentifyView.SubViews.ReturnToMain)
						AuthModel.continueWorkflow()
					} else {
						showRemoveCardFeedback(false)
						controller.nextView(IdentifyView.SubViews.Result)
					}
				} else if (ApplicationModel.currentWorkflow === "selfauthentication") {
					controller.nextView(IdentifyView.SubViews.Data)
				} else {
					controller.nextView(IdentifyView.SubViews.ReturnToMain)
					AuthModel.continueWorkflow()
				}
				controller.workflowProgressVisible = false
				break
			default:
				AuthModel.continueWorkflow()
		}
	}

	function setIdentifyWorkflowStateAndContinue(pState) {
		controller.workflowState = pState
		AuthModel.continueWorkflow()
	}
}
