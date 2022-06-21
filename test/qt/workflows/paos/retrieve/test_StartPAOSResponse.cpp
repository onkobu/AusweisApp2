/*!
 * \brief Unit tests for \InitializeFramework
 *
 * \copyright Copyright (c) 2014-2022 Governikus GmbH & Co. KG, Germany
 */

#include "paos/retrieve/StartPaosResponse.h"
#include "TestFileHelper.h"

#include <QtCore>
#include <QtTest>


using namespace governikus;

class test_StartPAOSResponse
	: public QObject
{
	Q_OBJECT

	private Q_SLOTS:
		void initTestCase()
		{
		}


		void parse_ResultMajor()
		{
			QByteArray content = TestFileHelper::readFile(":paos/StartPAOSResponse1.xml");

			StartPaosResponse message(content);

			QCOMPARE(message.getResult().getMajor(), ECardApiResult::Major::Ok);
			QCOMPARE(message.getResult().getMinor(), ECardApiResult::Minor::null);
			QVERIFY(message.getResult().getMessage().isNull());
		}


		void parse_ResultMajorResultMinor()
		{
			QByteArray content = TestFileHelper::readFile(":paos/StartPAOSResponse2.xml");

			StartPaosResponse message(content);

			QCOMPARE(message.getResult().getMajor(), ECardApiResult::Major::Error);
			QCOMPARE(message.getResult().getMinor(), ECardApiResult::Minor::DP_Timeout_Error);
			QVERIFY(message.getResult().getMessage().isNull());
		}


		void parse_ResultMajorResultMinorResultMessage()
		{
			QByteArray content = TestFileHelper::readFile(":paos/StartPAOSResponse3.xml");

			StartPaosResponse message(content);

			QCOMPARE(message.getResult().getMajor(), ECardApiResult::Major::Error);
			QCOMPARE(message.getResult().getMinor(), ECardApiResult::Minor::DP_Timeout_Error);
			QCOMPARE(message.getResult().getMessage(), QString("Detail message"));
		}


};

QTEST_GUILESS_MAIN(test_StartPAOSResponse)
#include "test_StartPAOSResponse.moc"
