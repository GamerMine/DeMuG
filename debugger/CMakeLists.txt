PROJECT(DeMuGGER)

set(IMGUI_PATH ../lib/imgui)
file(GLOB IMGUI_GLOB
        ${IMGUI_PATH}/*.h
        ${IMGUI_PATH}/*.cpp

        ${IMGUI_PATH}/backends/imgui_impl_glfw.h
        ${IMGUI_PATH}/backends/imgui_impl_glfw.cpp
        ${IMGUI_PATH}/backends/imgui_impl_opengl3.h
        ${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp
)
add_library(imgui STATIC ${IMGUI_GLOB})
target_include_directories(imgui PUBLIC ${IMGUI_PATH})
target_link_libraries(imgui PRIVATE glfw)

set(DEBUGGER_SOURCE_FILES
        main.cpp
        ../misc/ipc/SharedMemoryReader.cpp
        ../misc/ipc/SharedMemoryWriter.cpp
        ../misc/logging/Logger.cpp
        DebuggerWindow.cpp
        utils/RomReader.cpp
        windows/CodeViewer.cpp
        windows/EmulatorControls.cpp
)

set(DEBUGGER_HEADER_FILES
        ../misc/ipc/SharedMemoryReader.h
        ../misc/ipc/SharedMemoryWriter.h
        ../misc/logging/Logger.h
        DebuggerWindow.h
        fonts/IconsFontAwesome5.h
        utils/RomReader.h
        windows/CodeViewer.h
        windows/EmulatorControls.h
)

add_executable(DeMuGGER ${DEBUGGER_HEADER_FILES} ${DEBUGGER_SOURCE_FILES})
target_include_directories(DeMuGGER PUBLIC ${IMGUI_PATH})
target_link_libraries(DeMuGGER raylib imgui)