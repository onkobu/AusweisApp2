/*!
 * \brief Parse information for DidAuthenticateEac2.
 *
 * \copyright Copyright (c) 2014-2022 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "paos/element/Eac2InputType.h"
#include "paos/PaosMessage.h"
#include "paos/retrieve/DidAuthenticateEac2.h"
#include "paos/retrieve/PaosParser.h"

#include <QScopedPointer>


namespace governikus
{

class DidAuthenticateEac2Parser
	: public PaosParser
{
	public:
		DidAuthenticateEac2Parser();
		~DidAuthenticateEac2Parser() override;

	protected:
		PaosMessage* parseMessage() override;

	private:
		Eac2InputType parseEac2InputType();
		void parseCertificate(Eac2InputType& pEac2);
		void parseEphemeralPublicKey(Eac2InputType& pEac2, QString& pEphemeralPublicKey);
		void parseSignature(Eac2InputType& pEac2, QString& pSignature);

	private:
		QScopedPointer<DIDAuthenticateEAC2> mDidAuthenticateEac2;
};

} // namespace governikus
