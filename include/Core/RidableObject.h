#pragma once
#include "GameObject.h"
#include "GridManager.h"

class RidableObject : public GameObject {
public:
	std::string GetName() const override;

	virtual uint8_t GetTypeID();
private:
	uint8_t gridHeight_;
	uint8_t gridWidth_;  

	// f: Index -> ObjectID
	std::vector<uint32_t> grid_;  

	std::unique_ptr<MovementManager> movementManager_;  
	std::unique_ptr<GridTransformManager> gridTransformManager_;  

	

public: 
	RidableObject();

	RidableObject(uint32_t objID, uint32_t meshID, uint32_t textureID, uint8_t cubeEdgeLength);

	RidableObject(uint32_t objID, uint32_t meshID, uint32_t textureID, uint8_t gridHeight, uint8_t gridWidth);

	void Initialize();

	uint32_t GetObjectIDAt(Coord2d pos);
	Coord2d GetPosition(uint32_t objID);
	MovementManager* GetMovementManager();
	GridTransformManager* GetGridTransformManager() {
		return gridTransformManager_.get();
	}
	glm::mat4 GetGridTransformAt(uint8_t index) {
		// interpolate value 

		glm::mat4 resultMat = glm::mat4(1);

		Coord2d coord = this->index_on_vector_to_coord2d(index);
		Transform* currTransform = gridTransformManager_->grid2Transform[coord.first][coord.second];

		glm::mat4 currTransformMat = currTransform->GetTransformMatrix();

		// Calculate difference between current and previous transformations
		glm::mat4 diff = currTransformMat - prevGridTransform_;

		// Define a threshold for stopping interpolation
		float threshold = 0.01f; // Adjust this value as needed

		// Calculate the Frobenius norm of the difference matrix
		float norm = glm::length(glm::vec4(diff[0]));
		norm += glm::length(glm::vec4(diff[1]));
		norm += glm::length(glm::vec4(diff[2]));
		norm += glm::length(glm::vec4(diff[3]));

		// Adjust interpolation factor based on the difference
		float interpolationFactor = norm > threshold ? 0.5f : 0.0f;

		resultMat = prevGridTransform_ * (1.0f - interpolationFactor) +
			currTransformMat * interpolationFactor;

		prevGridTransform_ = resultMat;

		return resultMat;
	}

	std::vector<uint32_t>& GetGrid() {
		return this->grid_; 
	}

	// Return iterator to beginning
	std::vector<uint32_t>::iterator gridBegin() {
		return grid_.begin();
	}

	// Return iterator to end
	std::vector<uint32_t>::iterator gridEnd() {
		return grid_.end();
	}

	// Return const iterator to beginning (for const objects)
	std::vector<uint32_t>::const_iterator gridBegin() const {
		return grid_.begin();
	}

	// Return const iterator to end (for const objects)
	std::vector<uint32_t>::const_iterator gridEnd() const {
		return grid_.end();
	}

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

private: 
	void log_info() {
		log(
			LOG_INFO,
			"\n  meshID: " + std::to_string(meshID_)
			+ "\n  textureID: " + std::to_string(textureID_)
			+ "\n  height: " + std::to_string(gridHeight_)
			+ "\n  width: " + std::to_string(gridWidth_));
	}
};

class PlayableObject : public RidableObject {
public:

	// server & client
	uint8_t GetTypeID() override;;

	void TakeAction(Direction direction);
	void DropItem();
	void RequestWalk();
	void Walk();
	void PickUpItem(Item* item);
};