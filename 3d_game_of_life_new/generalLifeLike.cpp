/*
Todo:
	-Keep Track of Boxes:
		-initialize one D array of vectors (for faster accessing)
		-calculate grid size (space between each block)
		-divide by 2 to get the center for the first block (won't be exact but who cares)
		-
		-possible box class?

	-Implement rules:
		-Number of live cells needed to live/die(over and under population)/reproduce.
			-make this customizable
		-try to make it run faster that a tortise
		-Add some sort of delay between updates so that boxes are on the screen for more than a frame

	-Figure out skybox so player doesn't zoom off in to the distance

	-Add box placement mechanisms

	-Add menu window for options (eventually)

*/
#include "generalLifeLike.h"

//defining the type for the grid. It is a dynamic array of tuples. Each tuple contains the location
//of the box (generated when the grid is first created) and an int (0 or 1) determining if a box should
//be drawn at that position or not

generalLifeLike::generalLifeLike(int gridsz, std::vector<glm::vec3> initialBoxes, std::vector<glm::vec3> initialNeighbours, int decayStates, std::vector<int> s, std::vector<int> b, bool set2d) {
	survives = s;
	born = b;
	numOfNeighbours = 0;
	is2d = set2d;
	update = true;
	paused = true;
	gridSetup = false;

	setDecayStates(decayStates, initialNeighbours);

	if (is2d) {
		//if the program is set to 2d then it only needs to save the x and y coordinates of neighbours blocks
		fullGridSize = gridsz * gridsz;
	}
	else {
		fullGridSize = gridsz * gridsz * gridsz;
	}

	//creating the char arrays for the grids themselves
	grid = new unsigned char[fullGridSize];
	swapGrid = new unsigned char[fullGridSize];

	std::cout << initialNeighbours.size() << "\n";

	//create the vector to put the neighbours that will be checked for each cell in. Due to the way cells are stored, the program checks if there are too many neighbours in the initial neighbours vector and restricts the number
	
	
	//adds the user inputed neighbours to the neighbour offset list

	setNeighbours(initialNeighbours);

	gridSize = gridsz;
	memset(grid, 0, fullGridSize);
	gridCenter = int(gridSize / 2);

	for (glm::vec3 &box : initialBoxes) {
		if (!is2d) addNewCell(int(box.x), int(box.y), int(box.z), false);
		else addNewCell(int(box.x), int(box.y), 0, false);
	}

	modelMatrices = new glm::mat4[fullGridSize];
	modelMatrices = calculateOffsets(gridSize, is2d);

	//copys data from the temp grid to the new grid
	memcpy(swapGrid, grid, fullGridSize);

	setupGridOutlineModel();
	gridSetup = true;
	
}


void generalLifeLike::setupGridOutlineModel() {
	gridOutlineModel = glm::mat4(1.0f);
	gridOutlineModel = glm::translate(gridOutlineModel, glm::vec3(gridSize / 2, gridSize / 2, gridSize / 2));
	gridOutlineModel = glm::scale(gridOutlineModel, glm::vec3(gridSize, gridSize, gridSize));
}

int generalLifeLike::getState(int xin, int yin, int zin) const {
	unsigned char *gridBoxPtr;
	if(is2d) gridBoxPtr = grid + xin + yin * gridSize;
	else gridBoxPtr = grid + xin + yin * gridSize + zin * gridSize * gridSize;

	return *gridBoxPtr & compareNum;
}

void generalLifeLike::addNewCell(int xin, int yin, int zin, bool playerEdit) {
	unsigned char* gridToEdit;
	if (playerEdit) {
		gridToEdit = swapGrid;
	}
	else {
		gridToEdit = grid;
	}

	unsigned char *gridBoxPtr = gridToEdit + xin + yin * gridSize + zin * gridSize * gridSize;

	if (is2d) zin = 0;

	if (xin < gridSize && yin < gridSize && zin < gridSize && (*gridBoxPtr & compareNum) == 0) {

		*(gridBoxPtr) |= decayStateNum;

		int x, y, z;

		// unsigned int neighb = *gridBoxPtr >> bitShiftNum;

		for (int ni = 0; ni < numOfNeighbours * 3; ni += 3) {
			x = neighbourOffsets[ni];
			y = neighbourOffsets[ni + 1];
			z = neighbourOffsets[ni + 2];

			if (xin + x >= 0 && xin + x < gridSize &&
				yin + y >= 0 && yin + y < gridSize &&
				zin + z >= 0 && zin + z < gridSize &&
				!(x == 0 && y == 0 && z == 0)) {
				gridBoxPtr = gridToEdit + xin + x + (yin + y) * gridSize + (zin + z) * gridSize * gridSize;
				*(gridBoxPtr) += neighbourBitShiftNum * 2;
			}
		}
	}
}

