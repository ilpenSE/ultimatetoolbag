cmake_minimum_required(VERSION 3.16)

project(UltimateToolbag VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(TS_FILES
    assets/locale/en_US.ts
    assets/locale/tr_TR.ts
    assets/locale/de_DE.ts
    assets/locale/fr_FR.ts
    assets/locale/it_IT.ts
    assets/locale/es_ES.ts)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Core Network LinguistTools)

    qt_add_executable(UltimateToolbag
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        languagemanager.h languagemanager.cpp
        appdatamanager.h appdatamanager.cpp
        consts.h enums.h versionmanager.h
        etagcontroller.h etagcontroller.cpp
        logger.cpp logger.h LoggerStream.h
        settingsmanager.h settingsmanager.cpp
        thememanager.h thememanager.cpp
        iconutils.h iconutils.cpp
    )

    qt_add_translations(UltimateToolbag
        TS_FILES ${TS_FILES}
        QM_FILES_OUTPUT_VARIABLE QM_FILES
    )

    qt_add_resources(UltimateToolbag translations.qrc)


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

target_link_libraries(UltimateToolbag PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(UltimateToolbag PRIVATE Qt6::Core)

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

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(UltimateToolbag)
endif()
