cmake_minimum_required(VERSION 3.26)
project(Emu_GameBoy C CXX)

set(CMAKE_CXX_STANDARD 23)
set(PROJECT_VERSION 1.0.0)

set(SOURCE_FILES
        hardware/Bus.cpp
        hardware/InputManager.cpp
        logging/Logger.cpp
        hardware/Ppu.cpp
        hardware/Screen.cpp
        hardware/SharpSM83.cpp
        main.cpp
)

set(HEADER_FILES
        hardware/Bus.h
        hardware/InputManager.h
        logging/Logger.h
        hardware/Ppu.h
        hardware/Screen.h
        hardware/SharpSM83.h
)

add_compile_options(-Wall -Wextra)
add_subdirectory(lib/raylib)
add_executable(Emu_GameBoy ${HEADER_FILES} ${SOURCE_FILES})
if (UNIX OR LINUX)
    target_link_libraries(Emu_GameBoy pthread raylib)
elseif (WIN32)
    target_link_libraries(Emu_GameBoy pthread raylib)
endif ()

add_custom_command(TARGET Emu_GameBoy PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/)
