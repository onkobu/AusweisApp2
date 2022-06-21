/*
 * \copyright Copyright (c) 2016-2022 Governikus GmbH & Co. KG, Germany
 */

#include "MessageDispatcher.h"

#include "messages/MsgHandlerAccessRights.h"
#include "messages/MsgHandlerApiLevel.h"
#include "messages/MsgHandlerAuth.h"
#include "messages/MsgHandlerBadState.h"
#include "messages/MsgHandlerCertificate.h"
#include "messages/MsgHandlerChangePin.h"
#include "messages/MsgHandlerEnterCan.h"
#include "messages/MsgHandlerEnterNewPin.h"
#include "messages/MsgHandlerEnterPin.h"
#include "messages/MsgHandlerEnterPuk.h"
#include "messages/MsgHandlerInfo.h"
#include "messages/MsgHandlerInsertCard.h"
#include "messages/MsgHandlerInternalError.h"
#include "messages/MsgHandlerInvalid.h"
#include "messages/MsgHandlerLog.h"
#include "messages/MsgHandlerReader.h"
#include "messages/MsgHandlerReaderList.h"
#include "messages/MsgHandlerUnknownCommand.h"
#include "VolatileSettings.h"

#include "context/AuthContext.h"
#include "context/ChangePinContext.h"
#include "ReaderManager.h"

#include <QLoggingCategory>

#include <algorithm>

Q_DECLARE_LOGGING_CATEGORY(json)

#define HANDLE_CURRENT_STATE(msgType, msgHandler) handleCurrentState(requestType, msgType, [&] {return msgHandler;})
#define HANDLE_INTERNAL_ONLY(msgHandler) handleInternalOnly(requestType, [&] {return msgHandler;})

using namespace governikus;

MessageDispatcher::MessageDispatcher()
	: mContext()
{
}


QByteArray MessageDispatcher::init(const QSharedPointer<WorkflowContext>& pWorkflowContext)
{
	Q_ASSERT(!mContext.isActiveWorkflow());

	mContext.clear();
	mContext.setWorkflowContext(pWorkflowContext);

	if (mContext.getContext<AuthContext>())
	{
		return MsgHandlerAuth().getOutput();
	}
	else if (mContext.getContext<ChangePinContext>())
	{
		return MsgHandlerChangePin().getOutput();
	}

	return QByteArray();
}


void MessageDispatcher::reset()
{
	mContext.clear();
	Env::getSingleton<VolatileSettings>()->setMessages();
	Env::getSingleton<VolatileSettings>()->setHandleInterrupt();
	Env::getSingleton<VolatileSettings>()->setDeveloperMode();
}


QByteArrayList MessageDispatcher::processReaderChange(const ReaderInfo& pInfo)
{
	QByteArrayList messages;
	messages << MsgHandlerReader(pInfo).getOutput();

	const auto& lastStateMsg = mContext.getLastStateMsg();
	if (lastStateMsg == MsgType::INSERT_CARD && !lastStateMsg)
	{
		const MsgHandlerInsertCard msg;
		mContext.addStateMsg(msg);
		messages << msg.getOutput();
	}

	return messages;
}


QByteArray MessageDispatcher::finish()
{
	Q_ASSERT(mContext.isActiveWorkflow());

	QByteArray result;
	if (auto authContext = mContext.getContext<AuthContext>())
	{
		result = MsgHandlerAuth(authContext).getOutput();
	}
	else if (auto changePinContext = mContext.getContext<ChangePinContext>())
	{
		result = MsgHandlerChangePin(changePinContext).getOutput();
	}

	reset();
	return result;
}


QByteArray MessageDispatcher::processStateChange(const QString& pState)
{
	if (!mContext.isActiveWorkflow() || pState.isEmpty())
	{
		qCCritical(json) << "Unexpected condition:" << mContext.getContext() << '|' << pState;
		return MsgHandlerInternalError(QLatin1String("Unexpected condition")).getOutput();
	}

	const Msg& msg = createForStateChange(MsgHandler::getStateMsgType(pState, mContext.getContext()->getEstablishPaceChannelType()));
	mContext.addStateMsg(msg);
	return msg;
}


Msg MessageDispatcher::createForStateChange(MsgType pStateType)
{
	if (mContext.getContext()->isWorkflowCancelled())
	{
		mContext.getContext()->setStateApproved();
		return MsgHandler::Void;
	}

	switch (pStateType)
	{
		case MsgType::ENTER_PIN:
			return MsgHandlerEnterPin(mContext);

		case MsgType::ENTER_NEW_PIN:
			return MsgHandlerEnterNewPin(mContext);

		case MsgType::ENTER_CAN:
			return MsgHandlerEnterCan(mContext);

		case MsgType::ENTER_PUK:
			return MsgHandlerEnterPuk(mContext);

		case MsgType::ACCESS_RIGHTS:
			return MsgHandlerAccessRights(mContext);

		case MsgType::INSERT_CARD:
			return MsgHandlerInsertCard(mContext);

		default:
			mContext.getContext()->setStateApproved();
			return MsgHandler::Void;
	}
}


Msg MessageDispatcher::processCommand(const QByteArray& pMsg)
{
	QJsonParseError jsonError {};
	const auto& json = QJsonDocument::fromJson(pMsg, &jsonError);
	if (jsonError.error != QJsonParseError::NoError)
	{
		return MsgHandlerInvalid(jsonError);
	}

	const auto& obj = json.object();
	auto msg = createForCommand(obj);
	msg.setRequest(obj);
	return msg;
}


