#include "Editor/Project/ProjectSelectionScreen.h"
#include "RanakEngine/UI.h"
#include "imgui.h"

#include "imgui/imgui_impl_sdl3.h"
#include "imgui/imgui_impl_opengl3.h"

#include "imgui/misc/cpp/imgui_stdlib.h"

#define USE_STD_FILESYSTEM 1
#include "imguiFileDialog/ImGuiFileDialog.h"

#include <GL/gl.h>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <vector>

#if _WIN32
#include <stdlib.h>
#include <stdio.h>
#include <shlobj_core.h>
#include <direct.h>
#endif

std::string ProjectSelectionScreen::GetDataDir()
{
    std::filesystem::path l_base;
#if _WIN32
    PWSTR l_appdata;
    if (SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE, NULL, &l_appdata) == S_OK) {
        char l_pathAsString[MAX_PATH];
        wcstombs(l_pathAsString, l_appdata, MAX_PATH);
        printf("Appdata path: %s", l_pathAsString);
        l_base = std::filesystem::path(l_pathAsString);
    }
    else {
        fprintf(stderr, "Could not find appdata path!\n");
    }
#else
    const char* l_xdgRaw  = std::getenv("XDG_DATA_HOME");
    const char* l_homeRaw = std::getenv("HOME");
    const std::string l_xdg  = l_xdgRaw  ? l_xdgRaw  : "";
    const std::string l_home = l_homeRaw ? l_homeRaw : "";
    if (!l_xdg.empty())
        l_base = std::filesystem::path(l_xdg);
    else if (!l_home.empty())
        l_base = std::filesystem::path(l_home) / ".local" / "share";
    else
        l_base = std::filesystem::path(".");
#endif
    return (l_base / "GameDevIntro").string();
}

void ProjectSelectionScreen::LoadRecentProjects(State& _state)
{
    _state.recentProjects.clear();
    std::filesystem::path l_path = std::filesystem::path(GetDataDir()) / "RecentProjects.txt";
    std::ifstream l_file(l_path);
    if (!l_file.is_open())
        return;

    std::string l_line;
    while (std::getline(l_file, l_line))
    {
        if (!l_line.empty())
            _state.recentProjects.push_back(l_line);
    }
}

void ProjectSelectionScreen::SaveRecentProjects(const State& _state)
{
    std::filesystem::path l_dir(GetDataDir());
    std::filesystem::create_directories(l_dir);

    std::filesystem::path l_path = l_dir / "RecentProjects.txt";
    std::ofstream l_file(l_path, std::ios::trunc);
    if (!l_file.is_open())
        return;

    for (const auto& l_proj : _state.recentProjects)
        l_file << l_proj << "\n";
}

void ProjectSelectionScreen::AddRecentProject(State& _state, const std::string& _path)
{
    std::string l_canonical = std::filesystem::weakly_canonical(_path).string();

    // Remove any existing duplicate
    _state.recentProjects.erase(
        std::remove(_state.recentProjects.begin(), _state.recentProjects.end(), l_canonical),
        _state.recentProjects.end());

    // Insert at front
    _state.recentProjects.insert(_state.recentProjects.begin(), l_canonical);

    // Cap the list
    if ((int)_state.recentProjects.size() > k_maxRecentProjects)
        _state.recentProjects.resize(k_maxRecentProjects);

    SaveRecentProjects(_state);
}

void ProjectSelectionScreen::DrawCentredTitle(const std::string& _text, ImVec2 _displaySize)
{
    float w = ImGui::CalcTextSize(_text.c_str()).x;
    ImGui::SetCursorPosX((_displaySize.x - w) * 0.5f);
    ImGui::TextUnformatted(_text.c_str());
}

bool ProjectSelectionScreen::DrawCentredButton(const std::string& _label, ImVec2 _btnSize)
{
    float w = ImGui::GetIO().DisplaySize.x;
    ImGui::SetCursorPosX((w - _btnSize.x) * 0.5f);
    return ImGui::Button(_label.c_str(), _btnSize);
}

