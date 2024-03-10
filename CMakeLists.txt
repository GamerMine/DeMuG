cmake_minimum_required(VERSION 3.25)
project(Emu_GameBoy C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PROJECT_VERSION 1.0.0)

set(SOURCE_FILES
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

add_subdirectory(lib/raylib)
add_executable(Emu_GameBoy ${HEADER_FILES} ${SOURCE_FILES})
if (UNIX OR LINUX)
    target_compile_options(Emu_GameBoy PRIVATE -Wall -Wextra)
    target_link_libraries(Emu_GameBoy pthread raylib -static-libgcc -static-libstdc++)
elseif (WIN32)
    target_compile_options(Emu_GameBoy PRIVATE -Wall -Wextra -Wa,-mbig-obj)
    target_link_libraries(Emu_GameBoy pthread raylib -static)
endif ()

add_custom_command(TARGET Emu_GameBoy PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/)
