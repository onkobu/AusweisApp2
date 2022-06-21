/*!
 * \copyright Copyright (c) 2014-2022 Governikus GmbH & Co. KG, Germany
 */

#include "SelfAuthenticationData.h"

#include "LanguageLoader.h"

#include <QJsonDocument>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(secure)

using namespace governikus;


SelfAuthenticationData::SelfAuthenticationData(const QByteArray& pData)
	: d(new SelfData(pData))
{
}


bool SelfAuthenticationData::isValid() const
{
	return d->mValid;
}


QString SelfAuthenticationData::getValue(SelfAuthData pData) const
{
	return d->getValue(pData);
}


const QDateTime& SelfAuthenticationData::getDateTime() const
{
	return d->mDateTime;
}


SelfAuthenticationData::OrderedSelfData SelfAuthenticationData::getOrderedSelfData() const
{
	return d->getOrderedSelfInfo();
}


SelfAuthenticationData::SelfData::SelfData(const QByteArray& pData)
	: mValid(false)
	, mDateTime(QDateTime::currentDateTime())
	, mOperationsAllowed()
	, mSelfAuthData()
{
	if (pData.isEmpty())
	{
		return;
	}
	qCDebug(secure).noquote() << "parsing data:\n" << pData;

	mValid = parse(pData);
}


bool SelfAuthenticationData::SelfData::parse(const QByteArray& pData)
{
	QJsonParseError jsonError {};
	const auto& json = QJsonDocument::fromJson(pData, &jsonError);
	if (jsonError.error != QJsonParseError::NoError)
	{
		qDebug() << "JSON parsing failed:" << jsonError.errorString();
		return false;
	}

	const auto& obj = json.object();
	return parseOperationsAllowedByUser(obj.value(QLatin1String("OperationsAllowedByUser")).toObject())
		   && parsePersonalData(obj.value(QLatin1String("PersonalData")).toObject());
}


QString SelfAuthenticationData::SelfData::getValue(SelfAuthData pData) const
{
	if (mOperationsAllowed.value(pData) == SelfAuthDataPermission::ALLOWED)
	{
		return mSelfAuthData.value(pData);
	}
	else if (mOperationsAllowed.value(pData) == SelfAuthDataPermission::NOTONCHIP)
	{
		if (pData == SelfAuthData::Nationality && getValue(SelfAuthData::DocumentType) == QLatin1String("ID"))
		{
			return QStringLiteral("D");
		}

		//: INFO ALL_PLATFORMS The requested data is not stored on this chip's generation.
		return tr("This data has not been stored in this chip generation.");
	}

	return QString();
}


bool SelfAuthenticationData::SelfData::parseOperationsAllowedByUser(const QJsonObject& pObject)
{
	const auto& keys = pObject.keys();
	for (const auto& entry : keys)
	{
		auto authData = Enum<SelfAuthData>::fromString(entry, SelfAuthData::UNKNOWN);
		if (authData == SelfAuthData::UNKNOWN)
		{
			qWarning() << "SelfAuthData is unknown:" << entry;
			continue;
		}

		const auto tagName = pObject.value(entry).toString();
		auto permission = Enum<SelfAuthDataPermission>::fromString(tagName, SelfAuthDataPermission::UNKNOWN);
		if (permission == SelfAuthDataPermission::UNKNOWN)
		{
			qWarning() << "SelfAuthDataPermission is unknown:" << entry << '|' << tagName;
			continue;
		}

		mOperationsAllowed.insert(authData, permission);
	}

	return true;
}