void ProjectSelectionScreen::DrawMain(State& _state, ImVec2 _displaySize)
{
    const float k_leftW  = 300.0f;
    const float k_rightW = _displaySize.x - k_leftW;
    const float k_height = _displaySize.y;
    const float k_pad    = 32.0f;

    // Left sidebar: title + buttons
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.09f, 0.09f, 0.12f, 1.0f));
    if (ImGui::BeginChild("##LeftPanel", ImVec2(k_leftW, k_height), false,
                           ImGuiWindowFlags_NoScrollbar))
    {
        ImGui::SetCursorPosY(k_height * 0.26f);

        ImGui::SetCursorPosX(k_pad);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.94f, 0.94f, 0.98f, 1.0f));
        ImGui::TextUnformatted("Game Dev Intro");
        ImGui::PopStyleColor();

        ImGui::SetCursorPosX(k_pad);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.44f, 0.48f, 0.60f, 1.0f));
        ImGui::TextUnformatted("Project Manager");
        ImGui::PopStyleColor();

        ImGui::Spacing(); ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.22f, 0.26f, 0.38f, 1.0f));
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

        const ImVec2 k_btnSize(k_leftW - k_pad * 2.0f, 40.0f);

        ImGui::SetCursorPosX(k_pad);
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.20f, 0.40f, 0.68f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.27f, 0.50f, 0.80f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.15f, 0.32f, 0.54f, 1.0f));
        if (ImGui::Button("New Project", k_btnSize))
            _state.page = Page::NewProject;
        ImGui::PopStyleColor(3);

        ImGui::Spacing();

        ImGui::SetCursorPosX(k_pad);
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.17f, 0.19f, 0.26f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.26f, 0.38f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.13f, 0.15f, 0.21f, 1.0f));
        if (ImGui::Button("Open Project", k_btnSize))
            _state.page = Page::LoadProject;
        ImGui::PopStyleColor(3);

        // Exit — anchored to bottom of sidebar
        ImGui::SetCursorPosY(k_height - 56.0f);
        ImGui::SetCursorPosX(k_pad);
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.36f, 0.12f, 0.12f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.52f, 0.17f, 0.17f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.28f, 0.09f, 0.09f, 1.0f));
        if (ImGui::Button("Exit", k_btnSize))
        {
            _state.result  = { Action::Exit, Project{} };
            _state.decided = true;
        }
        ImGui::PopStyleColor(3);
    }
    ImGui::EndChild();
    ImGui::PopStyleColor(); // ChildBg

    ImGui::SameLine(0.0f, 0.0f);

    // Right panel: recent projects
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.13f, 0.13f, 0.17f, 1.0f));
    if (ImGui::BeginChild("##RightPanel", ImVec2(k_rightW, k_height), false,
                           ImGuiWindowFlags_NoScrollbar))
    {
        ImGui::SetCursorPosY(36.0f);
        ImGui::SetCursorPosX(k_pad);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.60f, 0.64f, 0.76f, 1.0f));
        ImGui::TextUnformatted("Recent Projects");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.20f, 0.22f, 0.32f, 1.0f));
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        const float l_listW = k_rightW - k_pad * 2.0f;
        const float l_listH = k_height - ImGui::GetCursorPosY() - 10.0f;
        ImGui::SetCursorPosX(k_pad);

        ImGui::PushStyleColor(ImGuiCol_ChildBg,       ImVec4(0.00f, 0.00f, 0.00f, 0.00f));
        ImGui::PushStyleColor(ImGuiCol_Header,        ImVec4(0.17f, 0.19f, 0.28f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.21f, 0.25f, 0.40f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive,  ImVec4(0.20f, 0.38f, 0.62f, 1.0f));
        if (ImGui::BeginChild("##RecentList", ImVec2(l_listW, l_listH), false))
        {
            if (_state.recentProjects.empty())
            {
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.34f, 0.36f, 0.46f, 1.0f));
                ImGui::TextUnformatted("No recent projects.");
                ImGui::PopStyleColor();
            }
            else
            {
                for (int i = 0; i < (int)_state.recentProjects.size(); i++)
                {
                    const std::string& l_projPath = _state.recentProjects[i];
                    std::string l_name = std::filesystem::path(l_projPath).filename().string();

                    ImGui::PushID(i);

                    float l_rowY = ImGui::GetCursorPosY();
                    bool l_clicked = ImGui::Selectable("##row", false,
                                                        ImGuiSelectableFlags_AllowOverlap,
                                                        ImVec2(l_listW, 52.0f));

                    // Project name
                    ImGui::SetCursorPosY(l_rowY + 5.0f);
                    ImGui::SetCursorPosX(10.0f);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.90f, 0.92f, 0.96f, 1.0f));
                    ImGui::TextUnformatted(l_name.c_str());
                    ImGui::PopStyleColor();

                    // Full path (muted)
                    ImGui::SetCursorPosY(l_rowY + 28.0f);
                    ImGui::SetCursorPosX(10.0f);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.38f, 0.41f, 0.52f, 1.0f));
                    ImGui::TextUnformatted(l_projPath.c_str());
                    ImGui::PopStyleColor();

                    ImGui::SetCursorPosY(l_rowY + 54.0f);
                    ImGui::Dummy(ImVec2(0.0f, 0.0f)); // Required: claim the extended boundary

                    if (l_clicked)
                    {
                        if (std::filesystem::is_directory(l_projPath) && Project::IsValid(l_projPath))
                        {
                            AddRecentProject(_state, l_projPath);
                            _state.result  = { Action::StartSandbox, Project(l_projPath) };
                            _state.decided = true;
                        }
                        else
                        {
                            _state.recentProjects.erase(_state.recentProjects.begin() + i);
                            SaveRecentProjects(_state);
                            ImGui::PopID();
                            break;
                        }
                    }

                    ImGui::PopID();
                }
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor(4);
    }
    ImGui::EndChild();
    ImGui::PopStyleColor(); // ChildBg
}

