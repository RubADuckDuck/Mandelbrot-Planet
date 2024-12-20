#pragma once
#include "GameObject.h" 



enum Condition {
	WALKABLE, 
	INTERACTABLE,
	IS_INPUTPORT,
	IS_OUTPUTPORT,
	IS_BUTTON
};

enum FactoryComponentType {
    INPUTPORT,
    OUTPUTPORT
};

class Item : public TerrainObject {
    ItemType itemType;
};

class FactoryManagerObject;

class TerrainObject : public GameObject {
public: 
	int yCoord; int xCoord; // coordinate on Terrain ChartMap 

	GeneralMesh* mesh; 
	Texture* texture;  

	std::map<Condition, bool> terrainCondition;  
    

    ~TerrainObject() {};
	
	bool& CheckCondition(Condition condition); 

    virtual void Interact(Item* item) = 0; // item designates the kind of interaction f: Item -> Interaction

    virtual void DropItem(Item* item) {
        PublishItemDrop(item);
    }

    virtual void PublishItemDrop(Item* item);
};

class FactoryComponentObject : public TerrainObject {
public:
    FactoryComponentType myType; 

    FactoryManagerObject* ptrParentStructure; 
    Item* heldItem;

    FactoryComponentObject(FactoryComponentType componentType) {
        myType = componentType; 
    } 

    ~FactoryComponentObject() {}

    void Interact(Item* item) override {
        if (myType == INPUTPORT) {
            // shout that Item is dropped
            PublishItemDrop(heldItem);

            // pick up item
            this->heldItem = item;
        }
        else {
            LOG(LOG_ERROR, "It was this moment, you realize you fu*k up.")
        }
    }

    Item* GetHeldItem();

    void DiscardHeldItem(Item* item);

private: 
    void ResetFactoryFromCrafting() {
        ptrParentStructure->ResetCrafting();
    }
};



struct Item2Probability {
    std::map<ItemType, float> item2ProbMap;

    Item* RandomRollDrop();
};

struct Recipe {
    std::vector<ItemType> inputPortIndex2RequiredItem;
    std::vector<Item2Probability*> outputPortIndex2ToProductItem;
    float craftingDuration;
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

    Recipe* CheckForMatchingIngredient(std::vector<ItemType>& ingredients);

    void TriggerInteraction(const std::string& msg);

    std::vector<Item*>& GetCurrentIngredients();

    void ResetCrafting();

    void SetCrafting(Recipe* currRecipe);

    void GenerateItemAndReset();

    void GenerateItem();

    void Update(float deltaTime);
};

class NaturalResourceStructureObject : public StructureObject {

    Item2Probability itemType2ProbabilityOfDroppingIt;

    void TriggerInteraction(const std::string& msg);

    ItemType RandomRollDrop();
};