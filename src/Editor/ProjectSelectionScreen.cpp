#include "Editor/ProjectSelectionScreen.h"

#include "RanakEngine/IO.h"

#include "SDL3/SDL.h"
#include <GL/gl.h>
#include <cstdlib>
#include <cstring>
#include <filesystem>

static std::string g_newProjectDialogName("PSS_NewProjectDir");
static std::string g_loadProjectDialogName("PSS_LoadProjectDir");

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
    // Centre the block vertically
    float l_blockH = 60.0f + 20.0f + 3.0f * 50.0f + 2.0f * 12.0f;
    ImGui::SetCursorPosY((_displaySize.y - l_blockH) * 0.5f);

    DrawCentredTitle("Game Dev Intro", _displaySize);
    ImGui::Spacing(); ImGui::Spacing();

    ImVec2 l_btnPosition(280.0f, 50.0f);

    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.20f, 0.45f, 0.75f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.55f, 0.90f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.15f, 0.35f, 0.60f, 1.0f));

    if (DrawCentredButton("Start New Project", l_btnPosition))
        _state.page = Page::NewProject;
    ImGui::Spacing();
    if (DrawCentredButton("Load Project", l_btnPosition))
        _state.page = Page::LoadProject;

    ImGui::PopStyleColor(3);
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.60f, 0.18f, 0.18f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.25f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.45f, 0.12f, 0.12f, 1.0f));
    if (DrawCentredButton("Exit", l_btnPosition))
    {
        _state.result  = { Action::Exit, Project{} };
        _state.decided = true;
    }
    ImGui::PopStyleColor(3);
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
    ImGui::InputText("##NewProjLocation", &_state.location);
    ImGui::SameLine(0.0f, 8.0f);
    if (ImGui::Button("Browse...##newloc", ImVec2(l_browseW, 0)))
    {
        IGFD::FileDialogConfig cfg;
        const char* l_home = std::getenv("HOME");
        std::string l_docsPath = l_home ? std::string(l_home) + "/Documents" : ".";
        cfg.path = _state.location.empty() ? l_docsPath : _state.location;
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
    if (ImGui::Checkbox("Tutorial", &_state.tutorial))  _state.tutorial = !_state.tutorial;

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

    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.18f, 0.55f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.70f, 0.35f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.12f, 0.40f, 0.18f, 1.0f));
    if (ImGui::Button("Create Project", ImVec2(240.0f, 50.0f)))
    {
        _state.errorMsg.clear();
        if (_state.location.empty())
            _state.errorMsg = "Please choose a location.";
        else if (_state.name.empty())
            _state.errorMsg = "Please enter a project name.";
        else
        {
            try
            {
                std::filesystem::path l_fullPath = std::filesystem::path(_state.location) / _state.name;
                Project l_project   = Project::Create(l_fullPath.string());
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
        const char* l_home = std::getenv("HOME");
        std::string l_docsPath = l_home ? std::string(l_home) + "/Documents" : ".";
        cfg.path = _state.loadDir.empty() ? l_docsPath : _state.loadDir;
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

    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.20f, 0.45f, 0.75f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.55f, 0.90f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.15f, 0.35f, 0.60f, 1.0f));
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
    if (ImGuiFileDialog::Instance()->Display(
            g_newProjectDialogName,
            ImGuiWindowFlags_NoCollapse,
            ImVec2(700, 450)))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string l_picked = ImGuiFileDialog::Instance()->GetCurrentPath();
            _state.location = l_picked;
        }
        ImGuiFileDialog::Instance()->Close();
    }

    if (ImGuiFileDialog::Instance()->Display(
            g_loadProjectDialogName,
            ImGuiWindowFlags_NoCollapse,
            ImVec2(700, 450)))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string l_picked = ImGuiFileDialog::Instance()->GetCurrentPath();
            _state.loadDir = l_picked;
        }
        ImGuiFileDialog::Instance()->Close();
    }
}

ProjectSelectionScreen::Result
ProjectSelectionScreen::Run(RE::EngineContents& _engineContents)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    auto l_window = _engineContents.io->GetWindow().lock();
    ImGui_ImplSDL3_InitForOpenGL(l_window->GetSDLWindow(), l_window->GetGLContext());
    ImGui_ImplOpenGL3_Init("#version 430");

    ImFont* l_font = io.Fonts->AddFontFromFileTTF("./resources/Fonts/MapleMono.ttf");

    State l_state;

    while (!l_state.decided && !_engineContents.io->GetQuitSignal())
    {
        std::vector<SDL_Event> l_events = _engineContents.io->UpdateInputs();
        for (const SDL_Event& ev : l_events)
            ImGui_ImplSDL3_ProcessEvent(&ev);

        glClearColor(0.08f, 0.08f, 0.12f, 1.0f);
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
            ImGuiWindowFlags_NoScrollbar        |
            ImGuiWindowFlags_NoSavedSettings   |
            ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.12f, 1.0f));
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