bool SelfAuthenticationData::SelfData::parsePersonalData(const QJsonObject& pObject)
{
	const auto& keys = pObject.keys();
	for (const auto& entry : keys)
	{
		const auto subvalue = [&pObject, &entry](const char* const pValue){
					return pObject.value(entry).toObject().value(QLatin1String(pValue));
				};

		auto authData = Enum<SelfAuthData>::fromString(entry, SelfAuthData::UNKNOWN);
		if (authData == SelfAuthData::UNKNOWN)
		{
			qWarning() << "PersonalData is unknown:" << entry;
			continue;
		}

		if (authData == SelfAuthData::PlaceOfBirth)
		{
			tryToInsertChild(subvalue("FreetextPlace"), authData);
		}
		else if (authData == SelfAuthData::PlaceOfResidence)
		{
			if (tryToInsertChild(subvalue("NoPlaceInfo"), SelfAuthData::PlaceOfResidenceNoPlaceInfo))
			{
				mOperationsAllowed.insert(SelfAuthData::PlaceOfResidenceNoPlaceInfo, mOperationsAllowed.value(authData));
			}
			else
			{
				static const QMap<QString, SelfAuthData> placeInfo = {
					{QStringLiteral("City"), SelfAuthData::PlaceOfResidenceCity},
					{QStringLiteral("Country"), SelfAuthData::PlaceOfResidenceCountry},
					{QStringLiteral("Street"), SelfAuthData::PlaceOfResidenceStreet},
					{QStringLiteral("ZipCode"), SelfAuthData::PlaceOfResidenceZipCode}
				};

				const auto structuredPlace = subvalue("StructuredPlace").toObject();
				for (auto iter = placeInfo.constBegin(); iter != placeInfo.constEnd(); ++iter)
				{
					if (tryToInsertChild(structuredPlace.value(iter.key()), iter.value()))
					{
						mOperationsAllowed.insert(iter.value(), mOperationsAllowed.value(authData));
					}
				}
			}
		}
		else
		{
			tryToInsertChild(pObject.value(entry), authData);
		}
	}

	return true;
}


bool SelfAuthenticationData::SelfData::tryToInsertChild(const QJsonValue& pValue, SelfAuthData pAuthData)
{
	if (pValue.isString())
	{
		mSelfAuthData.insert(pAuthData, pValue.toString());
		return true;
	}

	return false;
}


