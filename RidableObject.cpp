#include "RidableObject.h"

std::string RidableObject::GetName() const {
	return "RidableObject";
}

void RidableObject::log(LogLevel level, std::string text) {
	LOG(level, GetName() + "::" + text);
}

uint32_t RidableObject::GetParentID() {
	return parentID;
}

MovementManager* RidableObject::GetMovementManager() {
	return movementManager.get();
}

Coord2d RidableObject::GetPosition(uint32_t objID) {
	std::vector<uint32_t>::iterator it = std::find(grid.begin(), grid.end(), objID);

	uint8_t index;

	if (it == grid.end()) {
		log(LOG_ERROR, "Could Not find Obj of id: " + std::to_string(objID) + " in here");
	}

	index = it - grid.begin();

	return index_on_vector_to_coord2d(index);
}

bool RidableObject::IsPositionOccupied(Coord2d pos) {
	uint8_t index = coord2d_to_index_on_vector(pos);

	if (IsInBounds(index)) {
		// Occupied <=> not empty 
		return (grid[index] != 0);
	}
	else {

		return true;
	}
}

uint32_t RidableObject::GetObjectIDAt(Coord2d pos) {
	uint8_t index = coord2d_to_index_on_vector(pos);

	if (IsInBounds(index)) {
		return grid[index];
	}
	else {
		return 0;
	}
}

void RidableObject::SetObjIdAtPos(Coord2d pos, uint32_t objID) {
	uint8_t index = coord2d_to_index_on_vector(pos);

	SetObjIdAtPos(index, objID); 
} 

void RidableObject::SetObjIdAtPos(uint8_t pos_index, uint32_t objID) {
	if (IsInBounds(pos_index)) {
		grid[pos_index] = objID;
		return;
	}
	else {
		// do nothing if out of bounds 
		return;
	}
}

bool RidableObject::IsInBounds(int index) {
	log(LOG_WARNING, "Index is out of bounds");
	return index >= 0 && index < gridHeight * gridWidth;
}

void RidableObject::SwapObjOnGrid(Coord2d a, Coord2d b) {
	int idx_a = this->coord2d_to_index_on_vector(a);
	int idx_b = this->coord2d_to_index_on_vector(b);

	if (IsInBounds(idx_a) || IsInBounds(idx_b)) {
		log(LOG_ERROR, "Index Error");
		return;
	}

	uint32_t temp = grid[idx_a];

	grid[idx_a] = grid[idx_b];
	grid[idx_b] = temp;

	return;
}

uint8_t RidableObject::coord2d_to_index_on_vector(Coord2d pos) {
	return pos.first * gridWidth + pos.second;
}

Coord2d RidableObject::index_on_vector_to_coord2d(uint8_t pos) {
	int y = pos / gridWidth;
	int x = pos % gridWidth;

	return { y, x };
}

void RidableObject::AddChildObjectToGridAtPosition(uint32_t childID, Coord2d pos) {
	uint8_t curIndex = coord2d_to_index_on_vector(pos);

	// add child to grid 
	grid[curIndex] = childID;

	// registration of 'this' instance as the parent is dealt by Commands 

	return;
}

bool RidableObject::RemoveChildAtGrid(uint32_t childID) {

	for (int i = 0; i < grid.size(); i++) {
		if (grid[i] == childID) {
			grid[i] = 0;

			return true;
		}
	}

	log(LOG_WARNING, "Cannot find childID. The ID maybe wrong.");
	return false;
}

bool RidableObject::SetParentObjectAndExit(uint32_t newParentID) {
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
