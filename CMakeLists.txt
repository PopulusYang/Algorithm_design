cmake_minimum_required(VERSION 3.16)

project(AlgorithmDesign VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/src/ui)


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/img DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(PROJECT_SOURCES
        src/boss.cpp
        src/collect_coin.cpp
        src/dp.cpp
        src/gamechoose.cpp
        src/main.cpp
        src/player.cpp
        src/renderthread.cpp
        src/exam4.cpp
        src/mainwindow.cpp
        src/lock.cpp
        src/heads/autocontrol.h
        src/heads/backtrack_find_clue.h
        src/heads/boss.h
        src/heads/exam4.h
        src/heads/collect_coin.h
        src/heads/dp.h
        src/heads/gamechoose.h
        src/heads/gamecontrol.h
        src/heads/gamemain.h
        src/heads/mainwindow.h
        src/heads/mapbuild.h
        src/heads/player.h
        src/heads/renderthread.h
        src/heads/lock.h
        src/jsonlib/json.hpp
        src/ui/boss.ui
        src/ui/gamechoose.ui
        src/ui/mainwindow.ui
        src/ui/lock.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(AlgorithmDesign
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET AlgorithmDesign APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(AlgorithmDesign SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(AlgorithmDesign
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(AlgorithmDesign PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.AlgorithmDesign)
endif()
set_target_properties(AlgorithmDesign PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE FALSE
)

include(GNUInstallDirs)
install(TARGETS AlgorithmDesign
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(AlgorithmDesign)
endif()
