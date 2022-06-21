/*!
 * \copyright Copyright (c) 2019-2022 Governikus GmbH & Co. KG, Germany
 */

#include "SurveyModel.h"

#include "DeviceInfo.h"
#include "Env.h"
#include "LogHandler.h"
#include "NetworkManager.h"
#include "SecureStorage.h"
#include "TlsChecker.h"

#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QNetworkReply>
#include <QNetworkRequest>

Q_DECLARE_LOGGING_CATEGORY(network)

using namespace governikus;

#define VALUE_NAME(_name, _key)\
	inline QLatin1String _name(){\
		return QLatin1String(_key);\
	}


namespace
{
VALUE_NAME(ROM, "Rom")
VALUE_NAME(BUILD_NUMBER, "BuildNumber")
VALUE_NAME(ANDROID_VERSION, "AndroidVersion")
VALUE_NAME(KERNEL_VERSION, "KernelVersion")
VALUE_NAME(MAXIMUM_NFC_PACKET_LENGTH, "MaximumNfcPacketLength")
VALUE_NAME(VENDOR, "Vendor")
VALUE_NAME(MODEL_NUMBER, "ModelNumber")
VALUE_NAME(MODEL_NAME, "ModelName")
VALUE_NAME(AUSWEISAPP_VERSION_NUMBER, "AusweisAppVersionNumber")
} // namespace


SurveyModel::SurveyModel()
	: QAbstractListModel()
	, mBuildNumber(DeviceInfo::getOSBuildNumber())
	, mAndroidVersion(DeviceInfo::getOSVersion())
	, mKernelVersion(DeviceInfo::getKernelVersion())
	, mMaximumNfcPacketLength(0)
	, mVendor(DeviceInfo::getVendor())
	, mModelNumber(DeviceInfo::getModelNumber())
	, mModelName(DeviceInfo::getModelName())
	, mAusweisAppVersionNumber(QCoreApplication::applicationVersion())
{
	buildDataObject();
}


int SurveyModel::rowCount(const QModelIndex&) const
{
	return mData.size();
}


QVariant SurveyModel::data(const QModelIndex& pIndex, int pRole) const
{
	if (pIndex.isValid() && pIndex.row() < rowCount())
	{
		auto entry = mData[pIndex.row()];
		if (pRole == TITLE)
		{
			return entry.first;
		}
		if (pRole == VALUE)
		{
			return entry.second;
		}
	}
	return QVariant();
}


QHash<int, QByteArray> SurveyModel::roleNames() const
{
	QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
	roles.insert(TITLE, "title");
	roles.insert(VALUE, "value");
	return roles;
}


void SurveyModel::buildDataObject()
{
	beginResetModel();
	mData.clear();
	mData += qMakePair(tr("Vendor"), mVendor);
	mData += qMakePair(tr("Model Name"), mModelName);
	mData += qMakePair(tr("Model Number"), mModelNumber);
	mData += qMakePair(tr("Build Number"), mBuildNumber);
	mData += qMakePair(tr("Android version"), mAndroidVersion);
	mData += qMakePair(tr("Kernel version"), mKernelVersion);
	mData += qMakePair(tr("Max. NFC Packet Length"), QString::number(mMaximumNfcPacketLength));
	mData += qMakePair(tr("AusweisApp2 Version"), mAusweisAppVersionNumber);
	endResetModel();
}


QByteArray SurveyModel::toJsonByteArray() const
{
	QJsonObject rom;
	rom[BUILD_NUMBER()] = mBuildNumber;
	rom[ANDROID_VERSION()] = mAndroidVersion;
	rom[KERNEL_VERSION()] = mKernelVersion;
	rom[MAXIMUM_NFC_PACKET_LENGTH()] = QJsonValue(mMaximumNfcPacketLength);

	QJsonObject mainObject;
	mainObject[ROM()] = rom;
	mainObject[VENDOR()] = mVendor;
	mainObject[MODEL_NUMBER()] = mModelNumber;
	mainObject[MODEL_NAME()] = mModelName;
	mainObject[AUSWEISAPP_VERSION_NUMBER()] = mAusweisAppVersionNumber;

#ifndef QT_NO_DEBUG
	if (QCoreApplication::applicationName().startsWith(QLatin1String("Test")))
	{
		return QJsonDocument(mainObject).toJson(QJsonDocument::Indented);
	}
#endif

	return QJsonDocument(mainObject).toJson(QJsonDocument::Compact);
}


void SurveyModel::setMaximumNfcPacketLength(int pMaximumNfcPacketLength)
{
	mMaximumNfcPacketLength = pMaximumNfcPacketLength;
	buildDataObject();
}


void SurveyModel::transmitSurvey()
{
	const QUrl whitelistServerBaseUrl = Env::getSingleton<SecureStorage>()->getWhitelistServerBaseUrl();
	const QUrl postSurveyUrl(whitelistServerBaseUrl.toString() + QStringLiteral("/new"));
	QNetworkRequest request(postSurveyUrl);
	request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json; charset=UTF-8"));

	const QByteArray jsonData = toJsonByteArray();
	const auto& caCerts = Env::getSingleton<SecureStorage>()->getUpdateCertificates().toList();
	mReply.reset(Env::getSingleton<NetworkManager>()->post(request, jsonData, caCerts), &QNetworkReply::deleteLater);

	connect(mReply.data(), &QNetworkReply::sslErrors, this, &SurveyModel::onSslErrors);
	connect(mReply.data(), &QNetworkReply::encrypted, this, &SurveyModel::onSslHandshakeDone);
	connect(mReply.data(), &QNetworkReply::finished, this, &SurveyModel::onNetworkReplyFinished);

	qDebug().noquote() << "Sent survey to whitelist server:" << jsonData;
}


void SurveyModel::onSslErrors(const QList<QSslError>& pErrors)
{
	TlsChecker::containsFatalError(mReply, pErrors);
}


void SurveyModel::onSslHandshakeDone()
{
	const auto& cfg = mReply->sslConfiguration();
	TlsChecker::logSslConfig(cfg, spawnMessageLogger(network));

	if (!Env::getSingleton<NetworkManager>()->checkUpdateServerCertificate(mReply))
	{
		qCCritical(network) << "Untrusted certificate of Whitelist server found:" << cfg.peerCertificate();
		mReply->abort();
	}
}


void SurveyModel::onNetworkReplyFinished()
{
	if (mReply)
	{
		mReply->disconnect(this);
		mReply.reset();
	}
}
