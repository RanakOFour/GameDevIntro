# Editor sources
set(EDITOR_SOURCES
    "AutoCompleteTree.cpp"
    "CameraPanel.cpp"
    "CategoryPanel.cpp"
    "Editor.cpp"
    "EntityPanel.cpp"
    "Panel.cpp"
    "PropertiesPanel.cpp"
    "RulesPanel.cpp"
    "SceneEditTab.cpp"
    "SceneSerializer.cpp"
    "TextEditTab.cpp"
    "TutorialPanel.cpp"
)

target_sources(GameDevIntro PRIVATE ${EDITOR_SOURCES})