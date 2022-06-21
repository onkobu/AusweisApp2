/*!
 * \copyright Copyright (c) 2014-2022 Governikus GmbH & Co. KG, Germany
 */


#include "SecureStorage.h"

#include "FileDestination.h"
#include "SingletonHelper.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonValue>
#include <QLoggingCategory>
#include <QMetaEnum>
#include <QTextStream>


using namespace governikus;

Q_DECLARE_LOGGING_CATEGORY(securestorage)

namespace
{
#define CONFIG_NAME(_name, _key) /* NOLINT */\
	inline QLatin1String _name(){\
		return QLatin1String(_key);\
	}

CONFIG_NAME(CONFIGURATION_GROUP_NAME_CV_ROOT_CERTIFICATE, "cvRootCertificates")
CONFIG_NAME(CONFIGURATION_GROUP_NAME_CV_ROOT_CERTIFICATE_TEST, "cvRootCertificatesTest")

CONFIG_NAME(CONFIGURATION_GROUP_NAME_UPDATE_CERTIFICATES, "updateCertificates")

CONFIG_NAME(CONFIGURATION_GROUP_NAME_TLS_SETTINGS, "tlsSettings")
CONFIG_NAME(CONFIGURATION_GROUP_NAME_TLS_CONFIGURATION_PSK, "tlsSettingsPsk")
CONFIG_NAME(CONFIGURATION_GROUP_NAME_TLS_CONFIGURATION_REMOTE_READER, "tlsSettingsRemoteReader")
CONFIG_NAME(CONFIGURATION_GROUP_NAME_TLS_CONFIGURATION_REMOTE_READER_PAIRING, "tlsSettingsRemoteReaderPairing")
CONFIG_NAME(CONFIGURATION_GROUP_NAME_MIN_STATIC_KEY_SIZES, "minStaticKeySizes")
CONFIG_NAME(CONFIGURATION_GROUP_NAME_MIN_EPHEMERAL_KEY_SIZES, "minEphemeralKeySizes")

CONFIG_NAME(CONFIGURATION_GROUP_NAME_SELF_AUTHENTICATION, "selfAuthentication")
CONFIG_NAME(CONFIGURATION_NAME_SELF_AUTHENTICATION_URL, "url")
CONFIG_NAME(CONFIGURATION_NAME_SELF_AUTHENTICATION_TEST_URL, "testUrl")

CONFIG_NAME(CONFIGURATION_GROUP_NAME_UPDATE_SERVER, "updateServer")
CONFIG_NAME(CONFIGURATION_NAME_UPDATE_SERVER_BASE_URL, "baseUrl")

CONFIG_NAME(CONFIGURATION_GROUP_NAME_WHITELIST_SERVER, "whitelistServer")
CONFIG_NAME(CONFIGURATION_NAME_WHITELIST_SERVER_BASE_URL, "baseUrl")

CONFIG_NAME(CONFIGURATION_GROUP_NAME_UPDATES, "updates")
CONFIG_NAME(CONFIGURATION_NAME_APPCAST_UPDATE_URL, "release")
CONFIG_NAME(CONFIGURATION_NAME_APPCAST_BETA_UPDATE_URL, "beta")
} // namespace

defineSingleton(SecureStorage)


SecureStorage::SecureStorage()
	: mLoaded(false)
	, mCvcas()
	, mCvcasTest()
	, mUpdateCertificates()
	, mSelfAuthenticationUrl()
	, mSelfAuthenticationTestUrl()
	, mUpdateServerBaseUrl()
	, mWhitelistServerBaseUrl()
	, mAppcastUpdateUrl()
	, mAppcastBetaUpdateUrl()
	, mTlsConfig()
	, mTlsConfigPsk()
	, mTlsConfigRemote()
	, mTlsConfigRemotePsk()
	, mMinStaticKeySizes()
	, mMinEphemeralKeySizes()
{
	load();
}


bool SecureStorage::isLoaded() const
{
	return mLoaded;
}


QJsonObject SecureStorage::loadFile(const QStringList& pFiles) const
{
	for (const auto& path : pFiles)
	{
		qCDebug(securestorage) << "Load SecureStorage:" << path;

		QFile configFile(path);
		if (!configFile.exists())
		{
			qCCritical(securestorage) << "SecureStorage not found";
			continue;
		}

		if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			qCCritical(securestorage) << "Wasn't able to open SecureStorage";
			continue;
		}

		QJsonParseError parseError {};
		const auto& document = QJsonDocument::fromJson(configFile.readAll(), &parseError);
		if (parseError.error != QJsonParseError::NoError)
		{
			qCCritical(securestorage) << "Parse error while reading SecureStorage on position" << parseError.offset << ':' << parseError.errorString();
			continue;
		}

		return document.object();
	}

	return QJsonObject();
}


