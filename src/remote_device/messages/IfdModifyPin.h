/*!
 * \copyright Copyright (c) 2018-2022 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "RemoteMessage.h"


namespace governikus
{

class IfdModifyPin
	: public RemoteMessage
{
	private:
		QString mSlotHandle;
		QByteArray mInputData;

	public:
		IfdModifyPin(const QString& pSlotHandle = QString(), const QByteArray& pInputData = QByteArray());
		explicit IfdModifyPin(const QJsonObject& pMessageObject);
		~IfdModifyPin() override = default;

		[[nodiscard]] const QString& getSlotHandle() const;
		[[nodiscard]] const QByteArray& getInputData() const;
		[[nodiscard]] QByteArray toByteArray(const IfdVersion& pIfdVersion, const QString& pContextHandle) const override;
};


} // namespace governikus
