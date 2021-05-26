#pragma once
#include "main.h"
#include <tuple>
#include <vector>
#include <windows.h>
#include "Shader.h"

class gridManager {

public:
	//Variables to store the grid size and grid itself in a 1D array
	unsigned int gridSize;
	int gridCenter;
	unsigned char* grid;
	unsigned char* tempgrid;
	std::mutex m, m2;
	//Variables to store the outer limit of the checking functions. This will save time in the 
	//grid update algorithm, as it will not need to check the entire grid.
	

	//1 is currently updating grid, 0 is not updating
	bool update;
	bool notPaused;

	gridManager(int gridsz);
	~gridManager();

	//void initializeGrid(int gridsz, std::vector<glm::vec3> initialBoxes);
	virtual void editBox(int xin, int yin, int zin, int editType, bool playerEdit = false) const = 0;
	virtual int getState(int xin, int yin, int zin) const = 0;
	virtual void updateGrid() = 0;
	virtual void render(unsigned int &VAO, Shader &shaderProgram) = 0;
};


class gameOfLife3d : public gridManager {

public:
	int liveLower, liveUpper, replicateLower, replicateUpper;
	//Variables to store the outer limit of the checking functions. This will save time in the 
	//grid update algorithm, as it will not need to check the entire grid.

	gameOfLife3d(int gridsz, std::vector<glm::vec3> initialBoxes, int lL, int lU, int rL, int rU);

	//void initializeGrid(int gridsz, std::vector<glm::vec3> initialBoxes);
	void editBox(int xin, int yin, int zin, int editType, bool playerEdit = false) const;
	int getState(int xin, int yin, int zin) const;
	void updateGrid();
	void render(unsigned int &VAO, Shader &shaderProgram);
};



class gameOfLife2d : public gridManager {

public:
	int liveLower, liveUpper, replicateLower, replicateUpper;
	//Variables to store the outer limit of the checking functions. This will save time in the 
	//grid update algorithm, as it will not need to check the entire grid.

	gameOfLife2d(int gridsz, std::vector<glm::vec3> initialBoxes, int lL, int lU, int rL, int rU);

	//void initializeGrid(int gridsz, std::vector<glm::vec3> initialBoxes);
	void editBox(int xin, int yin, int zin, int editType, bool playerEdit = false) const;
	int getState(int xin, int yin, int zin) const;
	void updateGrid();
	void render(unsigned int &VAO, Shader &shaderProgram);
};

