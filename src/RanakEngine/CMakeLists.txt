add_subdirectory(Core)
add_subdirectory(Math)
add_subdirectory(IO)
add_subdirectory(Log)
add_subdirectory(Asset)

file(GLOB MAIN_SOURCES "*.cpp")

target_sources(GameDevIntro PUBLIC ${MAIN_SOURCES})