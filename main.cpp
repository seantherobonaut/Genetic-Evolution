/*
	Homework 7: Genetic Algorithm Robots
		This program recreates a study done by Harvard in 1968 to simulate genetic evolution with robots.
		A generation of robots randomly moves throughout the board by matching genes to sensor data.
		Each generation is improved by breeding the genes of the top peforming 50% robots.
		A pattern emerges in each generation of robots getting better and better at collecting batteries.
	Sean Leapley, CISP 400
	December 19th 2019
*/

#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <memory>
#include <cmath>

//Function prototype for getRandom
int getRandom(int, int);

//Class prototype for dyn_array
//Dynamically resizing arrays that accept any data type (vector wrapper)
//Works with pointers, but it's the programmer's responsibility to delete memory
template <class T>
class dyn_array
{
protected:
	std::vector<T> array;
public:
	dyn_array(){} //Empty constructor
	dyn_array(const dyn_array&);
	dyn_array<T>& operator=(const dyn_array&);
	unsigned size();
	void push_back(T);
	void erase(int);
	void clear();
	T& operator[](int);
	typedef typename std::vector<T>::iterator iterator;
	iterator begin() { return this->array.begin();}
	iterator end() { return this->array.end();}
};

//Function prototype for swapper
template <class T>
void swapper(T&, T&);

using namespace std;

//Declare classes before they are referenced to avoid errors
class EntityBase;
class EntityBattery;
class EntityRobot;
class Map;
class Generation;

struct Coord
{
	int row=0;
	int column=0;
};

struct Point
{
	Coord pos;
	char contents='n';
	shared_ptr<EntityBase> entity;
};

//Class prototype for EntityBase
class EntityBase
{
protected:
	Coord currentPos;
	bool active=true;
public:
	virtual bool isActive();
	virtual void interact(shared_ptr<EntityBase>);
	virtual char getType();
	virtual Coord getPos();
	virtual void setPos(Coord);
	virtual int getPower();
	virtual void setPower(int);
	virtual void updateMap(dyn_array<dyn_array<Point>>&, shared_ptr<EntityBase>);
	virtual void updateEntity(dyn_array<dyn_array<Point>>&);
};

//Class prototype for EntityBatter
class EntityBattery : public EntityBase
{
public:
	static int capacity;
	void interact(shared_ptr<EntityBase>);
	char getType();
	Coord getPos();
	void setPos(Coord);
};

//Class prototype for EntityRobot
//PLANNED To make all genes truly unique, design an algorithm to create all possible genes, then randomly take (remove and transfer to bot) 16 of them for each bot
class EntityRobot : public EntityBase
{
private:
	static int instance;
	unsigned id;
	unsigned generation;
	unsigned numMoves;
	Coord previousPos;
	int powerLevel;
	Coord nextCoord(dyn_array<dyn_array<Point>>);
public:
	dyn_array<string> genes;
	static int numGenes;
	EntityRobot(unsigned);
	unsigned getID();
	unsigned getGenID();
	unsigned getMoves();
	void setMoves(unsigned);
	void reset();
	//EntityBase
	void interact(shared_ptr<EntityBase>);
	char getType();
	Coord getPos();
	void setPos(Coord);
	int getPower();
	void setPower(int);
	void updateMap(dyn_array<dyn_array<Point>>&, shared_ptr<EntityBase>);
	void updateEntity(dyn_array<dyn_array<Point>>&);
};

//Class prototype for Map
class Map
{
private:
	Coord freeSpot();
public:
	dyn_array<dyn_array<Point>> grid;
	static int numRows;
	static int numColumns;
	static int percentBatteries;
	Map();
	void placeEntities(dyn_array<shared_ptr<EntityBase>>&);
	void clear();
};

//Class prototype for Generation
class Generation : public dyn_array<shared_ptr<EntityRobot>>
{
private:
	unsigned genInstance;
	void mutate(dyn_array<string>&);
public:
	static unsigned numRobots;
	static float mutationRate;
	Generation();
	unsigned getGeneration();
	void sortBots();
	float avgFitness();
	void nextGen();
	void breed();
};

