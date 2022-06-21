/*!
 * \brief Model implementation for displaying simple formatted text.
 *
 * \copyright Copyright (c) 2020-2022 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QPair>
#include <QString>
#include <QStringList>

namespace governikus
{

class FormattedTextModel
	: public QAbstractListModel
{
	Q_OBJECT

	public:
		enum LineType
		{
			EMPTY,
			HEADER,
			SECTION,
			SUBSECTION,
			REGULARTEXT,
			LISTITEM
		};
		Q_ENUM(LineType)

		enum FormattedTextRoles
		{
			ContentRole = Qt::UserRole + 1,
			LineTypeRole
		};

		explicit FormattedTextModel(QObject* pParent = nullptr);
		explicit FormattedTextModel(const QStringList& pLines);

		int rowCount(const QModelIndex& pIndex = QModelIndex()) const override;
		QVariant data(const QModelIndex& pIndex, int pRole = Qt::DisplayRole) const override;
		QHash<int, QByteArray> roleNames() const override;

		Q_INVOKABLE bool load(const QString& pFilepath);
		Q_INVOKABLE bool loadSeveral(const QStringList& pFilepaths);

#if defined(QT_NO_DEBUG)

	private:
#endif
		struct ReadLinesResult
		{
			bool mSuccess;
			QStringList mLines;
		};

		static bool isFormattingLine(LineType pType);
		static LineType lineType(const QString& pLine);
		static QString stripFormattingCharacters(const QString& pLine, LineType pType);
		static QString replaceControlCharactersWithHtml(const QString& pLine);
		static ReadLinesResult readLines(const QString& pFilepath);

	private:
		QList<QPair<QString, LineType>> mLines;

		void processLines(const QStringList& pLines);
		void processLine(const QString& pLine);
		bool lastLineIsEmpty() const;
};

} // namespace governikus
