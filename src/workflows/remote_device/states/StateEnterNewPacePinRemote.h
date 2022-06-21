/*!
 * \copyright Copyright (c) 2018-2022 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "context/RemoteServiceContext.h"
#include "states/AbstractState.h"
#include "states/GenericContextContainer.h"

namespace governikus
{

class StateEnterNewPacePinRemote
	: public AbstractState
	, public GenericContextContainer<RemoteServiceContext>
{
	Q_OBJECT
	friend class StateBuilder;

	private:
		explicit StateEnterNewPacePinRemote(const QSharedPointer<WorkflowContext>& pContext);
		void run() override;

	private Q_SLOTS:
		void onCancelChangePin();

	public:
		void onEntry(QEvent* pEvent) override;
};

} // namespace governikus
