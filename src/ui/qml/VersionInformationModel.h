/*!
 * \copyright Copyright (c) 2016-2022 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QPair>
#include <QString>

#include "Env.h"

namespace governikus
{

class VersionInformationModel
	: public QAbstractListModel
{
	Q_OBJECT
	friend class Env;

	private:
		enum HistoryRoles
		{
			LABEL = Qt::UserRole + 1,
			TEXT
		};
		QVector<QPair<QString, QString>> mData;

		VersionInformationModel();
		~VersionInformationModel() override = default;

		void init();

	public:
		int rowCount(const QModelIndex& = QModelIndex()) const override;
		QVariant data(const QModelIndex& pIndex, int pRole = Qt::DisplayRole) const override;
		QHash<int, QByteArray> roleNames() const override;
};

} // namespace governikus