int main()
{
	//Function greeting
	cout << "Below you will see robot generations gradually improving over time." << endl;
	
	//PLANNED add interface for changing values of simulation
	//PLANNED use less static constants, use relative values passed more often instead

	//Constants for the simulation 
	Generation::numRobots = 200; //Default 200 - must be divisible by 4
	Generation::mutationRate = 5.0; //Default 5%
	Map::numRows = 10; //Default 10
	Map::numColumns = 10; //Default 10
	Map::percentBatteries = 40.0; //Default 40%
	EntityRobot::numGenes = 16; //Default 16
	EntityBattery::capacity = 5; //Default 5

	Generation robots;
	Map botMap;
	dyn_array<shared_ptr<EntityBase>> entityQueue;
	unsigned iterations = 200;
	dyn_array<float> genAverages;

	//Loop for number of generations
	for(unsigned i=0; i<iterations; i++)
	{
		//Loop for each robot in the generation
		for(unsigned robotIndex=0; robotIndex<robots.size(); robotIndex++)
		{
			//Add current robot and other entities to the entityQueue
			shared_ptr<EntityBase> currentRobot = robots[robotIndex];
			entityQueue.clear();
			entityQueue.push_back(currentRobot);
			//PLANNED add some predators, etc...

			//Clear the map and randomly fill it with all entities in the entityQueue
			botMap.clear();
			botMap.placeEntities(entityQueue);

			//Keep cycling entityQueue while the current robot is sitll alive
			while(currentRobot->isActive() && entityQueue.size()>1)
			{
				for(unsigned j=0; j<entityQueue.size(); j++)
				{
					entityQueue[j]->updateEntity(botMap.grid);
					if(!entityQueue[j]->isActive())
					{
						entityQueue.erase(j);
						j--;
					}
				}
			}
		}

		genAverages.push_back(robots.avgFitness());
		cout << "gen" << robots.getGeneration() << " - " << robots.avgFitness() << endl;

		robots.sortBots();
		robots.breed();

		robots.nextGen();
	}

	return 0;
}

//Return a random number between min and max
int getRandom(int min, int max)
{
	//Set initial state of seeded for rand()
	static bool seeded = false;
	if(!seeded)
	{
		srand(time(0));
		seeded = true;
	}

	return (rand()%(max-min+1)) + min;
}

/* === dyn_array === */
//Copy constructor to deep copy
template <class T>
dyn_array<T>::dyn_array(const dyn_array& obj)
{
	this->array = obj.array;
}
//Overloaded = operator deep copy
template <class T>
dyn_array<T>& dyn_array<T>::operator=(const dyn_array& obj)
{
	//Check if it tries to copy itself
	if(this != &obj)
		this->array = obj.array;

	return *this;
}
//Get the array's current size
template <class T>
unsigned dyn_array<T>::size()
{
	return this->array.size();
}
//Allow pushing new elements to the array
template <class T>
void dyn_array<T>::push_back(T data)
{
	this->array.push_back(data);
}
//Deletes an element from the array if it is in range
template <class T>
void dyn_array<T>::erase(int index)
{	
	if(this->array.size() > 0)
	{
		if(index >= 0 && index <= static_cast<int>(this->array.size())-1)
		{
			this->array.erase(this->array.begin()+index);
		}
		else
		{
			std::cout << "Error(erase): index (" << index << ") out of bounds of array!" << std::endl;
			exit(0);  
		}
	}
	else
	{
		std::cout << "Error(erase): you can't delete items from an empty array!" << std::endl;
		exit(0);
	}
}
//Clear all elements from the array
template <class T>
void dyn_array<T>::clear()
{
	this->array.clear();
}
//Overloaded [] operator can only access elements within the array's boundaries
template <class T>
T& dyn_array<T>::operator[](int index)
{
	if(index >= 0 && index <= static_cast<int>(this->array.size())-1)
		return this->array[index];		
	else
	{
		std::cout << "Error(access): index (" << index << ") out of bounds of array!" << std::endl;
		exit(0);  
	}
}

//Swaps variables (pointers too)
template <class T>
void swapper(T& item1, T& item2)
{
	T temp = item1;
	item1 = item2;
	item2 = temp;
}

