/*!
 * \copyright Copyright (c) 2020-2022 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include <QAuthenticator>
#include <QEventLoop>
#include <QNetworkProxy>


namespace governikus
{

class ProxyCredentials
	: public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString proposedUser READ getProposedUser CONSTANT)
	Q_PROPERTY(QString url READ getUrl CONSTANT)
	Q_PROPERTY(QString user READ getUser WRITE setUser NOTIFY fireProxyCredentialsChanged)
	Q_PROPERTY(QString password READ getPassword WRITE setPassword NOTIFY fireProxyCredentialsChanged)

	private:
		QEventLoop mLoop;
		QAuthenticator* mAuthenticator;
		const QString mProposedUser;
		const QString mUrl;

		QString createUrl(const QNetworkProxy& pProxy) const;

	public:
		ProxyCredentials(const QNetworkProxy& pProxy, QAuthenticator* pAuthenticator);

		QString getProposedUser() const;
		QString getUrl() const;

		QString getUser() const;
		void setUser(const QString& pUser);

		QString getPassword() const;
		void setPassword(const QString& pPassword);

		Q_INVOKABLE void confirmInput();
		void waitForConfirmation();

	Q_SIGNALS:
		void fireProxyCredentialsChanged();
};


} // namespace governikus
