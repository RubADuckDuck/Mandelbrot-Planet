#include "GameObject.h"
#include "GridManager.h"

class RidableObject : GameObject {
public:
	std::string GetName() const {
		return "RidableObject";
	}

private:
	void log(LogLevel level, std::string text) {
		LOG(level, GetName() + "::" + text);
	}

	uint8_t gridHeight;
	uint8_t gridWidth;  

	// f: Index -> ObjectID
	std::vector<uint32_t> grid;  

	std::unique_ptr<MovementManager> movementManager;  
	std::unique_ptr<GridTransformManager> gridTransformManager;  

	// This tells the ID of the object we are riding on. 
	uint32_t parentID; 

private: 
	// position of objects are 

	Transform* ptrNodeTransform;
	glm::mat4 modelTransformMat;

public: 
	uint32_t GetParentID(); 

	MovementManager* GetMovementManager() {
		return movementManager.get(); 
	}

	Coord2d GetPosition(uint32_t objID); 

	bool CheckIfPositionIsOccupied(Coord2d pos); 

	void SwapObjOnGrid(Coord2d a, Coord2d b); 

	uint8_t coord2d_to_index_on_vector(Coord2d pos) {
		return pos.first * gridWidth + pos.second; 
	}

	Coord2d index_on_vector_to_coord2d(uint8_t pos) {
		int y = pos / gridWidth; 
		int x = pos % gridWidth;  

		return { y, x };
	}

	void AddChildObjectToGridAtPosition(uint32_t childID, Coord2d pos) {
		uint8_t curIndex = coord2d_to_index_on_vector(pos); 

		// add child to grid 
		grid[curIndex] = childID; 

		// registration of 'this' instance as the parent is dealt by Commands 
		
		return;
	} 

	bool SetParentObjectAndExit(uint32_t newParentID) {
		if (parentID == 0) {
			// previously had no parent  

			// find empty spot and place exit to parent there
			for (int i = 0; i < grid.size(); i++) {
				if (grid[i] == 0) {
					grid[i] = newParentID; 

					parentID = newParentID; 

					return true; 
				}
			} 

			log(LOG_ERROR, "There should be room to place an Exit to the Parent Object Grid. "); 
			return false; 
		}
		else {
			// find position of exit towards preivous parent and replace it with newParent ID 
			
			// find empty spot and place exit to parent there
			for (int i = 0; i < grid.size(); i++) {
				if (grid[i] == parentID) {
					grid[i] = newParentID;

					parentID = newParentID;

					return true;
				}
			}

			log(LOG_WARNING, "Wasn't able to find previous parent on grid. Retrying after assuming there was no parent set after all"); 
			parentID = 0; 

			bool success = SetParentObjectAndExit(newParentID); 

			return success; 
		}
	}



	/// <summary>
	/// These RidableObjects can be ridden by other objects. 
	/// When object::A is ridden by Object::B, object::B gets added to Object::A::grid. 
	/// 
	///
	/// 
	/// </summary>
	/// <param name="pos"></param>
	/// <param name="ptrObj"></param>
	/// <returns></returns>
	bool RideObject(Coord2d pos, RidableObject* ptrObj); 

	bool CreateExitToObject(Coord2d pos, RidableObject* ptrObj); 



	bool PickUpRidableObjectAt()
};
