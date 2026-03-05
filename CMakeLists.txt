cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)

project(GameDevIntro)

find_package(Lua REQUIRED)

#set(TESTS true)
#add_subdirectory(tests)

#if(NOT TESTS)
add_executable(GameDevIntro "src/main.cpp")

add_subdirectory(src)


target_include_directories(GameDevIntro PUBLIC include extern extern/imgui extern/pugixml ${LUA_INCLUDE_DIR})

if(UNIX)
    target_link_directories(GameDevIntro PUBLIC lib/unix)
    target_link_libraries(GameDevIntro PUBLIC GL GLEW SDL3 pugixml ${LUA_LIBRARIES})
endif()

#endif()