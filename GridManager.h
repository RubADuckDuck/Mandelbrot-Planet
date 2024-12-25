#pragma once
#include <map> 
#include "PlayerDirection.h"
#include <functional>
#include <vector>
#include <iostream>

using Coord2d = std::pair<int, int>;

using Coord2dWithDirection = std::pair<Coord2d, Direction>;

using Action2Coord2d = std::map<Direction, Coord2d>;  // when given action, get where that action leads the player 

using RotationsOnInt = std::function<int(int)>;
using Int2RotationOnInt = std::map<int, RotationsOnInt>;

struct NavigationInfo {
	Coord2d pos = { 0,0 }; 
	Direction direction = Direction::RIGHT; 
	int changeOfOrientation = 0; 
};

int PositiveModulo(int x, int mod);

class ParallelTransporter {
public:
	static int Direction2Int(Direction xDir);

	static Direction Int2Direction(int x);

	static int TurnLeft(int x);
	static int TurnRight(int x);
	static int Turn180(int x);
	static int StayStraight(int x);
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
	bool isDefault = true;

	Int2RotationOnInt directionInt2RotationInt; 
	std::map<int, int> int2Int;


	ParallelTransporter();

	ParallelTransporter(Int2RotationOnInt x);

	void SetDirectionInt2RotationInt(int directionInt, int rotationInt);

	Direction CalculateDireciton(Direction facingWhere, Direction goingWhere);
};



class MovementManager {
	/*
		Our objects live on a Non Euclidean Space
		There Exist Curvature.
		This class Helps the playes and other objects to move through the Geodisics of space
	*/
	static const int GRID_SIZE = 24;
public:
	std::vector<std::vector<Action2Coord2d*>> grid2Transporter;
	std::vector<std::vector<ParallelTransporter*>> grid2ParallelTransporter;

	MovementManager();

	void InitPlanaFigure(int startY, int startX, int size);

	NavigationInfo Move(Coord2d position, Direction movingDirection, Direction facingDirection); 

};