#include "Editor/Panels/UI/TopBar.h"

#include "Editor/Core/Editor.h"
#include "Editor/Core/TutorialRegistry.h"
#include "Editor/Tabs/SceneEditTab.h"
#include "Editor/Scene/SceneSerializer.h"
#include "Editor/Scene/BuiltIn/BuiltinRules.h"
#include "Editor/Project/ProjectExporter.h"

#include "imguiFileDialog/ImGuiFileDialog.h"

#include "RanakEngine/RanakEngine.h"
#include "RanakEngine/Log.h"


TopBar::TopBar(Editor& _editor)
    : m_editor(_editor)
{
}

void TopBar::OpenLoadDialog()
{
    IGFD::FileDialogConfig config;
    config.path = m_editor.GetProject().IsOpen() ? m_editor.GetProject().GetScenesDir() : ".";
    ImGuiFileDialog::Instance()->OpenDialog("SceneFileDlgKey", "Choose File", ".lua", config);
    m_showLoadDialog = true;
    m_showSaveDialog = false;
}

void TopBar::OpenSaveDialog()
{
    IGFD::FileDialogConfig config;
    config.path = m_editor.GetProject().IsOpen() ? m_editor.GetProject().GetScenesDir() : ".";
    ImGuiFileDialog::Instance()->OpenDialog("SceneFileDlgKey", "Choose File", ".lua", config);
    m_showLoadDialog = false;
    m_showSaveDialog = true;
}

