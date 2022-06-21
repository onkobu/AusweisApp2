/*
 * \copyright Copyright (c) 2020-2022 Governikus GmbH & Co. KG, Germany
 */

#include "FormattedTextModel.h"

#include <QFile>
#include <QLoggingCategory>
#include <QRegularExpression>
#include <QTextStream>

using namespace governikus;

Q_DECLARE_LOGGING_CATEGORY(qml)


FormattedTextModel::FormattedTextModel(QObject* pParent)
	: QAbstractListModel(pParent)
{
}


FormattedTextModel::FormattedTextModel(const QStringList& pLines)
{
	processLines(pLines);
}


int FormattedTextModel::rowCount(const QModelIndex& pIndex) const
{
	Q_UNUSED(pIndex)
	return mLines.size();
}


QVariant FormattedTextModel::data(const QModelIndex& pIndex, int pRole) const
{
	const auto row = pIndex.row();
	if (!pIndex.isValid() || row >= rowCount())
	{
		return QVariant();
	}

	const auto& line = mLines.at(row);
	switch (pRole)
	{
		case ContentRole:
			return line.first;

		case LineTypeRole:
			return line.second;

		default:
			return QVariant();
	}
}


QHash<int, QByteArray> FormattedTextModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles.insert(ContentRole, QByteArrayLiteral("content"));
	roles.insert(LineTypeRole, QByteArrayLiteral("lineType"));
	return roles;
}


bool FormattedTextModel::load(const QString& pFilepath)
{
	const auto [success, lines] = readLines(pFilepath);

	processLines(lines);

	return success;
}


bool FormattedTextModel::loadSeveral(const QStringList& pFilepaths)
{
	bool allReadSuccessfully = true;
	QStringList allLines;
	for (const auto& filepath : pFilepaths)
	{
		const auto [success, lines] = readLines(filepath);
		if (!success)
		{
			allReadSuccessfully = false;
			continue;
		}

		if (!allLines.isEmpty())
		{
			allLines << QString();
		}
		allLines += lines;
	}

	processLines(allLines);

	return allReadSuccessfully;
}


bool FormattedTextModel::isFormattingLine(FormattedTextModel::LineType pType)
{
	static const QList<LineType> formattingLineTypes({HEADER, SECTION, SUBSECTION});
	return formattingLineTypes.contains(pType);
}


FormattedTextModel::LineType FormattedTextModel::lineType(const QString& pLine)
{
	const auto trimmedLine = pLine.trimmed();
	if (trimmedLine.isEmpty())
	{
		return EMPTY;
	}
	if (pLine.count(QLatin1Char('*')) == trimmedLine.size())
	{
		return HEADER;
	}
	if (pLine.count(QLatin1Char('=')) == trimmedLine.size())
	{
		return SECTION;
	}
	if (pLine.count(QLatin1Char('-')) == trimmedLine.size())
	{
		return SUBSECTION;
	}

	const QRegularExpression reListElement(QStringLiteral("^\\s*\\*{1}\\s?\\w+"));
	if (reListElement.match(trimmedLine).hasMatch())
	{
		return LISTITEM;
	}

	return REGULARTEXT;

}


QString FormattedTextModel::stripFormattingCharacters(const QString& pLine, LineType pType)
{
	QString workingCopy = pLine;
	if (pType == LISTITEM)
	{
		workingCopy.remove(QRegularExpression(QStringLiteral("^\\s*\\*{1}\\s?")));
	}

	return workingCopy;
}


QString FormattedTextModel::replaceControlCharactersWithHtml(const QString& pLine)
{
	QString htmlString = pLine;

	const QRegularExpression reBold(QStringLiteral("\\*{2}([0-9a-zA-Z,;.:-_\\s]+)\\*{2}"));
	htmlString.replace(reBold, QStringLiteral("<b>\\1</b>"));

	return htmlString;
}


FormattedTextModel::ReadLinesResult FormattedTextModel::readLines(const QString& pFilepath)
{
	QFile file(pFilepath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qCWarning(qml) << "Could not load" << pFilepath << file.errorString();
		return {false, QStringList()};
	}

	QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	in.setCodec("UTF-8");
#endif

	QStringList lines;
	while (!in.atEnd())
	{
		lines << in.readLine();
	}

	file.close();

	return {true, lines};
}


void FormattedTextModel::processLines(const QStringList& pLines)
{
	beginResetModel();
	mLines.clear();

	for (const auto& line : pLines)
	{
		processLine(line);
	}

	endResetModel();
}


void FormattedTextModel::processLine(const QString& pLine)
{
	const auto type = lineType(pLine);
	const auto strippedLine = stripFormattingCharacters(pLine, type);
	const auto htmlLine = replaceControlCharactersWithHtml(strippedLine);

	if ((lastLineIsEmpty() && !isFormattingLine(type)) || type == EMPTY)
	{
		mLines << qMakePair(htmlLine, type);
		return;
	}

	Q_ASSERT(mLines.size() > 0);
	if (isFormattingLine(type))
	{
		mLines.last().second = type;
	}
	else
	{
		mLines.last().first += QLatin1Char(' ') + htmlLine.trimmed();
	}
}


bool FormattedTextModel::lastLineIsEmpty() const
{
	return mLines.isEmpty() || mLines.last().second == EMPTY;
}
