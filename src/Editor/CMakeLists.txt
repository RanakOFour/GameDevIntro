# Editor sources - recursively include all .cpp files from subdirectories
file(GLOB_RECURSE EDITOR_SOURCES "*.cpp")

target_sources(GameDevIntro PRIVATE ${EDITOR_SOURCES})