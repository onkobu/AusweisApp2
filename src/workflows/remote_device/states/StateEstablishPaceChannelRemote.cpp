/*!
 * \copyright Copyright (c) 2017-2022 Governikus GmbH & Co. KG, Germany
 */

#include "StateEstablishPaceChannelRemote.h"

#include "ServerMessageHandler.h"

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(statemachine)


using namespace governikus;


StateEstablishPaceChannelRemote::StateEstablishPaceChannelRemote(const QSharedPointer<WorkflowContext>& pContext)
	: AbstractState(pContext, false)
	, GenericContextContainer(pContext)
	, mPasswordId(PacePasswordId::UNKNOWN)
{
}


void StateEstablishPaceChannelRemote::run()
{
	Q_ASSERT(getContext());
	Q_ASSERT(!getContext()->getSlotHandle().isEmpty());

	const QSharedPointer<RemoteServiceContext>& context = getContext();
	auto cardConnection = context->getCardConnection();
	if (!cardConnection)
	{
		Q_EMIT fireContinue();
		return;
	}

	const EstablishPaceChannel& paceChannel = context->getEstablishPaceChannel();
	mPasswordId = paceChannel.getPasswordId();
	QByteArray pacePassword;
	switch (mPasswordId)
	{
		case PacePasswordId::PACE_CAN:
			pacePassword = context->getCan().toLatin1();
			break;

		case PacePasswordId::PACE_PIN:
			pacePassword = context->getPin().toLatin1();
			break;

		case PacePasswordId::PACE_PUK:
			pacePassword = context->getPuk().toLatin1();
			break;

		default:
			Q_EMIT fireAbort();
			return;
	}

	qDebug() << "Establish connection using" << mPasswordId;
	Q_ASSERT(!pacePassword.isEmpty() && cardConnection);
	if (mPasswordId == PacePasswordId::PACE_PUK)
	{
		mConnections += cardConnection->callUnblockPinCommand(this,
				&StateEstablishPaceChannelRemote::onEstablishConnectionDone,
				pacePassword);
	}
	else
	{
		mConnections += cardConnection->callEstablishPaceChannelCommand(this,
				&StateEstablishPaceChannelRemote::onEstablishConnectionDone,
				mPasswordId,
				pacePassword,
				paceChannel.getChat(),
				paceChannel.getCertificateDescription());
	}
}


void StateEstablishPaceChannelRemote::onReaderInfoChanged(const ReaderInfo& pReaderInfo)
{
	if (!pReaderInfo.hasEidCard())
	{
		EstablishPaceChannelOutput channelOutput = EstablishPaceChannelOutput();
		channelOutput.setPaceReturnCode(CardReturnCode::CARD_NOT_FOUND);
		getContext()->setEstablishPaceChannelOutput(channelOutput);

		Q_EMIT fireContinue();
	}
}


void StateEstablishPaceChannelRemote::onEstablishConnectionDone(QSharedPointer<BaseCardCommand> pCommand)
{
	const QSharedPointer<EstablishPaceChannelCommand> establishPaceChannelCommand = pCommand.objectCast<EstablishPaceChannelCommand>();
	const QSharedPointer<UnblockPinCommand> unblockPinCommand = pCommand.objectCast<UnblockPinCommand>();

	if (establishPaceChannelCommand)
	{
		getContext()->setEstablishPaceChannelOutput(establishPaceChannelCommand->getPaceOutput());

		if (mPasswordId == PacePasswordId::PACE_PIN)
		{
			const CardReturnCode paceReturnCode = establishPaceChannelCommand->getPaceOutput().getPaceReturnCode();
			const bool isWrongPacePassword = CardReturnCodeUtil::equalsWrongPacePassword(paceReturnCode);

			if (isWrongPacePassword)
			{
				const int nextExpectedCounter = getContext()->getExpectedRetryCounter() - 1;
				qCDebug(statemachine) << "Wrong PACE password. Decreasing expected retry counter to" << nextExpectedCounter;
				getContext()->setExpectedRetryCounter(nextExpectedCounter);
			}
			else if (paceReturnCode == CardReturnCode::OK)
			{
				const int nextExpectedCounter = 3;
				qCDebug(statemachine) << "Correct PACE password. Expected retry counter is now" << nextExpectedCounter;
				getContext()->setExpectedRetryCounter(nextExpectedCounter);
			}
		}
	}
	else if (unblockPinCommand)
	{
		const CardReturnCode returnCode = unblockPinCommand->getReturnCode();
		qCDebug(statemachine) << "PACE_PUK finished with:" << returnCode;

		if (returnCode == CardReturnCode::OK || CardReturnCodeUtil::equalsWrongPacePassword(returnCode))
		{
			qCDebug(statemachine) << "Resetting PACE passwords and setting expected retry counter to -1";
			getContext()->resetPacePasswords();
			getContext()->setExpectedRetryCounter(-1);
		}

		EstablishPaceChannelOutput channelOutput = EstablishPaceChannelOutput();
		channelOutput.setPaceReturnCode(unblockPinCommand->getReturnCode());
		getContext()->setEstablishPaceChannelOutput(channelOutput);
	}
	else
	{
		Q_ASSERT(false);
		qCDebug(statemachine) << "Expected an EstablishPaceChannelCommand as response!";
	}

	Q_EMIT fireContinue();
}
