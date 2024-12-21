#pragma once
#include "GameObject.h" 
#include "Item.h"
#include "FactoryType.h"

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
	
	bool& CheckCondition(Condition condition); 

    virtual void Interact(Item* item) = 0; // item designates the kind of interaction f: Item -> Interaction

    virtual void DropItem(Item* item);

    virtual void PublishItemDrop(Item* item);
};

class DroppedItemObject : public TerrainObject {
    void Interact(Item* item) override{
        item->itemState = ON_GROUND;
        DropItem(item); // drop players Item right here
    } 
};


class FactoryComponentObject : public TerrainObject {
public:
    FactoryComponentType myType; 

    FactoryManagerObject* ptrParentStructure; 
    Item* heldItem;

    FactoryComponentObject(FactoryComponentType componentType);

    ~FactoryComponentObject();

    void Interact(Item* item) override;

    Item* GetHeldItem();

    void DiscardHeldItem();

private: 
    void ResetFactoryFromCrafting();
}; 



class StructureObject : public GameObject {
public:
    Publisher publisher;

    bool buildingShape[MAX_STRUCTURE_LENGTH][MAX_STRUCTURE_LENGTH];

    virtual void TriggerInteraction(const std::string& msg) = 0;
};

class FactoryManagerObject : public StructureObject {
public:
    /*
        FactoryType Structures have input ports that receive certain kinds of resources and OutputPorts that throw the produced objects to a certain building.
        InputPorts and Output ports are designated from a block that is part of the buildings shape.

    */

    FactoryType factoryType;

    int nInputPort;
    int nOutputPort; // output port index is (nInputPort + index) 

    std::vector<std::pair<int, int>> portIndex2PortPosition;
    std::vector<FactoryComponentObject> portIndex2FactoryComponent;

    std::map<std::pair<int, int>, int> Position2PortIndex;

    std::vector<Recipe*> craftingRecipes;
    Recipe* isCrafting;
    float duration;
    float targetDuration;

    virtual void InitFactory() = 0;

    Recipe* CheckForMatchingIngredient(std::vector<Item*>& ingredients);

    std::vector<Item*>& GetCurrentIngredients();

    void ResetCrafting();

    void SetCrafting(Recipe* currRecipe);

    void GenerateItemAndReset();

    void GenerateItem();

    void Update(float deltaTime);
};