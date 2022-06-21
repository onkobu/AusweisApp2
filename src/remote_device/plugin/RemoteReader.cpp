/*!
 * \copyright Copyright (c) 2017-2022 Governikus GmbH & Co. KG, Germany
 */

#include "RemoteReader.h"

#include "CardConnectionWorker.h"

#include <QLoggingCategory>

using namespace governikus;

Q_DECLARE_LOGGING_CATEGORY(card_remote)


RemoteReader::RemoteReader(const QString& pReaderName, const QSharedPointer<RemoteDispatcherClient>& pRemoteDispatcher, const IfdStatus& pIfdStatus)
	: Reader(ReaderManagerPlugInType::REMOTE, pReaderName)
	, mRemoteDispatcher(pRemoteDispatcher)
{
	mReaderInfo.setBasicReader(!pIfdStatus.hasPinPad());
	mReaderInfo.setConnected(true);

	updateStatus(pIfdStatus);
}


RemoteReader::~RemoteReader()
{
	mCard.reset();
}


Card* RemoteReader::getCard() const
{
	if (mCard)
	{
		return mCard.data();
	}

	return nullptr;
}


Reader::CardEvent RemoteReader::updateCard()
{
	return CardEvent::NONE;
}


void RemoteReader::updateStatus(const IfdStatus& pIfdStatus)
{
	if (mReaderInfo.getMaxApduLength() != pIfdStatus.getMaxApduLength())
	{
		mReaderInfo.setMaxApduLength(pIfdStatus.getMaxApduLength());
		Q_EMIT fireReaderPropertiesUpdated(mReaderInfo);

		if (!mReaderInfo.sufficientApduLength())
		{
			qCDebug(card_remote) << "ExtendedLengthApduSupport missing. maxAPDULength:" << mReaderInfo.getMaxApduLength();
		}
	}

	if (mCard)
	{
		if (!pIfdStatus.getCardAvailable())
		{
			qCDebug(card_remote) << "Card removed";
			mCard.reset();
			mReaderInfo.setCardInfo(CardInfo(CardType::NONE));
			Q_EMIT fireCardRemoved(mReaderInfo);
		}
		return;
	}

	if (pIfdStatus.getCardAvailable())
	{
		qCDebug(card_remote) << "Card inserted";
		mCard.reset(new RemoteCard(mRemoteDispatcher, getName()));
		QSharedPointer<CardConnectionWorker> cardConnection = createCardConnectionWorker();
		CardInfoFactory::create(cardConnection, mReaderInfo);
		Q_EMIT fireCardInserted(mReaderInfo);
	}
}
