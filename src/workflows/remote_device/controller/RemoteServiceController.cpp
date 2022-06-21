/*!
 * \copyright Copyright (c) 2017-2022 Governikus GmbH & Co. KG, Germany
 */

#include "RemoteServiceController.h"

#include "context/RemoteServiceContext.h"
#include "states/FinalState.h"
#include "states/StateChangePinRemote.h"
#include "states/StateChangePinResponse.h"
#include "states/StateClearPacePasswords.h"
#include "states/StateEnterNewPacePinRemote.h"
#include "states/StateEnterPacePasswordRemote.h"
#include "states/StateEstablishPaceChannelRemote.h"
#include "states/StateEstablishPaceChannelResponse.h"
#include "states/StatePrepareChangePinRemote.h"
#include "states/StatePreparePaceRemote.h"
#include "states/StateProcessRemoteMessages.h"
#include "states/StateStartRemoteService.h"
#include "states/StateStopRemoteService.h"
#include "states/StateUpdateRetryCounter.h"
#include "states/StateVerifyRetryCounter.h"

#include <initializer_list>
#include <QDebug>


using namespace governikus;


RemoteServiceController::RemoteServiceController(QSharedPointer<RemoteServiceContext> pContext)
	: WorkflowController(pContext)
{
	mStateMachine.setProperty(AbstractState::cFORCE_START_STOP_SCAN, true);

	auto sStartRemoteService = addState<StateStartRemoteService>();
	mStateMachine.setInitialState(sStartRemoteService);
	auto sProcessRemoteMessages = addState<StateProcessRemoteMessages>();
	auto sUpdateRetryCounter = addState<StateUpdateRetryCounter>();
	auto sVerifyRetryCounter = addState<StateVerifyRetryCounter>();
	auto sPreparePaceRemote = addState<StatePreparePaceRemote>();
	auto sEnterPacePasswordRemote = addState<StateEnterPacePasswordRemote>();
	auto sEstablishPaceChannelRemote = addState<StateEstablishPaceChannelRemote>();
	auto sEstablishPaceChannelResponse = addState<StateEstablishPaceChannelResponse>();
	auto sPrepareChangePinRemote = addState<StatePrepareChangePinRemote>();
	auto sEnterNewPacePinRemote = addState<StateEnterNewPacePinRemote>();
	auto sChangePinRemote = addState<StateChangePinRemote>();
	auto sChangePinResponse = addState<StateChangePinResponse>();
	auto sClearPacePasswords = addState<StateClearPacePasswords>();
	auto sStopRemoteService = addState<StateStopRemoteService>();
	auto sFinal = addState<FinalState>();

	sStartRemoteService->addTransition(sStartRemoteService, &AbstractState::fireContinue, sProcessRemoteMessages);
	sStartRemoteService->addTransition(sStartRemoteService, &AbstractState::fireAbort, sStopRemoteService);

	sProcessRemoteMessages->addTransition(sProcessRemoteMessages, &StateProcessRemoteMessages::fireAbort, sStopRemoteService);
	sProcessRemoteMessages->addTransition(sProcessRemoteMessages, &StateProcessRemoteMessages::fireEstablishPaceChannel, sUpdateRetryCounter);
	sProcessRemoteMessages->addTransition(sProcessRemoteMessages, &StateProcessRemoteMessages::fireModifyPin, sPrepareChangePinRemote);
	sProcessRemoteMessages->addTransition(sProcessRemoteMessages, &StateProcessRemoteMessages::fireSecureMessagingStopped, sClearPacePasswords);

	sUpdateRetryCounter->addTransition(sUpdateRetryCounter, &AbstractState::fireContinue, sVerifyRetryCounter);
	sUpdateRetryCounter->addTransition(sUpdateRetryCounter, &AbstractState::fireAbort, sEstablishPaceChannelResponse);

	sVerifyRetryCounter->addTransition(sVerifyRetryCounter, &AbstractState::fireContinue, sPreparePaceRemote);
	sVerifyRetryCounter->addTransition(sVerifyRetryCounter, &AbstractState::fireAbort, sEstablishPaceChannelResponse);

	sPreparePaceRemote->addTransition(sPreparePaceRemote, &StatePreparePaceRemote::fireContinue, sEstablishPaceChannelRemote);
	sPreparePaceRemote->addTransition(sPreparePaceRemote, &StatePreparePaceRemote::fireEnterPacePassword, sEnterPacePasswordRemote);
	sPreparePaceRemote->addTransition(sPreparePaceRemote, &StatePreparePaceRemote::fireAbort, sEstablishPaceChannelResponse);

	sEnterPacePasswordRemote->addTransition(sEnterPacePasswordRemote, &AbstractState::fireContinue, sEstablishPaceChannelRemote);
	sEnterPacePasswordRemote->addTransition(sEnterPacePasswordRemote, &AbstractState::fireAbort, sEstablishPaceChannelResponse);

	sEstablishPaceChannelRemote->addTransition(sEstablishPaceChannelRemote, &StateEstablishPaceChannelRemote::fireContinue, sEstablishPaceChannelResponse);
	sEstablishPaceChannelRemote->addTransition(sEstablishPaceChannelRemote, &StateEstablishPaceChannelRemote::fireAbort, sEstablishPaceChannelResponse);

	sEstablishPaceChannelResponse->addTransition(sEstablishPaceChannelResponse, &StateEstablishPaceChannelResponse::fireContinue, sProcessRemoteMessages);
	sEstablishPaceChannelResponse->addTransition(sEstablishPaceChannelResponse, &StateEstablishPaceChannelResponse::fireWrongPacePassword, sClearPacePasswords);
	sEstablishPaceChannelResponse->addTransition(sEstablishPaceChannelResponse, &StateEstablishPaceChannelResponse::fireAbort, sProcessRemoteMessages);

	sPrepareChangePinRemote->addTransition(sPrepareChangePinRemote, &StatePrepareChangePinRemote::fireContinue, sChangePinRemote);
	sPrepareChangePinRemote->addTransition(sPrepareChangePinRemote, &StatePrepareChangePinRemote::fireEnterNewPacePin, sEnterNewPacePinRemote);
	sPrepareChangePinRemote->addTransition(sPrepareChangePinRemote, &StatePrepareChangePinRemote::fireAbort, sChangePinRemote);

	sEnterNewPacePinRemote->addTransition(sEnterNewPacePinRemote, &StateEnterNewPacePinRemote::fireContinue, sChangePinRemote);
	sEnterNewPacePinRemote->addTransition(sEnterNewPacePinRemote, &StateEnterNewPacePinRemote::fireAbort, sChangePinResponse);

	sChangePinRemote->addTransition(sChangePinRemote, &StateChangePinRemote::fireContinue, sChangePinResponse);
	sChangePinRemote->addTransition(sChangePinRemote, &StateChangePinRemote::fireAbort, sChangePinResponse);

	sChangePinResponse->addTransition(sChangePinResponse, &StateChangePinResponse::fireContinue, sProcessRemoteMessages);
	sChangePinResponse->addTransition(sChangePinResponse, &StateChangePinResponse::firePacePasswordModified, sClearPacePasswords);
	sChangePinResponse->addTransition(sChangePinResponse, &StateChangePinResponse::fireAbort, sProcessRemoteMessages);

	sClearPacePasswords->addTransition(sClearPacePasswords, &AbstractState::fireContinue, sProcessRemoteMessages);
	sClearPacePasswords->addTransition(sClearPacePasswords, &AbstractState::fireAbort, sProcessRemoteMessages);

	sStopRemoteService->addTransition(sStopRemoteService, &AbstractState::fireContinue, sFinal);
	sStopRemoteService->addTransition(sStopRemoteService, &AbstractState::fireAbort, sFinal);
}


RemoteServiceController::~RemoteServiceController()
{
}
