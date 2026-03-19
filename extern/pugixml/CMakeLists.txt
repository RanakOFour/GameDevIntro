add_library(pugixml STATIC "${CMAKE_CURRENT_SOURCE_DIR}/pugixml.cpp")
target_include_directories(pugixml PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/pugixml.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/pugiconfig.hpp")