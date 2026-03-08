#include "InventoryWidgetsConfig.h"

#include <fstream>
#include <sstream>

#include <rapidjson/document.h>

static bool ReadFile(const std::string& path, std::string& out)
{
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file.is_open())
        return false;

    std::ostringstream ss;
    ss << file.rdbuf();
    out = ss.str();
    return true;
}

static bool ParseWidget(const rapidjson::Value& v, WidgetInfo& out)
{
    if (!v.IsObject())                                              return false;
    if (!v.HasMember("name")   || !v["name"].IsString())           return false;
    if (!v.HasMember("x")      || !v["x"].IsNumber())              return false;
    if (!v.HasMember("y")      || !v["y"].IsNumber())              return false;
    if (!v.HasMember("width")  || !v["width"].IsNumber())          return false;
    if (!v.HasMember("height") || !v["height"].IsNumber())         return false;

    out.name   = v["name"].GetString();
    out.x      = v["x"].GetFloat();
    out.y      = v["y"].GetFloat();
    out.width  = v["width"].GetFloat();
    out.height = v["height"].GetFloat();
    out.label  = v.HasMember("label") && v["label"].IsString()
                    ? v["label"].GetString()
                    : "";

    return true;
}

bool LoadWidgetsFromFile(const std::string& path, std::vector<WidgetInfo>& outWidgets)
{
    outWidgets.clear();

    std::string json;
    if (!ReadFile(path, json))
        return false;

    rapidjson::Document doc;
    doc.Parse(json.c_str());

    if (doc.HasParseError() || !doc.IsObject())
        return false;

    if (!doc.HasMember("inventoryWidgets") ||
        !doc["inventoryWidgets"].IsObject())
        return false;

    const rapidjson::Value& root = doc["inventoryWidgets"];

    if (!root.HasMember("widgets") || !root["widgets"].IsArray())
        return false;

    const rapidjson::Value& arr = root["widgets"];

    for (rapidjson::SizeType i = 0; i < arr.Size(); ++i)
    {
        WidgetInfo w;
        if (!ParseWidget(arr[i], w))
            return false;

        outWidgets.push_back(w);
    }

    return !outWidgets.empty();
}