void SecureStorage::load()
{
	const QStringList files({FileDestination::getPath(QStringLiteral("config.json")), QStringLiteral(":/config.json")});
	const auto& config = loadFile(files);
	if (config.isEmpty())
	{
		return;
	}

	mCvcas.clear();
	mCvcas = readByteArrayList(config, CONFIGURATION_GROUP_NAME_CV_ROOT_CERTIFICATE());

	mCvcasTest.clear();
	mCvcasTest = readByteArrayList(config, CONFIGURATION_GROUP_NAME_CV_ROOT_CERTIFICATE_TEST());
	mCvcasTest = loadTestCvcsFromAppDir() + mCvcasTest;

	QByteArrayList certificates = readByteArrayList(config, CONFIGURATION_GROUP_NAME_UPDATE_CERTIFICATES());
	mUpdateCertificates.clear();
	for (int i = 0; i < certificates.size(); ++i)
	{
		QSslCertificate certificate(QByteArray::fromHex(certificates[i]), QSsl::Der);
		if (certificate.isNull())
		{
			qCCritical(securestorage) << "Failed to read update certificate[" << i << "] from SecureStorage";
			continue;
		}
		mUpdateCertificates += certificate;
	}

	QJsonValue tlsValue = config.value(CONFIGURATION_GROUP_NAME_TLS_SETTINGS());
	if (tlsValue.isObject())
	{
		mTlsConfig.load(tlsValue.toObject());
	}

	QJsonValue tlsPskValue = config.value(CONFIGURATION_GROUP_NAME_TLS_CONFIGURATION_PSK());
	if (tlsPskValue.isObject())
	{
		mTlsConfigPsk.load(tlsPskValue.toObject());
	}

	QJsonValue tlsRemoteReaderValue = config.value(CONFIGURATION_GROUP_NAME_TLS_CONFIGURATION_REMOTE_READER());
	if (tlsRemoteReaderValue.isObject())
	{
		mTlsConfigRemote.load(tlsRemoteReaderValue.toObject());
	}
	QJsonValue tlsRemoteReaderPairingValue = config.value(CONFIGURATION_GROUP_NAME_TLS_CONFIGURATION_REMOTE_READER_PAIRING());
	if (tlsRemoteReaderPairingValue.isObject())
	{
		mTlsConfigRemotePsk.load(tlsRemoteReaderPairingValue.toObject());
	}

	mMinStaticKeySizes = readKeySizes(config, CONFIGURATION_GROUP_NAME_MIN_STATIC_KEY_SIZES());
	mMinEphemeralKeySizes = readKeySizes(config, CONFIGURATION_GROUP_NAME_MIN_EPHEMERAL_KEY_SIZES());

	mSelfAuthenticationUrl = readGroup(config, CONFIGURATION_GROUP_NAME_SELF_AUTHENTICATION(), CONFIGURATION_NAME_SELF_AUTHENTICATION_URL());
	mSelfAuthenticationTestUrl = readGroup(config, CONFIGURATION_GROUP_NAME_SELF_AUTHENTICATION(), CONFIGURATION_NAME_SELF_AUTHENTICATION_TEST_URL());

	mUpdateServerBaseUrl = readGroup(config, CONFIGURATION_GROUP_NAME_UPDATE_SERVER(), CONFIGURATION_NAME_UPDATE_SERVER_BASE_URL());
	mWhitelistServerBaseUrl = readGroup(config, CONFIGURATION_GROUP_NAME_WHITELIST_SERVER(), CONFIGURATION_NAME_WHITELIST_SERVER_BASE_URL());

	mAppcastUpdateUrl = readGroup(config, CONFIGURATION_GROUP_NAME_UPDATES(), CONFIGURATION_NAME_APPCAST_UPDATE_URL());
	mAppcastBetaUpdateUrl = readGroup(config, CONFIGURATION_GROUP_NAME_UPDATES(), CONFIGURATION_NAME_APPCAST_BETA_UPDATE_URL());

	mLoaded = true;
	qCInfo(securestorage) << "SecureStorage successfully loaded";
}


QByteArrayList SecureStorage::loadTestCvcsFromAppDir()
{
	QByteArrayList testCvcs;
	const QDir appDir(QCoreApplication::applicationDirPath());
	const QStringList& dirEntries = appDir.entryList({QStringLiteral("*.cvcert.hex")}, QDir::Files);
	for (QString cvcFilePath : dirEntries)
	{
		cvcFilePath = appDir.absolutePath() + QDir::separator() + cvcFilePath;
		const QByteArray& hex = loadTestCvc(cvcFilePath);
		if (hex.isEmpty())
		{
			qWarning() << "Can not load CVC from" << cvcFilePath;
			continue;
		}

		qDebug() << "Adding CVC from" << cvcFilePath << ':' << hex;
		testCvcs += hex;
	}
	return testCvcs;
}


QByteArray SecureStorage::loadTestCvc(const QString& pPath) const
{
	QFile cvcFile(pPath);
	const int TEN_MEGA_BYTE = 10 * 1024 * 1024;
	if (cvcFile.size() > TEN_MEGA_BYTE)
	{
		return QByteArray();
	}
	if (!cvcFile.open(QFile::ReadOnly | QFile::Text))
	{
		return QByteArray();
	}
	return cvcFile.readAll();
}


