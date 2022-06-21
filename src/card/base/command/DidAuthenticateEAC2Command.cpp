/*!
 * \copyright Copyright (c) 2014-2022 Governikus GmbH & Co. KG, Germany
 */

#include "DidAuthenticateEAC2Command.h"

#include "asn1/ChipAuthenticationInfo.h"
#include "asn1/EFCardSecurity.h"
#include "CardConnection.h"
#include "EABuilder.h"
#include "GABuilder.h"
#include "GeneralAuthenticateResponse.h"
#include "GlobalStatus.h"
#include "MSEBuilder.h"
#include "PSOBuilder.h"

#include <QLoggingCategory>


Q_DECLARE_LOGGING_CATEGORY(card)


using namespace governikus;


DidAuthenticateEAC2Command::DidAuthenticateEAC2Command(QSharedPointer<CardConnectionWorker> pCardConnectionWorker,
		const CVCertificateChain& pCvcChain, const QByteArray& pEphemeralPublicKeyAsHex,
		const QByteArray& pSignatureAsHex, const QByteArray& pAuthenticatedAuxiliaryDataAsBinary)
	: BaseCardCommand(pCardConnectionWorker)
	, mCvcChain(pCvcChain)
	, mEphemeralPublicKeyAsHex(pEphemeralPublicKeyAsHex)
	, mSignatureAsHex(pSignatureAsHex)
	, mAuthenticatedAuxiliaryDataAsBinary(pAuthenticatedAuxiliaryDataAsBinary)
	, mEfCardSecurityAsHex()
	, mNonceAsHex()
	, mAuthTokenAsHex()
{
}


void DidAuthenticateEAC2Command::internalExecute()
{
	mReturnCode = putCertificateChain(mCvcChain);
	if (mReturnCode != CardReturnCode::OK)
	{
		return;
	}

	QByteArray taProtocol = mCvcChain.getTerminalCvc()->getBody().getPublicKey().getPublicKeyOidValueBytes();
	QByteArray chr = mCvcChain.getTerminalCvc()->getBody().getCertificateHolderReference();

	QByteArray ephemeralPublicKey = QByteArray::fromHex(mEphemeralPublicKeyAsHex);
	if (ephemeralPublicKey.size() % 2 == 0)
	{
		/*
		 * According to TR-03111, chapter 3.2.1 the uncompressed encoding of an elliptic curve point is:
		 *  0x04 | xCoordinate | yCoordinate
		 * In contrast the AGETO and mtG server just sends xCoordinate | yCoordinate.
		 *
		 * We fix this by prepending 0x04
		 */
		ephemeralPublicKey.prepend(0x04);
	}
	QByteArray compressedEphemeralPublicKey = ephemeralPublicKey.mid(1, (ephemeralPublicKey.size() - 1) / 2);

	QByteArray signature = QByteArray::fromHex(mSignatureAsHex);
	mReturnCode = performTerminalAuthentication(taProtocol,
			chr,
			mAuthenticatedAuxiliaryDataAsBinary,
			compressedEphemeralPublicKey,
			signature);

	if (mReturnCode != CardReturnCode::OK)
	{
		return;
	}

	QByteArray efCardSecurityBytes;
	qCDebug(card) << "Performing Read EF.CardSecurity";
	mReturnCode = mCardConnectionWorker->readFile(FileRef::efCardSecurity(), efCardSecurityBytes);
	if (mReturnCode != CardReturnCode::OK)
	{
		return;
	}
	mEfCardSecurityAsHex += efCardSecurityBytes.toHex();
	QSharedPointer<EFCardSecurity> efCardSecurity = EFCardSecurity::decode(efCardSecurityBytes);
	if (efCardSecurity == nullptr)
	{
		qCCritical(card) << "Cannot parse EF.CardSecurity";
		mReturnCode = CardReturnCode::PROTOCOL_ERROR;
		return;
	}

	const auto& chipAuthenticationInfoList = efCardSecurity->getSecurityInfos()->getChipAuthenticationInfos();
	if (chipAuthenticationInfoList.isEmpty())
	{
		qCCritical(card) << "No ChipAuthenticationInfo found in EF.CardAccess";
		mReturnCode = CardReturnCode::PROTOCOL_ERROR;
		return;
	}
	// we do not know of any procedural rule to determine the ChipAuthenticationInfo, so we just take the first one
	const auto& chipAuthenticationInfo = chipAuthenticationInfoList.at(0);
	qCDebug(card) << "Chose ChipAuthenticationInfo(0): protocol" << chipAuthenticationInfo->getProtocol() << ", keyId" << chipAuthenticationInfo->getKeyId().toHex();

	mReturnCode = performChipAuthentication(chipAuthenticationInfo, ephemeralPublicKey);
	if (CardReturnCode::OK == mReturnCode)
	{
		mCardConnectionWorker->stopSecureMessaging();
	}
}


