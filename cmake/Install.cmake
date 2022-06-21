################################################
# Implements install target!
# It will be included by ./src/CMakeLists.txt
################################################

if(MAC)
	set(DEFAULT_FILE_DESTINATION ${PROJECT_NAME}.app/Contents/Resources)
	set(TRANSLATION_DESTINATION ${DEFAULT_FILE_DESTINATION}/translations)
else()
	set(DEFAULT_FILE_DESTINATION .)
	set(TRANSLATION_DESTINATION translations)
endif()


if(CMAKE_PREFIX_PATH)
	string(REPLACE "\\" "/" TOOLCHAIN_PATH ${CMAKE_PREFIX_PATH})
	set(TOOLCHAIN_BIN_PATH ${TOOLCHAIN_PATH}/bin)
	set(TOOLCHAIN_LIB_PATH ${TOOLCHAIN_PATH}/lib)
endif()


set(SEARCH_ADDITIONAL_DIRS "
			set(CMAKE_MODULE_PATH \"${CMAKE_MODULE_PATH}\")
			include(Helper)
			DIRLIST_OF_FILES(ADDITIONAL_DIRS ${CMAKE_BINARY_DIR}/src/*${CMAKE_SHARED_LIBRARY_SUFFIX})
")

set(DEPENDENCY_CHECK "
	function(gp_resolved_file_type_override file type)
		message(STATUS \"Collect dependency: \${file}\")

		if(file MATCHES \"libstdc.+\.dll\"
			OR file MATCHES \"libwinpthread-.+\.dll\"
			OR file MATCHES \"libgcc_s_.+\.dll\"
			OR file MATCHES \"libssp-.+\.dll\")

			get_filename_component(path \"${CMAKE_CXX_COMPILER}\" DIRECTORY)
			get_filename_component(pathDest \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}\" DIRECTORY)
			if(NOT file MATCHES \"\${path}\" AND NOT file MATCHES \"\${pathDest}\")
				message(FATAL_ERROR \"Wrong source path detected: \${file} | Should be: \${path} or \${pathDest}\")
			endif()

		endif()
	endfunction()
")


# qt qml plugins (fixup_bundle needs to know this to fetch their dependencies)
if((WIN32 OR MAC) AND TARGET ${Qt}::Qml)
	set(modules QtQuick QtQuick.2 QtQml QtGraphicalEffects Qt)
	if(WIN32)
		list(APPEND modules QtWinExtras)
	endif()

	foreach(entry ${modules})
		set(_lib_dir ${QT_HOST_PREFIX}/qml/${entry})
		file(GLOB_RECURSE _libs "${_lib_dir}/*.dll" "${_lib_dir}/*.dylib")
		list(APPEND QML_LIBS ${_libs})
		install(DIRECTORY ${_lib_dir} DESTINATION ${DEFAULT_FILE_DESTINATION}/qml COMPONENT Runtime PATTERN "*.dylib" EXCLUDE)
	endforeach()
endif()


if(WIN32)
	if(MSVC OR CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
		set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .)
		set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
		include(InstallRequiredSystemLibraries)
	endif()

	if(TARGET ${Qt}::Qml)
		FETCH_TARGET_LOCATION(libQuickControls2 "${Qt}::QuickControls2")
		install(FILES ${libQuickControls2} DESTINATION . COMPONENT Runtime)
		list(APPEND QML_LIBS ${libQuickControls2})
		if(TARGET ${Qt}::QmlWorkerScript)
			FETCH_TARGET_LOCATION(libQmlWorkerScript "${Qt}::QmlWorkerScript")
			install(FILES ${libQmlWorkerScript} DESTINATION . COMPONENT Runtime)
		endif()

		if(TARGET ${Qt}::Gui_EGL AND TARGET ${Qt}::Gui_GLESv2)
			FETCH_TARGET_LOCATION(libEgl "${Qt}::Gui_EGL")
			FETCH_TARGET_LOCATION(libGLES "${Qt}::Gui_GLESv2")
			install(FILES ${libEgl} DESTINATION . COMPONENT Runtime)
			install(FILES ${libGLES} DESTINATION . COMPONENT Runtime)
		endif()
	endif()
	FETCH_TARGET_LOCATION(libSvg "${Qt}::Svg")
	FETCH_TARGET_LOCATION(pluginSvg "${Qt}::QSvgPlugin")
	FETCH_TARGET_LOCATION(pluginGif "${Qt}::QGifPlugin")
	FETCH_TARGET_LOCATION(pluginJpeg "${Qt}::QJpegPlugin")
	if(WINDOWS_STORE)
		FETCH_TARGET_LOCATION(platformWin "${Qt}::QWinRTIntegrationPlugin")
	else()
		FETCH_TARGET_LOCATION(platformWin "${Qt}::QWindowsIntegrationPlugin")
	endif()
	FETCH_TARGET_LOCATION(styleVista "${Qt}::QWindowsVistaStylePlugin")

	install(TARGETS AusweisApp DESTINATION . COMPONENT Application)
	install(FILES ${libSvg} DESTINATION . COMPONENT Runtime)
	install(FILES ${pluginSvg} DESTINATION imageformats COMPONENT Runtime)
	install(FILES ${pluginGif} DESTINATION imageformats COMPONENT Runtime)
	install(FILES ${pluginJpeg} DESTINATION imageformats COMPONENT Runtime)
	install(FILES ${platformWin} DESTINATION platforms COMPONENT Runtime)
	install(FILES ${styleVista} DESTINATION styles COMPONENT Runtime)

	install(CODE
		"
		${DEPENDENCY_CHECK}
		${SEARCH_ADDITIONAL_DIRS}
		include(BundleUtilities)
		FIXUP_BUNDLE(\"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}\" \"${QML_LIBS}\" \"${TOOLCHAIN_BIN_PATH};\${ADDITIONAL_DIRS}\")
		" COMPONENT Runtime)



elseif(MAC)
	set(MACOS_BUNDLE_MACOS_DIR ${DEFAULT_FILE_DESTINATION}/../MacOS)
	set(MACOS_BUNDLE_PLUGINS_DIR ${DEFAULT_FILE_DESTINATION}/../PlugIns)
	set(MACOS_BUNDLE_FRAMEWORKS_DIR ${DEFAULT_FILE_DESTINATION}/../Frameworks)
	set(MACOS_BUNDLE_RESOURCES_DIR ${DEFAULT_FILE_DESTINATION}/../Resources)
	set(MACOS_BUNDLE_LOGIN_ITEMS_DIR ${DEFAULT_FILE_DESTINATION}/../Library/LoginItems)

	# We need to include the following (i.e. all) image format plug-ins,
	# since those seem to be loaded upon program start-up. Not including
	# them would cause the respective add-on from a Qt installation (if
	# any) to be loaded, which would in turn cause the Qt libraries they
	# depend on to be loaded as well, thus resulting in two sets of Qt
	# libraries being loaded (ours from the bundle and the ones from the
	# installation) and the program misbehaving (crashing).
	foreach (qtComponent QtCore ${Qt}Gui ${Qt}Network ${Qt}Svg ${Qt}Widgets)
		foreach(plugin ${${qtComponent}_PLUGINS})
			get_target_property(pluginPath ${plugin} LOCATION)
			get_filename_component(pluginDir ${pluginPath} DIRECTORY)
			get_filename_component(pluginName ${pluginPath} NAME)
			get_filename_component(pluginDirName ${pluginDir} NAME)

			if(pluginDirName STREQUAL "platforms" AND NOT plugin STREQUAL "${Qt}::QCocoaIntegrationPlugin")
				continue()
			endif()

			install(FILES ${pluginPath} DESTINATION ${MACOS_BUNDLE_PLUGINS_DIR}/${pluginDirName} COMPONENT Runtime)
		endforeach()
	endforeach()

	if(TARGET ${Qt}::Qml)
		foreach(entry QtQuick QtQuick.2 QtQml QtGraphicalEffects Qt)
			set(_dir "${QT_HOST_PREFIX}/qml")
			file(GLOB_RECURSE DYLIB "${_dir}/${entry}/*.dylib")
			foreach(_lib ${DYLIB})
				file(RELATIVE_PATH _lib_dest "${_dir}" "${_lib}")
				if(NOT _lib_dest MATCHES "XmlListModel|Particles.2|LocalStorage|Shapes") # not needed stuff
					get_filename_component(_lib_dest_dir ${_lib_dest} DIRECTORY)
					install(FILES ${_lib} DESTINATION ${MACOS_BUNDLE_RESOURCES_DIR}/qml/${_lib_dest_dir} COMPONENT Runtime)
				endif()
			endforeach()
		endforeach()
	endif()

	install(TARGETS AusweisApp DESTINATION ${MACOS_BUNDLE_MACOS_DIR} COMPONENT Application)

	install(TARGETS AusweisApp2AutostartHelper DESTINATION ${MACOS_BUNDLE_LOGIN_ITEMS_DIR} COMPONENT Application)

	install(FILES ${PACKAGING_DIR}/macos/container-migration.plist DESTINATION ${MACOS_BUNDLE_RESOURCES_DIR} COMPONENT Application)
	install(FILES ${RESOURCES_DIR}/images/macos/bundle_icons.icns DESTINATION ${MACOS_BUNDLE_RESOURCES_DIR} RENAME ${PROJECT_NAME}.icns COMPONENT Application)
	install(FILES ${CMAKE_BINARY_DIR}/Info.plist DESTINATION ${DEFAULT_FILE_DESTINATION}/.. COMPONENT Application)

	install(CODE
		"
		${DEPENDENCY_CHECK}
		${SEARCH_ADDITIONAL_DIRS}
		file(GLOB_RECURSE QTPLUGINS \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${MACOS_BUNDLE_PLUGINS_DIR}/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
		file(GLOB_RECURSE QtQuickPLUGINS \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${MACOS_BUNDLE_RESOURCES_DIR}/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
		include(BundleUtilities)
		FIXUP_BUNDLE(\"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${MACOS_BUNDLE_MACOS_DIR}/${PROJECT_NAME}\" \"\${QTPLUGINS};\${QtQuickPLUGINS}\" \"${TOOLCHAIN_LIB_PATH};\${ADDITIONAL_DIRS}\")
		" COMPONENT Runtime)

elseif(IOS)


elseif(ANDROID)
	set(ANDROID_PACKAGE_SRC_DIR ${PROJECT_BINARY_DIR}/package-src-dir)
	set(ANDROID_DEST libs/${CMAKE_ANDROID_ARCH_ABI})
	set(PERMISSIONS PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
	install(TARGETS AusweisApp DESTINATION ${ANDROID_DEST} ${PERMISSIONS} COMPONENT Application)

	set(RESOURCES_IMG_ANDROID_DIR ${RESOURCES_DIR}/images/android)
	if(BUILD_PREVIEW)
		set(ANDROID_LAUNCHER_ICON "npa_preview.png")
		set(ANDROID_SPLASH_SCREEN_ICON_NAME "splash_npa_preview.png")
	elseif(IS_DEVELOPER_VERSION)
		set(ANDROID_LAUNCHER_ICON "npa_beta.png")
		set(ANDROID_SPLASH_SCREEN_ICON_NAME "splash_npa_beta.png")
	else()
		set(ANDROID_LAUNCHER_ICON "npa.png")
		set(ANDROID_SPLASH_SCREEN_ICON_NAME "splash_npa.png")
	endif()

	if(INTEGRATED_SDK)
		set(ANDROID_MANIFEST AndroidManifest.xml.aar.in)
		foreach(entry network/WifiInfo ui/aidl/AidlBinder android/AusweisApp2Service)
			set(_java_file "${SRC_DIR}/${entry}.java")
			if(NOT EXISTS "${_java_file}")
				message(FATAL_ERROR "Cannot find file: ${_java_file}")
			endif()
			list(APPEND JAVA_FILES "${_java_file}")
		endforeach()

		install(FILES ${PACKAGING_DIR}/android/res/values/strings.xml DESTINATION ${ANDROID_PACKAGE_SRC_DIR}/res/values COMPONENT Runtime)
	else()
		set(ANDROID_MANIFEST AndroidManifest.xml.apk.in)

		foreach(entry ldpi mdpi hdpi xhdpi xxhdpi xxxhdpi)
			install(FILES ${RESOURCES_IMG_ANDROID_DIR}/${entry}/background_npa.png DESTINATION ${ANDROID_PACKAGE_SRC_DIR}/res/mipmap-${entry} COMPONENT Runtime RENAME npa_background.png)
			install(FILES ${RESOURCES_IMG_ANDROID_DIR}/${entry}/foreground_${ANDROID_LAUNCHER_ICON} DESTINATION ${ANDROID_PACKAGE_SRC_DIR}/res/mipmap-${entry} COMPONENT Runtime RENAME npa_foreground.png)
			install(FILES ${RESOURCES_IMG_ANDROID_DIR}/${entry}/${ANDROID_LAUNCHER_ICON} DESTINATION ${ANDROID_PACKAGE_SRC_DIR}/res/mipmap-${entry} COMPONENT Runtime RENAME npa.png)
			install(FILES ${RESOURCES_IMG_ANDROID_DIR}/${entry}/${ANDROID_SPLASH_SCREEN_ICON_NAME} DESTINATION ${ANDROID_PACKAGE_SRC_DIR}/res/drawable-${entry} COMPONENT Runtime RENAME splash_npa.png)
		endforeach()

		install(DIRECTORY ${PACKAGING_DIR}/android/res DESTINATION ${ANDROID_PACKAGE_SRC_DIR} COMPONENT Runtime)

		file(GLOB_RECURSE JAVA_FILES "${SRC_DIR}/*.java")
	endif()

	install(FILES ${JAVA_FILES} DESTINATION ${ANDROID_PACKAGE_SRC_DIR}/src COMPONENT Runtime)
	install(FILES ${PACKAGING_DIR}/android/IAusweisApp2Sdk.aidl DESTINATION ${ANDROID_PACKAGE_SRC_DIR}/src/com/governikus/ausweisapp2/ COMPONENT Runtime)
	install(FILES ${PACKAGING_DIR}/android/IAusweisApp2SdkCallback.aidl DESTINATION ${ANDROID_PACKAGE_SRC_DIR}/src/com/governikus/ausweisapp2/ COMPONENT Runtime)

	if(VERSION_DVCS)
		set(ANDROID_VERSION_NAME ${VERSION_DVCS})
	else()
		set(ANDROID_VERSION_NAME ${PROJECT_VERSION})
	endif()
	configure_file(${PACKAGING_DIR}/android/${ANDROID_MANIFEST} ${ANDROID_PACKAGE_SRC_DIR}/AndroidManifest.xml @ONLY)
	configure_file(${PACKAGING_DIR}/android/backup_rules.xml ${ANDROID_PACKAGE_SRC_DIR}/res/xml/backup_rules.xml COPYONLY)
	if(NOT INTEGRATED_SDK)
		configure_file(${PACKAGING_DIR}/android/fileprovider.xml ${ANDROID_PACKAGE_SRC_DIR}/res/xml/fileprovider.xml COPYONLY)
	endif()

	set(ANDROID_SO_NAME libAusweisApp2_${CMAKE_ANDROID_ARCH_ABI}.so)
	set(ANDROID_APP_BINARY "${CMAKE_INSTALL_PREFIX}/${ANDROID_DEST}/${ANDROID_SO_NAME}")
	set(SYMBOL_FOLDER "${CMAKE_BINARY_DIR}/debug.symbols")
	set(ANDROID_APP_SYMBOLS "${SYMBOL_FOLDER}/${ANDROID_SO_NAME}")

	install(CODE
		"
		execute_process(COMMAND \"${CMAKE_COMMAND}\" -E make_directory \"${SYMBOL_FOLDER}\")
		execute_process(COMMAND \"${CMAKE_OBJCOPY}\" \"--only-keep-debug\" \"${ANDROID_APP_BINARY}\" \"${ANDROID_APP_SYMBOLS}\")
		" COMPONENT Runtime)

	set(ANDROID_DEPLOYMENT_SETTINGS ${PROJECT_BINARY_DIR}/libAusweisApp2.so-deployment-settings.json CACHE INTERNAL "apk deployment" FORCE)
	configure_file(${PACKAGING_DIR}/android/libAusweisApp2.so-deployment-settings.json.in ${ANDROID_DEPLOYMENT_SETTINGS} @ONLY)
	configure_file(${PACKAGING_DIR}/android/gradle.properties ${CMAKE_INSTALL_PREFIX}/gradle.properties COPYONLY)

	set(TRANSLATION_DESTINATION ${ANDROID_PACKAGE_SRC_DIR}/assets/${TRANSLATION_DESTINATION})
	set(DEFAULT_FILE_DESTINATION ${ANDROID_PACKAGE_SRC_DIR}/assets)

elseif(UNIX)
	if(BUILD_SHARED_LIBS)
		set(CMAKE_INSTALL_RPATH "\$ORIGIN")
	endif()

	set(DEFAULT_FILE_DESTINATION ${CMAKE_INSTALL_DATADIR}/${VENDOR}/AusweisApp2)
	set(TRANSLATION_DESTINATION ${DEFAULT_FILE_DESTINATION}/${TRANSLATION_DESTINATION})
	install(TARGETS AusweisApp DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Application)
	configure_file(${PACKAGING_DIR}/linux/${BUNDLE_IDENTIFIER}.metainfo.xml.in ${CMAKE_CURRENT_BINARY_DIR}/${BUNDLE_IDENTIFIER}.metainfo.xml @ONLY)
	configure_file(${PACKAGING_DIR}/linux/${BUNDLE_IDENTIFIER}.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/${BUNDLE_IDENTIFIER}.desktop @ONLY)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BUNDLE_IDENTIFIER}.metainfo.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo COMPONENT Application)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BUNDLE_IDENTIFIER}.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications COMPONENT Application)
	install(FILES ${RESOURCES_DIR}/images/npa.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps COMPONENT Application RENAME AusweisApp2.svg)
	install(FILES ${RESOURCES_DIR}/images/npa.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/96x96/apps COMPONENT Application RENAME AusweisApp2.png)
	install(FILES ${DOCS_DIR}/AusweisApp2.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 COMPONENT Application)
endif()




if(LINUX OR WIN32 OR MAC)
	option(SELFPACKER "Compress executable with self packer like UPX")
	if(SELFPACKER)
		find_package(SelfPackers)
		if(SELF_PACKER_FOR_EXECUTABLE)
			message(STATUS "Using SelfPacker: ${SELF_PACKER_FOR_EXECUTABLE} ${SELF_PACKER_FOR_EXECUTABLE_FLAGS}")
		else()
			message(FATAL_ERROR "Cannot find self packer")
		endif()

		install(CODE
			"
			execute_process(COMMAND
				${SELF_PACKER_FOR_EXECUTABLE} ${SELF_PACKER_FOR_EXECUTABLE_FLAGS} \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${DEFAULT_FILE_DESTINATION}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}\")
			" COMPONENT Application)
	endif()
endif()


if(LINUX OR BSD)
	install(FILES ${QM_FILES} DESTINATION ${TRANSLATION_DESTINATION} COMPONENT Translations)
elseif(NOT INTEGRATED_SDK)
	install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/translations/ DESTINATION ${TRANSLATION_DESTINATION} COMPONENT Translations)
endif()

if(NOT INTEGRATED_SDK)
	# resources file
	install(FILES ${RCC} DESTINATION ${DEFAULT_FILE_DESTINATION} COMPONENT Runtime)
endif()

# secure storage file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/config.json DESTINATION ${DEFAULT_FILE_DESTINATION} COMPONENT Runtime)


if(SIGNTOOL_CMD)
	if(IOS)
		set(SIGNTOOL_PARAMS "-s \"${XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY}\"")
	endif()

	configure_file(${CMAKE_DIR}/SignFiles.cmake.in ${CMAKE_BINARY_DIR}/SignFiles.cmake @ONLY)

	if(MAC)
		option(OSX_TIMESTAMP "Timestamp the application bundle" ON)

		install(CODE
			"
			execute_process(COMMAND \"${CMAKE_COMMAND}\" -DOSX_TIMESTAMP=${OSX_TIMESTAMP} -P \"${CMAKE_BINARY_DIR}/SignFiles.cmake\" WORKING_DIRECTORY \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}\")
			" COMPONENT Application)
	elseif(WIN32)
		install(CODE
			"
			execute_process(COMMAND \"${CMAKE_COMMAND}\" -DSIGN_EXT=*.exe -P \"${CMAKE_BINARY_DIR}/SignFiles.cmake\" WORKING_DIRECTORY \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${DEFAULT_FILE_DESTINATION}\")
			" COMPONENT Application)
	endif()
endif()
