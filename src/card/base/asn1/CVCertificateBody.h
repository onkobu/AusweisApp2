/*!
 * \copyright Copyright (c) 2014-2022 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "ASN1TemplateUtil.h"
#include "ASN1Util.h"
#include "Chat.h"
#include "EcdsaPublicKey.h"

#include <QCryptographicHash>
#include <QDate>
#include <QMap>
#include <QSharedPointer>

#include <openssl/ec.h>


namespace governikus
{

using CERTIFICATEEXTENSION = struct CERTIFICATEEXTENSION_st
{
	ASN1_OBJECT* mOid;
	ASN1_OCTET_STRING* mObject1;
	ASN1_OCTET_STRING* mObject2;
	ASN1_OCTET_STRING* mObject3;
	ASN1_OCTET_STRING* mObject4;
	ASN1_OCTET_STRING* mObject5;
	ASN1_OCTET_STRING* mObject6;
	ASN1_OCTET_STRING* mObject7;
	ASN1_OCTET_STRING* mObject8;
};
DECLARE_ASN1_FUNCTIONS(CERTIFICATEEXTENSION)

#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(LIBRESSL_VERSION_NUMBER)
DECLARE_STACK_OF(CERTIFICATEEXTENSION)
#else
DEFINE_STACK_OF(CERTIFICATEEXTENSION)
#endif

using CVCertificateBody = struct certificateprofilebody_st
{
	ASN1_OCTET_STRING* mCertificateProfileIdentifier;
	ASN1_STRING* mCertificationAuthorityReference;
	EcdsaPublicKey* mPublicKey;
	ASN1_STRING* mCertificateHolderReference;
	CHAT* mChat;
	ASN1_OCTET_STRING* mEffectiveDate;
	ASN1_OCTET_STRING* mExpirationDate;
	STACK_OF(CERTIFICATEEXTENSION) * mExtensions;

	static QSharedPointer<certificateprofilebody_st> decode(const QByteArray& pBytes);
	QByteArray encode();

	[[nodiscard]] QByteArray getCertificateProfileIdentifier() const;
	[[nodiscard]] QByteArray getCertificationAuthorityReference() const;

	[[nodiscard]] const EcdsaPublicKey& getPublicKey() const;
	[[nodiscard]] QByteArray getCertificateHolderReference() const;

	[[nodiscard]] const CHAT& getCHAT() const;

	void setCertificateExpirationDate(QDate date);
	[[nodiscard]] QDate getCertificateExpirationDate() const;

	void setCertificateEffectiveDate(QDate date);
	[[nodiscard]] QDate getCertificateEffectiveDate() const;

	[[nodiscard]] QCryptographicHash::Algorithm getHashAlgorithm() const;
	[[nodiscard]] QMap<QByteArray, QByteArray> getExtensions() const;
};

DECLARE_ASN1_FUNCTIONS(CVCertificateBody)
DECLARE_ASN1_OBJECT(CVCertificateBody)

} // namespace governikus
