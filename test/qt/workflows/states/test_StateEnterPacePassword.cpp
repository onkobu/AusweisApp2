/*!
 * \copyright Copyright (c) 2019-2022 Governikus GmbH & Co. KG, Germany
 */

#include "states/StateEnterPacePassword.h"

#include "TestAuthContext.h"

#include <QtTest>

using namespace governikus;


class test_StateEnterPacePassword
	: public QObject
{
	Q_OBJECT
	QSharedPointer<AuthContext> mAuthContext;
	QSharedPointer<StateEnterPacePassword> mState;

	private Q_SLOTS:
		void init()
		{
			mAuthContext.reset(new TestAuthContext(nullptr, ":/paos/DIDAuthenticateEAC1.xml"));
			mState.reset(new StateEnterPacePassword(mAuthContext));
		}


		void cleanup()
		{
			mState.clear();
			mAuthContext.clear();
		}


		void test_Run()
		{
			QSignalSpy spyContinue(mState.data(), &AbstractState::fireContinue);

			mState->run();

			QCOMPARE(spyContinue.count(), 1);
		}


		void test_RunContextError()
		{
			mAuthContext->setStatus(GlobalStatus::Code::Card_Cancellation_By_User);
			QSignalSpy spyAbort(mState.data(), &AbstractState::fireAbort);

			mState->run();

			QCOMPARE(spyAbort.count(), 1);
		}


};

QTEST_GUILESS_MAIN(test_StateEnterPacePassword)
#include "test_StateEnterPacePassword.moc"
