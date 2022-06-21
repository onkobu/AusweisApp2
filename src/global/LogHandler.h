/*
 * \brief Logging handler of QtMessageHandler
 *
 * \copyright Copyright (c) 2014-2022 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "Env.h"

#include <QContiguousCache>
#include <QDateTime>
#include <QDebug>
#include <QFileInfoList>
#include <QLoggingCategory>
#include <QMessageLogContext>
#include <QMutex>
#include <QPointer>
#include <QStringList>
#include <QTemporaryFile>

#define spawnMessageLogger(category)\
	QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC, category().categoryName())

class test_LogHandler;
class test_LogModel;

namespace governikus
{

class LogEventHandler
	: public QObject
{
	Q_OBJECT

	friend class LogHandler;

	private:
		LogEventHandler() = default;
		~LogEventHandler() override = default;

	Q_SIGNALS:
		/**
		 * \brief Every log will be fired by this signal. Be aware that you NEVER use a qDebug()
		 * or something like that function in your slot or you will get a deadlock!
		 */
		void fireLog(const QString& pMsg);
		void fireRawLog(const QString& pMsg, const QString& pCategoryName);
};

class LogHandler
{
	Q_GADGET

	friend class Env;
	friend class ::test_LogHandler;
	friend class ::test_LogModel;

	struct LogWindowEntry
	{
		qint64 mPosition;
		qint64 mLength;
	};

	private:
		static QString getLogFileTemplate();

		QPointer<LogEventHandler> mEventHandler;
		const bool mEnvPattern;
		const int mFunctionFilenameSize;
		qint64 mBacklogPosition;
		bool mCriticalLog;
		QContiguousCache<LogWindowEntry> mCriticalLogWindow;
		QStringList mCriticalLogIgnore;
		const QString mMessagePattern, mDefaultMessagePattern;
		QPointer<QTemporaryFile> mLogFile;
		QtMessageHandler mHandler;
		bool mUseHandler;
		bool mAutoRemove;
		bool mUseLogFile;
		const QByteArray mFilePrefix;
		QMutex mMutex;

		inline void copyMessageLogContext(const QMessageLogContext& pSource,
				QMessageLogContext& pDestination,
				const QByteArray& pFilename = QByteArray(),
				const QByteArray& pFunction = QByteArray(),
				const QByteArray& pCategory = QByteArray()) const;
		inline void logToFile(const QString& pOutput);
		inline QByteArray formatFunction(const char* const pFunction, const QByteArray& pFilename, int pLine) const;
		inline QByteArray formatFilename(const char* const pFilename) const;
		[[nodiscard]] inline QByteArray formatCategory(const QByteArray& pCategory) const;

		[[nodiscard]] QString getPaddedLogMsg(const QMessageLogContext& pContext, const QString& pMsg) const;
		void handleMessage(QtMsgType pType, const QMessageLogContext& pContext, const QString& pMsg);
		void handleLogWindow(QtMsgType pType, const char* pCategory, const QString& pMsg);
		void removeOldLogFiles();
		QByteArray readLogFile(qint64 pStart, qint64 pLength = -1);
		void setLogFileInternal(bool pEnable);

		static void messageHandler(QtMsgType pType, const QMessageLogContext& pContext, const QString& pMsg);
		friend QDebug operator<<(QDebug, const LogHandler&);

	protected:
		LogHandler();
		virtual ~LogHandler();
		static LogHandler& getInstance();

#ifndef QT_NO_DEBUG

	public:
#endif
		void reset();
		[[nodiscard]] bool isInstalled() const;

	public:
		void init();
		[[nodiscard]] const LogEventHandler* getEventHandler() const;

		void setAutoRemove(bool pRemove);
		bool copy(const QString& pDest);
		[[nodiscard]] bool copyOther(const QString& pSource, const QString& pDest) const;
		void resetBacklog();
		QByteArray getBacklog(bool pAll = false);
		QByteArray getCriticalLogWindow();
		[[nodiscard]] bool hasCriticalLog() const;
		[[nodiscard]] int getCriticalLogCapacity() const;
		void setCriticalLogCapacity(int pSize);

		static QDateTime getFileDate(const QFileInfo& pInfo);
		[[nodiscard]] QDateTime getCurrentLogFileDate() const;
		[[nodiscard]] QFileInfoList getOtherLogFiles() const;
		bool removeOtherLogFiles();
		void setLogFile(bool pEnable);
		[[nodiscard]] bool useLogFile() const;
		void setUseHandler(bool pEnable);
		[[nodiscard]] bool useHandler() const;
};

inline QDebug operator<<(QDebug pDbg, const governikus::LogHandler& pHandler)
{
	Q_ASSERT(pHandler.mLogFile);

	QDebugStateSaver saver(pDbg);
	pDbg.nospace() << pHandler.mLogFile->fileName();
	return pDbg.space();
}


} // namespace governikus