void TopBar::Draw()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Scene", "Ctrl+N"))
            {
                std::string l_currentScene = m_editor.GetCurrentScenePath();
                if (!l_currentScene.empty())
                {
                    SceneSerializer::SaveToFile(l_currentScene, m_editor.GetEngineContents(),
                                                m_editor.GetSceneEdit().GetSceneSettings());
                }

                std::shared_ptr<RE::Core::Scene> newScene = std::make_shared<RE::Core::Scene>();
                m_editor.GetEngineContents().core->SetScene(newScene);
                m_editor.SetCurrentScenePath("");

                BuiltinRules::Load(m_editor.GetEngineContents());
                m_editor.GetSceneEdit().RebuildRegistryFromScene();
                m_editor.GetSceneEdit().m_scene = m_editor.GetEngineContents().core->GetScene();
                m_editor.GetSceneEdit().m_entityPanel.RefreshEntityList();
            }
            if (ImGui::MenuItem("Load Scene", "Ctrl+O"))
            {
                IGFD::FileDialogConfig config;
                config.path = m_editor.GetProject().IsOpen() ? m_editor.GetProject().GetScenesDir() : ".";
                ImGuiFileDialog::Instance()->OpenDialog("SceneFileDlgKey", "Choose File", ".lua", config);
                m_showLoadDialog = true;
                m_showSaveDialog = false;
            }
            if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
            {
                std::string l_currentScene = m_editor.GetCurrentScenePath();
                if (!l_currentScene.empty())
                {
                    SceneSerializer::SaveToFile(l_currentScene, m_editor.GetEngineContents(),
                                                m_editor.GetSceneEdit().GetSceneSettings());
                    m_editor.SaveProjectInfo();
                    RE::Log::Message("Scene saved: " + l_currentScene);
                }
                else
                {
                    IGFD::FileDialogConfig config;
                    config.path = m_editor.GetProject().IsOpen() ? m_editor.GetProject().GetScenesDir() : ".";
                    ImGuiFileDialog::Instance()->OpenDialog("SceneFileDlgKey", "Choose File", ".lua", config);
                    m_showLoadDialog = false;
                    m_showSaveDialog = true;
                }
            }
            if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
            {
                IGFD::FileDialogConfig config;
                config.path = m_editor.GetProject().IsOpen() ? m_editor.GetProject().GetScenesDir() : ".";
                ImGuiFileDialog::Instance()->OpenDialog("SceneFileDlgKey", "Choose File", ".lua", config);
                m_showLoadDialog = false;
                m_showSaveDialog = true;
            }
            // ImGui::Separator();
            // if (ImGui::MenuItem("Export Project...", nullptr, false,
            //                     m_editor.GetProject().IsOpen() && !m_editor.GetCurrentScenePath().empty()))
            // {
            //     IGFD::FileDialogConfig config;
            //     config.path = m_editor.GetProject().GetRootPath();
            //     ImGuiFileDialog::Instance()->OpenDialog("ExportDirDlgKey", "Choose Export Directory", nullptr, config);
            //     m_showExportDialog = true;
            // }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Ctrl+Q"))
            {
                SDL_Event l_quitEvent;
                l_quitEvent.type = SDL_EVENT_QUIT;
                SDL_PushEvent(&l_quitEvent);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            UndoManager& l_undo = m_editor.GetUndoManager();
            std::string l_undoLabel = l_undo.CanUndo()
                ? "Undo " + l_undo.GetUndoDescription()
                : "Undo";
            std::string l_redoLabel = l_undo.CanRedo()
                ? "Redo " + l_undo.GetRedoDescription()
                : "Redo";

            if (ImGui::MenuItem(l_undoLabel.c_str(), "Ctrl+Z", false, l_undo.CanUndo()))
                l_undo.Undo();
            if (ImGui::MenuItem(l_redoLabel.c_str(), "Ctrl+Shift+Z", false, l_undo.CanRedo()))
                l_undo.Redo();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            SceneEditTab& l_sceneEdit = m_editor.GetSceneEdit();

            ImGui::MenuItem("Show Grid", nullptr, &l_sceneEdit.GetSceneSettings().showGrid);
            ImGui::MenuItem("Show Gizmos", nullptr, &l_sceneEdit.GetSceneSettings().showGizmos);

            if (ImGui::MenuItem("Camera Settings"))
                l_sceneEdit.m_cameraPanel.SetShown(true);

            if (ImGui::MenuItem("Console"))
                l_sceneEdit.m_consolePanel.SetShown(!l_sceneEdit.m_consolePanel.IsShown());

            if (ImGui::MenuItem("Asset Browser"))
                l_sceneEdit.m_assetBrowserPanel.SetShown(!l_sceneEdit.m_assetBrowserPanel.IsShown());

            ImGui::Separator();

            if (ImGui::BeginMenu("Layout"))
            {
                if (ImGui::MenuItem("Save Layout 1"))  m_editor.SaveLayout(0);
                if (ImGui::MenuItem("Save Layout 2"))  m_editor.SaveLayout(1);
                if (ImGui::MenuItem("Save Layout 3"))  m_editor.SaveLayout(2);

                ImGui::Separator();

                if (ImGui::MenuItem("Load Layout 1", nullptr, false, m_editor.HasLayout(0)))
                    m_editor.LoadLayout(0);
                if (ImGui::MenuItem("Load Layout 2", nullptr, false, m_editor.HasLayout(1)))
                    m_editor.LoadLayout(1);
                if (ImGui::MenuItem("Load Layout 3", nullptr, false, m_editor.HasLayout(2)))
                    m_editor.LoadLayout(2);

                ImGui::Separator();

                if (ImGui::MenuItem("Reset Layout"))
                    ImGui::LoadIniSettingsFromDisk("imgui.ini");

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Settings"))
        {
            if (ImGui::MenuItem("Project Settings"))
                m_editor.ShowProjectSettingsDialog();
            if (ImGui::MenuItem("Scene Settings"))
                m_editor.GetSceneEdit().m_settingsPanel.SetShown(true);
            if (ImGui::MenuItem("Theme Settings"))
                m_editor.ShowThemeSettingsPanel();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tutorials"))
        {
            TutorialRegistry& l_registry = m_editor.GetTutorialRegistry();
            TutorialPanel& l_tut = m_editor.GetTutorialPanel();
            for(const auto& _entry : l_registry.GetTutorialNames())
            {
                if (ImGui::MenuItem(_entry.c_str()))
                {
                    l_tut.LoadTutorial(_entry, l_registry.GetTutorialSource(_entry));
                }
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    // File dialogs
    if ((m_showLoadDialog || m_showSaveDialog) && ImGuiFileDialog::Instance()->IsOpened("SceneFileDlgKey"))
    {
        if (ImGuiFileDialog::Instance()->Display("SceneFileDlgKey", ImGuiWindowFlags_NoCollapse, ImVec2(700, 400)))
        {
            std::string l_filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            if (m_showLoadDialog)
            {
                SceneSerializer::LoadFromFile(l_filePathName, m_editor.GetEngineContents(),
                                              &m_editor.GetSceneEdit().GetSceneSettings());

                // Built-in rules are re-attached inside LoadFromString.
                m_editor.GetSceneEdit().RebuildRegistryFromScene();
                m_editor.GetSceneEdit().m_scene = m_editor.GetEngineContents().core->GetScene();
                m_editor.GetSceneEdit().m_entityPanel.RefreshEntityList();

                m_editor.SetCurrentScenePath(l_filePathName);
                m_editor.SaveProjectInfo();
            }
            else
            {
                SceneSerializer::SaveToFile(l_filePathName, m_editor.GetEngineContents(),
                                            m_editor.GetSceneEdit().GetSceneSettings());
                m_editor.SetCurrentScenePath(l_filePathName);
                m_editor.SaveProjectInfo();
            }

            ImGuiFileDialog::Instance()->Close();
            m_showLoadDialog = false;
            m_showSaveDialog = false;
        }
    }

    // Export directory dialog
    if (m_showExportDialog && ImGuiFileDialog::Instance()->IsOpened("ExportDirDlgKey"))
    {
        if (ImGuiFileDialog::Instance()->Display("ExportDirDlgKey", ImGuiWindowFlags_NoCollapse, ImVec2(700, 400)))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string l_dir = ImGuiFileDialog::Instance()->GetCurrentPath();
                ProjectExporter::Export(m_editor, l_dir);
            }
            ImGuiFileDialog::Instance()->Close();
            m_showExportDialog = false;
        }
    }
}
