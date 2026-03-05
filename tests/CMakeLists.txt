add_executable(CFacTest CFacTest.cpp)

target_include_directories(CFacTest PUBLIC ../include ../extern ${LUA_INCLUDE_DIRS})
target_link_directories(CFacTest PUBLIC lib)
target_link_libraries(CFacTest PUBLIC GL GLEW SDL3 pugixml ${LUA_LIBRARIES})