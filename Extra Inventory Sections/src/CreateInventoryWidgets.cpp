#include "CreateInventoryWidgets.h"

#include <mygui/MyGUI.h>
#include <mygui/MyGUI_Widget.h>
#include <mygui/MyGUI_TextBox.h>

void CreateInventoryWidgets(
    MyGUI::Widget* parent,
    const std::string& prefix,
    const std::vector<WidgetInfo>& widgets)
{
    for (size_t i = 0; i < widgets.size(); ++i)
    {
        const WidgetInfo& w = widgets[i];
        MyGUI::FloatCoord coord(w.x, w.y, w.width, w.height);

        if (w.label.empty())
        {
            // Slot widget
            parent->createWidgetReal<MyGUI::Widget>(
                "Kenshi_InventorySlotSkin",
                coord,
                MyGUI::Align::Default,
                prefix + w.name
            );
        }
        else
        {
            // Label widget
            MyGUI::TextBox* lb = parent->createWidgetReal<MyGUI::TextBox>(
                "Kenshi_TextboxPaintedText",
                coord,
                MyGUI::Align::Default,
                prefix + w.name
            );

            if (lb)
            {
                lb->setCaption(w.label);
                lb->setTextAlign(MyGUI::Align::Center);
            }
        }
    }
}