CardReturnCode DidAuthenticateEAC2Command::putCertificateChain(const CVCertificateChain& pCvcChain)
{
	for (const auto& cvCertificate : pCvcChain)
	{
		QByteArray car = cvCertificate->getBody().getCertificationAuthorityReference();
		MSEBuilder mseBuilder(MSEBuilder::P1::SET_DST, MSEBuilder::P2::COMPUTE_DIGITAL_SIGNATURE);
		mseBuilder.setPublicKey(car);

		qCDebug(card) << "Performing TA MSE:Set DST";
		qCDebug(card) << "Sending CAR" << car;
		auto [mseReturnCode, mseResult] = mCardConnectionWorker->transmit(mseBuilder.build());
		if (mseReturnCode != CardReturnCode::OK)
		{
			qCWarning(card) << "TA MSE:Set DST failed.";
			return mseReturnCode;
		}

		if (mseResult.getReturnCode() != StatusCode::SUCCESS)
		{
			qCWarning(card) << "TA MSE:Set DST failed:" << mseResult.getReturnCode();
			return CardReturnCode::PROTOCOL_ERROR;
		}

		PSOBuilder psoBuilder(PSOBuilder::P1::VERIFY, PSOBuilder::P2::CERTIFICATE);
		psoBuilder.setCertificateBody(cvCertificate->getRawBody());
		psoBuilder.setSignature(cvCertificate->getRawSignature());

		qCDebug(card) << "Performing TA PSO:Verify Certificate";
		qCDebug(card) << "Sending certificate" << *cvCertificate;
		auto [psoReturnCode, psoResult] = mCardConnectionWorker->transmit(psoBuilder.build());
		if (psoReturnCode != CardReturnCode::OK)
		{
			return psoReturnCode;
		}

		if (psoResult.getReturnCode() != StatusCode::SUCCESS)
		{
			qCWarning(card) << "TA PSO:Verify Certificate failed:" << psoResult.getReturnCode();
			return CardReturnCode::PROTOCOL_ERROR;
		}
	}

	return CardReturnCode::OK;
}


CardReturnCode DidAuthenticateEAC2Command::performTerminalAuthentication(const QByteArray& taProtocol,
		const QByteArray& chr,
		const QByteArray& auxiliaryData,
		const QByteArray& compressedEphemeralPublicKey,
		const QByteArray& signature)
{

	MSEBuilder mseBuilder(MSEBuilder::P1::SET_DST, MSEBuilder::P2::SET_AT);
	mseBuilder.setOid(taProtocol);
	mseBuilder.setPublicKey(chr);
	mseBuilder.setAuxiliaryData(auxiliaryData);
	mseBuilder.setEphemeralPublicKey(compressedEphemeralPublicKey);

	qCDebug(card) << "Performing TA MSE:Set AT";
	auto[mseReturnCode, mseResult] = mCardConnectionWorker->transmit(mseBuilder.build());
	if (mseReturnCode != CardReturnCode::OK)
	{
		qCWarning(card) << "TA MSE:Set AT failed:" << CardReturnCodeUtil::toGlobalStatus(mseReturnCode);
		return mseReturnCode;
	}

	if (mseResult.getReturnCode() != StatusCode::SUCCESS)
	{
		qCWarning(card) << "TA MSE:Set AT failed:" << mseResult.getReturnCode();
		return CardReturnCode::PROTOCOL_ERROR;
	}

	EABuilder eaBuilder;
	eaBuilder.setSignature(signature);
	qCDebug(card) << "Performing TA External Authenticate";
	auto [eaReturnCode, eaResult] = mCardConnectionWorker->transmit(eaBuilder.build());
	if (eaReturnCode != CardReturnCode::OK)
	{
		qCWarning(card) << "TA External Authenticate failed:" << CardReturnCodeUtil::toGlobalStatus(eaReturnCode);
		return eaReturnCode;
	}

	if (eaResult.getReturnCode() != StatusCode::SUCCESS)
	{
		qCWarning(card) << "TA External Authenticate failed:" << eaResult.getReturnCode();
		return CardReturnCode::PROTOCOL_ERROR;
	}

	return CardReturnCode::OK;
}


CardReturnCode DidAuthenticateEAC2Command::performChipAuthentication(QSharedPointer<const ChipAuthenticationInfo> pChipAuthInfo,
		const QByteArray& ephemeralPublicKey)
{
	MSEBuilder mseBuilder(MSEBuilder::P1::COMPUTE_DIGITAL_SIGNATURE, MSEBuilder::P2::SET_AT);
	mseBuilder.setOid(pChipAuthInfo->getProtocolValueBytes());
	mseBuilder.setPrivateKey(pChipAuthInfo->getKeyId());

	qCDebug(card) << "Performing CA MSE:Set AT";
	auto [mseReturnCode, mseResult] = mCardConnectionWorker->transmit(mseBuilder.build());
	if (mseReturnCode != CardReturnCode::OK)
	{
		return mseReturnCode;
	}

	if (mseResult.getReturnCode() != StatusCode::SUCCESS)
	{
		qCWarning(card) << "CA MSE:Set AT failed:" << mseResult.getReturnCode();
		return CardReturnCode::PROTOCOL_ERROR;
	}

	GABuilder gaBuilder;
	gaBuilder.setCaEphemeralPublicKey(ephemeralPublicKey);

	qCDebug(card) << "Performing CA General Authenticate";
	auto [gaReturnCode, gaGenericResponse] = mCardConnectionWorker->transmit(gaBuilder.build());
	if (gaReturnCode != CardReturnCode::OK)
	{
		return gaReturnCode;
	}

	if (gaGenericResponse.getReturnCode() != StatusCode::SUCCESS)
	{
		qCWarning(card) << "CA General Authenticate failed:" << gaGenericResponse.getReturnCode();
		return CardReturnCode::PROTOCOL_ERROR;
	}

	const GAChipAuthenticationResponse gaResponse(gaGenericResponse);
	mNonceAsHex += gaResponse.getNonce().toHex();
	mAuthTokenAsHex += gaResponse.getAuthenticationToken().toHex();

	return CardReturnCode::OK;
}