const QByteArrayList& SecureStorage::getCVRootCertificates(bool pProductive) const
{
	return pProductive ? mCvcas : mCvcasTest;
}


const QVector<QSslCertificate>& SecureStorage::getUpdateCertificates() const
{
	return mUpdateCertificates;
}


const QUrl& SecureStorage::getSelfAuthenticationUrl(bool pTest) const
{
	return pTest ? mSelfAuthenticationTestUrl : mSelfAuthenticationUrl;
}


const QUrl& SecureStorage::getUpdateServerBaseUrl() const
{
	return mUpdateServerBaseUrl;
}


const QUrl& SecureStorage::getWhitelistServerBaseUrl() const
{
	return mWhitelistServerBaseUrl;
}


const QUrl& SecureStorage::getAppcastUpdateUrl() const
{
	return mAppcastUpdateUrl;
}


const QUrl& SecureStorage::getAppcastBetaUpdateUrl() const
{
	return mAppcastBetaUpdateUrl;
}


const TlsConfiguration& SecureStorage::getTlsConfig(TlsSuite pTlsSuite) const
{
	return pTlsSuite == TlsSuite::PSK ? mTlsConfigPsk : mTlsConfig;
}


const TlsConfiguration& SecureStorage::getTlsConfigRemote(TlsSuite pTlsSuite) const
{
	return pTlsSuite == TlsSuite::PSK ? mTlsConfigRemotePsk : mTlsConfigRemote;
}


int SecureStorage::getMinimumStaticKeySize(QSsl::KeyAlgorithm pKeyAlgorithm) const
{
	if (!mMinStaticKeySizes.contains(pKeyAlgorithm))
	{
		qCWarning(securestorage) << "No minimum ephemeral key size specified, returning default";
	}
	return mMinStaticKeySizes.value(pKeyAlgorithm, 0);
}


int SecureStorage::getMinimumEphemeralKeySize(QSsl::KeyAlgorithm pKeyAlgorithm) const
{
	if (!mMinEphemeralKeySizes.contains(pKeyAlgorithm))
	{
		qCWarning(securestorage) << "No minimum ephemeral key size specified, returning default";
	}
	return mMinEphemeralKeySizes.value(pKeyAlgorithm, 0);
}


QJsonArray SecureStorage::readJsonArray(const QJsonObject& pConfig, const QLatin1String pName) const
{
	QJsonValue value = pConfig.value(pName);
	if (!value.isArray())
	{
		qCCritical(securestorage) << "Expecting array for" << pName << "in SecureStorage";
		return QJsonArray();
	}
	return value.toArray();
}


QString SecureStorage::readGroup(const QJsonObject& pConfig, const QLatin1String pGroup, const QLatin1String pName) const
{
	QJsonValue value = pConfig.value(pGroup);
	if (!value.isObject())
	{
		qCCritical(securestorage) << "Expecting group for" << pGroup << "in SecureStorage";
		return QString();
	}

	QJsonObject groupObject = value.toObject();
	value = groupObject.value(pName);
	if (!value.isString())
	{
		qCCritical(securestorage) << "Expecting string for" << pGroup << "/" << pName << "in SecureStorage";
		return QString();
	}

	return value.toString();
}


QMap<QSsl::KeyAlgorithm, int> SecureStorage::readKeySizes(const QJsonObject& pConfig, const QLatin1String pKey) const
{
	QMap<QSsl::KeyAlgorithm, int> keySizes;
	const auto& object = pConfig.value(pKey).toObject();
	if (!object.isEmpty())
	{
		const auto& keys = object.keys();
		for (const QString& key : keys)
		{
			const auto value = object.value(key).toInt(0);
			if (key == QLatin1String("Rsa"))
			{
				keySizes.insert(QSsl::KeyAlgorithm::Rsa, value);
			}
			else if (key == QLatin1String("Dsa"))
			{
				keySizes.insert(QSsl::KeyAlgorithm::Dsa, value);
			}
#if (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
			else if (key == QLatin1String("Dh"))
			{
				keySizes.insert(QSsl::KeyAlgorithm::Dh, value);
			}
#endif
			else if (key == QLatin1String("Ec"))
			{
				keySizes.insert(QSsl::KeyAlgorithm::Ec, value);
			}
			else
			{
				qCCritical(securestorage) << "Ignore unknown key type" << key;
			}
		}
	}
	return keySizes;
}


QByteArrayList SecureStorage::readByteArrayList(const QJsonObject& pConfig, const QLatin1String pName) const
{
	QJsonArray jsonArray = readJsonArray(pConfig, pName);
	QByteArrayList byteArrayList;

	for (int i = 0; i < jsonArray.size(); ++i)
	{
		QJsonValue certificate = jsonArray[i];
		if (!certificate.isString())
		{
			qCCritical(securestorage) << "Expected hexstring in array[" << i << "] " << pName << " in SecureStorage";
			continue;
		}
		byteArrayList += certificate.toString().toLatin1();
	}

	return byteArrayList;
}