void generalLifeLike::decayCell(int xin, int yin, int zin, bool playerEdit) {
	unsigned char* gridToEdit;
	if (playerEdit) {
		gridToEdit = swapGrid;
	}
	else {
		gridToEdit = grid;
	}

	unsigned char *gridBoxPtr = gridToEdit + xin + yin * gridSize + zin * gridSize * gridSize;

	if (is2d) zin = 0;

	if (xin < gridSize && yin < gridSize && zin < gridSize && (*gridBoxPtr & compareNum) > 0) {
		*(gridBoxPtr) -= 1;

			if ((*gridBoxPtr & compareNum) + 1 == decayStateNum) {
				int x, y, z;
					for (int ni = 0; ni < numOfNeighbours * 3; ni += 3) {
						x = neighbourOffsets[ni];
						y = neighbourOffsets[ni + 1];
						z = neighbourOffsets[ni + 2];

						if (xin + x >= 0 && xin + x < gridSize &&
							yin + y >= 0 && yin + y < gridSize &&
							zin + z >= 0 && zin + z < gridSize &&
							!(x == 0 && y == 0 && z == 0)) {
							gridBoxPtr = gridToEdit + xin + x + (yin + y) * gridSize + (zin + z) * gridSize * gridSize;
							*(gridBoxPtr) -= neighbourBitShiftNum * 2;
						}
					}

			}
	}
}

void generalLifeLike::removeCell(int xin, int yin, int zin, bool playerEdit) {
	unsigned char* gridToEdit;
	if (playerEdit) {
		gridToEdit = swapGrid;
	}
	else {
		gridToEdit = grid;
	}

	unsigned char *gridBoxPtr = gridToEdit + xin + yin * gridSize + zin * gridSize * gridSize;

	if (is2d) zin = 0;

	if (xin < gridSize && yin < gridSize && zin < gridSize && (*gridBoxPtr & compareNum) != 0) {
		int x, y, z;

		if ( decayStateNum == 1 || (playerEdit && (*gridBoxPtr & compareNum) == decayStateNum)) {
			for (int ni = 0; ni < numOfNeighbours * 3; ni += 3) {
				x = neighbourOffsets[ni];
				y = neighbourOffsets[ni + 1];
				z = neighbourOffsets[ni + 2];
				if (xin + x >= 0 && xin + x < gridSize &&
					yin + y >= 0 && yin + y < gridSize &&
					zin + z >= 0 && zin + z < gridSize &&
					!(x == 0 && y == 0 && z == 0)) {
					gridBoxPtr = gridToEdit + xin + x + (yin + y) * gridSize + (zin + z) * gridSize * gridSize;
					*(gridBoxPtr) -= neighbourBitShiftNum * 2;
				}
			}

		}
		gridBoxPtr = gridToEdit + xin + yin * gridSize + zin * gridSize * gridSize;
		*(gridBoxPtr) &= ~compareNum;
	}
}


void generalLifeLike::updateGrid() {
	int newStatus;
	unsigned int neighbours;
	unsigned char* gridPtr;

	//creating a temporary grid copy so that the current grid is not overwritten while updating
	m.lock();
	if (!is2d) memcpy(swapGrid, grid, gridSize * gridSize * gridSize);
	else memcpy(swapGrid, grid, gridSize * gridSize);
	
	m.unlock();

	//std::tuple <glm::vec3, int> temp;
	int x, y = 0;
	gridPtr = swapGrid;
	for (int z = 0; z < gridSize; z++) {
		y = 0;
		do {
			x = 0;
			do {

				while (*gridPtr == 0) {
					gridPtr++;

					if (++x >= gridSize) goto rowDone;
				}
				neighbours = *gridPtr >> bitShiftNum;
				if ((*gridPtr & compareNum) == 0) {
					for (int num : born) {
						if (neighbours == num) {
							addNewCell(x, y, z, false);
							goto next;
						}

					}

				}
				else if ((*gridPtr & compareNum) == decayStateNum) {
					for (int num : survives) {
						if (neighbours == num)  goto next;
					}
					decayCell(x, y, z, false);
				}
				else if ((*gridPtr & compareNum) > 0) {
					decayCell(x, y, z, false);
				}

			next:;
			x++;
			gridPtr++;

			} while (x < gridSize);

		rowDone:;
			y++;
		} while (y < gridSize);
		if (is2d) goto skip;
	}
	skip:;
}