MsgHandler MessageDispatcher::createForCommand(const QJsonObject& pObj)
{
	const auto& cmd = pObj.value(QLatin1String("cmd")).toString();
	if (cmd.isEmpty())
	{
		return MsgHandlerInvalid(QLatin1String("Command cannot be undefined"));
	}

	auto requestType = Enum<MsgCmdType>::fromString(cmd, MsgCmdType::UNDEFINED);
	qCDebug(json) << "Process type:" << requestType;
	switch (requestType)
	{
		case MsgCmdType::UNDEFINED:
			return MsgHandlerUnknownCommand(cmd);

		case MsgCmdType::CANCEL:
			return cancel();

		case MsgCmdType::ACCEPT:
			return accept();

		case MsgCmdType::INTERRUPT:
			return interrupt();

		case MsgCmdType::GET_API_LEVEL:
			return MsgHandlerApiLevel(mContext);

		case MsgCmdType::SET_API_LEVEL:
			return MsgHandlerApiLevel(pObj, mContext);

		case MsgCmdType::GET_READER:
			return MsgHandlerReader(pObj);

		case MsgCmdType::GET_READER_LIST:
			return MsgHandlerReaderList();

		case MsgCmdType::GET_LOG:
			return HANDLE_INTERNAL_ONLY(MsgHandlerLog());

		case MsgCmdType::GET_INFO:
			return MsgHandlerInfo();

		case MsgCmdType::RUN_AUTH:
			return mContext.isActiveWorkflow() ? MsgHandler(MsgHandlerBadState(requestType)) : MsgHandler(MsgHandlerAuth(pObj));

		case MsgCmdType::RUN_CHANGE_PIN:
			return mContext.isActiveWorkflow() ? MsgHandler(MsgHandlerBadState(requestType)) : MsgHandler(MsgHandlerChangePin(pObj));

		case MsgCmdType::GET_CERTIFICATE:
			return HANDLE_CURRENT_STATE({MsgType::ACCESS_RIGHTS}, MsgHandlerCertificate(mContext));

		case MsgCmdType::SET_PIN:
			return HANDLE_CURRENT_STATE({MsgType::ENTER_PIN}, MsgHandlerEnterPin(pObj, mContext));

		case MsgCmdType::SET_NEW_PIN:
			return HANDLE_CURRENT_STATE({MsgType::ENTER_NEW_PIN}, MsgHandlerEnterNewPin(pObj, mContext));

		case MsgCmdType::SET_CAN:
			return HANDLE_CURRENT_STATE({MsgType::ENTER_CAN}, MsgHandlerEnterCan(pObj, mContext));

		case MsgCmdType::SET_PUK:
			return HANDLE_CURRENT_STATE({MsgType::ENTER_PUK}, MsgHandlerEnterPuk(pObj, mContext));

		case MsgCmdType::GET_ACCESS_RIGHTS:
			return HANDLE_CURRENT_STATE({MsgType::ACCESS_RIGHTS}, MsgHandlerAccessRights(mContext));

		case MsgCmdType::SET_ACCESS_RIGHTS:
			return HANDLE_CURRENT_STATE({MsgType::ACCESS_RIGHTS}, MsgHandlerAccessRights(pObj, mContext));
	}

	return MsgHandlerInternalError(QLatin1String("Cannot process request"));
}


MsgHandler MessageDispatcher::handleInternalOnly(MsgCmdType pCmdType, const std::function<MsgHandler()>& pFunc) const
{
#ifdef QT_NO_DEBUG
	Q_UNUSED(pFunc)
	return MsgHandlerUnknownCommand(pCmdType);

#else
	Q_UNUSED(pCmdType)
	return pFunc();

#endif
}


MsgHandler MessageDispatcher::handleCurrentState(MsgCmdType pCmdType, std::initializer_list<MsgType> pMsgType, const std::function<MsgHandler()>& pFunc) const
{
	if (std::any_of(pMsgType.begin(), pMsgType.end(), [this](const MsgType pCurrentType)
			{
				return mContext.getLastStateMsg() == pCurrentType;
			}))
	{
		return pFunc();
	}

	return MsgHandlerBadState(pCmdType);
}


MsgHandler MessageDispatcher::cancel()
{
	if (mContext.isActiveWorkflow())
	{
		mContext.getContext()->killWorkflow();
		return MsgHandler::Void;
	}

	return MsgHandlerBadState(MsgCmdType::CANCEL);
}


MsgHandler MessageDispatcher::accept()
{
	if (mContext.getLastStateMsg() == MsgType::ACCESS_RIGHTS)
	{
		mContext.getContext()->setStateApproved();
		return MsgHandler::Void;
	}

	return MsgHandlerBadState(MsgCmdType::ACCEPT);
}


MsgHandler MessageDispatcher::interrupt()
{
	const auto cmdType = MsgCmdType::INTERRUPT;
#ifdef Q_OS_IOS
	{
		const auto allowedStates = {MsgType::ENTER_PIN, MsgType::ENTER_CAN, MsgType::ENTER_PUK, MsgType::ENTER_NEW_PIN};
		return handleCurrentState(cmdType, allowedStates, [] {
				Env::getSingleton<ReaderManager>()->stopScanAll();
				return MsgHandler::Void;
			});
	}
#else
	return MsgHandlerUnknownCommand(cmdType);

#endif
}
