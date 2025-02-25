#pragma once

#include <map>
#include <string>
#include "ItemType.h" // Include the ItemType enum
#include "FactoryType.h"
#include "GroundType.h"


// Declare global mappings
extern std::map<ItemType, const std::string> itemType2ItemName;
extern std::map<const std::string, ItemType> itemName2ItemType;

// Declare global mappings
extern std::map<std::pair<FactoryType, FactoryComponentType>, const std::string> factoryType2FactoryName;
extern std::map<const std::string, std::pair<FactoryType, FactoryComponentType>> factoryName2FactoryType; 

extern std::map<GroundType, const std::string> groundType2GroundName; 
extern std::map<const std::string, GroundType> groundName2GroundType;