void generalLifeLike::render(unsigned int &VAO, Shader &shaderProgram) {

	glBindVertexArray(VAO);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	unsigned char* gridPtr;
	gridPtr = swapGrid;
	int y, x = 0;
	int neighbours;
	m.lock();
	//float mult_num = 1.0 / gridSize;

	for (int z = 0; z < gridSize; z++)
	{
		y = 0;
		do {
			x = 0;
			do {

				while (*gridPtr == 0) {
					gridPtr++;
					if (++x >= gridSize) goto rowDone;
				}


				if ((*gridPtr & compareNum) != 0) {
					glm::mat4 model = modelMatrices[x + y * gridSize + z * gridSize * gridSize];
					//glm::mat4 model = glm::mat4(1.0f);
					//translate the box to the right position. adding .5 since the boxes are centered at the passed in vector
					//model = glm::translate(model, glm::vec3(x + 0.5, y + 0.5, z + 0.5));
					//assign the model uniform
					shaderProgram.setMat4("model", model);

					//glm::vec3 boxColor = glm::vec3(neighbours * 0.1, neighbours * 0.15, 1);
					//glm::vec3 boxColor = glm::vec3(x * mult_num, y * mult_num, z * mult_num);

					//shaderProgram.setVec3("inColor", boxColor);
					//draw the box
					glBindVertexArray(VAO);
					glDrawArrays(GL_TRIANGLES, 0, 36);
				}
				gridPtr++;
				x++;

			} while (x < gridSize);
		rowDone:;
			y++;
		} while (y < gridSize);

		if (is2d) goto skip;

	}

	skip:;
	//std::cout << sizeof(unsigned int) << " " << sizeof(unsigned char) << " \n";
	m.unlock();
	//Sleep(10);

}

void generalLifeLike::renderGridOutline(unsigned int &VAO, Shader &shaderProgram) {
	
	//Drawing the grid outline
	glm::vec3 boxColor = glm::vec3(0.9f, 0.9f, 1.0f);
	shaderProgram.setVec3("inColor", boxColor);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	shaderProgram.setMat4("model", gridOutlineModel);

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

glm::mat4* calculateOffsets(int gridSize, bool is2d) {
	glm::mat4 *translations;
	if (is2d) translations = new glm::mat4[gridSize * gridSize];
	else translations = new glm::mat4[gridSize * gridSize * gridSize];

	for (int z = 0; z < gridSize; z++) {
		for (int y = 0; y < gridSize; y++) {
			for (int x = 0; x < gridSize; x++) {
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(x + 0.5, y + 0.5, z + 0.5));
				translations[x + y * gridSize + z * gridSize * gridSize] = model;
			}
		}
		if (is2d) goto skip;
	}
	skip:;
	return translations;
}

//--------------------------------------------------------------Getters--------------------------------------------------------------------------------------------------------

bool generalLifeLike::inGrid(int x, int y, int z) {
	return (x > 0 && y > 0
		&& z > 0 && x < gridSize
		&& y < gridSize && z < gridSize);
}

int generalLifeLike::getGridSize1d() {
	return gridSize;
}

int generalLifeLike::getFullGridSize() {
	return fullGridSize;
}

std::vector<int> generalLifeLike::getBorn() {
	return born;
}

std::vector<int> generalLifeLike::getSurvives() {
	return survives;
}

std::vector<int> generalLifeLike::getNeighbours() {
	return neighbourOffsets;
}

int generalLifeLike::getNumOfDecayStates() {
	return decayStateNum;
}

bool generalLifeLike::getIs2d() {
	return is2d;
}


//--------------------------------------------Setters----------------------------------------------------------------------------------------------------------------------------------------------------------

