#include <map> 
#include "PlayerDirection.h"
#include <functional>


using Coord2d = std::pair<int, int>;

using Coord2dWithDirection = std::pair<Coord2d, Direction>;

using Action2Coord2d = std::map<Direction, Coord2d>;  // when given action, get where that action leads the player 

using RotationsOnInt = std::function<int(int)>; 
using Int2RotationOnInt = std::map<int, RotationsOnInt>; 



class ParallelTransporter {
public:
	static int Direction2Int(Direction xDir) {
		return static_cast<int>(xDir);
	}

	static Direction Int2Direction(int x) {
		return static_cast<Direction>(x % 4); // Ensure cyclic behavior for rotations
	}

	static int TurnLeft(int x) {
		return (x + 1) % 4;
	} 
	static int TurnRight(int x) {
		return (x - 1) % 4; 
	} 
	static int Turn180(int x) {
		return (x + 2) % 4; 
	}
	static int StayStraight(int x) {
		return x;
	}
	static Int2RotationOnInt int2Rotation;

	/* 
		This could be thought of as a map f: Direction x Direction -> Direction 
		Mathematically, a parallel tranport is a way of pushing tangent vectors on a manifold. 
		First argument corresponds to what TangentVector we are pushing.
		Second argument corresponds to in what direction we are pushing the first argument in terms of the TangentVector. 
		Third the result corresponds to how the first argument changes as it is pushed in the direction of the second argument.

		However, in our case, since the world is discrete and there are only four directions,
		the third argument, the result is not the change of the first argument, 
		but rather the resulting tangent vector of pushing first arg through the direction of second arg
		One could think of this as a trivial version of the christophel symbol  

		This Transporter exists at every point of the grid. 
		Each one of the instance acts as a piece of information of how the space is connected 


		for efficient expressions of the relations between the directions, 
		we use a number to compactly represent the change that is applied to the tangent vector. 

		0: right 
		1: up 
		2: left 
		3: down 

		a +1 would 

		+1(0) = 1 ~ 90 degree rotation: right |-> up
		+1(1) = 2 ~ 90 degree rotation: up |-> left
		...  

		As we can see, each number corresponds to a rotation. 
		In the optimal cases, where the manifold is metric-compatible, 
		all possible alternations between the tangent vectors coresspond to each number. 

		Direction -> Direction 
		
		Z / 4 -> Z / 4 

		for that reason, this class don't directly work with the Directions, 
		but work with numbers as a proxy to it.

		The default maps each direction to 0 where all transports have 0 curvature respect to the current coordinate system.
		0 -> +0
		1 -> +0 
		2 -> +0 
		3 -> +0 
	*/
public:
	bool isDefault = true;

	Int2RotationOnInt directionInt2RotationInt; 


	ParallelTransporter(): isDefault(true) { 
		for (int i = 0; i < 4; i++) { 
			directionInt2RotationInt[i] = StayStraight; 
		} 
	}  

	ParallelTransporter(Int2RotationOnInt x) { 
		directionInt2RotationInt = x; 
	} 

	void SetDirectionInt2RotationInt(int directionInt, int rotationInt) {
		directionInt2RotationInt[directionInt] = int2Rotation[rotationInt];
	}

	Direction CalculateDireciton(Direction facingWhere, Direction goingWhere) {
		int facingInt = Direction2Int(facingWhere); 
		int goingInt = Direction2Int(goingWhere); 

		// RotationsOnInt curRotation = directionInt2RotationInt[goingInt]; 
		RotationsOnInt curRotation = directionInt2RotationInt.count(goingInt) ?
			directionInt2RotationInt[goingInt] : StayStraight;

		int resultingDirectionInt = curRotation(facingInt);  

		return Int2Direction(resultingDirectionInt);
	}
};



class MovementManager {
	/*
		Our objects live on a Non Euclidean Space 
		There Exist Curvature. 
		This class Helps the playes and other objects to move through the Geodisics of space 
	*/
	static const int GRID_SIZE = 10;
public: 
	Action2Coord2d* grid2Transporter[GRID_SIZE][GRID_SIZE];  
	ParallelTransporter* grid2ParallelTransporter[GRID_SIZE][GRID_SIZE]; 

