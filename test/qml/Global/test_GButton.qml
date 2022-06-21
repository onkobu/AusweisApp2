/*
 * \copyright Copyright (c) 2020-2022 Governikus GmbH & Co. KG, Germany
 */

import QtQuick 2.12
import QtTest 1.12

import Governikus.Global 1.0
import Governikus.Style 1.0

TestCase {
	id: testCase

	name: "test_GButton"
	visible: true
	when: windowShown

	function createTestObject() {
		return createTemporaryQmlObject("import Governikus.Global 1.0; GButton {}", testCase)
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

	function test_textStyle() {
		let testObject = createTestObject()

		compare(testObject.textStyle, Style.text.button, "Initial textStyle: button")

		testObject.textStyle = Style.text.hint_warning
		compare(testObject.textStyle, Style.text.hint_warning, "textStyle: hint_warning")
	}

	function test_text() {
		let testObject = createTestObject()

		compare(testObject.text, "", "Initial empty text")

		testObject.text = "test"
		compare(testObject.text, "test", "text: test")
	}

	function test_icon() {
		let testObject = createTestObject()

		compare(testObject.icon.source, "", "Initial no image")
		testObject.icon.source = "qrc:///images/material_check.svg"
		compare(testObject.icon.source, "qrc:///images/material_check.svg", "Image: qrc:///images/material_check.svg")
	}

	function test_enableButton() {
		let testObject = createTestObject()

		verify(testObject.enableButton, "enableButton: true")
		verify(testObject.enabled, "enabled: true")

		testObject.enableButton = false
		verify(!testObject.enableButton, "enableButton: false")
		verify(testObject.enabled, "enabled: true")
	}

	function test_tooltipText() {
		let testObject = createTestObject()

		compare(testObject.enabledTooltipText, "", "Initial enabledTooltipText: empty")
		compare(testObject.disabledTooltipText, "", "Initial disabledTooltipText: empty")

		testObject.enabledTooltipText = "enabledTooltipText"
		testObject.disabledTooltipText = "disabledTooltipText"
		compare(testObject.enabledTooltipText, "enabledTooltipText", "Initial enabledTooltipText: empty")
		compare(testObject.disabledTooltipText, "disabledTooltipText", "Initial disabledTooltipText: empty")

		testObject.enableButton = false
	}

	GButton {
		id: testObject

		text: "test"
		icon.source: "qrc:///images/material_check.svg"

		TestCase {
			when: windowShown

			function  test_click() {
				verify(!testObject.pressed, "pressed: false")
				mousePress(testObject)
				tryVerify(function(){ return testObject.pressed })
				mouseRelease(testObject)
				tryVerify(function(){ return !testObject.pressed })
			}

			function test_tintIcon() {
				verify(!testObject.tintIcon, "Initial tintIcon: false")

				testObject.tintIcon = true
				verify(testObject.tintIcon, "tintIcon: true")
				testObject.tintIcon = false
				verify(!testObject.tintIcon, "tintIcon: false")
			}

		}
	}

}
