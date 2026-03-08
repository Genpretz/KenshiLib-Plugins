#include "InventorySectionsConfig.h"

#include <cstring>
#include <fstream>
#include <sstream>

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

// -----------------------------
// Slot name -> AttachSlot mapping
// -----------------------------

struct SlotMapEntry
{
    const char* name;
    AttachSlot  slot;
};

static const SlotMapEntry kSlotTable[] =
{
    { "ATTACH_WEAPON",    ATTACH_WEAPON    },
    { "ATTACH_BACK",      ATTACH_BACK      },
    { "ATTACH_HAIR",      ATTACH_HAIR      },
    { "ATTACH_HAT",       ATTACH_HAT       },
    { "ATTACH_EYES",      ATTACH_EYES      },
    { "ATTACH_BODY",      ATTACH_BODY      },
    { "ATTACH_LEGS",      ATTACH_LEGS      },
    { "ATTACH_NONE",      ATTACH_NONE      },
    { "ATTACH_SHIRT",     ATTACH_SHIRT     },
    { "ATTACH_BOOTS",     ATTACH_BOOTS     },
    { "ATTACH_GLOVES",    ATTACH_GLOVES    },
    { "ATTACH_NECK",      ATTACH_NECK      },
    { "ATTACH_BACKPACK",  ATTACH_BACKPACK  },
    { "ATTACH_BEARD",     ATTACH_BEARD     },
    { "ATTACH_BELT",      ATTACH_BELT      },
    { "ATTACH_LEFT_ARM",  ATTACH_LEFT_ARM  },
    { "ATTACH_RIGHT_ARM", ATTACH_RIGHT_ARM },
    { "ATTACH_LEFT_LEG",  ATTACH_LEFT_LEG  },
    { "ATTACH_RIGHT_LEG", ATTACH_RIGHT_LEG }
};

static bool MapSlotNameToAttachSlot(const char* name, AttachSlot& out)
{
    if (!name)
        return false;

    const size_t count = sizeof(kSlotTable) / sizeof(kSlotTable[0]);

    for (size_t i = 0; i < count; ++i)
    {
        if (_stricmp(name, kSlotTable[i].name) == 0)
        {
            out = kSlotTable[i].slot;
            return true;
        }
    }

    return false;
}

// -----------------------------
// JSON parsing
// -----------------------------

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

static bool ParseSection(const rapidjson::Value& v, SectionInfo& out)
{
    if (!v.IsObject())
        return false;

    if (!v.HasMember("name") || !v["name"].IsString())         return false;
    if (!v.HasMember("width") || !v["width"].IsInt())           return false;
    if (!v.HasMember("height") || !v["height"].IsInt())          return false;
    if (!v.HasMember("slot") || !v["slot"].IsString())         return false;
    if (!v.HasMember("equipCallbacks") || !v["equipCallbacks"].IsBool()) return false;
    if (!v.HasMember("isContainerSlot") || !v["isContainerSlot"].IsBool())return false;
    if (!v.HasMember("enabled") || !v["enabled"].IsBool())        return false;
    if (!v.HasMember("limit") || !v["limit"].IsInt())           return false;

    out.name = v["name"].GetString();
    out.width = v["width"].GetInt();
    out.height = v["height"].GetInt();
    out.equipCallbacks = v["equipCallbacks"].GetBool();
    out.isContainerSlot = v["isContainerSlot"].GetBool();
    out.enabled = v["enabled"].GetBool();
    out.limit = v["limit"].GetInt();

    if (!MapSlotNameToAttachSlot(v["slot"].GetString(), out.slot))
        return false;

    return true;
}

bool LoadFromFile(const std::string& path, std::vector<SectionInfo>& outSections)
{
    outSections.clear();

    std::string json;
    if (!ReadFile(path, json))
        return false;

    rapidjson::Document doc;
    doc.Parse(json.c_str());

    if (doc.HasParseError() || !doc.IsObject())
        return false;

    if (!doc.HasMember("inventorySections") ||
        !doc["inventorySections"].IsObject())
        return false;

    const rapidjson::Value& inv = doc["inventorySections"];

    if (!inv.HasMember("sections") ||
        !inv["sections"].IsArray())
        return false;

    const rapidjson::Value& arr = inv["sections"];

    for (rapidjson::SizeType i = 0; i < arr.Size(); ++i)
    {
        SectionInfo section;
        if (!ParseSection(arr[i], section))
            return false;

        outSections.push_back(section);
    }

    return !outSections.empty();
}