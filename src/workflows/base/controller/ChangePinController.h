/*!
 * \brief Controller for the PIN changing process.
 *
 * \copyright Copyright (c) 2014-2022 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "WorkflowController.h"


namespace governikus
{

class ChangePinContext;

class ChangePinController
	: public WorkflowController
{
	Q_OBJECT

	public:
		explicit ChangePinController(QSharedPointer<ChangePinContext> pContext);
		~ChangePinController() override;
};

} // namespace governikus
