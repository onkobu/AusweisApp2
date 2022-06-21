/*!
 * \copyright Copyright (c) 2014-2022 Governikus GmbH & Co. KG, Germany
 */

#include "AbstractState.h"

#include "ReaderManager.h"
#include "VolatileSettings.h"

#include <QLoggingCategory>
#include <QStateMachine>

Q_DECLARE_LOGGING_CATEGORY(statemachine)
Q_DECLARE_LOGGING_CATEGORY(support)


using namespace governikus;


const char* const AbstractState::cFORCE_START_STOP_SCAN = "FORCE_START_STOP_SCAN";


AbstractState::AbstractState(const QSharedPointer<WorkflowContext>& pContext, bool pConnectOnCardRemoved)
	: mContext(pContext)
	, mConnectOnCardRemoved(pConnectOnCardRemoved)
	, mConnections()
{
	Q_ASSERT(mContext);
}


AbstractState::~AbstractState()
{
}


QString AbstractState::getStateName() const
{
	Q_ASSERT(!objectName().isEmpty());
	return objectName();
}


void AbstractState::setStateName(const QString& pName)
{
	setObjectName(pName);
}


QString AbstractState::getClassName(const char* const pName)
{
	QString className = QString::fromLatin1(pName);
	if (className.contains(QLatin1Char(':')))
	{
		className = className.mid(className.lastIndexOf(QLatin1Char(':')) + 1);
	}
	return className;
}


void AbstractState::onStateApprovedChanged(bool pApproved)
{
	if (pApproved)
	{
		qCDebug(statemachine) << "Running state" << getStateName();
		run();
	}
}


void AbstractState::onEntry(QEvent* pEvent)
{
	Q_UNUSED(pEvent)
	if (mConnectOnCardRemoved)
	{
		const auto readerManager = Env::getSingleton<ReaderManager>();
		mConnections += connect(readerManager, &ReaderManager::fireCardRemoved, this, &AbstractState::onCardRemoved);
		mConnections += connect(readerManager, &ReaderManager::fireReaderRemoved, this, &AbstractState::onCardRemoved);
	}
	mConnections += connect(mContext.data(), &WorkflowContext::fireCancelWorkflow, this, &AbstractState::onUserCancelled);
	mConnections += connect(mContext.data(), &WorkflowContext::fireStateApprovedChanged, this, &AbstractState::onStateApprovedChanged, Qt::QueuedConnection);

	qCDebug(statemachine) << "Next state is" << getStateName();
	mContext->setCurrentState(getStateName());

	if (mContext->isWorkflowCancelled() && !mContext->isWorkflowCancelledInState())
	{
		onUserCancelled();
	}
}


void AbstractState::onExit(QEvent* pEvent)
{
	QState::onExit(pEvent);
	clearConnections();
	mContext->setStateApproved(false);
	qCDebug(statemachine) << "Leaving state" << getStateName()
						  << "with status: [" << mContext->getLastPaceResult() << "+" << mContext->getStatus() << "]";
}


void AbstractState::clearConnections()
{
	for (const auto& connection : qAsConst(mConnections))
	{
		QObject::disconnect(connection);
	}
	mConnections.clear();
}


bool AbstractState::isCancellationByUser()
{
	return mContext->getStatus().isCancellationByUser();
}


void AbstractState::onUserCancelled()
{
	qCInfo(support) << "Cancellation by user";
	mContext->setWorkflowCancelledInState();
	updateStatus(GlobalStatus::Code::Workflow_Cancellation_By_User);
	Q_EMIT fireAbort();
}


void AbstractState::onCardRemoved(const ReaderInfo& pInfo)
{
	if (pInfo.getName() == mContext->getReaderName())
	{
		updateStatus(GlobalStatus::Code::Workflow_Card_Removed);
		Q_EMIT fireAbort();
	}
}


void AbstractState::updateStatus(const GlobalStatus& pStatus)
{
	if (pStatus.isError() && mContext->getStatus().isNoError())
	{
		mContext->setStatus(pStatus);
	}
}


void AbstractState::updateStartPaosResult(const ECardApiResult& pStartPaosResult)
{
	if (!pStartPaosResult.isOk() && mContext->getStartPaosResult().isOk())
	{
		mContext->setStartPaosResult(pStartPaosResult);
		updateStatus(pStartPaosResult.toStatus());
	}
}


bool AbstractState::isStartStopEnabled() const
{
	return mContext->getReaderPlugInTypes().contains(ReaderManagerPlugInType::NFC)
		   || machine()->property(cFORCE_START_STOP_SCAN).toBool();
}


void AbstractState::startScanIfNecessary()
{
#if defined(Q_OS_IOS)
	if (isStartStopEnabled())
	{
		Env::getSingleton<ReaderManager>()->startScan(ReaderManagerPlugInType::NFC);
	}
#endif
}


void AbstractState::stopScanIfNecessary(const QString& pError)
{
#if defined(Q_OS_IOS)
	if (isStartStopEnabled() && Env::getSingleton<VolatileSettings>()->handleInterrupt())
	{
		Env::getSingleton<ReaderManager>()->stopScan(ReaderManagerPlugInType::NFC, pError);
	}
#else
	Q_UNUSED(pError)
#endif
}