/* === EntityBase === */
//Return entity's active state
bool EntityBase::isActive()
{
	return this->active;
}
//Default action for interactions
void EntityBase::interact(shared_ptr<EntityBase> entity)
{
	entity->setPos(this->currentPos);
	this->active = false;
}
//Return the type of the entity for mapping
char EntityBase::getType()
{
	return 'n';
}
//Return the position of the entity
Coord EntityBase::getPos()
{
	return this->currentPos;
}
//Set the position of the entity
void EntityBase::setPos(Coord pt)
{
	this->currentPos = pt;
}
//Default for getting the entity's current power level
int EntityBase::getPower()
{
	return 0;
}
//Default empty function for setting an entity's power
void EntityBase::setPower(int value){}
//Default for updating the map with the entity's current location while it is alive
void EntityBase::updateMap(dyn_array<dyn_array<Point>>& grid, shared_ptr<EntityBase> entity)
{
	if(this->isActive())
	{
		grid[this->currentPos.row][this->currentPos.column].contents = this->getType();
		grid[this->currentPos.row][this->currentPos.column].entity = entity;
	}
}
//Default empty function for updating the entity
void EntityBase::updateEntity(dyn_array<dyn_array<Point>>& grid){}

/* === EntityBattery === */
int EntityBattery::capacity = 0;
//Battery interaction 
void EntityBattery::interact(shared_ptr<EntityBase> entity)
{
	int before=entity->getPower();
	entity->setPower(entity->getPower()+EntityBattery::capacity);
	int after = entity->getPower();

	if(before!=after)
	{
		//Entity can take it's spot
		entity->setPos(this->currentPos);
		this->active = false;
	}
}
//Return battery type for mapping
char EntityBattery::getType()
{
	return 'b';
}
//Get the battery's position
Coord EntityBattery::getPos()
{
	return this->currentPos;
}
//Set the battery's position
void EntityBattery::setPos(Coord pt)
{
	this->currentPos = pt;
}

