/*!
 * \copyright Copyright (c) 2018-2022 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include <QCoreApplication>
#include <QFile>
#include <QString>

namespace governikus
{

class PortFile
{
	private:
		quint16 mDefaultPort;
		QFile mPortFile;

	public:
		static const quint16 cDefaultPort;

		static QString getPortFilename(const QString& pUsage = QString(),
				qint64 pPid = QCoreApplication::applicationPid(),
				const QString& pApp = QCoreApplication::applicationName());

		PortFile(const QString& pUsage = QString(), quint16 pDefaultPort = cDefaultPort);
		~PortFile();

		void handlePort(quint16 pCurrentPort);
};

} // namespace governikus
