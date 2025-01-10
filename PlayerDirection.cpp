#include "PlayerDirection.h" // Include the header file

#include <map>
#include <string>

std::map<Direction, std::string> direction2String = {
    {Direction::RIGHT, "RIGHT"},
    {Direction::UP, "UP"},
    {Direction::LEFT, "LEFT"},
    {Direction::DOWN, "DOWN"},
    {Direction::IDLE, "IDLE"}
};