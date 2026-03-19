add_library(pugixml STATIC "${CMAKE_CURRENT_SOURCE_DIR}/pugixml.cpp")
target_include_directories(pugixml PUBLIC pugixml.hpp pugiconfig.hpp)