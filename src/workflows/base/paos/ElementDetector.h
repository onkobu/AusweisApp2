/**
 * \brief Example class
 *
 * \copyright Copyright (c) 2014-2022 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include <QByteArray>
#include <QStringList>
#include <QXmlStreamReader>

namespace governikus
{

class ElementDetector
{
	private:
		Q_DISABLE_COPY(ElementDetector)

	protected:
		QXmlStreamReader mReader;
		const QByteArray mXmlData;

		void handleStartElements(const QStringList& pStartElementNames);
		void detectStartElements(const QStringList& pStartElementNames);
		virtual bool handleFoundElement(const QString& pElementName, const QString& pValue, const QXmlStreamAttributes& pAttributes) = 0;

	public:
		explicit ElementDetector(const QByteArray& pXmlData);
		virtual ~ElementDetector();
};

} // namespace governikus
