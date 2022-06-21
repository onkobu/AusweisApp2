/*
 * \copyright Copyright (c) 2016-2022 Governikus GmbH & Co. KG, Germany
 */

import QtTest 1.12

import Governikus.Global 1.0

TestCase {
	id: testCase

	name: "test_Utils"
	visible: true
	when: windowShown

	function test_escapeHtml() {
		compare(Utils.escapeHtml("a&b"), "a&amp;b", "escape &")
		compare(Utils.escapeHtml("<br/>"), "&lt;br/&gt;", "escape < and >")
		compare(Utils.escapeHtml("\"Hello\""), "&quot;Hello&quot;", "escape \"")
	}

	function test_isToday()
	{
		let today = new Date;
		compare(Utils.isToday(today), true)

		let yesterday = new Date;
		yesterday.setDate(yesterday.getDate() - 1);
		compare(Utils.isToday(yesterday), false)
	}

	function test_isYesterday(date)
	{
		let today = new Date;
		compare(Utils.isYesterday(today), false)

		let yesterday = new Date;
		yesterday.setDate(yesterday.getDate() - 1);
		compare(Utils.isYesterday(yesterday), true)
	}

	function test_isThisWeek(date)
	{
		let today = new Date;
		compare(Utils.isThisWeek(today), true)

		let lastWeek = new Date;
		lastWeek.setDate(lastWeek.getDate() - 10);
		compare(Utils.isThisWeek(lastWeek), false)
	}

	function test_isSameDate()
	{
		let today = new Date;
		compare(Utils.isSameDate(today, today), true)

		let yesterday = new Date;
		yesterday.setDate(yesterday.getDate() - 1);
		compare(Utils.isSameDate(today, yesterday), false)
	}

	function test_shuffle() {
		let numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 0]
		let shuffledNumbers = Utils.shuffle([1, 2, 3, 4, 5, 6, 7, 8, 9, 0])

		compare(shuffledNumbers.count, numbers.count)

		let isSameOrder = numbers.toString() == shuffledNumbers.toString()
		compare(isSameOrder, false, "Shuffled arrays have same order")
	}

	function test_helpTopicOf() {
		let defaultHelpTopic = "defaultHelp"

		let componentWithoutHelp = createTemporaryQmlObject("import QtQuick 2.12; Item {}", testCase)
		let componentWithHelp = createTemporaryQmlObject("import QtQuick 2.12; Item {property string helpTopic: \"dummyHelp\";}", testCase)

		compare(Utils.helpTopicOf(null, defaultHelpTopic), defaultHelpTopic, "Get default help topic if component is null")
		compare(Utils.helpTopicOf(componentWithoutHelp, defaultHelpTopic), defaultHelpTopic, "Get default help topic if component got no helpTopic")
		compare(Utils.helpTopicOf(componentWithHelp, defaultHelpTopic), "dummyHelp", "Get component help topic")
	}
}
