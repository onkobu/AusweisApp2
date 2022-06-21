/*
 * \copyright Copyright (c) 2016-2022 Governikus GmbH & Co. KG, Germany
 */

#include "MsgContext.h"

#include "MsgHandler.h"

#include <QLoggingCategory>

using namespace governikus;


MsgContext::MsgContext()
	: mApiLevel(MsgHandler::DEFAULT_MSG_LEVEL)
	, mStateMessages()
	, mContext()
{
}


bool MsgContext::isActiveWorkflow() const
{
	return !mContext.isNull();
}


void MsgContext::setApiLevel(MsgLevel pApiLevel)
{
	mApiLevel = pApiLevel;
}


MsgLevel MsgContext::getApiLevel() const
{
	return mApiLevel;
}


Msg MsgContext::getLastStateMsg() const
{
	if (mStateMessages.isEmpty())
	{
		return Msg();
	}

	return mStateMessages.last();
}


void MsgDispatcherContext::setWorkflowContext(const QSharedPointer<WorkflowContext>& pContext)
{
	mContext = pContext;
}


void MsgDispatcherContext::addStateMsg(const Msg& pMsg)
{
	mStateMessages += pMsg;
}


void MsgDispatcherContext::clear()
{
	mStateMessages.clear();
	mContext.clear();
}