/* === EntityRobot === */
int EntityRobot::numGenes = 0;
int EntityRobot::instance = 0;
//Constructor for setting up EntityRobot
EntityRobot::EntityRobot(unsigned generation)
{
	EntityRobot::instance++;
	this->id = EntityRobot::instance;
	this->generation = generation;
	this->numMoves = 0;

	//Randomly create X number of genes with sensor states N,E,S,W and move	
	//Don't care, nothing, wall, battery
	char sensor_states[] = {'x','n','w','b'};
	//Up, down, left, right, random
	char moves[] = {'u', 'd', 'l', 'r','?'};

	for(int i=0; i<EntityRobot::numGenes; i++)
	{
		string newGene = "";
		for(int j=0; j<4; j++)
			newGene += sensor_states[getRandom(0,3)];
		newGene += moves[getRandom(0,4)];

		this->genes.push_back(newGene);
	}
}
//Return the robot's ID
unsigned EntityRobot::getID()
{
	return this->id;
}
//Return the robot's current generation
unsigned EntityRobot::getGenID()
{
	return this->generation;
}
//Return the number of moves the robot has survived so far
unsigned EntityRobot::getMoves()
{
	return this->numMoves;
}
//Set the robot's current moves to any non-negative integer
void EntityRobot::setMoves(unsigned moves)
{
	this->numMoves = moves;
}
//Return the next coordinate the robot chooses to move to
Coord EntityRobot::nextCoord(dyn_array<dyn_array<Point>> grid)
{
	this->powerLevel--;
	this->numMoves++;

	//Build a string of sensor states for comparing with genes
	string state = "";
	//Check North boundary
	if(this->currentPos.row>0)
		state += grid[this->currentPos.row-1][this->currentPos.column].contents;
	else
		state += 'w';
	//Check East boundary
	if(this->currentPos.column<Map::numColumns-1)
		state += grid[this->currentPos.row][this->currentPos.column+1].contents;
	else
		state += 'w';
	//Check South boundary
	if(this->currentPos.row<Map::numRows-1)
		state += grid[this->currentPos.row+1][this->currentPos.column].contents;
	else
		state += 'w';
	//Check West boundary
	if(this->currentPos.column>0)
		state += grid[this->currentPos.row][this->currentPos.column-1].contents;
	else
		state += 'w';

	//Search for matching genes
	dyn_array<string> matches;
	char move = '\0';
	for(unsigned i=0; i<this->genes.size(); i++)
	{
		bool match=true;
		//go char by char to see if "don't cares trip it... think moore machine reset"
		if(genes[i][0]!=state[0] && genes[i][0]!='x')
			match=false;
		if(genes[i][1]!=state[1] && genes[i][1]!='x')
			match=false;
		if(genes[i][2]!=state[2] && genes[i][2]!='x')
			match=false;
		if(genes[i][3]!=state[3] && genes[i][3]!='x')
			match=false;
		
		if(match)
			matches.push_back(this->genes[i]);
	}

	//PLANNED - These are picked by first match, not best match (least number of "don't cares"), sort by best match later
	
	//Translate first matched or last gene into move
	if(matches.size()>0)
		move = matches[0][4];
	else
		move = this->genes[this->genes.size()-1][4];

	//Convert direction letters into numbers for switch statement
	int action;
	if(move=='u')action = 0;
	if(move=='r')action = 1;
	if(move=='d')action = 2;
	if(move=='l')action = 3;
	if(move=='?')action = getRandom(0,3);

	//Alter currentPosition into up,right,down,left actions
	Coord nextMove = this->currentPos;
	switch(action)
	{
		//Up
		case 0:
			nextMove.row--;
			break;
		//Right
		case 1:
			nextMove.column++;
			break;
		//Down
		case 2:
			nextMove.row++;
			break;
		//Left
		default:
			nextMove.column--;
			break;
	}

	return nextMove;
}
//Reset the robot to its default state
void EntityRobot::reset()
{
	this->powerLevel = EntityBattery::capacity;
	this->active = true;
	this->numMoves = 0;
}
//Make sure the robot isn't deleted by other entities trying to take its position
void EntityRobot::interact(shared_ptr<EntityBase> entity)
{
	if(this->powerLevel<1)
		this->active = false;
}
//Return the robot's type for mapping
char EntityRobot::getType()
{
	return 'r';
}
//Get the robot's current position
Coord EntityRobot::getPos()
{
	return this->currentPos;
}
//Set the robot's current position
void EntityRobot::setPos(Coord pt)
{
	this->currentPos = pt;
}
//Get the robot's current power level
int EntityRobot::getPower()
{
	return this->powerLevel;
}
//Set the robot's current power level
void EntityRobot::setPower(int value)
{
	this->powerLevel = value;
}
//Update the map with the robot's current location and erase the previous one
void EntityRobot::updateMap(dyn_array<dyn_array<Point>>& grid, shared_ptr<EntityBase> entity)
{
	//Erase previous position 
	if(this->numMoves>0)
	{
		shared_ptr<EntityBase> empty = make_shared<EntityBase>();
		empty->setPos(this->previousPos);
		empty->updateMap(grid,empty);
	}

	//Write this entity to the new position
	grid[this->currentPos.row][this->currentPos.column].contents = this->getType();
	grid[this->currentPos.row][this->currentPos.column].entity = entity;

	//Copy currentPos to previousPos
	this->previousPos = this->currentPos;
}
//Function called to have the robot do something
void EntityRobot::updateEntity(dyn_array<dyn_array<Point>>& grid)
{
	Coord nextPoint = this->nextCoord(grid);

	//Check if the point is out of range
	if(nextPoint.row>=0 && nextPoint.row<Map::numRows && nextPoint.column>=0 && nextPoint.column<Map::numColumns)
	{
		shared_ptr<EntityBase> self = grid[this->currentPos.row][this->currentPos.column].entity;
		shared_ptr<EntityBase> target = grid[nextPoint.row][nextPoint.column].entity;

		//Make entities interact with each other
		self->interact(target);
		target->interact(self);

		//Update map after interactions are complete
		self->updateMap(grid,self);
		target->updateMap(grid,target);
	}

	if(this->powerLevel<1)
		this->active = false;
}

/* === Map === */
int Map::numRows = 0;
int Map::numColumns = 0;
int Map::percentBatteries = 0.0;
//Constructor for map
Map::Map()
{	
	//Fill map with new Points of size row * column
	for(int row=0; row<Map::numRows; row++)
	{
		this->grid.push_back(dyn_array<Point>());
		for(int column=0; column<Map::numColumns; column++)
		{
			this->grid[row].push_back(Point());
			this->grid[row][column].pos.row = row;
			this->grid[row][column].pos.column = column;
		}
	}
	//Fill map with EntityBase instances
	this->clear();
}
//Return the coordinate for a spot on the map that hasn't been taken
Coord Map::freeSpot()
{
	Point free;

	//Keep looping until a free spot (Empty obj) is found
	do
	{
		free = this->grid[getRandom(0,Map::numRows-1)][getRandom(0,Map::numColumns-1)];
	}
	while(free.contents!='n');

	return free.pos;
}
//Place "entities" randomly on the map
void Map::placeEntities(dyn_array<shared_ptr<EntityBase>>& entities)
{
	//Fill entity array with batteries
	int numBatteries = round((Map::numRows*Map::numColumns)*float(Map::percentBatteries/100.0));
	for(int i=0; i<numBatteries; i++)
		entities.push_back(make_shared<EntityBattery>());

	//Randomly place ALL entities on the board
	for(shared_ptr<EntityBase> entity : entities)
	{
		entity->setPos(this->freeSpot());
		entity->updateMap(this->grid,entity);	
	}
}
//Clears all entities from the map by replacing them with a shared instance of EntityBase
void Map::clear()
{
	for(int row=0; row<Map::numRows; row++)
	{
		for(int column=0; column<Map::numColumns; column++)
		{
			shared_ptr<EntityBase> empty = make_shared<EntityBase>();
			empty->setPos(this->grid[row][column].pos);
			empty->updateMap(this->grid,empty);			
		}
	}
}

