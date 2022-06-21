/*!
 * \brief Class representing a smart card
 *
 * \copyright Copyright (c) 2014-2022 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "CardReturnCode.h"
#include "CommandApdu.h"
#include "EstablishPaceChannelOutput.h"
#include "ResponseApdu.h"
#include "SmartCardDefinitions.h"

#include <QObject>
#include <QPointer>


namespace governikus
{

class Card
	: public QObject
{
	Q_OBJECT

	public:
		Card();
		~Card() override = default;

		/*!
		 * Establish a connection to the smart card
		 */
		virtual CardReturnCode connect() = 0;

		/*!
		 * Destroys the previously established connection to the smart card
		 */
		virtual CardReturnCode disconnect() = 0;

		/*!
		 * Is the smart card connected, i.e. has a connection successfully been established?
		 */
		virtual bool isConnected() = 0;

		/*!
		 * Sets the current workflow progress message. This is necessary for platforms like iOS,
		 * where interacting with a card leads to a dialog where the message needs to be updated.
		 */
		virtual void setProgressMessage(const QString& pMessage, int pProgress = -1);

		/*!
		 * Performs a transmit to the smart card.
		 * The command APDU buffer is transmitted to the card.
		 * The response APDU buffer is filled with the data returned from the card.
		 */
		virtual ResponseApduResult transmit(const CommandApdu& pCmd) = 0;

		/*!
		 * Establishes a PACE channel, i.e. the corresponding reader is no basic reader.
		 */
		virtual EstablishPaceChannelOutput establishPaceChannel(PacePasswordId pPasswordId, int pPreferredPinLength, const QByteArray& pChat, const QByteArray& pCertificateDescription, quint8 pTimeoutSeconds = 60);

		/*!
		 * Destroys an existing  PACE channel, i.e. the corresponding reader is no basic reader.
		 */
		virtual CardReturnCode destroyPaceChannel();

		/*!
		 * Sets a new eID PIN, i.e. the corresponding reader is no basic reader.
		 */
		virtual ResponseApduResult setEidPin(quint8 pTimeoutSeconds);

		/*!
		 * Combines the message and progressvalue depending on the environment.
		 */
		static QString generateProgressMessage(const QString& pMessage, int pProgress = -1);
};

} // namespace governikus