SelfAuthenticationData::OrderedSelfData SelfAuthenticationData::SelfData::getOrderedSelfInfo() const
{
	OrderedSelfData orderedSelfData;

	if (!mValid)
	{
		return orderedSelfData;
	}

	const auto& formatDate = [](const QString& pIn){
				static const QVector<QPair<QString, QLatin1String> > formattingPattern({
							qMakePair(QStringLiteral("yyyy-MM-dd+hh:mm"), QLatin1String(QT_TR_NOOP("dd.MM.yyyy"))),
							qMakePair(QStringLiteral("yyyy-MM"), QLatin1String(QT_TR_NOOP("xx.MM.yyyy"))),
							qMakePair(QStringLiteral("yyyy"), QLatin1String(QT_TR_NOOP("xx.xx.yyyy"))),
						});

				for (const auto& entry : formattingPattern)
				{
					const QDateTime dateTime = QDateTime::fromString(pIn, entry.first);
					if (dateTime.isValid())
					{
						return LanguageLoader::getInstance().getUsedLocale().toString(dateTime, tr(entry.second.data()));
					}
				}

				return QString();
			};

	const auto& add = [&](const QString& pKey, const QString& pValue){
#if !defined(Q_OS_ANDROID)
				if (!pKey.isEmpty())
				{
					orderedSelfData << qMakePair(pKey + QLatin1Char(':'), pValue);
				}
				else
#endif
				{
					orderedSelfData << qMakePair(pKey, pValue);
				}
			};

	//fill layout with new data, see 18 Personalausweisgesetz (PAuswG)
	if (!getValue(SelfAuthData::FamilyNames).isNull())
	{
		//: LABEL ALL_PLATFORMS
		add(tr("Family name"), getValue(SelfAuthData::FamilyNames));
	}
	if (!getValue(SelfAuthData::BirthName).isNull())
	{
		//: LABEL ALL_PLATFORMS
		add(tr("Birth name"), getValue(SelfAuthData::BirthName));
	}
	if (!getValue(SelfAuthData::GivenNames).isNull())
	{
		//: LABEL ALL_PLATFORMS
		add(tr("Given name(s)"), getValue(SelfAuthData::GivenNames));
	}
	if (!getValue(SelfAuthData::AcademicTitle).isNull())
	{
		//: LABEL ALL_PLATFORMS
		add(tr("Doctoral degree"), getValue(SelfAuthData::AcademicTitle));
	}
	if (!getValue(SelfAuthData::DateOfBirth).isNull())
	{
		//: LABEL ALL_PLATFORMS
		add(tr("Date of birth"), formatDate(getValue(SelfAuthData::DateOfBirth)));
	}
	if (!getValue(SelfAuthData::PlaceOfBirth).isNull())
	{
		//: LABEL ALL_PLATFORMS
		add(tr("Place of birth"), getValue(SelfAuthData::PlaceOfBirth));
	}
	if (!getValue(SelfAuthData::PlaceOfResidenceNoPlaceInfo).isNull())
	{
		//: LABEL ALL_PLATFORMS
		add(tr("Address"), getValue(SelfAuthData::PlaceOfResidenceNoPlaceInfo));
	}
	if (!getValue(SelfAuthData::PlaceOfResidenceStreet).isNull())
	{
		//: LABEL ALL_PLATFORMS
		add(getValue(SelfAuthData::PlaceOfResidenceNoPlaceInfo).isNull() ? tr("Address") : QString(), getValue(SelfAuthData::PlaceOfResidenceStreet));
	}
	if (!getValue(SelfAuthData::PlaceOfResidenceZipCode).isNull() || !getValue(SelfAuthData::PlaceOfResidenceCity).isNull())
	{
		//: LABEL ALL_PLATFORMS
		add(getValue(SelfAuthData::PlaceOfResidenceStreet).isNull() ? tr("Address") : QString(), getValue(SelfAuthData::PlaceOfResidenceZipCode) + QLatin1Char(' ') + getValue(SelfAuthData::PlaceOfResidenceCity));
	}
	if (!getValue(SelfAuthData::PlaceOfResidenceCountry).isNull())
	{
		add(QString(), getValue(SelfAuthData::PlaceOfResidenceCountry));
	}

	const auto& documentType = getValue(SelfAuthData::DocumentType);
	if (!documentType.isNull())
	{
		//: LABEL ALL_PLATFORMS
		add(tr("Document type"), documentType);
	}
	if (!getValue(SelfAuthData::Nationality).isNull())
	{
		//: LABEL ALL_PLATFORMS
		add(tr("Nationality"), getValue(SelfAuthData::Nationality));
	}
	if (!getValue(SelfAuthData::ArtisticName).isNull())
	{
		//: LABEL ALL_PLATFORMS
		add(tr("Religious / artistic name"), getValue(SelfAuthData::ArtisticName));
	}
	if (!getValue(SelfAuthData::IssuingState).isNull())
	{
		//: LABEL ALL_PLATFORMS
		add(tr("Issuing country"), getValue(SelfAuthData::IssuingState));
	}

	// Show "Residence Permit" for eAT- and Test-Cards only
	// AR, AS, AF --> see TR-03127 (v1.16) chapter 3.2.3
	// TA --> Used by Test-Cards
	if (!getValue(SelfAuthData::ResidencePermitI).isNull() && (
				documentType == QLatin1String("AR") ||
				documentType == QLatin1String("AS") ||
				documentType == QLatin1String("AF") ||
				documentType == QLatin1String("TA")))
	{
		//: LABEL ALL_PLATFORMS
		add(tr("Residence permit I"), getValue(SelfAuthData::ResidencePermitI));
	}

	if (!getValue(SelfAuthData::DateOfExpiry).isNull())
	{
		//: LABEL ALL_PLATFORMS
		add(tr("Date of expiry"), formatDate(getValue(SelfAuthData::DateOfExpiry)));
	}

	return orderedSelfData;
}


#include "moc_SelfAuthenticationData.cpp"
