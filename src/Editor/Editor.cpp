#include "Editor/Editor.h"
#include "Editor/EntityPanel.h"
#include "Editor/CategoryPanel.h"
#include "Editor/RulesPanel.h"
#include "Editor/CameraPanel.h"

#include "RanakEngine/IO.h"
#include "RanakEngine/Core.h"

#include "SDL3/SDL.h"
#include <GL/gl.h>

Editor::Editor()
: m_selectedEntityId(-1)
, m_isEditorRunning(true)
{
    // Initialize the engine (this creates the SDL window and GL context)
    m_engineContents = RE::Initialise(true, Vector2(1920, 1080));
    RE::Log::Message("Engine Initialised for Editor");

    // Get the scene
    m_scene = m_engineContents.core->GetScene().lock();
    m_camera = m_engineContents.core->GetCamera().lock();

    m_gridShader = m_engineContents.resources->Load<RE::Asset::Shader>("./resources/Shaders/infinite_grid/frag.fs;./resources/Shaders/infinite_grid/vertd.vs").lock();

    auto l_renderRuleFile = m_engineContents.resources->Load<RE::Asset::LuaFile>("./resources/Rules/EditorRender.lua");
    RE::Core::Rule l_renderRule = m_engineContents.core->GetLuaContext()->RunScript<RE::Core::Rule>(l_renderRuleFile);

    m_scene->AddRule(l_renderRule);

    // Get the window from the IO Manager
    auto ioManager = m_engineContents.io;
    m_window = ioManager->GetWindow().lock();
    if (!m_window)
    {
        RE::Log::Error("Failed to get window from IO Manager");
        m_isEditorRunning = false;
        return;
    }

    // Initialize ImGui with the window from IO Manager
    InitImGui();

    RE::Log::Message("Editor constructed");
}

std::shared_ptr<Editor> Editor::Create()
{
    std::shared_ptr<Editor> l_editor = std::make_shared<Editor>();
    
    l_editor->m_entityPanel = std::make_unique<EntityPanel>(l_editor);
    l_editor->m_categoryPanel = std::make_unique<CategoryPanel>(l_editor);
    l_editor->m_rulesPanel = std::make_unique<RulesPanel>(l_editor);
    l_editor->m_cameraPanel = std::make_unique<CameraPanel>(l_editor);

    RE::Log::Message("Editor initialized with UI panels");
    
    return l_editor;
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
    float l_oneSixtieth = 1.0f / 60.0f;
    while (m_isEditorRunning)
    {
        if(m_isGameRunning)
        {
            Update(l_oneSixtieth);
        }
        else
        {
            HandleInput();
            Draw();
        }
    }

    printf("Editor no longer running\n");

    m_categoryPanel.reset();
    m_entityPanel.reset();
    m_rulesPanel.reset();
    m_cameraPanel.reset();
}

void Editor::Update(float _deltaTime)
{
    m_scene->Update(_deltaTime);
}

void Editor::Draw()
{
    // Clear the screen
    glClearColor(0.2f, 0.2f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw grid

    // Render the infinite grid first (before ImGui)
    if (m_gridShader)
    //if(false)
    {
        GLuint emptyVAO = 0;
        glGenVertexArrays(1, &emptyVAO);
        glBindVertexArray(emptyVAO);

        // Disable depth testing for grid
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        m_gridShader->Use();
        m_gridShader->SetUniform("u_Projection", m_camera->GetProjection());
        m_gridShader->SetUniform("u_View", m_camera->GetView());
        m_gridShader->SetUniform("u_cameraPos", m_camera->GetPosition());

        float l_cameraWidth = m_camera->GetCameraWidth();
        Vector2 l_viewportSize = m_window->GetScreenSize();
        float l_aspectRatio = l_viewportSize.x / l_viewportSize.y;
        
        // Calculate the height based on the width and viewport aspect ratio to prevent stretching
        float l_orthoHeight = l_cameraWidth / l_aspectRatio;

        m_gridShader->SetUniform("u_cameraSize", Vector2(l_cameraWidth, l_orthoHeight));

        glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, 1, 0);

        // Re-enable depth testing
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        glBindVertexArray(0);
        glDeleteVertexArrays(1, &emptyVAO);
        glUseProgram(0);
    }

    // Draw scene with EditorRenderer
    m_scene->Draw();

    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    //RenderDockspace();
    DrawMenuBar();
    DrawEditorUI();

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    m_window->Swap();
}

void Editor::DrawMenuBar()
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
                m_isEditorRunning = false;
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

            if(ImGui::MenuItem("Camera Settings"))
            {
                m_cameraPanel->SetShown(true);
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}



