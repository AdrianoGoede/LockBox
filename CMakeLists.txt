cmake_minimum_required(VERSION 3.16)

project(LockBox VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools sodium)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

set(TS_FILES LockBox_en_US.ts)

set(PROJECT_SOURCES
    main.cpp
    ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(LockBox
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        src/core/Crypto.h
        src/core/Crypto.cpp
        src/core/Database.h
        src/core/Database.cpp
        src/core/DatabaseEntry.h
        src/core/DatabaseEntry.cpp
        src/core/DatabaseGroup.h
        src/core/DatabaseGroup.cpp
        src/ui/PasswordGenerator.h
        src/ui/PasswordGenerator.cpp
        src/ui/PasswordGenerator.ui
        src/config/Constants.h
        src/ui/MainWindow.h
        src/ui/MainWindow.cpp
        src/ui/MainWindow.ui
        src/core/DatabaseEntryTableModel.h
        src/core/DatabaseEntryTableModel.cpp
        src/core/DatabaseEntryTableProxyModel.h
        src/core/DatabaseEntryTableProxyModel.cpp
        src/core/DatabaseGroupTreeModel.h
        src/core/DatabaseGroupTreeModel.cpp
        src/core/EntryActionButtonDelegate.h
        src/core/EntryActionButtonDelegate.cpp
        src/ui/DatabaseGroupManager.h
        src/ui/DatabaseGroupManager.cpp
        src/ui/DatabaseGroupManager.ui
        src/ui/DatabaseEntryManager.h
        src/ui/DatabaseEntryManager.cpp
        src/ui/DatabaseEntryManager.ui
        src/core/DatabaseEntryHistoryItem.h
        src/core/DatabaseEntryHistoryItem.cpp
        src/core/HistoryActionButtonDelegate.h
        src/core/HistoryActionButtonDelegate.cpp
        src/ui/DatabaseSettingsManager.h
        src/ui/DatabaseSettingsManager.cpp
        src/ui/DatabaseSettingsManager.ui
        src/resources/Wordlists.qrc
        src/core/SecureBuffer.h
        src/core/SizeOnlyDevice.h
        src/core/MemoryWriter.h
        src/ui/PasswordDialog.h
        src/ui/PasswordDialog.cpp
        src/ui/PasswordDialog.ui
        src/core/Utils.h
        src/core/Utils.cpp
        src/ui/autotype/Autotyper.h
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET LockBox APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(LockBox SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(LockBox
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

if (UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    target_link_libraries(LockBox PRIVATE X11::X11 X11::Xtst)
    target_sources(LockBox PRIVATE src/ui/autotype/UnixX11Autotyper.h)
    target_sources(LockBox PRIVATE src/ui/autotype/UnixX11Autotyper.cpp)
elseif (WIN32)
    target_sources(LockBox PRIVATE src/ui/autotype/WindowsAutotyper.h)
    target_sources(LockBox PRIVATE src/ui/autotype/WindowsAutotyper.cpp)
elseif (APPLE)
    target_sources(LockBox PRIVATE src/ui/autotype/MacOsAutotyper.h)
    target_sources(LockBox PRIVATE src/ui/autotype/MacOsAutotyper.cpp)
endif()

target_link_libraries(LockBox PRIVATE Qt${QT_VERSION_MAJOR}::Widgets sodium)
target_link_libraries(LockBox PRIVATE Qt${QT_VERSION_MAJOR}::Core)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.LockBox)
endif()
set_target_properties(LockBox PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS LockBox
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(LockBox)
endif()
