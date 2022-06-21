/*!
 * \brief UIPlugIn implementation of the Websocket.
 *
 * \copyright Copyright (c) 2016-2022 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "HttpRequest.h"
#include "HttpServer.h"
#include "UIPlugIn.h"
#include "UIPlugInJson.h"

#include <QDir>
#include <QMap>
#include <QScopedPointer>
#include <QWebSocket>
#include <QWebSocketServer>

namespace governikus
{

class UIPlugInWebSocket
	: public UIPlugIn
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "governikus.UIPlugIn" FILE "metadata.json")
	Q_INTERFACES(governikus::UIPlugIn)

	private:
		QSharedPointer<HttpServer> mHttpServer;
		QWebSocketServer mServer;
		QScopedPointer<QWebSocket, QScopedPointerDeleteLater> mConnection;
		QSharedPointer<HttpRequest> mRequest;
		UIPlugInJson* mJson;
		QSharedPointer<WorkflowContext> mContext;
		bool mUiDomination;
		bool mUiDominationPrevUsedAsSDK;

	private Q_SLOTS:
		void doShutdown() override;
		void onWorkflowStarted(QSharedPointer<WorkflowContext> pContext) override;
		void onWorkflowFinished(QSharedPointer<WorkflowContext> pContext) override;
		void onUiDomination(const UIPlugIn* pUi, const QString& pInformation, bool pAccepted) override;
		void onUiDominationReleased() override;
		void onNewWebSocketRequest(const QSharedPointer<HttpRequest>& pRequest);
		void onNewConnection();
		void onClientDisconnected();
		void onTextMessageReceived(const QString& pMessage);
		void onJsonMessage(const QByteArray& pMessage);

	public:
		UIPlugInWebSocket();
		~UIPlugInWebSocket() override;
};

} // namespace governikus
