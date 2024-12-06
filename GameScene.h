#pragma once 
#include <vector>

class GameObject; 

class GameScene {
public: 
	std::vector<GameObject> gameObjList;

	void AddGameObject(GameObject& gameObj); 
	void RemoveGameObject(GameObject& gameObj); 

	void DrawSceneInPerspectiveOf(GameObject& gameObj); 

	
};