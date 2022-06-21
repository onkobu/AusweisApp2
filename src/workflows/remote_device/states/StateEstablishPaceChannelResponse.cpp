/*!
 * \copyright Copyright (c) 2017-2022 Governikus GmbH & Co. KG, Germany
 */

#include "StateEstablishPaceChannelResponse.h"

#include "ServerMessageHandler.h"

using namespace governikus;


StateEstablishPaceChannelResponse::StateEstablishPaceChannelResponse(const QSharedPointer<WorkflowContext>& pContext)
	: AbstractState(pContext, false)
	, GenericContextContainer(pContext)
{
}


void StateEstablishPaceChannelResponse::run()
{
	const QSharedPointer<RemoteServiceContext>& context = getContext();
	const auto& establishPaceChannelOutput = context->getEstablishPaceChannelOutput();

	const auto& remoteServer = context->getRemoteServer();
	if (remoteServer)
	{
		const auto& messageHandler = remoteServer->getMessageHandler();
		if (messageHandler)
		{
			Q_ASSERT(!context->getSlotHandle().isEmpty());

			messageHandler->sendEstablishPaceChannelResponse(
					context->getSlotHandle(),
					establishPaceChannelOutput
					);
		}
	}

	const bool isWrongPacePassword = CardReturnCodeUtil::equalsWrongPacePassword(establishPaceChannelOutput.getPaceReturnCode());
	context->setEstablishPaceChannel(nullptr);

	if (isWrongPacePassword)
	{
		Q_EMIT fireWrongPacePassword();
		return;
	}

	Q_EMIT fireContinue();
}
