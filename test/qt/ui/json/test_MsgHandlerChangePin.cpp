/*!
 * \brief Unit tests for \ref MsgHandlerChangePin
 *
 * \copyright Copyright (c) 2020-2022 Governikus GmbH & Co. KG, Germany
 */

#include "messages/MsgHandlerChangePin.h"

#include "controller/AppController.h"
#include "Env.h"
#include "MessageDispatcher.h"
#include "UILoader.h"
#include "UIPlugInJson.h"
#include "VolatileSettings.h"

#include <QSignalSpy>
#include <QtTest>

Q_IMPORT_PLUGIN(UIPlugInJson)

using namespace governikus;

class test_MsgHandlerChangePin
	: public QObject
{
	Q_OBJECT

	private Q_SLOTS:
		void initTestCase()
		{
			qRegisterMetaType<QSharedPointer<WorkflowContext> >("QSharedPointer<WorkflowContext>");
		}


		void cleanup()
		{
			auto* uiLoader = Env::getSingleton<UILoader>();
			if (uiLoader->isLoaded())
			{
				QSignalSpy spyUi(uiLoader, &UILoader::fireShutdownComplete);
				uiLoader->shutdown();
				QTRY_COMPARE(spyUi.count(), 1); // clazy:exclude=qstring-allocations
			}
		}


		void runChangePinCmd()
		{
			Env::getSingleton<UILoader>()->load(UIPlugInName::UIPlugInJson);
			auto ui = qobject_cast<UIPlugInJson*>(Env::getSingleton<UILoader>()->getLoaded(UIPlugInName::UIPlugInJson));
			QVERIFY(ui);
			QSignalSpy spy(ui, &UIPlugIn::fireChangePinRequested);

			MessageDispatcher dispatcher;
			const QByteArray msg = R"({"cmd": "RUN_CHANGE_PIN"})";
			QCOMPARE(dispatcher.processCommand(msg), QByteArray());

			QCOMPARE(spy.count(), 1);
		}


		void initChangePin()
		{
			const auto context = QSharedPointer<WorkflowContext>::create();
			MessageDispatcher dispatcher;
			QCOMPARE(dispatcher.init(context), QByteArray());
			QCOMPARE(dispatcher.finish(), QByteArray());

			const auto ctx = QSharedPointer<ChangePinContext>::create();
			QCOMPARE(dispatcher.init(ctx), QByteArray(R"({"msg":"CHANGE_PIN"})"));
		}


		void allowCancel()
		{
			MessageDispatcher dispatcher;

			const auto ctx = QSharedPointer<ChangePinContext>::create();
			QCOMPARE(dispatcher.init(ctx), QByteArray(R"({"msg":"CHANGE_PIN"})"));

			QVERIFY(!ctx->isWorkflowCancelled());
			QByteArray msg = R"({"cmd": "CANCEL"})";
			QCOMPARE(dispatcher.processCommand(msg), QByteArray());
			QVERIFY(ctx->isWorkflowCancelled());

			msg = R"({"msg":"CHANGE_PIN","success":false})";
			QCOMPARE(dispatcher.finish(), msg);
		}


		void badState()
		{
			const QSharedPointer<WorkflowContext> context(new WorkflowContext());
			MessageDispatcher dispatcher;
			dispatcher.init(context);
			const QByteArray expected(R"({"error":"RUN_CHANGE_PIN","msg":"BAD_STATE"})");

			const QByteArray msg(R"({"cmd": "RUN_CHANGE_PIN"})");
			QCOMPARE(dispatcher.processCommand(msg), expected);
		}


		void cancelChangePin()
		{
			UILoader::setDefault({QStringLiteral("json")});
			AppController controller;
			QVERIFY(controller.start());

			auto ui = qobject_cast<UIPlugInJson*>(Env::getSingleton<UILoader>()->getLoaded(UIPlugInName::UIPlugInJson));
			QVERIFY(ui);
			ui->setEnabled(true);
			QSignalSpy spyStarted(&controller, &AppController::fireWorkflowStarted);
			QSignalSpy spyFinished(&controller, &AppController::fireWorkflowFinished);

			const QByteArray msg(R"({"cmd": "RUN_CHANGE_PIN"})");
			ui->doMessageProcessing(msg);
			QTRY_COMPARE(spyStarted.count(), 1); // clazy:exclude=qstring-allocations

			// If CANCEL is not quick enough this unit test blocks until QTRY_COMPARE timeout.
			// Even without the fix it is flaky to reproduce the problem.
			const QByteArray msgCancel(R"({"cmd": "CANCEL"})");
			ui->doMessageProcessing(msgCancel);
			QTRY_COMPARE(spyFinished.count(), 1); // clazy:exclude=qstring-allocations
		}


		void iosScanDialogMessages()
		{
			UILoader::setDefault({QStringLiteral("json")});
			AppController controller;
			QVERIFY(controller.start());

			const auto& initialMessages = Env::getSingleton<VolatileSettings>()->getMessages();
			QVERIFY(initialMessages.getSessionStarted().isNull());
			QVERIFY(initialMessages.getSessionInProgress().isNull());
			QVERIFY(initialMessages.getSessionSucceeded().isNull());
			QVERIFY(initialMessages.getSessionFailed().isEmpty());
			QVERIFY(!initialMessages.getSessionFailed().isNull());

			auto ui = qobject_cast<UIPlugInJson*>(Env::getSingleton<UILoader>()->getLoaded(UIPlugInName::UIPlugInJson));
			QVERIFY(ui);
			ui->setEnabled(true);
			QSignalSpy spyUi(ui, &UIPlugIn::fireChangePinRequested);
			QSignalSpy spyStarted(&controller, &AppController::fireWorkflowStarted);
			QSignalSpy spyFinished(&controller, &AppController::fireWorkflowFinished);

			const QByteArray msg("{"
								 "\"cmd\": \"RUN_CHANGE_PIN\","
								 "\"messages\": {"
								 "  \"sessionStarted\": \"start\","
								 "  \"sessionFailed\": \"stop failed\","
								 "  \"sessionSucceeded\": \"stop success\","
								 "  \"sessionInProgress\": \"progress\""
								 "  }"
								 "}");

			ui->doMessageProcessing(msg);
			QCOMPARE(spyUi.count(), 1);
			QTRY_COMPARE(spyStarted.count(), 1); // clazy:exclude=qstring-allocations

			QCOMPARE(Env::getSingleton<VolatileSettings>()->handleInterrupt(), true); // default
			auto messages = Env::getSingleton<VolatileSettings>()->getMessages();
			QCOMPARE(messages.getSessionStarted(), QLatin1String("start"));
			QCOMPARE(messages.getSessionInProgress(), QLatin1String("progress"));
			QCOMPARE(messages.getSessionSucceeded(), QLatin1String("stop success"));
			QCOMPARE(messages.getSessionFailed(), QLatin1String("stop failed"));

			const QByteArray msgCancel(R"({"cmd": "CANCEL"})");
			ui->doMessageProcessing(msgCancel);
			QTRY_COMPARE(spyFinished.count(), 1); // clazy:exclude=qstring-allocations

			messages = Env::getSingleton<VolatileSettings>()->getMessages();
			QVERIFY(messages.getSessionStarted().isNull());
			QVERIFY(messages.getSessionInProgress().isNull());
			QVERIFY(messages.getSessionSucceeded().isNull());
			QVERIFY(messages.getSessionFailed().isEmpty());
			QVERIFY(!messages.getSessionFailed().isNull());
		}


		void handleInterrupt_data()
		{
			QTest::addColumn<QVariant>("handleInterrupt");

			QTest::newRow("shouldStop") << QVariant(true);
			QTest::newRow("shouldNotStop") << QVariant(false);
		}


		void handleInterrupt()
		{
			QFETCH(QVariant, handleInterrupt);

			UILoader::setDefault({QStringLiteral("json")});
			AppController controller;
			QVERIFY(controller.start());

			QCOMPARE(Env::getSingleton<VolatileSettings>()->handleInterrupt(), true); // default

			auto ui = qobject_cast<UIPlugInJson*>(Env::getSingleton<UILoader>()->getLoaded(UIPlugInName::UIPlugInJson));
			QVERIFY(ui);
			ui->setEnabled(true);
			QSignalSpy spyUi(ui, &UIPlugIn::fireChangePinRequested);
			QSignalSpy spyStarted(&controller, &AppController::fireWorkflowStarted);
			QSignalSpy spyFinished(&controller, &AppController::fireWorkflowFinished);

			QByteArray msg("{"
						   "\"cmd\": \"RUN_CHANGE_PIN\","
						   "\"handleInterrupt\": %REPLACE%"
						   "}");
			msg.replace("%REPLACE%", handleInterrupt.toByteArray());

			ui->doMessageProcessing(msg);
			QCOMPARE(spyUi.count(), 1);
			QTRY_COMPARE(spyStarted.count(), 1); // clazy:exclude=qstring-allocations

			QCOMPARE(Env::getSingleton<VolatileSettings>()->handleInterrupt(), handleInterrupt.toBool());

			const QByteArray msgCancel(R"({"cmd": "CANCEL"})");
			ui->doMessageProcessing(msgCancel);
			QTRY_COMPARE(spyFinished.count(), 1); // clazy:exclude=qstring-allocations

			QCOMPARE(Env::getSingleton<VolatileSettings>()->handleInterrupt(), true); // default
		}


};

QTEST_GUILESS_MAIN(test_MsgHandlerChangePin)
#include "test_MsgHandlerChangePin.moc"
