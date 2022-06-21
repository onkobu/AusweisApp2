/*!
 * \copyright Copyright (c) 2017-2022 Governikus GmbH & Co. KG, Germany
 */

#include "ReaderDriverModel.h"

#include "Env.h"
#include "MockReaderConfiguration.h"
#include "MockReaderDetector.h"
#include "ReaderManager.h"
#include "ResourceLoader.h"

#include <QtTest>


using namespace governikus;


class MockReaderManager
	: public ReaderManager
{
	Q_OBJECT

	private:
		const QVector<ReaderInfo>& mReaderInfos;

	public:
		MockReaderManager(const QVector<ReaderInfo>& pReaderInfos);
		QVector<ReaderInfo> getReaderInfos(const ReaderFilter& pFilter = ReaderFilter()) const override;
};


MockReaderManager::MockReaderManager(const QVector<ReaderInfo>& pReaderInfos) :
	mReaderInfos(pReaderInfos)
{
}


QVector<ReaderInfo> MockReaderManager::getReaderInfos(const ReaderFilter& pFilter) const
{
	Q_UNUSED(pFilter)
	return mReaderInfos;
}


class test_ReaderDriverModel
	: public QObject
{
	Q_OBJECT

	public:
		test_ReaderDriverModel()
			: mMockReaderConfiguration()
			, mUsbIds()
			, mMockReaderDetector(mUsbIds)
			, mReaderInfos()
			, mMockReaderManager(mReaderInfos)
		{
			Env::set(ReaderDetector::staticMetaObject, &mMockReaderDetector);
			Env::set(ReaderManager::staticMetaObject, &mMockReaderManager);
		}

	private:
		QScopedPointer<MockReaderConfiguration> mMockReaderConfiguration;

		QVector<UsbId> mUsbIds;
		MockReaderDetector mMockReaderDetector;

		QVector<ReaderInfo> mReaderInfos;
		MockReaderManager mMockReaderManager;

	private Q_SLOTS:
		void initTestCase()
		{
			ResourceLoader::getInstance().init();
		}


		void init()
		{
			mMockReaderConfiguration.reset(new MockReaderConfiguration());
			Env::set(ReaderConfiguration::staticMetaObject, mMockReaderConfiguration.data());

			mUsbIds.clear();
			mReaderInfos.clear();
		}


		void cleanup()
		{
			mMockReaderConfiguration.reset();
		}


		void test_empty()
		{
			mMockReaderConfiguration->clearReaderConfiguration();

			ReaderDriverModel readerDriverModel;
			QCOMPARE(readerDriverModel.rowCount(), 0);
		}


		void test_01_settings()
		{
			ReaderDriverModel readerDriverModel;
			QCOMPARE(readerDriverModel.rowCount(), 0);
		}


		void test_02_usbId()
		{
#if defined(Q_OS_FREEBSD)
			QSKIP("No driver available for UsbId(0x0C4B, 0x0501) on FreeBSD");
#endif

			mUsbIds += UsbId(0x0C4B, 0x0501);    // REINER SCT cyberJack RFID komfort

			ReaderDriverModel readerDriverModel;
			QCOMPARE(readerDriverModel.rowCount(), 1);
			const auto& index = readerDriverModel.index(0, ReaderDriverModel::ColumnId::ReaderStatus, QModelIndex());
			QCOMPARE(readerDriverModel.data(index).toString(), tr("No driver installed"));
		}


		void test_03_usbId_unknown()
		{
			mUsbIds += UsbId(0x1, 0x2);    // Unknown

			ReaderDriverModel readerDriverModel;
			QCOMPARE(readerDriverModel.rowCount(), 0);
		}


		void test_04_usbId_readerManager_equal()
		{
#if defined(Q_OS_FREEBSD)
			QSKIP("No driver available for UsbId(0x04E6, 0x5790) on FreeBSD");
#endif
			mUsbIds += UsbId(0x04E6, 0x5790);    // Identiv Cloud 3700 F
			mReaderInfos += ReaderInfo("Identiv CLOUD 3700 F Contactless Reader 0", ReaderManagerPlugInType::PCSC);

			ReaderDriverModel readerDriverModel;
			QCOMPARE(readerDriverModel.rowCount(), 1);
			const auto& index = readerDriverModel.index(0, ReaderDriverModel::ColumnId::ReaderStatus, QModelIndex());
			QCOMPARE(readerDriverModel.data(index).toString(), tr("Driver installed"));
		}


		void test_05_usbId_readerManager_different()
		{
#if defined(Q_OS_FREEBSD)
			QSKIP("No driver available for UsbId(0x04E6, 0x5790) on FreeBSD");
#endif
			mUsbIds += UsbId(0x0C4B, 0x0501);    // REINER SCT cyberJack RFID komfort
			mUsbIds += UsbId(0x04E6, 0x5790);    // Identiv Cloud 3700 F
			mReaderInfos += ReaderInfo("Identiv CLOUD 3700 F Contactless Reader 0", ReaderManagerPlugInType::PCSC);

			QModelIndex index;
			ReaderDriverModel readerDriverModel;
			QCOMPARE(readerDriverModel.rowCount(), 2);
			index = readerDriverModel.index(0, ReaderDriverModel::ColumnId::ReaderStatus, QModelIndex());
			QCOMPARE(readerDriverModel.data(index).toString(), tr("Driver installed"));
			index = readerDriverModel.index(1, ReaderDriverModel::ColumnId::ReaderStatus, QModelIndex());
			QCOMPARE(readerDriverModel.data(index).toString(), tr("No driver installed"));
		}


		void test_06_settings_readerManager_unknown()
		{
			mReaderInfos += ReaderInfo("Governikus Special Reader", ReaderManagerPlugInType::PCSC);

			ReaderDriverModel readerDriverModel;
			QCOMPARE(readerDriverModel.rowCount(), 1);
		}


};


QTEST_GUILESS_MAIN(test_ReaderDriverModel)
#include "test_ReaderDriverModel.moc"
