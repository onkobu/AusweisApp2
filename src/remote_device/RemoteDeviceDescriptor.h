/*!
 * \brief Information needed to display a remote device in the GUI and to open
 * a websocket connection to it.
 *
 * \copyright Copyright (c) 2017-2022 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "messages/Discovery.h"

#include <QHostAddress>
#include <QSharedDataPointer>
#include <QUrl>
#include <QVector>


namespace governikus
{

class RemoteDeviceDescriptor
{
	private:
		class RemoteDeviceDescriptorData
			: public QSharedData
		{
			public:
				RemoteDeviceDescriptorData(const QString& pIfdName,
						const QString& pIfdId,
						const QVector<IfdVersion::Version>& pApiVersions,
						const QUrl& pUrl);

				virtual ~RemoteDeviceDescriptorData();

				const QString mIfdName;
				const QString mIfdId;
				const QVector<IfdVersion::Version> mApiVersions;
				const QUrl mUrl;
				bool operator==(const RemoteDeviceDescriptorData& pOther) const;
				bool isSameIfd(const RemoteDeviceDescriptorData& pOther) const;
		};

		static RemoteDeviceDescriptorData* createRemoteDeviceDescriptorData(const Discovery& pDiscovery,
				const QHostAddress& pHostAddress);

		QSharedDataPointer<RemoteDeviceDescriptorData> d;

	public:
		RemoteDeviceDescriptor() = default;
		RemoteDeviceDescriptor(const Discovery& pDiscovery, const QHostAddress& pHostAddress);

		~RemoteDeviceDescriptor() = default;

		[[nodiscard]] const QString& getIfdName() const;
		[[nodiscard]] const QString& getIfdId() const;
		[[nodiscard]] const QVector<IfdVersion::Version>& getApiVersions() const;
		[[nodiscard]] bool isSupported() const;
		[[nodiscard]] const QUrl& getUrl() const;
		[[nodiscard]] bool isNull() const;

		bool operator==(const RemoteDeviceDescriptor& pOther) const;
		[[nodiscard]] bool isSameIfd(const RemoteDeviceDescriptor& pOther) const;

};


inline QDebug operator<<(QDebug pDbg, const RemoteDeviceDescriptor& pRemoteDeviceDescriptor)
{
	QDebugStateSaver saver(pDbg);
	return pDbg.noquote().nospace() << "RemoteDevice(" << pRemoteDeviceDescriptor.getIfdName() << ", " <<
		   pRemoteDeviceDescriptor.getIfdId() << ", " <<
		   pRemoteDeviceDescriptor.getUrl() << ", " <<
		   pRemoteDeviceDescriptor.getApiVersions() << ")";
}


} // namespace governikus
