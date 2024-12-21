#include "GlobalMappings.h"
#include "MessageParser.h"
#include <sstream>

std::map<std::string, std::string> MessageParser::parse(const std::string& msg) {
    std::map<std::string, std::string> keyValues;
    std::stringstream ss(msg);
    std::string pair;

    while (std::getline(ss, pair, ';')) {
        size_t delimiterPos = pair.find(':');
        if (delimiterPos == std::string::npos) {
            throw std::invalid_argument("Invalid message format: " + pair);
        }
        std::string key = pair.substr(0, delimiterPos);
        std::string value = pair.substr(delimiterPos + 1);
        keyValues[key] = value;
    }

    return keyValues;
}

std::pair<int, int> MessageParser::parsePosition(const std::string& positionStr) {
    size_t delimiterPos = positionStr.find(',');
    if (delimiterPos == std::string::npos) {
        throw std::invalid_argument("Invalid position format: " + positionStr);
    }
    int x = std::stoi(positionStr.substr(0, delimiterPos));
    int y = std::stoi(positionStr.substr(delimiterPos + 1));
    return { x, y };
}

ItemType MessageParser::parseItem(const std::string& itemStr) {

    auto it = itemName2ItemType.find(itemStr);
    if (it == itemName2ItemType.end()) {
        throw std::invalid_argument("Unknown item type: " + itemStr);
    }
    return it->second;
}
