#pragma once

#include <string>
#include <vector>

struct WidgetInfo
{
    std::string name;
    float       x;
    float       y;
    float       width;
    float       height;
    std::string label;      // empty if this is a slot widget, non-empty if label widget
};

bool LoadWidgetsFromFile(const std::string& path, std::vector<WidgetInfo>& outWidgets);
