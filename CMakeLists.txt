cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)

project(GameDevIntro)

find_package(Lua REQUIRED)

#set(TESTS true)
#add_subdirectory(tests)

#if(NOT TESTS)

# Collect ImGui source files
file(GLOB IMGUI_SOURCES "extern/imgui/*.cpp")

# Create game executable
add_executable(TestRun "src/main.cpp" ${IMGUI_SOURCES})

# Create editor executable
add_executable(GameEditor "src/editor_main.cpp" ${IMGUI_SOURCES})

# Add subdirectories (this will create RanakEngine library and add Editor sources)
add_subdirectory(src)

# Set up TestRun target
target_include_directories(TestRun PUBLIC include extern extern/imgui extern/imgui/backends extern/pugixml ${LUA_INCLUDE_DIR})
target_link_libraries(TestRun PUBLIC RanakEngine)

if(UNIX)
    target_link_directories(TestRun PUBLIC lib/unix)
    target_link_libraries(TestRun PUBLIC GL GLEW SDL3 pugixml ${LUA_LIBRARIES})
endif()

# Set up GameEditor target (links against RanakEngine library)
target_include_directories(GameEditor PUBLIC include extern extern/imgui extern/imgui/backends extern/pugixml ${LUA_INCLUDE_DIR})
target_link_libraries(GameEditor PUBLIC RanakEngine)

if(UNIX)
    target_link_directories(GameEditor PUBLIC lib/unix)
    target_link_libraries(GameEditor PUBLIC GL GLEW SDL3 pugixml ${LUA_LIBRARIES})
endif()

#endif()