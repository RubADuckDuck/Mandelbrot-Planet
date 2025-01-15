#include "GameObject.h"
#include "GridManager.h"

class RidableObject : public GameObject {
public:
	std::string GetName() const;

private:
	void log(LogLevel level, std::string text);

	uint8_t gridHeight;
	uint8_t gridWidth;  

	// f: Index -> ObjectID
	std::vector<uint32_t> grid;  

	std::unique_ptr<MovementManager> movementManager;  
	std::unique_ptr<GridTransformManager> gridTransformManager;  

	// This tells the ID of the object we are riding on. 
	uint32_t parentID; 

public: 
	uint32_t GetParentID();
	uint32_t GetObjectIDAt(Coord2d pos);
	Coord2d GetPosition(uint32_t objID);
	MovementManager* GetMovementManager();

	bool IsInBounds(int index);
	bool IsPositionOccupied(Coord2d pos);

	void SetObjIdAtPos(Coord2d pos, uint32_t objID);
	void SetObjIdAtPos(uint8_t pos_index, uint32_t objID);
	bool SetParentObjectAndExit(uint32_t newParentID);
	
	void SwapObjOnGrid(Coord2d a, Coord2d b);

	// 2d index <-> 1d index 
	uint8_t coord2d_to_index_on_vector(Coord2d pos); 
	Coord2d index_on_vector_to_coord2d(uint8_t pos);

	void AddChildObjectToGridAtPosition(uint32_t childID, Coord2d pos);

	bool RemoveChildAtGrid(uint32_t childID);
};
