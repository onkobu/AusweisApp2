/*
 *
 * \brief Writes the history entry .
 *
 * \copyright Copyright (c) 2014-2022 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "AbstractState.h"
#include "context/AuthContext.h"
#include "GenericContextContainer.h"


namespace governikus
{

class StateWriteHistory
	: public AbstractState
	, public GenericContextContainer<AuthContext>
{
	Q_OBJECT
	friend class StateBuilder;

	explicit StateWriteHistory(const QSharedPointer<WorkflowContext>& pContext);
	void run() override;

	public:
		void onEntry(QEvent* pEvent) override;
};

} // namespace governikus
