/*
 * \copyright Copyright (c) 2014-2022 Governikus GmbH & Co. KG, Germany
 */

#include "ResourceLoader.h"

#include "FileDestination.h"
#include "SingletonHelper.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QResource>

using namespace governikus;

defineSingleton(ResourceLoader)

ResourceLoader::ResourceLoader()
	: mFilenames(
		{
			QStringLiteral("AusweisApp2.rcc")
		})
	, mLoadedResources()
{
}


ResourceLoader::~ResourceLoader()
{
	shutdown();
}


void ResourceLoader::init()
{
	for (const auto& file : mFilenames)
	{
		QString path = FileDestination::getPath(file);

#ifndef QT_NO_DEBUG
		if (const auto& filePathFromTestDir = QCoreApplication::applicationDirPath() + QStringLiteral("/../../src/") + file;
				!QFile::exists(path) && QFile::exists(filePathFromTestDir))
		{
			path = filePathFromTestDir;
		}
#endif

		const auto loaded = QResource::registerResource(path);
		qDebug() << "Register resource:" << path << '|' << loaded;
		if (loaded)
		{
			mLoadedResources << path;
		}
	}
}


void ResourceLoader::shutdown()
{
	for (const auto& path : qAsConst(mLoadedResources))
	{
		const auto result = QResource::unregisterResource(path);
		qDebug() << "Unregister resource:" << path << '|' << result;
	}
	mLoadedResources.clear();
}


bool ResourceLoader::isLoaded() const
{
	return !mLoadedResources.isEmpty();
}
