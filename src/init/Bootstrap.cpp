/*
 * \copyright Copyright (c) 2014-2022 Governikus GmbH & Co. KG, Germany
 */

#include "Bootstrap.h"

#include "BuildHelper.h"
#include "controller/AppController.h"
#include "CommandLineParser.h"
#include "Env.h"
#include "LogHandler.h"
#include "SignalHandler.h"

#include <openssl/crypto.h>

#include <QLoggingCategory>
#include <QProcess>
#include <QScopedPointer>
#include <QSslSocket>
#include <QThread>

#include "config.h"

using namespace governikus;

Q_DECLARE_LOGGING_CATEGORY(init)


#if defined(INTEGRATED_SDK)
	#ifdef Q_OS_ANDROID
		#include <QAndroidService>
using QAPP = QAndroidService;

	#else
		#include <QCoreApplication>
using QAPP = QCoreApplication;
	#endif

#elif defined(Q_OS_ANDROID) || defined(Q_OS_IOS) || defined(Q_OS_WINRT)
	#ifdef Q_OS_ANDROID
		#include <QAndroidService>
		#include <QtAndroid>
	#endif

	#include <QGuiApplication>
using QAPP = QGuiApplication;
#else
	#include <QApplication>
using QAPP = QApplication;
#endif


static inline void printInfo()
{
	qCDebug(init) << "Logging to" << *Env::getSingleton<LogHandler>();

	qCInfo(init) << "##################################################";
	const auto& info = BuildHelper::getInformationHeader();
	for (const auto& entry : info)
	{
		qCInfo(init).noquote().nospace() << "### " << entry.first << ": " << entry.second;
	}
	qCInfo(init) << "##################################################";

	#if OPENSSL_VERSION_NUMBER < 0x10100000L
		#define OpenSSL_version SSLeay_version
		#define OPENSSL_VERSION SSLEAY_VERSION
	#endif

	if (QSslSocket::sslLibraryVersionString() != QLatin1String(OpenSSL_version(OPENSSL_VERSION)))
	{
		qCWarning(init) << "Linked OpenSSL Version differs:" << OpenSSL_version(OPENSSL_VERSION);
	}

	const auto libPathes = QCoreApplication::libraryPaths();
	for (const auto& path : libPathes)
	{
		qCDebug(init) << "Library path:" << path;
	}
}


static inline QCoreApplication* initQt(int& argc, char** argv)
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
	QCoreApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

	QCoreApplication::setOrganizationName(QStringLiteral(VENDOR));
	QCoreApplication::setOrganizationDomain(QStringLiteral(VENDOR_DOMAIN));
	QCoreApplication::setApplicationName(QStringLiteral(PRODUCT));
	QCoreApplication::setApplicationVersion(QStringLiteral(VERSION));

#ifndef INTEGRATED_SDK
	QGuiApplication::setQuitOnLastWindowClosed(false);
#endif

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID) && !defined(INTEGRATED_SDK)
	QGuiApplication::setDesktopSettingsAware(false);
#endif

#if defined(Q_OS_ANDROID) && !defined(INTEGRATED_SDK)
	if (QtAndroid::androidService().isValid())
	{
		return new QAndroidService(argc, argv);
	}
#endif

	return new QAPP(argc, argv);
}


static inline int exec(const QScopedPointer<QCoreApplication>& pApp)
{
#if defined(Q_OS_ANDROID) && !defined(INTEGRATED_SDK)
	if (QLatin1String(pApp->metaObject()->className()) == QLatin1String("QAndroidService"))
	{
		return QAndroidService::exec();
	}
#else
	Q_UNUSED(pApp)
#endif
	return QAPP::exec();
}


#if defined(Q_OS_WIN) || defined(Q_OS_MACOS) || (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID))
static void restartApp(const QString& pApplicationFilePath, QStringList pArgumentList, int pArgc)
{
	if (pArgumentList.size() == pArgc)
	{
		pArgumentList.removeFirst();
	}

	pArgumentList << QStringLiteral("--show");

	qCInfo(init) << "Attempting to start new process:" << pApplicationFilePath << ", args:" << pArgumentList;
	if (qint64 pid = -1; QProcess::startDetached(pApplicationFilePath, pArgumentList, QString(), &pid))
	{
		qCInfo(init) << "New process successfully launched, PID:" << pid;
	}
	else
	{
		qCCritical(init) << "Could not launch new process.";
	}
}


#endif

int governikus::initApp(int& argc, char** argv)
{
	const QScopedPointer<QCoreApplication> app(initQt(argc, argv));
	QThread::currentThread()->setObjectName(QStringLiteral("MainThread"));

	CommandLineParser::getInstance().parse();
	Env::getSingleton<LogHandler>()->init();
	Env::getSingleton<SignalHandler>()->init();
	printInfo();

	AppController controller;
	if (!controller.start())
	{
		qCCritical(init) << "Cannot start application controller, exit application";
		return EXIT_FAILURE;
	}

	Env::getSingleton<SignalHandler>()->setController(controller);
	if (Env::getSingleton<SignalHandler>()->shouldQuit())
	{
		return EXIT_SUCCESS;
	}

	const int returnCode = exec(app);

#if defined(Q_OS_WIN) || defined(Q_OS_MACOS) || (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID))
	if (controller.shouldApplicationRestart())
	{
		restartApp(QCoreApplication::applicationFilePath(), QCoreApplication::arguments(), argc);
	}
#endif

	qCDebug(init) << "Leaving application... bye bye!";
	return returnCode;
}
