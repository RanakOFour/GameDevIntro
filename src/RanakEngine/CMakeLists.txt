# RanakEngine Library
file(GLOB RANAKENGINE_SOURCES 
    "*.cpp"
    "Core/*.cpp"
    "Math/*.cpp"
    "IO/*.cpp"
    "Log/*.cpp"
    "Asset/*.cpp"
)

add_library(RanakEngine STATIC ${RANAKENGINE_SOURCES})

# Include directories for RanakEngine
target_include_directories(RanakEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../extern
    ${LUA_INCLUDE_DIR}
)

# Link dependencies
if(UNIX)
    target_link_directories(RanakEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/unix)
    target_link_libraries(RanakEngine PUBLIC GL GLEW SDL3 pugixml ${LUA_LIBRARIES})
endif()