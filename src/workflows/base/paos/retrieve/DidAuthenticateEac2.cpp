/*!
 * \copyright Copyright (c) 2014-2022 Governikus GmbH & Co. KG, Germany
 */

#include "paos/retrieve/DidAuthenticateEac2.h"

using namespace governikus;


DIDAuthenticateEAC2::DIDAuthenticateEAC2()
	: PaosMessage(PaosType::DID_AUTHENTICATE_EAC2)
{
}


DIDAuthenticateEAC2::~DIDAuthenticateEAC2()
{
}


const ConnectionHandle& DIDAuthenticateEAC2::getConnectionHandle() const
{
	return mConnectionHandle;
}


const QString& DIDAuthenticateEAC2::getDidName() const
{
	return mDidName;
}


const QString& DIDAuthenticateEAC2::getSignature() const
{
	return mEac2.getSignature();
}


const QVector<QSharedPointer<const CVCertificate> >& DIDAuthenticateEAC2::getCvCertificates() const
{
	return mEac2.getCvCertificates();
}


const QByteArrayList& DIDAuthenticateEAC2::getCvCertificatesAsBinary() const
{
	return mEac2.getCvCertificatesAsBinary();
}


const QString& DIDAuthenticateEAC2::getEphemeralPublicKey() const
{
	return mEac2.getEphemeralPublicKey();
}


void DIDAuthenticateEAC2::setConnectionHandle(const ConnectionHandle& connectionHandle)
{
	mConnectionHandle = connectionHandle;
}


void DIDAuthenticateEAC2::setDidName(const QString& didName)
{
	mDidName = didName;
}


void DIDAuthenticateEAC2::setEac2InputType(const Eac2InputType& pEac2)
{
	mEac2 = pEac2;
}
