#pragma once
#include "GameObject.h" 
#include "FactoryType.h"

class Item;
class Recipe;

enum Condition {
	WALKABLE, 
	INTERACTABLE,
	IS_INPUTPORT,
	IS_OUTPUTPORT,
	IS_BUTTON
};



class FactoryManagerObject;

class Item;

class TerrainObject : public GameObject {
public: 
	int yCoord; int xCoord; // coordinate on Terrain ChartMap 

	std::map<Condition, bool> terrainCondition;  

    ~TerrainObject() {};

    void SetCoordinates(int y, int x) {
        yCoord = y; 
        xCoord = x;
    }
	
	bool& CheckCondition(Condition condition); 

    virtual void Interact(Item* item) = 0; // item designates the kind of interaction f: Item -> Interaction

    virtual void DropItem(Item* item);

    virtual void PublishItemDrop(Item* item);
};

class DroppedItemObject : public TerrainObject {
public:
    Item* item; 

    void SetItem(Item* ptrItem);

    void Interact(Item* item) override;

    void SetTransform(Transform* transform) override;

    void DrawGameObject(CameraObject& cameraObj);
};


class FactoryComponentObject : public TerrainObject {
public:
    FactoryComponentType componentType; 

    FactoryManagerObject* ptrParentStructure; 
    Item* heldItem;

    FactoryComponentObject(FactoryComponentType componentType, FactoryManagerObject* factoryManager);

    ~FactoryComponentObject();

    void Interact(Item* item) override;

    Item* GetHeldItem();

    void DiscardHeldItem();

    void SetTransform(Transform* transform) override;

    void DrawGameObject(CameraObject& cameraObj) override;

private: 
    void ResetFactoryFromCrafting();
}; 


using Publisher = std::function<void(const std::string&)>; // using 'Alias' = std::function<'returnType'('argType')>


class StructureObject : public GameObject {
public:
    Publisher publisher;

    // virtual void TriggerInteraction(const std::string& msg) = 0;
};

class FactoryManagerObject : public StructureObject {
public:
    /*
        FactoryType Structures have input ports that receive certain kinds of resources and OutputPorts that throw the produced objects to a certain building.
        InputPorts and Output ports are designated from a block that is part of the buildings shape.

    */

    FactoryType factoryType;

    FactoryComponentType buildingShape[MAX_STRUCTURE_LENGTH][MAX_STRUCTURE_LENGTH];

    int nInputPort;
    int nOutputPort; // output port index is (nInputPort + index) 

    std::vector<std::pair<int, int>> portIndex2PortPosition;
    std::map<std::pair<int, int>, int> position2PortIndex;

    std::vector<FactoryComponentObject*> portIndex2FactoryComponent;

    std::vector<Recipe*> craftingRecipes;
    Recipe* isCrafting;
    float duration;
    float targetDuration;


    FactoryManagerObject(FactoryType factoryType, int nInput, int nOutput) : factoryType(factoryType), nInputPort(nInput), nOutputPort(nOutput) {
        RandomlyGenerateBuildingShape(); 
    }
    
    void RandomlyGenerateBuildingShape();

    Recipe* CheckForMatchingIngredient(std::vector<Item*>& ingredients);

    std::vector<Item*>& GetCurrentIngredients();

    void ResetCrafting();

    void SetCrafting(Recipe* currRecipe);

    void GenerateItemAndReset();

    void GenerateItem();

    void Update(float deltaTime);
};