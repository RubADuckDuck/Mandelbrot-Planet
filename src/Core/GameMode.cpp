#include "Core/GameMode.h"

std::unordered_map<GameModeType, std::string> mode2string = {
    {GameModeType::MAIN_MENU, "Main Menu"},
    {GameModeType::PLAYING, "Playing"},
    {GameModeType::SETTINGS, "Settings"},
    {GameModeType::HOST_LOBBY, "Host Lobby"},
    {GameModeType::JOIN_LOBBY, "Join Lobby"}
};