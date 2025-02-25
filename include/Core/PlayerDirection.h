#pragma once
#include <map> 
#include <string>

enum class Direction { 
	RIGHT = 0, 
	UP = 1, 
	LEFT = 2, 
	DOWN = 3, 
	IDLE = 4
};

extern std::map<Direction, std::string> direction2String;

