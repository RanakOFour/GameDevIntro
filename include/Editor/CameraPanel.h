#ifndef CAMERAOPTIONPANEL_H
#define CAMERAOPTIONPANEL_H

#include "Editor/Panel.h"

#include "RanakEngine/RanakEngine.h"

#include <memory>

class Editor;
class CameraPanel : public Panel
{
    private:
    std::weak_ptr<RE::Core::Camera> m_camera;

    public:
    CameraPanel(std::weak_ptr<Editor> _editor);
    ~CameraPanel();

    void Draw();
};

#endif