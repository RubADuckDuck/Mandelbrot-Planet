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

class TerrainObject : public GameObjectOnGrid {
public: 
	std::map<Condition, bool> terrainCondition;  

    ~TerrainObject() {};


    // server
	bool& CheckCondition(Condition condition); 

    virtual void Interact(Item* item) = 0; // item designates the kind of interaction f: Item -> Interaction

    virtual void DropItem(Item* item);

    virtual void PublishItemDrop(Item* item, int y, int x);
};

class DroppedItemObject : public TerrainObject {
public:
    Item* item; 

    // server & client 
    uint8_t GetTypeID() override; 

    // server
    void SetItem(Item* ptrItem);

    void Interact(Item* item) override;


    // client todo: make this go away?
    void SetTransform(Transform* transform) override;

    void DrawGameObject(CameraObject& cameraObj);
};


class FactoryComponentObject : public TerrainObject {
public:
    int initY; int initX; // offset 
    int yLocalCoord; int xLocalCoord; // Global = Init + Local 

    uint8_t GetTypeID() override;

    void SetLocalCoord(int y, int x) { 
        yLocalCoord = y; 
        xLocalCoord = x; 
    } 
    void SetOffset(int y, int x) { 
        initY = y; 
        initX = x; 
    } 

    std::pair<int, int> Global2LocalCoord(int y, int x) {
        return { y - initY, x - initX };
    }
    std::pair<int, int> Local2GlobalCoord(int y, int x) {
        return { y + initY, x + initX };
    }

    // server & client
    FactoryComponentType componentType; 
    // server
    FactoryManagerObject* ptrParentStructure; 
    Item* heldItem;

    FactoryComponentObject(FactoryComponentType componentType, FactoryManagerObject* factoryManager);

    ~FactoryComponentObject();


    // server 
    void Interact(Item* item) override; 

    Item* GetHeldItem(); 

    void DropItem(Item* item) override; 

    void DiscardHeldItem(); 


    // client
    void SetTransform(Transform* transform) override; 

    void DrawGameObject(CameraObject& cameraObj) override; 

private: 
    void ResetFactoryFromCrafting(); 
}; 


using Publisher = std::function<void(const std::string&)>; // using 'Alias' = std::function<'returnType'('argType')>


// Server only GameObject
class StructureManagerObject : public GameObject {
public:
    // to do: add more kinds of structure
};


// Server only GameObject
class FactoryManagerObject : public StructureManagerObject {
public:
    /*
        FactoryType Structures have input ports that receive certain kinds of resources and OutputPorts that throw the produced objects to a certain building.
        InputPorts and Output ports are designated from a block that is part of the buildings shape.

    */

    uint8_t GetTypeID() override;

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

    void Update(float deltaTime) override;
};