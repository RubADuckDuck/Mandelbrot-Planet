#pragma once
#ifndef MESSAGE_PARSER_H
#define MESSAGE_PARSER_H

#include <string>
#include <map>
#include <utility>
#include <stdexcept>
#include "ItemType.h"

class MessageParser {
public:
    // Parses the entire message into a key-value map
    static std::map<std::string, std::string> parse(const std::string& msg);

    // Parses a position string into a pair of integers
    static std::pair<int, int> parsePosition(const std::string& positionStr);

    // Converts a string into an ItemType
    static ItemType parseItem(const std::string& itemStr);
};

#endif // MESSAGE_PARSER_H
