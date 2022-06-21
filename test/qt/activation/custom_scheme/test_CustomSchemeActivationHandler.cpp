/*!
 * \brief Unit tests for \ref CustomSchemeActivationHandler
 *
 * \copyright Copyright (c) 2021-2022 Governikus GmbH & Co. KG, Germany
 */

#include "CustomSchemeActivationHandler.h"

#include "ActivationContext.h"

#include <QDesktopServices>
#include <QtTest>


using namespace governikus;


class test_CustomSchemeActivationHandler
	: public QObject
{
	Q_OBJECT

	private:
		CustomSchemeActivationHandler mHandler;

	private Q_SLOTS:
		void initTestCase()
		{
			QVERIFY(mHandler.start());
		}


		void cleanupTestCase()
		{
			mHandler.stop();
		}


		void authentication_data()
		{
			QTest::addColumn<QUrl>("url");

			QTest::newRow(" 1") << QUrl("eid://127.0.0.1:24727/eID-Client?tctokenURL=bla");
			QTest::newRow(" 2") << QUrl("eid://localhost:24727/eID-Client?tctokenURL=bla");
			QTest::newRow(" 3") << QUrl("eid://www.ausweisapp.bund.de/eID-Client?tctokenURL=bla");
			QTest::newRow(" 4") << QUrl("eid://www.ausweisapp.bund.de:24727/eID-Client?tctokenURL=bla");
			QTest::newRow(" 5") << QUrl("http://127.0.0.1:24727/eID-Client?tctokenURL=bla");
			QTest::newRow(" 6") << QUrl("http://localhost:24727/eID-Client?tctokenURL=bla");
			QTest::newRow(" 7") << QUrl("http://www.ausweisapp.bund.de/eID-Client?tctokenURL=bla");
			QTest::newRow(" 8") << QUrl("http://www.ausweisapp.bund.de:24727/eID-Client?tctokenURL=bla");
			QTest::newRow(" 9") << QUrl("https://127.0.0.1:24727/eID-Client?tctokenURL=bla");
			QTest::newRow("10") << QUrl("https://localhost:24727/eID-Client?tctokenURL=bla");
			QTest::newRow("11") << QUrl("https://www.ausweisapp.bund.de/eID-Client?tctokenURL=bla");
			QTest::newRow("12") << QUrl("https://www.ausweisapp.bund.de:24727/eID-Client?tctokenURL=bla");
		}


		void authentication()
		{
			QFETCH(QUrl, url);

			QSignalSpy spy(&mHandler, &CustomSchemeActivationHandler::fireAuthenticationRequest);

			QDesktopServices::openUrl(url);
			QCOMPARE(spy.count(), 1);

			const auto context = spy.at(0).at(0).value<QSharedPointer<ActivationContext> >();
			QCOMPARE(context->getActivationURL(), url);
		}


		void showui_data()
		{
			QTest::addColumn<UiModule>("module");
			QTest::addColumn<QString>("urlTemplate");

			const auto& uiModules = Enum<UiModule>::getList();
			for (const auto& module : uiModules)
			{
				QTest::newRow(QString("%1 - eid - ip").arg(getEnumName(module)).toUtf8().constData()) << module << QString("eid://127.0.0.1:24727/eID-Client?ShowUI=%1");
				QTest::newRow(QString("%1 - eid - localhost").arg(getEnumName(module)).toUtf8().constData()) << module << QString("eid://localhost:24727/eID-Client?ShowUI=%1");
				QTest::newRow(QString("%1 - eid - url").arg(getEnumName(module)).toUtf8().constData()) << module << QString("eid://www.ausweisapp.bund.de/eID-Client?ShowUI=%1");
				QTest::newRow(QString("%1 - eid - url - port").arg(getEnumName(module)).toUtf8().constData()) << module << QString("eid://www.ausweisapp.bund.de:24727/eID-Client?ShowUI=%1");
				QTest::newRow(QString("%1 - http - ip").arg(getEnumName(module)).toUtf8().constData()) << module << QString("http://127.0.0.1:24727/eID-Client?ShowUI=%1");
				QTest::newRow(QString("%1 - http - localhost").arg(getEnumName(module)).toUtf8().constData()) << module << QString("http://localhost:24727/eID-Client?ShowUI=%1");
				QTest::newRow(QString("%1 - http - url").arg(getEnumName(module)).toUtf8().constData()) << module << QString("http://www.ausweisapp.bund.de/eID-Client?ShowUI=%1");
				QTest::newRow(QString("%1 - http - url - port").arg(getEnumName(module)).toUtf8().constData()) << module << QString("http://www.ausweisapp.bund.de:24727/eID-Client?ShowUI=%1");
				QTest::newRow(QString("%1 - https - ip").arg(getEnumName(module)).toUtf8().constData()) << module << QString("https://127.0.0.1:24727/eID-Client?ShowUI=%1");
				QTest::newRow(QString("%1 - https - localhost").arg(getEnumName(module)).toUtf8().constData()) << module << QString("https://localhost:24727/eID-Client?ShowUI=%1");
				QTest::newRow(QString("%1 - https - url").arg(getEnumName(module)).toUtf8().constData()) << module << QString("https://www.ausweisapp.bund.de/eID-Client?ShowUI=%1");
				QTest::newRow(QString("%1 - https - url - port").arg(getEnumName(module)).toUtf8().constData()) << module << QString("https://www.ausweisapp.bund.de:24727/eID-Client?ShowUI=%1");
			}
		}


		void showui()
		{
			QFETCH(UiModule, module);
			QFETCH(QString, urlTemplate);

			QSignalSpy spy(&mHandler, &CustomSchemeActivationHandler::fireShowUiRequest);

			const QLatin1String moduleName = Enum<UiModule>::getName(module);

			QDesktopServices::openUrl(QUrl(urlTemplate.arg(moduleName)));
			QCOMPARE(spy.count(), 1);
			QCOMPARE(spy.at(0).at(0).value<UiModule>(), module);

			QDesktopServices::openUrl(QUrl(urlTemplate.arg(QString(moduleName).toLower())));
			QCOMPARE(spy.count(), 2);
			QCOMPARE(spy.at(1).at(0).value<UiModule>(), module);
		}


		void external_data()
		{
			QTest::addColumn<QUrl>("url");

			QTest::newRow("1") << QUrl("https://127.0.0.1/eID-Client?tctokenURL=bla");
			QTest::newRow("2") << QUrl("https://localhost/eID-Client?tctokenURL=bla");
			QTest::newRow("3") << QUrl("https://localhost:24727/eID?tctokenURL=bla");
			QTest::newRow("4") << QUrl("https://www.ausweisapp.bund.de/de/aa2/a11y");
			QTest::newRow("5") << QUrl("https://www.ausweisapp.bund.de/en/aa2/a11y");
		}


		void external()
		{
			QFETCH(QUrl, url);

			QSignalSpy spy(&mHandler, &CustomSchemeActivationHandler::fireAuthenticationRequest);

			QTest::ignoreMessage(QtDebugMsg, "Not our business. Using the appropriate mechanism for the user's desktop environment.");
			QDesktopServices::openUrl(url);
			QCOMPARE(spy.count(), 0);
		}


		void unknown_data()
		{
			QTest::addColumn<QUrl>("url");

			QTest::newRow(" 1") << QUrl("eid://127.0.0.1/eID-Client?tctokenURL=bla");
			QTest::newRow(" 2") << QUrl("eid://localhost/eID-Client?tctokenURL=bla");
			QTest::newRow(" 3") << QUrl("eid://localhost:24727/eID?tctokenURL=bla");
			QTest::newRow(" 4") << QUrl("eid://www.ausweisapp.bund.de/de/aa2/a11y");
			QTest::newRow(" 5") << QUrl("eid://www.ausweisapp.bund.de/en/aa2/a11y");
			QTest::newRow(" 6") << QUrl("http://127.0.0.1/eID-Client?tctokenURL=bla");
			QTest::newRow(" 7") << QUrl("http://localhost/eID-Client?tctokenURL=bla");
			QTest::newRow(" 8") << QUrl("http://localhost:24727/eID?tctokenURL=bla");
			QTest::newRow(" 9") << QUrl("http://www.ausweisapp.bund.de/de/aa2/a11y");
			QTest::newRow("10") << QUrl("http://www.ausweisapp.bund.de/en/aa2/a11y");
		}


		void unknown()
		{
			QFETCH(QUrl, url);

			QSignalSpy spy(&mHandler, &CustomSchemeActivationHandler::fireAuthenticationRequest);

			QTest::ignoreMessage(QtWarningMsg, "Request type: unknown");
			QDesktopServices::openUrl(url);
			QCOMPARE(spy.count(), 0);
		}


		void unknownQuery()
		{
			QSignalSpy spyUi(&mHandler, &CustomSchemeActivationHandler::fireShowUiRequest);
			QSignalSpy spyAu(&mHandler, &CustomSchemeActivationHandler::fireAuthenticationRequest);

			QTest::ignoreMessage(QtWarningMsg, "Unknown query type: QUrl(\"eid://127.0.0.1:24727/eID-Client?foo=bar\")");
			QDesktopServices::openUrl(QUrl("eid://127.0.0.1:24727/eID-Client?foo=bar"));
			QCOMPARE(spyUi.count(), 0);
			QCOMPARE(spyAu.count(), 0);
		}


};

QTEST_GUILESS_MAIN(test_CustomSchemeActivationHandler)
#include "test_CustomSchemeActivationHandler.moc"