void generalLifeLike::resetAllDecayingCells() {
	unsigned char* gridPtr;
	gridPtr = swapGrid;

	m.lock();
	int x, y;
	for (int z = 0; z < gridSize; z++)
	{
		y = 0;
		do {
			x = 0;
			do {

				while (*gridPtr == 0) {
					gridPtr++;
					if (++x >= gridSize) goto rowDone;
				}

				if ((*gridPtr & compareNum) > 0) {
					removeCell(x, y, z, false);
					addNewCell(z, y, z, false);
				}
				
				gridPtr++;
				x++;

			} while (x < gridSize);
		rowDone:;
			y++;
		} while (y < gridSize);

		if (is2d) goto skip;

	}

skip:;
	//std::cout << sizeof(unsigned int) << " " << sizeof(unsigned char) << " \n";
	m.unlock();
}

void generalLifeLike::setBorn(std::vector<int> b) {
	if (paused){
		born = b;
	}
}

void generalLifeLike::setSurvives(std::vector<int> s) {
	if (paused) {
		survives = s;
	}
}

void generalLifeLike::setNeighbours(std::vector<glm::vec3> neighs) {
	if (paused) {
		for (glm::vec3 neighbour : neighs) {

			if (!is2d) addToNeighbours(neighbour.x, neighbour.y, neighbour.z);
			else addToNeighbours(neighbour.x, neighbour.y);

			if (numOfNeighbours > numOfTotalPossibleNeighbours) break;
		}
	}
}

void generalLifeLike::setDecayStates(int decayStates, std::vector<glm::vec3> initialNeighbours) {
	if (paused) {
		decayStateNum = decayStates;
		bitShiftNum = 1;
		neighbourBitShiftNum = 1;
		while (decayStateNum > neighbourBitShiftNum) {
			bitShiftNum++;
			neighbourBitShiftNum *= 2;
		}
		compareNum = neighbourBitShiftNum * 2 - 1;

		//Calculate the number of neighbours possible given the number of decay states
		numOfTotalPossibleNeighbours = 255 - std::pow(2, bitShiftNum);
		std::cout << "With the current number of decay states, you can have at most: " << numOfTotalPossibleNeighbours << " neighbours for a cell. \n";

		if (numOfTotalPossibleNeighbours < 1) {
			std::cout << "ERROR :: TOO MANY DECAY STATES \n";
		}

		//this resizes the neighbours vector if the number of neighbour offsets is now too large
		if (gridSetup) {
			while (numOfTotalPossibleNeighbours < numOfNeighbours) {
				neighbourOffsets.pop_back();
				neighbourOffsets.pop_back();
				neighbourOffsets.pop_back();
				numOfNeighbours--;
			}

			resetAllDecayingCells();
		}

		
	}
}

void generalLifeLike::setUpGrid(int gridSz) {
	
}


void generalLifeLike::resizeGrid(int newGridSize) {
	if (paused) {
		int tempFullGridSize;
		if (is2d) {
			//if the program is set to 2d then it only needs to save the x and y coordinates of neighbours blocks
			tempFullGridSize = newGridSize;
		}
		else {
			tempFullGridSize = newGridSize;
		}

		//creating the char arrays for the grids themselves
		unsigned char* newGrid = new unsigned char[tempFullGridSize];
		unsigned char* newTempGrid = new unsigned char[tempFullGridSize];

		unsigned char* gridPtr;
		gridPtr = swapGrid;

		if (newGridSize >= gridSize) {
			m.lock();
			int x, y;
			for (int z = 0; z < gridSize; z++)
			{
				y = 0;
				do {
					x = 0;
					do {

						while (*gridPtr == 0) {
							gridPtr++;
							if (++x >= gridSize) goto rowDone;
						}



						gridPtr++;
						x++;

					} while (x < gridSize);
				rowDone:;
					y++;
				} while (y < gridSize);

				if (is2d) goto skip;

			}

			skip:;
			//std::cout << sizeof(unsigned int) << " " << sizeof(unsigned char) << " \n";
			m.unlock();

			gridSize = newGridSize;
			memset(grid, 0, fullGridSize);
			gridCenter = int(gridSize / 2);

			memcpy(swapGrid, grid, fullGridSize);
		}
	}
	
}

