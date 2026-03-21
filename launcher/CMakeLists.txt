# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

add_library(umbra_static STATIC)

ecm_add_qml_module(umbra_static GENERATE_PLUGIN_SOURCE
        URI zone.xiv.umbra
        VERSION 1.0)

ecm_qt_declare_logging_category(umbra_static
        HEADER umbra_log.h
        IDENTIFIER UMBRA_LOG
        CATEGORY_NAME zone.xiv.umbra
        DESCRIPTION "Umbra logs"
        EXPORT UMBRA
)

ecm_qt_declare_logging_category(umbra_static
        HEADER umbra_http_log.h
        IDENTIFIER UMBRA_HTTP
        CATEGORY_NAME zone.xiv.umbra.http
        DESCRIPTION "Umbra HTTP requests"
        EXPORT UMBRA
)

ecm_qt_declare_logging_category(umbra_static
        HEADER umbra_patcher_log.h
        IDENTIFIER UMBRA_PATCHER
        CATEGORY_NAME zone.xiv.umbra.patcher
        DESCRIPTION "Umbra patcher logs"
        EXPORT UMBRA
)

target_sources(umbra_static PRIVATE
        include/account.h
        include/processwatcher.h
        include/profilemanager.h
        include/profile.h
        include/utility.h
        include/accountmanager.h
        include/assetupdater.h
        include/gamerunner.h
        include/launchercore.h
        include/processlogger.h
        include/squareenixlogin.h
        include/encryptedarg.h
        include/blowfish.h
        include/patcher.h
        include/gamepatcher.h

        src/accountmanager.cpp
        src/assetupdater.cpp
        src/gamerunner.cpp
        src/launchercore.cpp
        src/account.cpp
        src/processwatcher.cpp
        src/profilemanager.cpp
        src/profile.cpp
        src/utility.cpp
        src/processlogger.cpp
        src/squareenixlogin.cpp
        src/encryptedarg.cpp
        src/blowfish.cpp
        src/patcher.cpp
        src/gamepatcher.cpp)
target_include_directories(umbra_static PUBLIC include)
target_link_libraries(umbra_static PUBLIC
        Qt6::Core
        Qt6::Quick
        Qt6::Network
        Qt6::Concurrent
        Qt6Keychain::Qt6Keychain
        KF6::I18n
        KF6::CoreAddons
        KF6::ConfigCore
        KF6::ConfigGui
        KF6::Archive
        QCoro::Core
        QCoro::Network
        QCoro::Qml
        physis)
kconfig_target_kcfg_file(umbra_static FILE config.kcfg CLASS_NAME Config MUTATORS GENERATE_PROPERTIES GENERATE_MOC DEFAULT_VALUE_GETTERS PARENT_IN_CONSTRUCTOR QML_REGISTRATION QML_UNCREATABLE USE_ENUM_TYPES)
kconfig_target_kcfg_file(umbra_static FILE accountconfig.kcfg CLASS_NAME AccountConfig MUTATORS GENERATE_PROPERTIES GENERATE_MOC DEFAULT_VALUE_GETTERS PARENT_IN_CONSTRUCTOR QML_REGISTRATION QML_UNCREATABLE USE_ENUM_TYPES)
kconfig_target_kcfg_file(umbra_static FILE profileconfig.kcfg CLASS_NAME ProfileConfig MUTATORS GENERATE_PROPERTIES GENERATE_MOC DEFAULT_VALUE_GETTERS PARENT_IN_CONSTRUCTOR QML_REGISTRATION QML_UNCREATABLE USE_ENUM_TYPES)
target_include_directories(umbra_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_BINARY_DIR})

if (NOT MSVC)
    target_compile_options(umbra_static PUBLIC -fexceptions)
endif ()

if (BUILD_FLATPAK)
    target_compile_definitions(umbra_static PUBLIC FLATPAK)
endif ()

# For screensaver inhibition on Linux
if (TARGET Qt6::DBus)
    target_link_libraries(umbra_static PRIVATE Qt6::DBus)
    target_compile_definitions(umbra_static PRIVATE -DHAS_DBUS)
endif ()

add_executable(umbra)

target_sources(umbra PRIVATE
        include/logger.h

        src/logger.cpp
        src/main.cpp
)

qt_target_qml_sources(umbra_static
        QML_FILES
        ui/Components/FormFileDelegate.qml
        ui/Components/FormFolderDelegate.qml
        ui/Pages/AutoLoginPage.qml
        ui/Pages/LoginPage.qml
        ui/Pages/MainPage.qml
        ui/Pages/StatusPage.qml
        ui/Settings/AboutPage.qml
        ui/Settings/AccountSettings.qml
        ui/Settings/AccountsPage.qml
        ui/Settings/DeveloperSettings.qml
        ui/Settings/GeneralSettings.qml
        ui/Settings/ProfileSettings.qml
        ui/Settings/ProfilesPage.qml
        ui/Settings/SettingsPage.qml
        ui/Setup/AccountSetup.qml
        ui/Setup/AddSquareEnix.qml
        ui/Setup/SetupPage.qml
        ui/Main.qml
)

set_source_files_properties(../zone.xiv.umbra.svg PROPERTIES
        QT_RESOURCE_ALIAS /zone.xiv.umbra.svg
)

qt_target_qml_sources(umbra_static
        PREFIX /
        RESOURCES
        ../zone.xiv.umbra.svg
)

target_link_libraries(umbra PRIVATE
        umbra_static
        umbra_staticplugin
        KDAB::kdsingleapplication
        Qt6::Widgets
        Qt6::QuickControls2
        KF6::Kirigami
        KF6::IconThemes)

if (WIN32)
    set_target_properties(umbra PROPERTIES
            WIN32_EXECUTABLE TRUE
            OUTPUT_NAME "Umbra")
endif ()

install(TARGETS umbra ${KF6_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_add_app_icon(umbra
        ICONS
        ${CMAKE_SOURCE_DIR}/resources/16-umbra.png
        ${CMAKE_SOURCE_DIR}/resources/32-umbra.png
        ${CMAKE_SOURCE_DIR}/resources/48-umbra.png
        ${CMAKE_SOURCE_DIR}/resources/256-umbra.png
)

qt_finalize_target(umbra)

# the current tooling completely breaks in Flatpak, and is unnecessary there anyway
if (WIN32)
    qt_generate_deploy_qml_app_script(
            TARGET umbra
            OUTPUT_SCRIPT deploy_script
            NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script})
endif ()
