# Editor sources
file(GLOB EDITOR_SOURCES "*.cpp")

# Add sources to both GameDevIntro and GameEditor targets
target_sources(GameDevIntro PRIVATE ${EDITOR_SOURCES})
target_sources(GameEditor PRIVATE ${EDITOR_SOURCES})

# Link GameEditor against RanakEngine library
target_link_libraries(GameEditor PRIVATE RanakEngine)

# Ensure GameDevIntro has ImGui dependencies from Editor
target_include_directories(GameEditor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../extern
    ${CMAKE_CURRENT_SOURCE_DIR}/../../extern/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/../../extern/imgui/backends
)
