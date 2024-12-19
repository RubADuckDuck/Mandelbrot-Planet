#pragma once

#include <map>
#include <string>
#include "ItemType.h" // Include the ItemType enum

// Declare global mappings
extern std::map<ItemType, const std::string> itemType2ItemName;
extern std::map<const std::string, ItemType> itemName2ItemType;
