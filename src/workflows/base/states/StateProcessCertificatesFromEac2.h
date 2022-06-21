/*!
 * \brief State to process certificates from EAC2.
 *
 * \copyright Copyright (c) 2014-2022 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "AbstractState.h"
#include "context/AuthContext.h"
#include "GenericContextContainer.h"


namespace governikus
{

class StateProcessCertificatesFromEac2
	: public AbstractState
	, public GenericContextContainer<AuthContext>
{
	Q_OBJECT
	friend class StateBuilder;

	explicit StateProcessCertificatesFromEac2(const QSharedPointer<WorkflowContext>& pContext);
	void run() override;
};

} // namespace governikus
