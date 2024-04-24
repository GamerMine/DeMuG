CMAKE_MINIMUM_REQUIRED(VERSION 3.25)
PROJECT(DeMuG C CXX)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_PROJECT_HOMEPAGE_URL "https://dwightstudio.fr/DeMuG")
SET(PROJECT_VERSION 1.0.0)

SET(SOURCE_FILES
        gui/MainWindow.cpp
        hardware/Apu.cpp
        hardware/Bus.cpp
        hardware/InputManager.cpp
        hardware/Ppu.cpp
        hardware/Screen.cpp
        hardware/SerialIO.cpp
        hardware/SharpSM83.cpp
        hardware/Timer.cpp
        logging/Logger.cpp
        main.cpp
)

SET(HEADER_FILES
        debug/Debug.h
        gui/FileChooser.h
        gui/MainWindow.h
        gui/raygui.h
        hardware/Apu.h
        hardware/audio/SC1PulseSweep.h
        hardware/audio/SC2Pulse.h
        hardware/audio/SC3Wave.h
        hardware/audio/SC4Noise.h
        hardware/Bus.h
        hardware/InputManager.h
        hardware/mappers/Cartridge.h
        hardware/mappers/CartridgeHelper.h
        hardware/mappers/NoMBC.h
        hardware/mappers/MBC1.h
        hardware/Ppu.h
        hardware/Screen.h
        hardware/SerialIO.h
        hardware/SharpSM83.h
        hardware/Timer.h
        logging/Logger.h
)

if (APPLE)
    FILE(GLOB_RECURSE RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/resources/macos/*")
elseif(WIN32)
    SET(RESOURCES 
        resources/windows/DeMuG.rc
        resources/windows/icon.ico
    )
endif ()

ADD_SUBDIRECTORY(lib/raylib EXCLUDE_FROM_ALL)
ADD_SUBDIRECTORY(lib/nativefiledialog-extended EXCLUDE_FROM_ALL)

ADD_EXECUTABLE(DeMuG ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCES})
if (UNIX AND NOT APPLE OR LINUX)
    TARGET_COMPILE_OPTIONS(DeMuG PRIVATE -Wall -Wextra)
    TARGET_LINK_LIBRARIES(DeMuG pthread raylib nfd)
elseif (WIN32)
    SET_TARGET_PROPERTIES(DeMuG PROPERTIES LINK_FLAGS "-mwindows")
    TARGET_COMPILE_OPTIONS(DeMuG PRIVATE -mwindows -Wall -Wextra -Wa,-mbig-obj)
    TARGET_LINK_LIBRARIES(DeMuG pthread raylib nfd -static)
elseif (APPLE)
    SET_TARGET_PROPERTIES(DeMuG PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_FRAMEWORK_IDENTIFIER fr.dwightstudio.DeMuG
            MACOSX_BUNDLE_BUNDLE_NAME DeMuG
            MACOSX_BUNDLE_ICON_FILE icon.icns
    )

    foreach (FILE ${RESOURCES})
        FILE(RELATIVE_PATH NEW_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/macos" ${FILE})
        GET_FILENAME_COMPONENT(NEW_FILE_PATH ${NEW_FILE} DIRECTORY)
        SET_PROPERTY(SOURCE ${FILE} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${NEW_FILE_PATH}")
    endforeach ()

    TARGET_COMPILE_OPTIONS(DeMuG PRIVATE -Wall -Wextra)
    TARGET_LINK_LIBRARIES(DeMuG pthread raylib nfd)
endif ()

if (UNIX AND NOT APPLE OR LINUX)
    ADD_CUSTOM_COMMAND(TARGET DeMuG PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources/linux ${CMAKE_CURRENT_BINARY_DIR}/resources)
elseif (WIN32)
    ADD_CUSTOM_COMMAND(TARGET DeMuG PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources/windows ${CMAKE_CURRENT_BINARY_DIR}/resources)
elseif(APPLE)
    ADD_CUSTOM_COMMAND(TARGET DeMuG PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources/macos ${CMAKE_CURRENT_BINARY_DIR}/resources)
endif ()

SET(CPACK_PACKAGE_NAME "DeMuG")
SET(CPACK_PACKAGE_VERSION "1.0.0")
SET(CPACK_PACKAGE_DESCRIPTION "GameBoy emulator for Linux, Windows and MacOSX")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GameBoy emulator for Linux, Windows and MacOSX")
SET(CPACK_PACKAGE_CONTACT "GamerMine <gamermine@dwightstudio.fr>")
SET(CPACK_PACKAGE_VENDOR "Dwight Studio")
SET(CPACK_PACKAGE_HOMEPAGE_URL "https://dwightstudio.fr/DeMuG")
SET(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${PROJECT_VERSION}")
SET(CPACK_PACKAGE_EXECUTABLES "DeMuG;DeMuG")

if (UNIX AND NOT APPLE OR LINUX)
    INSTALL(TARGETS DeMuG DESTINATION opt/DeMuG)
    INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources DESTINATION opt/DeMuG)
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/resources/DeMuG.desktop DESTINATION share/applications)

    SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    SET(CPACK_DEBIAN_PACKAGE_SECTION "games")

    SET(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")

    SET(CPACK_GENERATOR "DEB;RPM")
elseif (WIN32)
    INSTALL(TARGETS DeMuG DESTINATION ".")
    INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources DESTINATION ".")

    SET(CPACK_INNOSETUP_ARCHITECTURE "x64")
    SET(CPACK_INNOSETUP_PROGRAM_MENU_FOLDER ".")
    SET(CPACK_INNOSETUP_LANGUAGES "english;french")
    SET(CPACK_INNOSETUP_IGNORE_LICENSE_PAGE "ON")
    SET(CPACK_INNOSETUP_ICON_FILE "${CMAKE_CURRENT_BINARY_DIR}/resources/icon.ico")
    SET(CPACK_INNOSETUP_RUN_EXECUTABLES "DeMuG")

    SET(CPACK_GENERATOR "INNOSETUP")
elseif (APPLE)
    INSTALL(TARGETS DeMuG DESTINATION "." COMPONENT DeMuG)

    SET(CPACK_DMG_DS_STORE "${CMAKE_CURRENT_BINARY_DIR}/resources/DS_Store")
    SET(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_CURRENT_BINARY_DIR}/resources/DMG_Background.png")
    #SET(CPACK_DMG_FORMAT "UDRW")

    SET(CPACK_GENERATOR "DragNDrop")
endif ()

INCLUDE(CPack)
