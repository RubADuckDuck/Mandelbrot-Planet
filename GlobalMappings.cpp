#include "GlobalMappings.h"

// Define and initialize the global map from ItemType to string
std::map<ItemType, const std::string> itemType2ItemName = {
    {ItemType::WOOD, "Wood"},
    {ItemType::ROCK, "Rock"},
    {ItemType::IRON, "Iron"},
    {ItemType::RUBY, "Ruby"},
    {ItemType::PYTHON, "Python"}
};

// Define and initialize the global map from string to ItemType
std::map<const std::string, ItemType> itemName2ItemType = {
    {"Wood", ItemType::WOOD},
    {"Rock", ItemType::ROCK},
    {"Iron", ItemType::IRON},
    {"Ruby", ItemType::RUBY},
    {"Python", ItemType::PYTHON}
};

// Define and initialize the global map from FactoryType and FactoryComponentType to string
std::map<std::pair<FactoryType, FactoryComponentType>, const std::string> factoryType2FactoryName = {
    {{FactoryType::Farm, FactoryComponentType::INPUTPORT}, "Farm_Input"},
    {{FactoryType::Farm, FactoryComponentType::OUTPUTPORT}, "Farm_Output"},
    {{FactoryType::Farm, FactoryComponentType::DEFAULT}, "Farm_Default"},
    {{FactoryType::Bakery, FactoryComponentType::INPUTPORT}, "Bakery_Input"},
    {{FactoryType::Bakery, FactoryComponentType::OUTPUTPORT}, "Bakery_Output"},
    {{FactoryType::Bakery, FactoryComponentType::DEFAULT}, "Bakery_Default"}
};

// Define and initialize the global map from string to FactoryType and FactoryComponentType
std::map<const std::string, std::pair<FactoryType, FactoryComponentType>> factoryName2FactoryType = {
    {"Farm_Input", {FactoryType::Farm, FactoryComponentType::INPUTPORT}},
    {"Farm_Output", {FactoryType::Farm, FactoryComponentType::OUTPUTPORT}},
    {"Farm_Default", {FactoryType::Farm, FactoryComponentType::DEFAULT}},
    {"Bakery_Input", {FactoryType::Bakery, FactoryComponentType::INPUTPORT}},
    {"Bakery_Output", {FactoryType::Bakery, FactoryComponentType::OUTPUTPORT}},
    {"Bakery_Default", {FactoryType::Bakery, FactoryComponentType::DEFAULT}}
};

// Define and initialize the global map from GroundType to string
std::map<GroundType, const std::string> groundType2GroundName = {
    {GroundType::WATER, "Water"},
    {GroundType::GRASS, "Grass"}
};

// Define and initialize the global map from string to GroundType
std::map<const std::string, GroundType> groundName2GroundType = {
    {"Water", GroundType::WATER},
    {"Grass", GroundType::GRASS}
};