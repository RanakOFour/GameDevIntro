#include "Editor/Editor.h"
#include "Editor/EntityPanel.h"
#include "Editor/CategoryPanel.h"
#include "Editor/RulesPanel.h"
#include "Editor/PropertiesPanel.h"
#include "RanakEngine/IO.h"

#include "SDL3/SDL.h"
#include <GL/gl.h>

Editor::Editor()
: m_selectedEntityId(-1)
, m_isRunning(true)
{
    // Initialize the engine (this creates the SDL window and GL context)
    m_engineContents = RE::Initialise(true, Vector2(1920, 1080));
    RE::Log::Message("Engine Initialised for Editor");

    // Get the window from the IO Manager
    auto ioManager = m_engineContents.io;
    m_window = ioManager->GetWindow().lock();
    if (!m_window)
    {
        RE::Log::Error("Failed to get window from IO Manager");
        m_isRunning = false;
        return;
    }

    // Get the scene
    m_scene = m_engineContents.core->GetScene().lock();

    // Initialize ImGui with the window from IO Manager
    InitImGui();

    RE::Log::Message("Editor constructed");
}

std::shared_ptr<Editor> Editor::Create()
{
    auto editor = std::make_shared<Editor>();
    
    // Now that editor is in a shared_ptr, we can initialize panels
    auto editorPtr = editor->shared_from_this();
    
    editor->m_entityPanel = std::make_unique<EntityPanel>(editorPtr);
    editor->m_categoryPanel = std::make_unique<CategoryPanel>(editorPtr);
    editor->m_rulesPanel = std::make_unique<RulesPanel>(editorPtr);
    editor->m_propertiesPanel = std::make_unique<PropertiesPanel>(editorPtr);

    RE::Log::Message("Editor initialized with UI panels");
    
    return editor;
}

Editor::~Editor()
{
    // Clean up ImGui
    CleanupImGui();

    // Shut down engine
    RE::Shutdown(m_engineContents);

    // Log is shutdown, so now we print
    printf("Editor shutdown complete\n");
}

void Editor::InitImGui()
{
    // Get ImGui context from IO Manager
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    // Enable mouse drag for window operations
    io.ConfigDragClickToInputText = 0.0f;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Tweak style for better docking
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowMenuButtonPosition = ImGuiDir_Right;

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(m_window->GetSDLWindow(), m_window->GetGLContext());
    ImGui_ImplOpenGL3_Init("#version 430");

    RE::Log::Message("ImGui initialized");
}

void Editor::CleanupImGui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void Editor::Run()
{
    while (m_isRunning)
    {
        HandleInput();
        Update(1.0f / 60.0f); // Assume 60 FPS
        Render();
    }
}

void Editor::Update(float _deltaTime)
{
    m_scene->Update(_deltaTime);
}

void Editor::Render()
{
    // Clear the screen
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    RenderDockspace();
    RenderMenuBar();
    RenderEditorUI();

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    m_window->Swap();
}

void Editor::RenderMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Scene", "Ctrl+N"))
            {
                // TODO: Implement new scene
            }
            if (ImGui::MenuItem("Load Scene", "Ctrl+O"))
            {
                // TODO: Implement load scene
            }
            if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
            {
                // TODO: Implement save scene
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Ctrl+Q"))
            {
                m_isRunning = false;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "Ctrl+Z"))
            {
                // TODO: Implement undo
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Y"))
            {
                // TODO: Implement redo
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Show Grid");
            ImGui::MenuItem("Show Gizmos");
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void Editor::RenderDockspace()
{
    ImGuiDockNodeFlags l_dockFlags = ImGuiDockNodeFlags_PassthruCentralNode;

    ImGuiWindowFlags l_windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    l_windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    l_windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, l_windowFlags);
    ImGui::PopStyleVar();

    ImGui::PopStyleVar(2);

    // DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID l_dockID = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(l_dockID, ImVec2(0.0f, 0.0f), l_dockFlags);
    }

    ImGui::End();
}

void Editor::RenderEditorUI()
{
    // Render all panels
    if (m_entityPanel) m_entityPanel->Draw();
    if (m_categoryPanel) m_categoryPanel->Draw();
    if (m_rulesPanel) m_rulesPanel->Draw();
    if (m_propertiesPanel) m_propertiesPanel->Draw();
}

void Editor::HandleInput()
{
    SDL_Event l_event;
    while (SDL_PollEvent(&l_event))
    {
        ImGui_ImplSDL3_ProcessEvent(&l_event);

        switch(l_event.type)
        {
            case SDL_EVENT_QUIT:
            m_isRunning = false;
            break;
            
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            if (l_event.window.windowID == SDL_GetWindowID(m_window->GetSDLWindow()))
            {
                m_isRunning = false;
            }
            break;
        }
    }
}
