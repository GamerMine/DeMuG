cmake_minimum_required(VERSION 3.25)
project(DeMuG C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PROJECT_VERSION 1.0.0)

set(SOURCE_FILES
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

set(HEADER_FILES
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
    file(GLOB_RECURSE RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/resources/macos/*")
endif ()

add_subdirectory(lib/raylib)
add_subdirectory(lib/nativefiledialog-extended)

add_executable(DeMuG ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCES})
if (UNIX AND NOT APPLE OR LINUX)
    target_compile_options(DeMuG PRIVATE -Wall -Wextra)
    target_link_libraries(DeMuG pthread raylib nfd)
elseif (WIN32)
    target_compile_options(DeMuG PRIVATE -Wall -Wextra -Wa,-mbig-obj)
    target_link_libraries(DeMuG pthread raylib nfd -static)
elseif (APPLE)
    set_target_properties(DeMuG PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_FRAMEWORK_IDENTIFIER fr.dwightstudio.DeMuG
            MACOSX_BUNDLE_BUNDLE_NAME DeMuG
            MACOSX_BUNDLE_ICON_FILE icon.icns
    )

    foreach (FILE ${RESOURCES})
        file(RELATIVE_PATH NEW_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources" ${FILE})
        get_filename_component(NEW_FILE_PATH ${NEW_FILE} DIRECTORY)
        set_property(SOURCE ${FILE} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${NEW_FILE_PATH}")
    endforeach ()

    target_compile_options(DeMuG PRIVATE -Wall -Wextra)
    target_link_libraries(DeMuG pthread raylib nfd -static-libgcc -static-libstdc++)
endif ()

if (UNIX AND NOT APPLE OR LINUX)
    add_custom_command(TARGET DeMuG PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources/linux ${CMAKE_CURRENT_BINARY_DIR}/resources)
elseif (WIN32)
    add_custom_command(TARGET DeMuG PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources/windows ${CMAKE_CURRENT_BINARY_DIR}/resources)
endif ()

if (UNIX AND NOT APPLE OR LINUX)
    install(TARGETS DeMuG DESTINATION opt/DeMuG)
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources DESTINATION opt/DeMuG)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/resources/DeMuG.desktop DESTINATION share/applications)

    set(CPACK_PACKAGE_NAME "DeMuG")
    set(CPACK_PACKAGE_VERSION "1.0.0")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GameBoy emulator for Linux, Windows and MacOSX")
    set(CPACK_PACKAGE_CONTACT "GamerMine <gamermine@dwightstudio.fr>")
    SET(CPACK_PACKAGE_VENDOR "Dwight Studio")
    SET(CPACK_PACKAGE_HOMEPAGE_URL "https://dwightstudio.fr/DeMuG")
    SET(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${MAJOR_VERSION}.${MINOR_VERSION}")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    SET(CPACK_GENERATOR "DEB")

    INCLUDE(CPack)
endif ()