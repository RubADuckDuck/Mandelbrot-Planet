#include "GridManager.h" 

int PositiveModulo(int x, int mod) {
    return ((x) % mod + mod) % mod;
}

int ParallelTransporter::Direction2Int(Direction xDir) {
	return static_cast<int>(xDir);
}

Direction ParallelTransporter::Int2Direction(int x) {
	return static_cast<Direction>(PositiveModulo(x, 4)); // Ensure cyclic behavior for rotations
}

int ParallelTransporter::TurnLeft(int x) {
	return PositiveModulo(x + 1, 4);
}
int ParallelTransporter::TurnRight(int x) {
	return PositiveModulo(x - 1, 4);
}
int ParallelTransporter::Turn180(int x) {
	return PositiveModulo(x + 2, 4);
}
int ParallelTransporter::StayStraight(int x) {
	return x;
}

Int2RotationOnInt ParallelTransporter::int2Rotation = {
    {0, StayStraight},
    {1, TurnLeft},
    {2, Turn180},
    {3, TurnRight}
};

ParallelTransporter::ParallelTransporter() : isDefault(true) {
	for (int i = 0; i < 4; i++) {
		directionInt2RotationInt[i] = StayStraight;
	}
}

ParallelTransporter::ParallelTransporter(Int2RotationOnInt x) {
	directionInt2RotationInt = x;
}

void ParallelTransporter::SetDirectionInt2RotationInt(int directionInt, int rotationInt) {
	directionInt2RotationInt[directionInt] = int2Rotation[rotationInt];
	int2Int[directionInt] = rotationInt;
}

Direction ParallelTransporter::CalculateDireciton(Direction facingWhere, Direction goingWhere) {
	int facingInt = Direction2Int(facingWhere);
	int goingInt = Direction2Int(goingWhere);

	// RotationsOnInt curRotation = directionInt2RotationInt[goingInt]; 
	RotationsOnInt curRotation = directionInt2RotationInt.count(goingInt) ?
		directionInt2RotationInt[goingInt] : StayStraight;

	int resultingDirectionInt = curRotation(facingInt);

	return Int2Direction(resultingDirectionInt);
}


MovementManager::MovementManager() {
	grid2Transporter.resize(GRID_SIZE, std::vector<Action2Coord2d*>(GRID_SIZE, nullptr));
	grid2ParallelTransporter.resize(GRID_SIZE, std::vector<ParallelTransporter*>(GRID_SIZE, nullptr));

	for (int y = 0; y < GRID_SIZE; ++y) {
		for (int x = 0; x < GRID_SIZE; ++x) {
			// Create a new Action2Coord2d map for the current cell
			grid2Transporter[y][x] = new Action2Coord2d({
				{Direction::RIGHT, {y, (x + 1) % GRID_SIZE}},                        // Wrap horizontally to the left
				{Direction::UP, {(y - 1 + GRID_SIZE) % GRID_SIZE, x}},              // Wrap vertically to the bottom
				{Direction::LEFT, {y, (x - 1 + GRID_SIZE) % GRID_SIZE}},           // Wrap horizontally to the right
				{Direction::DOWN, {(y + 1) % GRID_SIZE, x}}                        // Wrap vertically to the top
				});

			// Initialize the ParallelTransporter for the current cell
			grid2ParallelTransporter[y][x] = new ParallelTransporter();
		}
	}

	this->InitPlanaFigure(0, 0, 4);
}