	MovementManager() {
		for (int y = 0; y < GRID_SIZE; ++y) {
			for (int x = 0; x < GRID_SIZE; ++x) {
				// Create a new Action2Coord2d map for the current cell
				grid2Transporter[y][x] = new Action2Coord2d({
					{Direction::RIGHT, {(x + 1) % GRID_SIZE, y}},                        // Wrap horizontally to the left
					{Direction::UP, {x, (y - 1 + GRID_SIZE) % GRID_SIZE}},              // Wrap vertically to the bottom
					{Direction::LEFT, {(x - 1 + GRID_SIZE) % GRID_SIZE, y}},           // Wrap horizontally to the right
					{Direction::DOWN, {x, (y + 1) % GRID_SIZE}}                        // Wrap vertically to the top
					});

				// Initialize the ParallelTransporter for the current cell
				grid2ParallelTransporter[y][x] = new ParallelTransporter();
			}
		}
	}

	void InitPlanaFigure(int startY, int startX, int size) {

		int curY = 0; 
		int curX = 0; 
		Direction curDirection = Direction::UP;
		int curCurvatureFromLateralFace2BottomBase = 0; 
		int curCurvatureFromLateralFace2UpperBase = 0;


		Action2Coord2d curAction2Coord;
		ParallelTransporter curParallelTransporter;

		int curIndex = 0; 
		for (int i = 0; i < 4; i++) { 
			for (int j = 0; j < size; j++) { 
				curCurvatureFromLateralFace2BottomBase = ( - j) % 4;
				curCurvatureFromLateralFace2UpperBase = j % 4; 
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
				curAction2Coord = *grid2Transporter[startY + size - 1][startX + curIndex];
				curAction2Coord[Direction::DOWN] = { curY, curX};
				curParallelTransporter = *grid2ParallelTransporter[startY + size - 1][startX + curIndex]; 
				curParallelTransporter.SetDirectionInt2RotationInt(3, curCurvatureFromLateralFace2BottomBase);
				// base 2 latteral face 
				curAction2Coord = *grid2Transporter[curY][curX];
				curAction2Coord[curDirection] = { startY + size - 1, startX + curIndex };
				curParallelTransporter = *grid2ParallelTransporter[curY][curX];
				curParallelTransporter.SetDirectionInt2RotationInt(3,  (- curCurvatureFromLateralFace2BottomBase) % 4);


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
				curAction2Coord = *grid2Transporter[startY + 0][startX + curIndex];
				curAction2Coord[Direction::DOWN] = { curY, curX };
				curParallelTransporter = *grid2ParallelTransporter[startY + 0][startX + curIndex];
				curParallelTransporter.SetDirectionInt2RotationInt(0, curCurvatureFromLateralFace2UpperBase);

				curAction2Coord = *grid2Transporter[curY][curX];
				curAction2Coord[curDirection] = { startY + 0, startX + curIndex };
				curParallelTransporter = *grid2ParallelTransporter[curY][curX];
				curParallelTransporter.SetDirectionInt2RotationInt(0, curCurvatureFromLateralFace2UpperBase);

				// end
				curIndex += 1;
			}
		}

		// stitch up the latteral faces
		for (int i = 0; i < size; i++) {
			// left to right
			curAction2Coord = *grid2Transporter[startY + i][startX + 0];
			curAction2Coord[Direction::LEFT] = { startY + i, startX + size * 4 - 1 };


			// right to left
			curAction2Coord = *grid2Transporter[startY + i][startX + size * 4 - 1];
			curAction2Coord[Direction::RIGHT] = { startY + i, startX + 0 };
		}
	}

	Coord2dWithDirection Move(Coord2d position, Direction movingDirection, Direction facingDirection) {
		int curY = position.first; 
		int curX = position.second;  
		
		Action2Coord2d* curAction2Coord2d = grid2Transporter[curY][curX]; 
		Coord2d targetCoord2d = (*curAction2Coord2d)[movingDirection];

		ParallelTransporter* curParallelTransporter = grid2ParallelTransporter[curY][curX];  
		Direction newFacingDirection =  curParallelTransporter->CalculateDireciton(facingDirection, movingDirection); 

		return { targetCoord2d, newFacingDirection };
	}
};