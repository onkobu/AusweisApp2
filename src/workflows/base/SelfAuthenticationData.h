/*!
 * \brief Parses self authentication data from JSON data and provides its content.
 *
 * \copyright Copyright (c) 2014-2022 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "EnumHelper.h"

#include <functional>

#include <QCoreApplication>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonValue>
#include <QMap>
#include <QPair>
#include <QSharedData>
#include <QString>
#include <QVector>

namespace governikus
{

defineEnumType(SelfAuthDataPermission,
		UNKNOWN /* used for parser only */,
		ALLOWED,
		PROHIBITED,
		NOTONCHIP)

defineEnumType(SelfAuthData,
		UNKNOWN, /* used for parser only */
		DocumentType,
		IssuingState,
		DateOfExpiry,
		GivenNames,
		FamilyNames,
		ArtisticName,
		AcademicTitle,
		BirthName,
		DateOfBirth,
		PlaceOfBirth,
		PlaceOfResidence,
		Nationality,
		ResidencePermitI,
		RestrictedID,
		AgeVerification,
		PlaceVerification,
		/* additional fields */
		PlaceOfResidenceStreet,
		PlaceOfResidenceCity,
		PlaceOfResidenceCountry,
		PlaceOfResidenceZipCode,
		PlaceOfResidenceNoPlaceInfo)


class SelfAuthenticationData
{
	public:
		using OrderedSelfData = QVector<QPair<QString, QString>>;

	private:
		class SelfData
			: public QSharedData
		{
			Q_DECLARE_TR_FUNCTIONS(governikus::SelfData)

			private:
				bool parse(const QByteArray& pData);
				bool parseOperationsAllowedByUser(const QJsonObject& pObject);
				bool parsePersonalData(const QJsonObject& pObject);
				bool tryToInsertChild(const QJsonValue& pValue, SelfAuthData pAuthData);

			public:
				bool mValid;
				const QDateTime mDateTime;
				QMap<SelfAuthData, SelfAuthDataPermission> mOperationsAllowed;
				QMap<SelfAuthData, QString> mSelfAuthData;

				explicit SelfData(const QByteArray& pData);
				QString getValue(SelfAuthData pData) const;
				OrderedSelfData getOrderedSelfInfo() const;
		};

		QSharedDataPointer<SelfData> d;

	public:
		explicit SelfAuthenticationData(const QByteArray& pData = QByteArray());
		~SelfAuthenticationData() = default;

		/**
		 * Check if parsing of given data was successful.
		 */
		[[nodiscard]] bool isValid() const;
		[[nodiscard]] QString getValue(SelfAuthData pData) const;
		[[nodiscard]] const QDateTime& getDateTime() const;
		[[nodiscard]] OrderedSelfData getOrderedSelfData() const;
};

} // namespace governikus
