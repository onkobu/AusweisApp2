/*!
 * \copyright Copyright (c) 2014-2022 Governikus GmbH & Co. KG, Germany
 */

#include "ProviderConfigurationParser.h"

#include "LanguageString.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>


Q_DECLARE_LOGGING_CATEGORY(update)


using namespace governikus;


QVector<ProviderConfigurationInfo> ProviderConfigurationParser::parseProvider(const QByteArray& pData, const QOperatingSystemVersion& pCurrentOS)
{
	QJsonParseError jsonError {};
	const auto& json = QJsonDocument::fromJson(pData, &jsonError);
	if (jsonError.error != QJsonParseError::NoError)
	{
		qCCritical(update) << "Cannot parse providers:" << jsonError.errorString();
		return QVector<ProviderConfigurationInfo>();
	}

	const bool eidIsRequired =
			pCurrentOS.type() == QOperatingSystemVersion::IOS
			|| pCurrentOS >= QOperatingSystemVersion(QOperatingSystemVersion::Android, 12);

	const QJsonObject doc = json.object();
	const QJsonArray& array = doc[QLatin1String("provider")].toArray();
	QVector<ProviderConfigurationInfo> providers;
	providers.reserve(array.size());
	for (const auto& entry : array)
	{
		const QJsonObject prov = entry.toObject();

		if (eidIsRequired && !prov[QLatin1String("eidSupport")].toBool(true))
		{
			continue;
		}

		const QString postalAddressCamel = prov[QLatin1String("postalAddress")].toString();
		const QString postalAddressLower = prov[QLatin1String("postaladdress")].toString();
		providers << ProviderConfigurationInfo(
				LanguageString(prov[QLatin1String("shortName")]),
				LanguageString(prov[QLatin1String("longName")]),
				LanguageString(prov[QLatin1String("shortDescription")]),
				LanguageString(prov[QLatin1String("longDescription")]),
				prov[QLatin1String("address")].toString(),
				prov[QLatin1String("homepage")].toString(),
				prov[QLatin1String("category")].toString(),
				prov[QLatin1String("phone")].toString(),
				prov[QLatin1String("email")].toString(),
				postalAddressCamel.isEmpty() ? postalAddressLower : postalAddressCamel, // Accept old format
				prov[QLatin1String("icon")].toString(),
				prov[QLatin1String("image")].toString(),
				prov[QLatin1String("subjectUrls")].toVariant().toStringList(),
				prov[QLatin1String("subjectUrlInfo")].toString(),
				prov[QLatin1String("internalId")].toString());
	}

	return providers;
}


QMap<QString, CallCost> ProviderConfigurationParser::parseCallCosts(const QByteArray& pData)
{
	QJsonParseError jsonError {};
	const auto& json = QJsonDocument::fromJson(pData, &jsonError);
	if (jsonError.error != QJsonParseError::NoError)
	{
		qCCritical(update) << "Cannot parse call costs:" << jsonError.errorString();
		return QMap<QString, CallCost>();
	}
	QJsonObject doc = json.object();

	QMap<QString, CallCost> callCosts;
	const auto& callCostArray = doc[QLatin1String("callcosts")].toArray();
	for (const auto& callCostElem : callCostArray)
	{
		const auto cost = CallCost(callCostElem);
		const auto& prefixArray = callCostElem.toObject()[QLatin1String("prefixes")].toArray();
		for (const auto& prefixElem : prefixArray)
		{
			const auto& prefix = prefixElem.toString();
			callCosts.insert(prefix, cost);
		}
	}

	return callCosts;
}


QVector<ProviderConfigurationInfo> ProviderConfigurationParser::parseProvider(const QByteArray& pData)
{
	return parseProvider(pData, QOperatingSystemVersion::current());
}
