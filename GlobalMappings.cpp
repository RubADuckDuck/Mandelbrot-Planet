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
