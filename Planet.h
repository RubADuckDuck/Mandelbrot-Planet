#pragma once
#include "GameObject.h" 

class Planet : public GameObject {
	uint8_t GetTypeID() override {
		return 10; 
	}
};