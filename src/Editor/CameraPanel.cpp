#include "Editor/CameraPanel.h"
#include "Editor/Panel.h"
#include "Editor/Editor.h"

#include "RanakEngine/Math.h"

#include "imgui/imgui.h"

CameraPanel::CameraPanel(Editor& _editor)
: Panel("Camera Settings", _editor)
{
    m_camera = _editor.GetEngineContents().core->GetCamera();
}

CameraPanel::~CameraPanel()
{

}

void CameraPanel::Draw()
{
    //ImGui::SetWindowSize(ImVec2(300, 150));
    auto l_camera = m_camera.lock();

    Vector3 l_cameraPos = l_camera->GetPosition();
    
    ImGui::DragFloat3("Position", &l_cameraPos.x, 0.1f, -std::numeric_limits<float>().infinity(), std::numeric_limits<float>().infinity(), "%.3f");

    l_camera->SetPosition(l_cameraPos);

    float l_rotation = l_camera->GetRotation();

    ImGui::DragFloat("Rotation", &l_rotation, 0.1f, -360.0f, 360.0f, "%.3f");

    l_camera->SetRotation(l_rotation);

    bool l_perspective = l_camera->IsPerspective();
    
    ImGui::Checkbox("Perspective", &l_perspective);

    if(l_perspective)
    {
        l_camera->SetPerspective();
        float l_fov = l_camera->GetFOV();

        ImGui::DragFloat("FOV", &l_fov, 0.1f, 0.0f, 360.0f, "%0.3f");

        l_camera->SetFOV(l_fov);
    }
    else
    {
        l_camera->SetOrthographic();
        float l_cameraWidth = l_camera->GetCameraWidth();

        ImGui::DragFloat("Camera Size", &l_cameraWidth, 0.1f, 0.0001f, std::numeric_limits<float>().infinity(), "%.3f");

        l_camera->SetCameraWidth(l_cameraWidth);
    }
}