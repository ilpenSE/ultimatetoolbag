cmake_minimum_required(VERSION 3.16)

project(UltimateToolbag VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Network Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Network Svg)

set(TS_FILES
    assets/locale/en_US.ts
    assets/locale/tr_TR.ts
    assets/locale/de_DE.ts
    assets/locale/fr_FR.ts
    assets/locale/it_IT.ts
    assets/locale/es_ES.ts
    assets/locale/ru_RU.ts)

set(ICONS
    assets/icons/account.svg
    assets/icons/add.svg
    assets/icons/back.svg
    assets/icons/cancel.svg
    assets/icons/delete.svg
    assets/icons/edit.svg
    assets/icons/fav.svg
    assets/icons/faved.svg
    assets/icons/info.svg
    assets/icons/save.svg
    assets/icons/select.svg
    assets/icons/settings.svg
)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        skeleton/languagemanager.h skeleton/languagemanager.cpp
        skeleton/fileorganizer.h skeleton/fileorganizer.cpp
        etc/consts.h etc/enums.h skeleton/versionmanager.h
        etc/apptranslations.h
        skeleton/logger.cpp skeleton/logger.h skeleton/loggerstream.h
        skeleton/settingsmanager.h skeleton/settingsmanager.cpp
        skeleton/thememanager.h skeleton/thememanager.cpp
        skeleton/iconutils.h skeleton/iconutils.cpp
        lib/imagerepair.h lib/imagerepair.cpp
        skeleton/assetvalidator.h skeleton/assetvalidator.cpp
        skeleton/groupmanager.h skeleton/groupmanager.cpp
        etc/instances.h
        skeleton/favoritesmanager.h skeleton/favoritesmanager.cpp
        skeleton/toastmanager.h skeleton/toastmanager.cpp
        aboutdialog.h aboutdialog.cpp aboutdialog.ui
        settingswin.h settingswin.cpp settingswin.ui
        lib/generictoolwidget.h lib/generictoolwidget.cpp
        ${TS_FILES}
)

set(PROJECT_RESOURCES
    translations.qrc
    icons.qrc
    resources.qrc
    assets/global.qss
)

set(APPICON_SVG "${CMAKE_SOURCE_DIR}/assets/icons/appicon.svg")
set(APPICON_ICO "${CMAKE_SOURCE_DIR}/assets/icons/appicon.ico")
set(APPICON_PNG "${CMAKE_SOURCE_DIR}/assets/icons/appicon.png")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

    qt_add_executable(UltimateToolbag
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${PROJECT_RESOURCES}
        ${ICONS}
    )

    qt_add_translations(UltimateToolbag
        TS_FILES ${TS_FILES}
        QM_FILES_OUTPUT_VARIABLE QM_FILES
        OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}"
    )

    qt_add_resources(UltimateToolbag ${PROJECT_RESOURCES})

else()
    if(ANDROID)
        add_library(UltimateToolbag SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(UltimateToolbag
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(UltimateToolbag
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Svg
)
target_link_libraries(UltimateToolbag PRIVATE Qt6::Widgets)
target_link_libraries(UltimateToolbag PRIVATE Qt6::Widgets)
target_link_libraries(UltimateToolbag PRIVATE Qt6::Widgets)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER dev.ilpen.UltimateToolbag)
endif()
set_target_properties(UltimateToolbag PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS UltimateToolbag
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# App icon
if(WIN32) # Windows
    target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/app.rc")
endif()

if(UNIX AND NOT APPLE) # Linux
    install(FILES "${CMAKE_SOURCE_DIR}/ultimate-toolbag.desktop"
        DESTINATION share/applications)

    install(FILES "${CMAKE_SOURCE_DIR}/assets/icons/appicon.png"
            DESTINATION share/icons/hicolor/256x256/apps
            RENAME ultimate-toolbag.png)
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(UltimateToolbag)
endif()