//--------------------------------------------Mutators------------------------------------------------------------------------------------------------------------------------------------------------- 

void addBlocks(std::vector<int>& listToEdit, int numToAdd, int optionalEndRange) {
	if (numToAdd > 0 && optionalEndRange > numToAdd) {
		for (int i = numToAdd; i < optionalEndRange; i++) {
			listToEdit.push_back(i);
		}
	}
	else if (numToAdd > 0) {
		listToEdit.push_back(numToAdd);
	}
}

void removeBlocks(std::vector<int>& listToEdit, int numToRemove) {
	std::vector<int> indexsToRemove{};
		for (int i = 0; i < listToEdit.size(); i++) {
			if (listToEdit[i] == numToRemove) {
				indexsToRemove.push_back(i);
			}
		}
		int numRemoved = 0;
		for (int i : indexsToRemove) {
			listToEdit.erase(listToEdit.begin() + i - numRemoved);
			numRemoved++;
		}
}

void generalLifeLike::addBlocksToBorn(int numToAdd, int optionalEndRange) {
	if (paused) {
		addBlocks(born, numToAdd, optionalEndRange);
	}
}

void generalLifeLike::removeBlocksFromBorn(int numToRemove) {
	if (paused) {
		removeBlocks(born, numToRemove);
	}
}

void generalLifeLike::addBlocksToSurvives(int numToAdd, int optionalEndRange) {
	if (paused) {
		addBlocks(survives, numToAdd, optionalEndRange);
	}
}

void generalLifeLike::removeBlocksFromSurvives(int numToRemove) {
	if (paused) {
		removeBlocks(survives, numToRemove);
	}
}
void generalLifeLike::addToNeighbours(int x, int y, int z) {
	//This function adds a new neighbour offset to the list of offsets. z is set to zero by default incase the list is 2d, and the z value is not given

	bool alreadyInList = false;

	//First, it checks if the offset is already in the list. If it is, then it won't add the new block
	for (int ni = 0; ni < numOfNeighbours; ni++) {
		if (neighbourOffsets[ni] == x && neighbourOffsets[ni + 1] == y && neighbourOffsets[ni + 2] == z) {
			alreadyInList = true;
			break;
		}
	}
	//If it's not in the list, then it is added
	if (!alreadyInList) {
		neighbourOffsets.push_back(x);
		neighbourOffsets.push_back(y);
		neighbourOffsets.push_back(z);
		numOfNeighbours++;
	}
	
}

void generalLifeLike::removeFromNeighbours(int x, int y, int z) {
	//This function searches the list of neighbours and removes the neighbour offset if it is found.

	for (int ni = 0; ni < numOfNeighbours; ni++) {
		if (neighbourOffsets[ni] == x && neighbourOffsets[ni + 1] == y && neighbourOffsets[ni + 2] == z) {
			neighbourOffsets.erase(neighbourOffsets.begin() + ni);
			neighbourOffsets.erase(neighbourOffsets.begin() + ni);
			neighbourOffsets.erase(neighbourOffsets.begin() + ni);
			break;
		}
	}
}


void createMooreNeighbourhood() {
	if (paused) {
		while (numOfNeighbours > 0) {
			neighbourOffsets.pop_back();
			neighbourOffsets.pop_back();
			neighbourOffsets.pop_back();
			numOfNeighbours--;
		}

		for (int x = -1; x < 2; x++) {
			for (int y = -1; y < 2; y++) {
				for (int z = -1; z < 2; z++) {
					if (!(x == 0 && y == 0 && z == 0)) {
						addToNeighbours(x, y, z);
					}

				}
			}
		}
	}

}

void createVonNeumanNeighbourhood() {
	if (paused) {
		while (numOfNeighbours > 0) {
			neighbourOffsets.pop_back();
			neighbourOffsets.pop_back();
			neighbourOffsets.pop_back();
			numOfNeighbours--;
		}

		addToNeighbours(-1, 0, 0);
		addToNeighbours(0, -1, 0);
		addToNeighbours(1, 0, 0);
		addToNeighbours(0, 1, 0);
		if (!is2d) {
			addToNeighbours(0, 0, 1);
			addToNeighbours(0, 0, -1);
		}
	
	}
	
}

void generateRandomSeed(int numBlocksToGenerate) {

}

