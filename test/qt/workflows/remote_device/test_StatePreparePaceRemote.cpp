/*!
 * \copyright Copyright (c) 2019-2022 Governikus GmbH & Co. KG, Germany
 */

#include "states/StatePreparePaceRemote.h"

#include "AppSettings.h"
#include "states/StateBuilder.h"

#include <QtTest>


using namespace governikus;

class test_StatePreparePaceRemote
	: public QObject
{
	Q_OBJECT
	QSharedPointer<RemoteServiceContext> mContext;
	QSharedPointer<StatePreparePaceRemote> mState;

	static QSharedPointer<const IfdEstablishPaceChannel> createMessage(PacePasswordId pId)
	{
		EstablishPaceChannel establishPaceChannel(pId);
		return QSharedPointer<const IfdEstablishPaceChannel>(new IfdEstablishPaceChannel("slot", establishPaceChannel, 6));
	}

	private Q_SLOTS:
		void init()
		{
			mContext.reset(new RemoteServiceContext());
			mState.reset(StateBuilder::createState<StatePreparePaceRemote>(mContext));
			Env::getSingleton<AppSettings>()->getRemoteServiceSettings().setPinPadMode(true);

			mState->onEntry(nullptr);
		}


		void cleanup()
		{
			mContext.clear();
			mState.clear();
		}


		void test_RunPin()
		{
			const auto& message = createMessage(PacePasswordId::PACE_PIN);
			mContext->setEstablishPaceChannel(message);
			QSignalSpy spyEnterPacePassword(mState.data(), &StatePreparePaceRemote::fireEnterPacePassword);
			QSignalSpy spyContinue(mState.data(), &StatePreparePaceRemote::fireContinue);

			mContext->setStateApproved();
			QTRY_COMPARE(spyEnterPacePassword.count(), 1); // clazy:exclude=qstring-allocations
			QCOMPARE(spyContinue.count(), 0);

			mContext->setPin(QString("111111"));
			mContext->setStateApproved(false);
			mContext->setStateApproved();
			QTRY_COMPARE(spyContinue.count(), 1); // clazy:exclude=qstring-allocations
		}


		void test_RunCan()
		{
			const auto& message = createMessage(PacePasswordId::PACE_CAN);
			mContext->setEstablishPaceChannel(message);
			QSignalSpy spyEnterPacePassword(mState.data(), &StatePreparePaceRemote::fireEnterPacePassword);
			QSignalSpy spyContinue(mState.data(), &StatePreparePaceRemote::fireContinue);

			mContext->setStateApproved();
			QTRY_COMPARE(spyEnterPacePassword.count(), 1); // clazy:exclude=qstring-allocations
			QCOMPARE(spyContinue.count(), 0);

			mContext->setCan(QString("111111"));
			mContext->setStateApproved(false);
			mContext->setStateApproved();
			QTRY_COMPARE(spyContinue.count(), 1); // clazy:exclude=qstring-allocations
		}


		void test_RunPuk()
		{
			const auto& message = createMessage(PacePasswordId::PACE_PUK);
			mContext->setEstablishPaceChannel(message);
			QSignalSpy spyEnterPacePassword(mState.data(), &StatePreparePaceRemote::fireEnterPacePassword);
			QSignalSpy spyContinue(mState.data(), &StatePreparePaceRemote::fireContinue);

			mContext->setStateApproved();
			QTRY_COMPARE(spyEnterPacePassword.count(), 1); // clazy:exclude=qstring-allocations
			QCOMPARE(spyContinue.count(), 0);

			mContext->setPuk(QString("111111"));
			mContext->setStateApproved(false);
			mContext->setStateApproved();
			QTRY_COMPARE(spyContinue.count(), 1); // clazy:exclude=qstring-allocations
		}


		void test_RunUnknown()
		{
			const auto& message = createMessage(PacePasswordId::UNKNOWN);
			mContext->setEstablishPaceChannel(message);
			QSignalSpy spyContinue(mState.data(), &StatePreparePaceRemote::fireAbort);

			QTest::ignoreMessage(QtCriticalMsg, "Cannot handle unknown PacePasswordId");
			mContext->setStateApproved();
			QTRY_COMPARE(spyContinue.count(), 1); // clazy:exclude=qstring-allocations
		}


		void test_RunPinPadModeFalse()
		{
			Env::getSingleton<AppSettings>()->getRemoteServiceSettings().setPinPadMode(false);
			QSignalSpy spyContinue(mState.data(), &StatePreparePaceRemote::fireContinue);

			mContext->setStateApproved();
			QTRY_COMPARE(spyContinue.count(), 1); // clazy:exclude=qstring-allocations
		}


};

QTEST_GUILESS_MAIN(test_StatePreparePaceRemote)
#include "test_StatePreparePaceRemote.moc"
