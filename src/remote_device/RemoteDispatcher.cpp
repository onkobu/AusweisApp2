/*!
 * \copyright Copyright (c) 2017-2022 Governikus GmbH & Co. KG, Germany
 */

#include "RemoteDispatcher.h"

#include "AppSettings.h"
#include "Initializer.h"
#include "messages/IfdError.h"
#include "messages/IfdVersion.h"

#include <QLoggingCategory>


Q_DECLARE_LOGGING_CATEGORY(remote_device)


using namespace governikus;


INIT_FUNCTION([] {
			qRegisterMetaType<RemoteCardMessageType>("RemoteCardMessageType");
		})


RemoteDispatcher::RemoteDispatcher(IfdVersion::Version pVersion, const QSharedPointer<DataChannel>& pDataChannel)
	: QObject()
	, mDataChannel(pDataChannel)
	, mVersion(pVersion)
	, mContextHandle()
{
	Q_ASSERT(mDataChannel);

	connect(mDataChannel.data(), &DataChannel::fireReceived, this, &RemoteDispatcher::onReceived);
	connect(mDataChannel.data(), &DataChannel::fireClosed, this, &RemoteDispatcher::onClosed);
}


RemoteDispatcher::~RemoteDispatcher()
{
	disconnect(mDataChannel.data(), &DataChannel::fireReceived, this, &RemoteDispatcher::onReceived);
	disconnect(mDataChannel.data(), &DataChannel::fireClosed, this, &RemoteDispatcher::onClosed);

	close();
}


void RemoteDispatcher::onReceived(const QByteArray& pDataBlock)
{
	const auto& msgObject = RemoteMessage::parseByteArray(pDataBlock);
	const RemoteMessage remoteMessage(msgObject);
	const RemoteCardMessageType messageType = remoteMessage.getType();

	if (messageType == RemoteCardMessageType::UNDEFINED)
	{
		const QSharedPointer<const IfdError>& errorMessage = QSharedPointer<IfdError>::create(QString(), ECardApiResult::Minor::AL_Unkown_API_Function);
		send(errorMessage);
		return;
	}

	if (processContext(messageType, msgObject))
	{
		return;
	}

	const auto& contextHandle = remoteMessage.getContextHandle();
	if (contextHandle.isEmpty() || mContextHandle != contextHandle)
	{
		qCWarning(remote_device) << "Invalid context handle received. Expecting:" << mContextHandle << "but got:" << pDataBlock;
		return;
	}

	qCDebug(remote_device) << "Received message type:" << messageType;
	Q_EMIT fireReceived(messageType, msgObject, getId());
}


void RemoteDispatcher::onClosed(GlobalStatus::Code pCloseCode)
{
	qCDebug(remote_device) << "Connection closed";
	Q_EMIT fireClosed(pCloseCode, getId());
}


QString RemoteDispatcher::getId() const
{
	if (!mDataChannel)
	{
		return QString();
	}

	return mDataChannel->getId();
}


const QString& RemoteDispatcher::getContextHandle() const
{
	return mContextHandle;
}


void RemoteDispatcher::saveRemoteNameInSettings(const QString& pName)
{
	RemoteServiceSettings& settings = Env::getSingleton<AppSettings>()->getRemoteServiceSettings();
	auto info = settings.getRemoteInfo(getId());
	info.setNameUnescaped(pName);
	settings.updateRemoteInfo(info);
}


void RemoteDispatcher::send(const QSharedPointer<const RemoteMessage>& pMessage)
{
	Q_ASSERT(mDataChannel);

	const RemoteCardMessageType messageType = pMessage->getType();
	qCDebug(remote_device) << "Send message of type:" << messageType << "with context handle:" << mContextHandle;

	Q_ASSERT(!mContextHandle.isEmpty()
			|| messageType == RemoteCardMessageType::IFDError
			|| messageType == RemoteCardMessageType::IFDEstablishContext
			|| messageType == RemoteCardMessageType::IFDEstablishContextResponse);

	mDataChannel->send(pMessage->toByteArray(mVersion, mContextHandle));
}


void RemoteDispatcher::close()
{
	if (mDataChannel)
	{
		mDataChannel->close();
	}
}