void MovementManager::InitPlanaFigure(int startY, int startX, int size) {
	int curY = 0;
	int curX = 0;
	Direction curDirection = Direction::UP;
	int curCurvatureFromLateralFace2BottomBase = 0;
	int curCurvatureFromLateralFace2UpperBase = 0;


	Action2Coord2d* curAction2Coord;
	ParallelTransporter* curParallelTransporter;

	int curIndex = 0;
	for (int i = 0; i < 4; i++) {
		curCurvatureFromLateralFace2BottomBase = ((-i) % 4 + 4) % 4;
		curCurvatureFromLateralFace2UpperBase = i % 4;
		for (int j = 0; j < size; j++) {

			
			if (i == 0) {
				curY = 0;
				curX = j;
				curDirection = Direction::UP;
			}
			else if (i == 1) {
				curY = j;
				curX = size - 1;
				curDirection = Direction::RIGHT;
			}
			else if (i == 2) {
				curY = size - 1;
				curX = size - j - 1;
				curDirection = Direction::DOWN;
			}
			else {
				curY = size - j - 1;
				curX = 0;
				curDirection = Direction::LEFT;
			}

			curY = curY + startY;
			curX = curX + startX + size * 5; // bottom starts at size * 5 

			// remember 
			// right -> 0 
			// up -> 1 
			// left -> 2 
			// down -> 3

			// bottom rim 
			// latteral face 2 base 
			curAction2Coord = grid2Transporter[startY + size - 1][startX + curIndex];
			(*curAction2Coord)[Direction::DOWN] = { curY, curX };
			curParallelTransporter = grid2ParallelTransporter[startY + size - 1][startX + curIndex];
			curParallelTransporter->SetDirectionInt2RotationInt(3, curCurvatureFromLateralFace2BottomBase);
			// base 2 latteral face 
			curAction2Coord = grid2Transporter[curY][curX];
			(*curAction2Coord)[curDirection] = { startY + size - 1, startX + curIndex };
			curParallelTransporter = grid2ParallelTransporter[curY][curX];
			curParallelTransporter->SetDirectionInt2RotationInt(static_cast<int>(curDirection), PositiveModulo(-curCurvatureFromLateralFace2BottomBase, 4));


			if (i == 0) {
				curY = size - 1;
				curX = j;
				curDirection = Direction::DOWN;
			}
			else if (i == 1) {
				curY = size - j - 1;
				curX = size - 1;
				curDirection = Direction::RIGHT;
			}
			else if (i == 2) {
				curY = 0;
				curX = size - j - 1;
				curDirection = Direction::UP;
			}
			else {
				curY = j;
				curX = 0;
				curDirection = Direction::LEFT;
			}

			curY = curY + startY;
			curX = curX + startX + size * 4; // bottom starts at size * 4 

			// upper rim 
			curAction2Coord = grid2Transporter[startY + 0][startX + curIndex];
			(*curAction2Coord)[Direction::UP] = { curY, curX };
			curParallelTransporter = grid2ParallelTransporter[startY + 0][startX + curIndex];
			curParallelTransporter->SetDirectionInt2RotationInt(1, curCurvatureFromLateralFace2UpperBase);

			curAction2Coord = grid2Transporter[curY][curX];
			(*curAction2Coord)[curDirection] = { startY + 0, startX + curIndex };
			curParallelTransporter = grid2ParallelTransporter[curY][curX];
			curParallelTransporter->SetDirectionInt2RotationInt(static_cast<int>(curDirection), PositiveModulo(-curCurvatureFromLateralFace2UpperBase, 4));

			// end
			curIndex += 1;
		}
	}

	// stitch up the latteral faces
	for (int i = 0; i < size; i++) {
		// left to right
		curAction2Coord = grid2Transporter[startY + i][startX + 0];
		(*curAction2Coord)[Direction::LEFT] = { startY + i, startX + size * 4 - 1 };


		// right to left
		curAction2Coord = grid2Transporter[startY + i][startX + size * 4 - 1];
		(*curAction2Coord)[Direction::RIGHT] = { startY + i, startX + 0 };
	}
}

NavigationInfo MovementManager::Move(Coord2d position, Direction movingDirection, Direction facingDirection) {
	int curY = position.first;
	int curX = position.second;

	Action2Coord2d* curAction2Coord2d = grid2Transporter[curY][curX];
	Coord2d targetCoord2d = (*curAction2Coord2d)[movingDirection];

	ParallelTransporter* curParallelTransporter = grid2ParallelTransporter[curY][curX];
	Direction newFacingDirection = curParallelTransporter->CalculateDireciton(facingDirection, movingDirection);

	NavigationInfo newInfo = NavigationInfo();
	newInfo.pos = targetCoord2d;  // sometimes 0. why?
	newInfo.direction = newFacingDirection; 
	newInfo.changeOfOrientation = curParallelTransporter->int2Int[static_cast<int>(movingDirection)];

	return newInfo;
}