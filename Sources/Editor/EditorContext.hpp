#pragma once

#include <Termina/Core/IInspectable.hpp>

class ContentViewerPanel;

struct EditorContext
{
    Termina::IInspectable* ItemToInspect = nullptr;
    ContentViewerPanel* ContentViewer = nullptr;
};
