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
#include "gridManager.h"

//defining the type for the grid. It is a dynamic array of tuples. Each tuple contains the location
//of the box (generated when the grid is first created) and an int (0 or 1) determining if a box should
//be drawn at that position or not

gameOfLife2d::gameOfLife2d(int gridsz, std::vector<glm::vec3> initialBoxes, int lL, int lU, int rL, int rU) : gridManager(gridsz) {
	//grid = new unsigned char[gridsz * gridsz * gridsz];
	//tempgrid = new unsigned char[gridsz * gridsz * gridsz];
	//gridSize = gridsz;
	//memset(grid, 0, gridsz * gridsz * gridsz);
	//gridCenter = int(gridSize / 2);

	for (glm::vec3 &box : initialBoxes) {
		editBox(int(box.x), int(box.y), int(box.z), 1);
	}

	memcpy(tempgrid, grid, gridSize * gridSize);

	//update = true;
	//notPaused = false;
	replicateLower = rL;
	replicateUpper = rU;
	liveLower = lL;
	liveUpper = lU;
}
//
//game_of_life_3d::~game_of_life_3dr() {
//	delete[] grid;
//	delete[] tempgrid;
//}

int gameOfLife2d::getState(int xin, int yin, int zin) const {
	unsigned char *gridBoxPtr = grid + xin + yin * gridSize;

	return *gridBoxPtr & 0x01;
}


void gameOfLife2d::editBox(int xin, int yin, int zin, int editType, bool playerEdit) const {
	unsigned char* gridToEdit;
	if (playerEdit) {
		gridToEdit = tempgrid;
	}
	else {
		gridToEdit = grid;
	}

	if (xin < gridSize && yin < gridSize) {
		//1 adds a box
		unsigned char *gridBoxPtr = gridToEdit + xin + yin * gridSize;

		if (editType == 1 && !(*gridBoxPtr & 0x01)) {
			*(gridBoxPtr) |= 0x01;
			for (int x = -1; x < 2; x++) {
				for (int y = -1; y < 2; y++) {
						//this nightmare if statement checks for boxes that are out of range and makes sure to not count the actual box itself
						if (xin + x >= 0 && xin + x < gridSize &&
							yin + y >= 0 && yin + y < gridSize &&
							!(x == 0 && y == 0 )) {
							gridBoxPtr = gridToEdit + xin + x + (yin + y) * gridSize;
							*(gridBoxPtr) += 0x02;
						}
					}
			}


		}
		//0 removes it
		else if (editType == 0 && *gridBoxPtr & 0x01) {
			*(gridBoxPtr) &= ~0x01;

			for (int x = -1; x < 2; x++) {
				for (int y = -1; y < 2; y++) {
						//this nightmare if statement checks for boxes that are out of range and makes sure to not count the actual box itself
						if (xin + x >= 0 && xin + x < gridSize &&
							yin + y >= 0 && yin + y < gridSize &&
							!(x == 0 && y == 0)) {
							gridBoxPtr = gridToEdit + xin + x + (yin + y) * gridSize;
							*(gridBoxPtr) -= 0x02;
						}
				}
			}

		}
		if (playerEdit) memcpy(grid, tempgrid, gridSize * gridSize);

	}
}


void gameOfLife2d::updateGrid() {
	int newStatus;
	unsigned int neighbours;
	unsigned char* gridPtr;

	//creating a temporary grid copy so that the current grid is not overwritten while updating
	m.lock();
	memcpy(tempgrid, grid, gridSize * gridSize);
	m.unlock();

	//std::tuple <glm::vec3, int> temp;
	int x, y = 0;
	gridPtr = tempgrid;
	for (int y = 0; y < gridSize; y++) {
			x = 0;
			do {

				while (*gridPtr == 0) {
					gridPtr++;

					if (++x >= gridSize) goto rowDone;
				}
				neighbours = *gridPtr >> 1;

				if (!(*gridPtr & 0x01) && *gridPtr && neighbours >= replicateLower && neighbours <= replicateUpper) {
					gameOfLife2d::editBox(x, y, 0, 1);

				}
				else if (*gridPtr & 0x01 && *gridPtr && neighbours < liveLower || *gridPtr & 0x01 == 1 && neighbours > liveUpper) {
					gameOfLife2d::editBox(x, y, 0, 0);
				}
				x++;
				gridPtr++;

			} while (x < gridSize);

		rowDone:;
	}

}

void gameOfLife2d::render(unsigned int &VAO, Shader &shaderProgram) {

	unsigned char* gridPtr;
	gridPtr = tempgrid;
	int x = 0;
	int neighbours;
	m.lock();
	for (int y = 0; y < gridSize; y++)
	{
			x = 0;
			do {

				while (*gridPtr == 0) {
					gridPtr++;
					if (++x >= gridSize) goto rowDone;
				}

				if (*gridPtr & 0x01) {
					neighbours = *gridPtr >> 1;

					glm::mat4 model = glm::mat4(1.0f);
					//translate the box to the right position. adding .5 since the boxes are centered at the passed in vector
					model = glm::translate(model, glm::vec3(x + 0.5, y + 0.5, 0.5f));
					//assign the model uniform
					shaderProgram.setMat4("model", model);

					glm::vec3 boxColor;
					if (neighbours >= liveUpper && neighbours <= liveUpper)
						boxColor = glm::vec3(0.05f, 0.7f, 0.6f);
					else if (neighbours >= replicateLower && neighbours <= replicateUpper)
						boxColor = glm::vec3(0.3f, 0.2f, 0.9f);
					else
						boxColor = glm::vec3(((neighbours - liveUpper) * (neighbours - liveUpper)) % 10 * 0.04,
						((neighbours - liveUpper) * (neighbours - liveUpper)) % 10 * 0.065,
							((neighbours - liveUpper) * (neighbours - liveUpper)) % 10 * 0.1);


					shaderProgram.setVec3("inColor", boxColor);
					//draw the box
					glBindVertexArray(VAO);
					glDrawArrays(GL_TRIANGLES, 0, 36);
				}
				gridPtr++;
				x++;

			} while (x < gridSize);
		rowDone:;
	}
	//std::cout << sizeof(unsigned int) << " " << sizeof(unsigned char) << " \n";
	m.unlock();
	//Sleep(10);

}