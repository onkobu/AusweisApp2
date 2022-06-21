/*!
 * \copyright Copyright (c) 2014-2022 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include <QObject>
#include <QSettings>
#include <QSharedPointer>
#include <QVariant>

#ifndef QT_NO_DEBUG
#include <QTemporaryDir>
#endif

#define SETTINGS_NAME(_name, _key)\
	QString _name(){\
		return QStringLiteral(_key);\
	}

namespace governikus
{

class AbstractSettings
	: public QObject
{
	Q_OBJECT

	protected:
		AbstractSettings();
		~AbstractSettings() override;

	public:
#ifndef QT_NO_DEBUG
		static QSharedPointer<QTemporaryDir> mTestDir;
#endif

		static QSharedPointer<QSettings> getStore(const QString& pFilename = QString(), QSettings::Format pFormat = QSettings::InvalidFormat);

		virtual void save() = 0;

	Q_SIGNALS:
		void fireSettingsChanged();
};


} // namespace governikus
