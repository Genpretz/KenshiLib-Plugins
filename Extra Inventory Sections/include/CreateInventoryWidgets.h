#pragma once

#include <mygui/MyGUI_Widget.h>
#include "InventoryWidgetsConfig.h"
#include <vector>
#include <string>

void CreateInventoryWidgets(
    MyGUI::Widget* parent,
    const std::string& prefix,
    const std::vector<WidgetInfo>& widgets);


