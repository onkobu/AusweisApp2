/*!
 * \brief Provide a HTTP server.
 *
 * \copyright Copyright (c) 2016-2022 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "HttpRequest.h"
#include "PortFile.h"

#include <QMetaMethod>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QTcpServer>

namespace governikus
{

class HttpServer
	: public QObject
{
	Q_OBJECT

	private:
		QScopedPointer<QTcpServer, QScopedPointerDeleteLater> mServer;
		PortFile mPortFile;

		bool checkReceiver(const QMetaMethod& pSignal, HttpRequest* pRequest);

	public:
		static quint16 cPort;

		explicit HttpServer(quint16 pPort = HttpServer::cPort);
		~HttpServer() override;

		[[nodiscard]] bool isListening() const;
		[[nodiscard]] quint16 getServerPort() const;

	private Q_SLOTS:
		void onNewConnection();
		void onMessageComplete(HttpRequest* pRequest);

	Q_SIGNALS:
		void fireNewHttpRequest(const QSharedPointer<HttpRequest>& pRequest);
		void fireNewWebSocketRequest(const QSharedPointer<HttpRequest>& pRequest);
};

} // namespace governikus
