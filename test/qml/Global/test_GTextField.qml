/*
 * \copyright Copyright (c) 2020-2022 Governikus GmbH & Co. KG, Germany
 */

import QtQuick 2.12
import QtTest 1.12

import Governikus.Global 1.0
import Governikus.Style 1.0

TestCase {
	id: testCase

	name: "test_GTextField"
	visible: true
	when: windowShown

	function createTestObject() {
		return createTemporaryQmlObject("import Governikus.Global 1.0; GTextField {}", testCase)
	}

	TestCase {
		when: testCase.completed
		optional: true

		function benchmark_load() {
			let testObject = createTestObject()
			testObject.destroy()
		}
	}

	function test_load() {
		let testObject = createTestObject()
		verify(testObject, "Object loaded")
	}

	function test_valid() {
		let testObject = createTestObject()

		verify(testObject.valid, "Initial valid true")

		testObject.valid = false
		verify(!testObject.valid, "valid false")
	}

	function test_enabled() {
		let testObject = createTestObject()

		verify(testObject.enabled, "Initial enabled true")

		testObject.enabled = false
		verify(!testObject.enabled, "enabled false")
	}

	function test_text() {
		let testObject = createTestObject()

		compare(testObject.text, "", "Initial text empty")

		testObject.text = "test"
		compare(testObject.text, "test", "text: test")
	}

	function test_enterKeyType() {
		let testObject = createTestObject()

		compare(testObject.enterKeyType, Qt.EnterKeyDefault, "Initial enterKeyType: Qt.EnterKeyDefault")

		testObject.enterKeyType = Qt.EnterKeyDone
		compare(testObject.enterKeyType, Qt.EnterKeyDone, "enterKeyType Qt.EnterKeyDone")
	}

	function test_textStyle() {
		let testObject = createTestObject()

		compare(testObject.textStyle, Constants.is_desktop ? Style.text.normal_inverse : Style.text.normal, "Initial textStyle: normal")

		testObject.textStyle = Style.text.hint_warning
		compare(testObject.textStyle, Style.text.hint_warning, "textStyle: hint_warning")
	}

}
