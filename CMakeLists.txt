cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)

project(GameDevIntro VERSION 0.5
                     LANGUAGES CXX)

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


add_subdirectory(extern/pugixml)
add_subdirectory(extern/RanakEngine2)

add_executable(TestRun "src/main.cpp" ${IMGUI_SOURCES})

add_executable(GameEditor "src/editor_main.cpp" ${IMGUI_SOURCES})

add_subdirectory(src)


target_include_directories(TestRun PUBLIC include extern
                                                  extern/imgui
                                                  extern/imgui/backends
                                                  extern/pugixml)

target_link_libraries(TestRun PUBLIC RanakEngine)


target_include_directories(GameEditor PUBLIC include extern
                                                     extern/imgui
                                                     extern/imgui/backends
                                                     extern/imguiTextEdit
                                                     extern/pugixml)

target_link_libraries(GameEditor PUBLIC RanakEngine)

if(UNIX)
    target_link_directories(TestRun PUBLIC lib/unix)
    target_link_libraries(TestRun PUBLIC GL GLEW SDL3 pugixml ${LUA_LIBRARIES} )
    
    target_link_directories(GameEditor PUBLIC lib/unix)
    target_link_libraries(GameEditor PUBLIC GL GLEW SDL3 pugixml ${LUA_LIBRARIES})
elseif(WIN32)
    target_link_directories(TestRun PUBLIC lib/win/release)
    target_link_libraries(TestRun PUBLIC opengl32 glew32 SDL3 lua)
    target_link_directories(GameEditor PUBLIC lib/win/release)
    target_link_libraries(GameEditor PUBLIC opengl32 glew32 SDL3 lua)

    
    add_custom_command(TARGET TestRun POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                       "${CMAKE_CURRENT_SOURCE_DIR}/lib/win/bin"
                       $<TARGET_FILE_DIR:TestRun>)

    
    add_custom_command(TARGET GameEditor POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                       "${CMAKE_CURRENT_SOURCE_DIR}/lib/win/bin"
                       $<TARGET_FILE_DIR:GameEditor>)

    add_custom_command(TARGET GameEditor POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                       "${CMAKE_CURRENT_SOURCE_DIR}/resources"
                       $<TARGET_FILE_DIR:GameEditor>/resources)
endif()

#endif()