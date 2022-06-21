/*
 * \copyright Copyright (c) 2017-2022 Governikus GmbH & Co. KG, Germany
 */

import QtTest 1.12

TestCase {
	name: "ModuleImportTest"
	id: parent

	function test_load_Navigation() {
		var item = createTemporaryQmlObject("
			import Governikus.Navigation 1.0;
			Navigation {}
			", parent);
		item.destroy();
	}
}