void ProjectSelectionScreen::DrawNewProject(State& _state, ImVec2 _displaySize)
{
    ImGui::SetCursorPosY(_displaySize.y * 0.28f);

    DrawCentredTitle("Start New Project", _displaySize);
    ImGui::Spacing(); ImGui::Spacing();

    float l_labelW  = 160.0f;
    float l_inputW  = _displaySize.x * 0.45f;
    float l_browseW = 100.0f;
    float l_rowX    = (_displaySize.x - l_labelW - 8.0f - l_inputW - 8.0f - l_browseW) * 0.5f;

    ImGui::SetCursorPosX(l_rowX);
    ImGui::Text("Location:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(l_inputW);
    ImGui::InputText("##NewProjLocation", &_state.loadDir);
    ImGui::SameLine(0.0f, 8.0f);
    if (ImGui::Button("Browse...##newloc", ImVec2(l_browseW, 0)))
    {
        IGFD::FileDialogConfig cfg;
        cfg.path = _state.loadDir.empty() ? m_documentsPath : _state.loadDir;
        ImGuiFileDialog::Instance()->OpenDialog(g_newProjectDialogName, "Choose Location", nullptr, cfg);
    }

    ImGui::Spacing();

    ImGui::SetCursorPosX(l_rowX);
    ImGui::Text("Project Name:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(l_inputW);
    ImGui::InputText("##NewProjName", &_state.name);

    ImGui::Spacing();

    ImGui::SetCursorPosX(l_rowX);
    ImGui::Text("Mode:");
    ImGui::SameLine();
    
    //TODO: eventually replace with class members visual selection (cards with descriptions), but this will do for now.
    static std::vector<std::string> l_modes = { "Sandbox", "Tutorial" };
    static int l_currentMode = 0;

    if(ImGui::BeginCombo("##combo", l_modes[l_currentMode].c_str()))
    {
        for (int i = 0; i < l_modes.size(); i++)
        {
            bool isSelected = (l_currentMode == i);
            if (ImGui::Selectable(l_modes[i].c_str(), isSelected))
            {
                l_currentMode = i;
                _state.tutorial = (i == 1);
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Spacing();

    if (!_state.errorMsg.empty())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        float l_errW = ImGui::CalcTextSize(_state.errorMsg.c_str()).x;
        ImGui::SetCursorPosX((_displaySize.x - l_errW) * 0.5f);
        ImGui::TextUnformatted(_state.errorMsg.c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }

    float l_pairW = 240.0f * 2.0f + 16.0f;
    ImGui::SetCursorPosX((_displaySize.x - l_pairW) * 0.5f);

    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.20f, 0.40f, 0.68f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.27f, 0.50f, 0.80f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.15f, 0.32f, 0.54f, 1.0f));
    if (ImGui::Button("Create Project", ImVec2(240.0f, 50.0f)))
    {
        _state.errorMsg.clear();
        if (_state.loadDir.empty())
            _state.errorMsg = "Please choose a location.";
        else if (_state.name.empty())
            _state.errorMsg = "Please enter a project name.";
        else
        {
            try
            {
#if _WIN32
				std::string l_projectPath = _state.loadDir;
                std::filesystem::path l_fullPath(l_projectPath);
				l_fullPath.append(_state.name.begin(), _state.name.end());
#else
                std::filesystem::path l_fullPath = std::filesystem::path(_state.loadDir) / _state.name;
#endif
                Project l_project   = Project::Create(l_fullPath.string());
                AddRecentProject(_state, l_fullPath.string());
                _state.result  = { _state.tutorial ? Action::StartTutorial
                                                 : Action::StartSandbox,
                                  l_project };
                _state.decided = true;
            }
            catch (const std::exception& e)
            {
                _state.errorMsg = std::string("Could not create project: ") + e.what();
            }
        }
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine(0.0f, 16.0f);

    if (ImGui::Button("Back", ImVec2(240.0f, 50.0f)))
    {
        _state.page     = Page::Main;
        _state.errorMsg.clear();
    }
}

void ProjectSelectionScreen::DrawLoadProject(State& _state, ImVec2 _displaySize)
{
    ImGui::SetCursorPosY(_displaySize.y * 0.32f);

    DrawCentredTitle("Load Project", _displaySize);
    ImGui::Spacing(); ImGui::Spacing();

    float l_labelW  = 180.0f;
    float l_inputW  = _displaySize.x * 0.45f;
    float l_browseW = 100.0f;
    float l_rowX    = (_displaySize.x - l_labelW - 8.0f - l_inputW - 8.0f - l_browseW) * 0.5f;

    ImGui::SetCursorPosX(l_rowX);
    ImGui::Text("Project Directory:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(l_inputW);
    ImGui::InputText("##LoadProjDir", &_state.loadDir);
    ImGui::SameLine(0.0f, 8.0f);
    if (ImGui::Button("Browse...##loaddir", ImVec2(l_browseW, 0)))
    {
        IGFD::FileDialogConfig cfg;
        ImGuiFileDialog::Instance()->OpenDialog(g_loadProjectDialogName, "Choose Project Directory", nullptr, cfg);
    }

    ImGui::Spacing();

    if (!_state.errorMsg.empty())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        float l_errW = ImGui::CalcTextSize(_state.errorMsg.c_str()).x;
        ImGui::SetCursorPosX((_displaySize.x - l_errW) * 0.5f);
        ImGui::TextUnformatted(_state.errorMsg.c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }

    float l_pairW = 240.0f * 2.0f + 16.0f;
    ImGui::SetCursorPosX((_displaySize.x - l_pairW) * 0.5f);

    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.20f, 0.40f, 0.68f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.27f, 0.50f, 0.80f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.15f, 0.32f, 0.54f, 1.0f));
    if (ImGui::Button("Load", ImVec2(240.0f, 50.0f)))
    {
        _state.errorMsg.clear();
        std::string l_dir(_state.loadDir);
        if (l_dir.empty())
            _state.errorMsg = "Please choose a project directory.";
        else if (!std::filesystem::exists(l_dir))
            _state.errorMsg = "Directory does not exist.";
        else if (!Project::IsValid(l_dir))
            _state.errorMsg = "Not a valid project (missing Categories / Rules / Scenes).";
        else
        {
            AddRecentProject(_state, l_dir);
            _state.result  = { Action::StartSandbox, Project(l_dir) };
            _state.decided = true;
        }
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine(0.0f, 16.0f);

    if (ImGui::Button("Back", ImVec2(240.0f, 50.0f)))
    {
        _state.page     = Page::Main;
        _state.errorMsg.clear();
    }
}

void ProjectSelectionScreen::DrawDirDialogs(State& _state)
{
    std::string l_dialogToCheck = (_state.page == Page::NewProject) ? g_newProjectDialogName : g_loadProjectDialogName;
    if (ImGuiFileDialog::Instance()->IsOpened(l_dialogToCheck))
    {
        if(ImGuiFileDialog::Instance()->Display(l_dialogToCheck, ImGuiWindowFlags_NoCollapse, ImVec2(700, 400)))
        {
            std::string l_picked = ImGuiFileDialog::Instance()->GetCurrentPath();
            _state.loadDir = l_picked;
            ImGuiFileDialog::Instance()->Close();
        }
    }

    
}

ProjectSelectionScreen::Result
ProjectSelectionScreen::Run(RE::EngineContents& _engineContents)
{
    // Create temp imgui context for this screen.
    // The editor will create its own context when it starts
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    auto l_window = _engineContents.io->GetWindow().lock();
    ImGui_ImplSDL3_InitForOpenGL(l_window->GetSDLWindow(), l_window->GetGLContext());
    ImGui_ImplOpenGL3_Init("#version 430");

    ImFontConfig l_fontConfig;
    l_fontConfig.FontDataOwnedByAtlas = false;
    ImFont* l_font = io.Fonts->AddFontFromMemoryTTF((void*)RE::UI::DefaultFontData(), RE::UI::DefaultFontDataSize(), 16.0f, &l_fontConfig);

    State l_state;
    LoadRecentProjects(l_state);

    // Find documents path.
#if _WIN32
        char* l_path = std::getenv("USER");
        if (l_path)
        {
            m_documentsPath = std::string(l_path) + "/Documents";
        }
        else
        {
            // For some reason, the program cannot find USER/Documents on uni pc, though that's probably a network issue
            m_documentsPath = "C:/Users/Public/Documents";
        }
#else
        const char* l_homeChar = std::getenv("HOME");
        m_documentsPath = std::string(l_homeChar) + "/Documents";;
#endif

    //Default load dir to documents, since that's sensible 
    l_state.loadDir = m_documentsPath;

    while (!l_state.decided && !_engineContents.io->GetQuitSignal())
    {
        std::vector<SDL_Event> l_events = _engineContents.io->UpdateInputs();
        for (const SDL_Event& ev : l_events)
            ImGui_ImplSDL3_ProcessEvent(&ev);

        glClearColor(0.09f, 0.09f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        ImGui::PushFont(l_font, 17.5f);

        ImVec2 l_displaySize = io.DisplaySize;

        // Fullscreen host window
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(l_displaySize);
        ImGui::SetNextWindowBgAlpha(1.0f);

        ImGuiWindowFlags l_flags =
            ImGuiWindowFlags_NoTitleBar        |
            ImGuiWindowFlags_NoResize          |
            ImGuiWindowFlags_NoMove            |
            ImGuiWindowFlags_NoScrollbar       |
            ImGuiWindowFlags_NoSavedSettings   |
            ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.09f, 0.09f, 0.12f, 1.0f));
        ImGui::Begin("##ProjectSelectionScreen", nullptr, l_flags);

        switch (l_state.page)
        {
            case Page::Main:
                DrawMain       (l_state, l_displaySize);
                break;
            case Page::NewProject:
                DrawNewProject (l_state, l_displaySize);
                break;
            case Page::LoadProject:
                DrawLoadProject(l_state, l_displaySize);
                break;
        }

        ImGui::End();
        ImGui::PopStyleColor();

        DrawDirDialogs(l_state);

        ImGui::PopFont();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        l_window->Swap();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    return l_state.result;
}