/* === Generation === */
unsigned Generation::numRobots = 0;
float Generation::mutationRate = 0.0;
//Constructor for Generation
Generation::Generation()
{
	this->genInstance = 1;

	for(unsigned i=0; i<Generation::numRobots; i++)
	{
		this->array.push_back(make_shared<EntityRobot>(this->genInstance));
		this->array[i]->setPower(EntityBattery::capacity);
	}
}
//Get the generation's current number
unsigned Generation::getGeneration()
{
	return this->genInstance;
}
//Bubble sort robots by number of moves they have remained alive on the board
void Generation::sortBots()
{
	for(unsigned i=0,limit=0,iterations=this->array.size()-1; i<iterations; i++,limit++)
		for(unsigned j=this->array.size()-1; j>limit; j--)
			if(this->array[j]->getMoves() > this->array[j-1]->getMoves())
				swapper(this->array[j], this->array[j-1]);
}
//Return the average fitness of all robots in this generation
float Generation::avgFitness()
{
	float avg = 0.0;
	for(unsigned i=0; i<this->array.size(); i++)
		avg+=this->array[i]->getMoves();
	avg /= this->array.size();

	return avg;
}
//Reset robots to defaults and increment generation count
void Generation::nextGen()
{
	for(unsigned i=0; i<this->array.size(); i++)
		this->array[i]->reset();
	
	this->genInstance++;
}
//Breed robots
void Generation::breed()
{
	//Kill off bottom 50% of bots
	int weakBot = Generation::numRobots/2;
	for(unsigned i=weakBot; i<Generation::numRobots; i++)
		this->erase(weakBot);
	
	//Breed
	for(unsigned i=0; i<Generation::numRobots/2; i+=2)
	{
		shared_ptr<EntityRobot> parentA = this->array[i];
		shared_ptr<EntityRobot> parentB = this->array[i+1];

		shared_ptr<EntityRobot> child;

		//Create 2 children
		for(unsigned j=0; j<2; j++)
		{
			this->array.push_back(make_shared<EntityRobot>(this->genInstance+1));
			child = this->array[this->size()-1];
			child->genes.clear();
			for(unsigned k=0; k<unsigned(EntityRobot::numGenes); k+=2)
			{
				child->genes.push_back(parentA->genes[k]);
				child->genes.push_back(parentB->genes[k+1]);
			}
			this->mutate(child->genes);
			swapper(parentA, parentB);
		}
	}
}
//Mutate the genes of a robot
void Generation::mutate(dyn_array<string>& geneArray)
{
	int numMutations = round(EntityRobot::numGenes*float(Generation::mutationRate/100.0));

	//Randomly choose which gene strings will be mutated (non-repeating)
	dyn_array<unsigned> indexes;
	dyn_array<unsigned> selections;
	for(unsigned i=0; i<geneArray.size(); i++)
		indexes.push_back(i);
	for(int i=0; i<numMutations; i++)
	{
		int pos=getRandom(0,indexes.size()-1);
		selections.push_back(indexes[pos]);
		indexes.erase(pos);
	}

	//Randomly change 1 char of each selected gene string 
	char sensor_states[] = {'x','n','w','b'};
	char moves[] = {'u', 'd', 'l', 'r','?'};
	for(unsigned geneIndex : selections)
	{
		char mutation = '\0';
		unsigned stringPos = getRandom(0,4);

		if(stringPos<4)
			mutation = sensor_states[getRandom(0,3)];
		else
			mutation = moves[getRandom(0,4)];

		geneArray[geneIndex][stringPos] = mutation;
	}
}
