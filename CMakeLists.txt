cmake_minimum_required(VERSION 3.20)

project(GameDevIntro VERSION 0.5
                     LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftime-trace=timetrace.json")

# Collect ImGui source files
file(GLOB IMGUI_SOURCES "extern/imgui/imgui.cpp"
                        "extern/imgui/imgui_draw.cpp"
                        "extern/imgui/imgui_tables.cpp"
                        "extern/imgui/imgui_widgets.cpp"
                        "extern/imgui/backends/imgui_impl_sdl3.cpp"
                        "extern/imgui/backends/imgui_impl_opengl3.cpp"
                        "extern/imgui/misc/cpp/imgui_stdlib.cpp"
                        "extern/imguiTextEdit/TextEditor.cpp"
                        "extern/imguiFileDialog/ImGuiFileDialog.cpp")


add_subdirectory(extern/RanakEngine2)

add_executable(GameDevIntro "src/editor_main.cpp" ${IMGUI_SOURCES})

add_subdirectory(src)


target_include_directories(GameDevIntro PUBLIC include extern
                                                     extern/imgui
                                                     extern/imgui/backends
                                                     extern/imguiTextEdit)

target_link_libraries(GameDevIntro PUBLIC RanakEngine)

if(UNIX)
    target_link_directories(GameDevIntro PUBLIC lib/unix)
    target_link_libraries(GameDevIntro PUBLIC GL GLEW SDL3 ${LUA_LIBRARIES})
elseif(WIN32)
    target_link_directories(GameDevIntro PUBLIC lib/win/release)
    target_link_libraries(GameDevIntro PUBLIC opengl32 glew32 SDL3 lua)

    add_custom_command(TARGET GameDevIntro POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                       "${CMAKE_CURRENT_SOURCE_DIR}/lib/win/bin"
                       $<TARGET_FILE_DIR:GameDevIntro>)

    add_custom_command(TARGET GameDevIntro POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                       "${CMAKE_CURRENT_SOURCE_DIR}/resources"
                       $<TARGET_FILE_DIR:GameDevIntro>/resources)
endif()