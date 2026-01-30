cmake_minimum_required(VERSION 3.16)

project(animedl VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(animedl
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        resources.qrc
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET animedl APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(animedl SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(animedl
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(animedl PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.animedl)
endif()
set_target_properties(animedl PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS animedl
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
if (WIN32)
    set(BACKEND_BIN backend.exe)
    set(YTDLP_BIN yt-dlp.exe)
else()
    set(BACKEND_BIN backend)
    set(YTDLP_BIN yt-dlp)
endif()

add_custom_command(TARGET animedl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf
        $<TARGET_FILE_DIR:animedl>/backend
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/backend/dist/backend
        $<TARGET_FILE_DIR:animedl>/backend
)

add_custom_command(TARGET animedl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/backend/${YTDLP_BIN}
        $<TARGET_FILE_DIR:animedl>/${YTDLP_BIN}
)

if (WIN32)
    target_sources(animedl PRIVATE icon.rc)
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(animedl)
endif()