void Editor::DrawEditorUI()
{
    // Context menu
    if(ImGui::BeginPopupContextVoid("ContextMenu", ImGuiPopupFlags_MouseButtonRight))
    {
        if(ImGui::Button("Create Entity", ImVec2(140, 20)))
        {
            Vector3 l_entityPos = m_engineContents.core->ScreenToWorldPoint(m_mouseInfo.position);
            l_entityPos.z = 0.0f;

            m_entityPanel->SetShown(true);
            m_entityPanel->AddEntity();
        }

        if(m_selectedEntityId > -1)
        {
            if(ImGui::Button("Delete Entity", ImVec2(140, 20)))
            {
                m_scene->RemoveEntity(m_selectedEntityId);
                m_selectedEntityId = -1;
            }
        }

        if(!m_cameraPanel->IsShown())
        {
            if(ImGui::Button("Show Camera Settings", ImVec2(140, 20)))
            {
                m_cameraPanel->SetShown(true);
            }
        }


        if(!m_entityPanel->IsShown())
        {
            if(ImGui::Button("Show Entity List", ImVec2(140, 20)))
            {
                m_entityPanel->SetShown(true);
            }
        }

        if(!m_categoryPanel->IsShown())
        {
            if(ImGui::Button("Show Category List", ImVec2(140, 20)))
            {
                m_categoryPanel->SetShown(true);
            }
        }

        if(!m_rulesPanel->IsShown())
        {
            if(ImGui::Button("Show Rules List", ImVec2(140, 20)))
            {
                m_rulesPanel->SetShown(true);
            }
        }

        ImGui::EndPopup();
    }

    // Draw panels
    m_entityPanel->Draw();
    m_categoryPanel->Draw();
    m_rulesPanel->Draw();
    m_cameraPanel->Draw();
}

void Editor::HandleInput()
{
    SDL_Event l_event;

    m_mouseInfo.deltaPosition.x = 0.0f;
    m_mouseInfo.deltaPosition.y = 0.0f;
    m_mouseInfo.deltaScroll = 0.0f;

    bool l_resized = false;

    while (SDL_PollEvent(&l_event))
    {
        ImGui_ImplSDL3_ProcessEvent(&l_event);

        switch(l_event.type)
        {
            case SDL_EVENT_QUIT:
                m_isEditorRunning = false;
            break;
            
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                if (l_event.window.windowID == SDL_GetWindowID(m_window->GetSDLWindow()))
                {
                    m_isEditorRunning = false;
                }
            break;

            case SDL_EVENT_KEY_DOWN:
                // Discard key events when ImGui input field is active
                if (ImGui::GetIO().WantTextInput)
                {
                    break;
                }
                
                if (l_event.key.key == SDLK_ESCAPE)
                {
                    m_rulesPanel->SetShown(false);
                    m_entityPanel->SetShown(false);
                    m_categoryPanel->SetShown(false);
                }
                else if(l_event.key.key == SDLK_C)
                {
                    m_categoryPanel->SetShown(!m_categoryPanel->IsShown());
                }
                else if(l_event.key.key == SDLK_E)
                {
                    m_entityPanel->SetShown(!m_entityPanel->IsShown());
                }
                else if(l_event.key.key == SDLK_R)
                {
                    m_rulesPanel->SetShown(!m_rulesPanel->IsShown());
                }
                
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (l_event.button.button == SDL_BUTTON_LEFT)
                {
                    m_mouseInfo.LMBDown = true;
                }
                else
                {
                    m_mouseInfo.RMBDown = true;
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (l_event.button.button == SDL_BUTTON_LEFT)
                {
                    m_mouseInfo.LMBDown = false;
                }
                else
                {
                    m_mouseInfo.RMBDown = false;
                }
                break;

            case SDL_EVENT_MOUSE_WHEEL:
                m_mouseInfo.deltaScroll = -l_event.wheel.y;
                break;

            case SDL_EVENT_MOUSE_MOTION:
                m_mouseInfo.deltaPosition.x = l_event.motion.xrel;
                m_mouseInfo.deltaPosition.y = l_event.motion.yrel;
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                l_resized = true;
        }
    }

    if(l_resized)
    {
        int l_w, l_h;
        SDL_GetWindowSizeInPixels(m_window->GetSDLWindow(), &l_w, &l_h);
        m_window->SetScreenSize(Vector2(l_w, l_h));
    }

    SDL_GetMouseState(&m_mouseInfo.position.x, &m_mouseInfo.position.y);

    // Flip Y position so 0,0 is bottom left
    m_mouseInfo.position.y = m_window->GetScreenSize().y - m_mouseInfo.position.y;

    RE::Log::Message("Input handled");
}
