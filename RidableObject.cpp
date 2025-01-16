#include "RidableObject.h"

std::string RidableObject::GetName() const {
	return "RidableObject";
}

uint8_t RidableObject::GetTypeID() { return 1; }

RidableObject::RidableObject()
	: GameObject(),
	gridHeight_(0), gridWidth_(0) {
	movementManager_ = std::unique_ptr<MovementManager>(new MovementManager());
	gridTransformManager_ = std::unique_ptr<GridTransformManager>(new GridTransformManager());
}

RidableObject::RidableObject(uint32_t meshID, uint32_t textureID, uint8_t cubeEdgeLength)
	: GameObject(meshID, textureID),
	gridHeight_(cubeEdgeLength), gridWidth_(cubeEdgeLength * 6) {
	movementManager_ = std::unique_ptr<MovementManager>(new MovementManager(cubeEdgeLength));
	gridTransformManager_ = std::unique_ptr<GridTransformManager>(new GridTransformManager(cubeEdgeLength));
}

RidableObject::RidableObject(uint32_t meshID, uint32_t textureID, uint8_t gridHeight, uint8_t gridWidth)
	: GameObject(meshID, textureID),
	gridHeight_(gridHeight), gridWidth_(gridWidth) {
	movementManager_ = std::unique_ptr<MovementManager>(new MovementManager());
	gridTransformManager_ = std::unique_ptr<GridTransformManager>(new GridTransformManager());
}

RidableObject::RidableObject(uint32_t objID, uint32_t meshID, uint32_t textureID, uint8_t gridHeight, uint8_t gridWidth)
	: GameObject(objID, meshID, textureID),
	gridHeight_(gridHeight), gridWidth_(gridWidth) {
	movementManager_ = std::unique_ptr<MovementManager>(new MovementManager());
	gridTransformManager_ = std::unique_ptr<GridTransformManager>(new GridTransformManager());
}

uint32_t RidableObject::GetParentID() {
	return parentID_;
}

MovementManager* RidableObject::GetMovementManager() {
	return movementManager_.get();
}

Coord2d RidableObject::GetPosition(uint32_t objID) {
	std::vector<uint32_t>::iterator it = std::find(grid_.begin(), grid_.end(), objID);

	uint8_t index;

	if (it == grid_.end()) {
		log(LOG_ERROR, "Could Not find Obj of id: " + std::to_string(objID) + " in here");
	}

	index = it - grid_.begin();

	return index_on_vector_to_coord2d(index);
}

bool RidableObject::IsPositionOccupied(Coord2d pos) {
	uint8_t index = coord2d_to_index_on_vector(pos);

	if (IsInBounds(index)) {
		// Occupied <=> not empty 
		return (grid_[index] != 0);
	}
	else {

		return true;
	}
}

uint32_t RidableObject::GetObjectIDAt(Coord2d pos) {
	uint8_t index = coord2d_to_index_on_vector(pos);

	if (IsInBounds(index)) {
		return grid_[index];
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
		grid_[pos_index] = objID;
		return;
	}
	else {
		// do nothing if out of bounds 
		return;
	}
}

bool RidableObject::IsInBounds(int index) {
	log(LOG_WARNING, "Index is out of bounds");
	return index >= 0 && index < gridHeight_ * gridWidth_;
}

void RidableObject::SwapObjOnGrid(Coord2d a, Coord2d b) {
	int idx_a = this->coord2d_to_index_on_vector(a);
	int idx_b = this->coord2d_to_index_on_vector(b);

	if (IsInBounds(idx_a) || IsInBounds(idx_b)) {
		log(LOG_ERROR, "Index Error");
		return;
	}

	uint32_t temp = grid_[idx_a];

	grid_[idx_a] = grid_[idx_b];
	grid_[idx_b] = temp;

	return;
}

uint8_t RidableObject::coord2d_to_index_on_vector(Coord2d pos) {
	return pos.first * gridWidth_ + pos.second;
}

Coord2d RidableObject::index_on_vector_to_coord2d(uint8_t pos) {
	int y = pos / gridWidth_;
	int x = pos % gridWidth_;

	return { y, x };
}

void RidableObject::AddChildObjectToGridAtPosition(uint32_t childID, Coord2d pos) {
	uint8_t curIndex = coord2d_to_index_on_vector(pos);

	// add child to grid 
	grid_[curIndex] = childID;

	// registration of 'this' instance as the parent is dealt by Commands 

	return;
}

bool RidableObject::RemoveChildAtGrid(uint32_t childID) {

	for (int i = 0; i < grid_.size(); i++) {
		if (grid_[i] == childID) {
			grid_[i] = 0;

			return true;
		}
	}

	log(LOG_WARNING, "Cannot find childID. The ID maybe wrong.");
	return false;
}

bool RidableObject::SetParentObjectAndExit(uint32_t newParentID) {
	if (parentID_ == 0) {
		// previously had no parent  

		// find empty spot and place exit to parent there
		for (int i = 0; i < grid_.size(); i++) {
			if (grid_[i] == 0) {
				grid_[i] = newParentID;

				parentID_ = newParentID;

				return true;
			}
		}

		log(LOG_ERROR, "There should be room to place an Exit to the Parent Object Grid. ");
		return false;
	}
	else {
		// find position of exit towards preivous parent and replace it with newParent ID 

		// find empty spot and place exit to parent there
		for (int i = 0; i < grid_.size(); i++) {
			if (grid_[i] == parentID_) {
				grid_[i] = newParentID;

				parentID_ = newParentID;

				return true;
			}
		}

		log(LOG_WARNING, "Wasn't able to find previous parent on grid. Retrying after assuming there was no parent set after all");
		parentID_ = 0;

		bool success = SetParentObjectAndExit(newParentID);

		return success;
	}
}


// server & client

uint8_t PlayableObject::GetTypeID() {
	return 2;
}

void PlayableObject::TakeAction(Direction direction) {
	int curDirectionInt = 0;

	switch (direction) {
	case(Direction::RIGHT):
		curDirectionInt += 3; // turn right
		curDirectionInt += static_cast<int>(direction);
		curDirectionInt = curDirectionInt % 4;
		direction = static_cast<Direction>(curDirectionInt);
		break;
	case(Direction::UP):
		RequestWalk();
		break;
	case(Direction::LEFT):
		curDirectionInt += 1; // turn right
		curDirectionInt += static_cast<int>(direction);
		curDirectionInt = curDirectionInt % 4;
		direction = static_cast<Direction>(curDirectionInt);
		break;
	case(Direction::DOWN):
		curDirectionInt += 2; // turn around
		curDirectionInt += static_cast<int>(direction);
		curDirectionInt = curDirectionInt % 4;
		direction = static_cast<Direction>(curDirectionInt);
		break;
	case(Direction::IDLE):
		LOG(LOG_INFO, "Publishing Item from Player");

		DropItem();
	}
}

void PlayableObject::DropItem()
{
}

void PlayableObject::RequestWalk()
{
}

void PlayableObject::Walk()
{
}

void PlayableObject::PickUpItem(Item* item)
{
}