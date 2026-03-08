#pragma once

#include <string>
#include <vector>
#include <kenshi/Enums.h>

struct SectionInfo
{
    int         id;
    std::string name;
    int         width;
    int         height;
    AttachSlot  slot;
    bool        equipCallbacks;
    bool        isContainerSlot;
    bool        enabled;
    int         limit;
};

bool LoadFromFile(const std::string& path, std::vector<SectionInfo>& outSections);