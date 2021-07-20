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

generalLifeLike::generalLifeLike(int gridsz, std::vector<glm::vec3> initialBoxes, std::vector<glm::vec3> initialNeighbours, int decayStates, std::vector<int> s, std::vector<int> b, bool set2d) {
	survives = s;
	born = b;
	numOfNeighbours = 0;

	decayStateNum = decayStates;
	bitShiftNum = 1;
	testTwo = 1;
	while (decayStateNum > testTwo) {
		bitShiftNum++;
		testTwo *= 2;
	}
	compareNum = testTwo * 2 - 1;

	std::cout << "comp : " << compareNum << " bit: " << bitShiftNum << "\n";
	////world's most efficent algorithm
	//int num = states[0];
	//if (num == 0) {
	//	state0Prev = states[states.size() - 1];
	//}
	//else if (num == 1) {
	//	state1Prev = states[states.size() - 1];
	//}
	//else if (num == 2) {
	//	state2Prev = states[states.size() - 1];
	//}
	//else {
	//	state3Prev = states[states.size() - 1];
	//}
	//for (int num = 1; num < states.size(); num++) {
	//	if (num == 0) {
	//		state0Prev = states[states.size() - 1];
	//	}
	//	else if (num == 1) {
	//		state1Prev = states[states.size() - 1];
	//	}
	//	else if (num == 2) {
	//		state2Prev = states[states.size() - 1];
	//	}
	//	else {
	//		state3Prev = states[states.size() - 1];
	//	}
	//}

	int possible_neighbours = 255 - std::pow(2, bitShiftNum);
	if (possible_neighbours < 1) {
		std::cout << "ERROR :: TOO MANY DECAY STATES \n";
	}


	if (set2d) {
		is2d = true;
		grid = new unsigned char[gridsz * gridsz];
		tempgrid = new unsigned char[gridsz * gridsz];



		//Creating the array storing the user specified neighbours to check
		if (initialNeighbours.size() < possible_neighbours) {
			neighbourOffsets = new unsigned int[initialNeighbours.size() * 2];
		}
		else {
			neighbourOffsets = new unsigned int[possible_neighbours * 2];
		}

		int curIndex = 0;

		for (glm::vec3 neighbour : initialNeighbours) {
			//if (curIndex >= possible_neighbours) break;

			numOfNeighbours++;
			neighbourOffsets[curIndex] = neighbour.x;
			neighbourOffsets[curIndex + 1] = neighbour.y;
			curIndex += 2;
		}



		gridSize = gridsz;
		memset(grid, 0, gridsz * gridsz);
		gridCenter = int(gridSize / 2);

		for (glm::vec3 &box : initialBoxes) {
			editBox(int(box.x), int(box.y), 0, 1);
		}

		memcpy(tempgrid, grid, gridSize * gridSize);
	}
	else {
		is2d = false;
		grid = new unsigned char[gridsz * gridsz * gridsz];
		tempgrid = new unsigned char[gridsz * gridsz * gridsz];



		//Creating the array storing the user specified neighbours to check
		if (initialNeighbours.size() < possible_neighbours) {
			neighbourOffsets = new unsigned int[initialNeighbours.size() * 3];
		}
		else {
			neighbourOffsets = new unsigned int[possible_neighbours * 3];
		}
		
		int curIndex = 0;
		
		for (glm::vec3 neighbour : initialNeighbours) {
			//if (curIndex >= possible_neighbours) break;

			numOfNeighbours++;
			neighbourOffsets[curIndex] = neighbour.x;
			neighbourOffsets[curIndex + 1] = neighbour.y;
			neighbourOffsets[curIndex + 2] = neighbour.z;
			curIndex += 3;
		}



		gridSize = gridsz;
		memset(grid, 0, gridsz * gridsz * gridsz);
		gridCenter = int(gridSize / 2);

		for (glm::vec3 &box : initialBoxes) {
			editBox(int(box.x), int(box.y), int(box.z), 1);
			std::cout << "added box: " << box.x << " " << box.y << " " << box.z << " \n";
		}

		memcpy(tempgrid, grid, gridSize * gridSize * gridSize);
	}


	update = true;
	notPaused = false;
}
//
//game_of_life_3d::~game_of_life_3dr() {
//	delete[] grid;
//	delete[] tempgrid;
//}

int generalLifeLike::getState(int xin, int yin, int zin) const {
	unsigned char *gridBoxPtr = grid + xin + yin * gridSize + zin * gridSize * gridSize;

	return *gridBoxPtr & 0x01;
}


void generalLifeLike::editBox(int xin, int yin, int zin, int editType, bool playerEdit) const {
	unsigned char* gridToEdit;
	if (playerEdit) {
		gridToEdit = tempgrid;
	}
	else {
		gridToEdit = grid;
	}

	//std::cout << xin << " " << yin << " " << zin << "\n";
	if (xin < gridSize && yin < gridSize && zin < gridSize && !is2d) {
		unsigned char *gridBoxPtr = gridToEdit + xin + yin * gridSize + zin * gridSize * gridSize;

		
		if (editType == 1 && (*gridBoxPtr & compareNum) == 0) {
			
			*(gridBoxPtr) |= decayStateNum;

			if (xin == 24 && yin == 24) {
				xin = 24;
			}

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
					*(gridBoxPtr) += testTwo * 2;
				}
			}
		}
		else if (editType == 0 && (*gridBoxPtr & compareNum) != 0) {

			*(gridBoxPtr) &= ~compareNum;
			int x, y, z;

			if (playerEdit || decayStateNum == 1) {
				for (int ni = 0; ni < numOfNeighbours * 3; ni += 3) {
					x = neighbourOffsets[ni];
					y = neighbourOffsets[ni + 1];
					z = neighbourOffsets[ni + 2];
					if (xin + x >= 0 && xin + x < gridSize &&
						yin + y >= 0 && yin + y < gridSize &&
						zin + z >= 0 && zin + z < gridSize &&
						!(x == 0 && y == 0 && z == 0)) {
						gridBoxPtr = gridToEdit + xin + x + (yin + y) * gridSize + (zin + z) * gridSize * gridSize;
						*(gridBoxPtr) -= testTwo * 2;
					}
				}
			}
			

		}
		else if (!playerEdit && editType == 2 && (*gridBoxPtr & compareNum) > 0) {
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
						*(gridBoxPtr) -= testTwo * 2;
					}
				}

			}

		}
		if (playerEdit) memcpy(grid, tempgrid, gridSize * gridSize * gridSize);
	}
		
	else if (xin < gridSize && yin < gridSize && zin < gridSize) {
		unsigned char *gridBoxPtr = gridToEdit + xin + yin * gridSize;

		if (editType == 1 && (*gridBoxPtr & testTwo) == 0) {
			*(gridBoxPtr) |= decayStateNum;

			int x, y;

			for (int ni = 0; ni < numOfNeighbours * 2; ni += 2) {
				x = neighbourOffsets[ni];
				y = neighbourOffsets[ni + 1];
				if (xin + x >= 0 && xin + x < gridSize &&
					yin + y >= 0 && yin + y < gridSize &&
					!(x == 0 && y == 0)) {
					gridBoxPtr = gridToEdit + xin + x + (yin + y) * gridSize;
					*(gridBoxPtr) += bitShiftNum * 2;
				}
			}
		}
		else if (editType == 0 && (*gridBoxPtr & testTwo) == 1) {
			*(gridBoxPtr) &= ~0x01;
		}
		else if (editType == 2) {

			if ((*gridBoxPtr & compareNum) == compareNum - 1) {
				int x, y;

				for (int ni = 0; ni < numOfNeighbours * 2; ni += 2) {
					x = neighbourOffsets[ni];
					y = neighbourOffsets[ni + 1];
					if (xin + x >= 0 && xin + x < gridSize &&
						yin + y >= 0 && yin + y < gridSize &&
						!(x == 0 && y == 0)) {
						gridBoxPtr = gridToEdit + xin + x + (yin + y) * gridSize;
						*(gridBoxPtr) -= bitShiftNum * 2;
					}
				}
			}

			*(gridBoxPtr) -= 0x01;
		}
		if (playerEdit) memcpy(grid, tempgrid, gridSize * gridSize);

	}
}



void generalLifeLike::editState0(int xin, int yin, int zin, unsigned char *gridBoxPtr, unsigned char* gridToEdit) {
	*(gridBoxPtr) &= ~0x01;
	if (bitShiftNum > 1) *(gridBoxPtr) &= ~0x02;
	int x, y, z;

	for (int ni = 0; ni < numOfNeighbours; ni += 3) {
		x = neighbourOffsets[ni];
		y = neighbourOffsets[ni + 1];
		z = neighbourOffsets[ni + 2];
		if (xin + x >= 0 && xin + x < gridSize &&
			yin + y >= 0 && yin + y < gridSize &&
			zin + z >= 0 && zin + z < gridSize &&
			!(x == 0 && y == 0 && z == 0)) {
			gridBoxPtr = gridToEdit + xin + x + (yin + y) * gridSize + (zin + z) * gridSize * gridSize;
			*(gridBoxPtr) -= 2 * bitShiftNum;
		}
	}
	
}

void generalLifeLike::editState1(int xin, int yin, int zin, unsigned char *gridBoxPtr, unsigned char* gridToEdit) {
	*(gridBoxPtr) |= 0x01;
	if (bitShiftNum > 1)*(gridBoxPtr) &= ~0x02;
	
	int x, y, z;

	for (int ni = 0; ni < numOfNeighbours; ni += 3) {
		x = neighbourOffsets[ni];
		y = neighbourOffsets[ni];
		z = neighbourOffsets[ni];
		if (xin + x >= 0 && xin + x < gridSize &&
			yin + y >= 0 && yin + y < gridSize &&
			zin + z >= 0 && zin + z < gridSize &&
			!(x == 0 && y == 0 && z == 0)) {
			gridBoxPtr = gridToEdit + xin + x + (yin + y) * gridSize + (zin + z) * gridSize * gridSize;
			*(gridBoxPtr) += 2 * bitShiftNum;
		}
	}


}
/*
void generalLifeLike::editState2(int xin, int yin, int zin, unsigned char *gridBoxPtr, unsigned char* gridToEdit) {
	*(gridBoxPtr) &= ~0x01;
	*(gridBoxPtr) |= 0x02;
	int x, y, z;

	for (int ni = 0; ni < numOfNeighbours; ni += 3) {
		x = neighbourOffsets[ni];
		y = neighbourOffsets[ni];
		z = neighbourOffsets[ni];
		if (xin + x >= 0 && xin + x < gridSize &&
			yin + y >= 0 && yin + y < gridSize &&
			zin + z >= 0 && zin + z < gridSize &&
			!(x == 0 && y == 0 && z == 0)) {
			gridBoxPtr = gridToEdit + xin + x + (yin + y) * gridSize + (zin + z) * gridSize * gridSize;
			*(gridBoxPtr) -= 2 * bitShiftNum;
		}
	}

}

void generalLifeLike::editState3(int xin, int yin, int zin, unsigned char *gridBoxPtr, unsigned char* gridToEdit) {
	*(gridBoxPtr) |= 0x01;
	*(gridBoxPtr) |= 0x02;
	int x, y, z;

	for (int ni = 0; ni < numOfNeighbours; ni += 3) {
		x = neighbourOffsets[ni];
		y = neighbourOffsets[ni];
		z = neighbourOffsets[ni];
		if (xin + x >= 0 && xin + x < gridSize &&
			yin + y >= 0 && yin + y < gridSize &&
			zin + z >= 0 && zin + z < gridSize &&
			!(x == 0 && y == 0 && z == 0)) {
			gridBoxPtr = gridToEdit + xin + x + (yin + y) * gridSize + (zin + z) * gridSize * gridSize;
			*(gridBoxPtr) += 2 * bitShiftNum;
		}
	}
}
*/



void generalLifeLike::updateGrid() {
	int newStatus;
	unsigned int neighbours;
	unsigned char* gridPtr;

	//creating a temporary grid copy so that the current grid is not overwritten while updating
	m.lock();
	if (!is2d) memcpy(tempgrid, grid, gridSize * gridSize * gridSize);
	else memcpy(tempgrid, grid, gridSize * gridSize);
	
	m.unlock();

	//std::tuple <glm::vec3, int> temp;
	int x, y = 0;
	gridPtr = tempgrid;
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
							editBox(x, y, z, 1);
							goto next;
						}

					}

				}
				else if ((*gridPtr & compareNum) == decayStateNum) {
					for (int num : survives) {
						if (neighbours == num)  goto next;
					}
					editBox(x, y, z, 2);
					//editBox(x, y, z, 0);
				}
				else if ((*gridPtr & compareNum) > 0) {
					editBox(x, y, z, 2);
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

	unsigned char* gridPtr;
	gridPtr = tempgrid;
	int y, x = 0;
	int neighbours;
	m.lock();
	float mult_num = 1.0 / gridSize;
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
					//neighbours = *gridPtr >> bitShiftNum;
					//std::cout << "neigherbours: " << neighbours << " compare num: " << compareNum << " compare and grid: " << (*gridPtr & compareNum) << "grid" << (*gridPtr - (neighbours << bitShiftNum)) << " decay" << decayStateNum << "\n";

					glm::mat4 model = glm::mat4(1.0f);
					//translate the box to the right position. adding .5 since the boxes are centered at the passed in vector
					model = glm::translate(model, glm::vec3(x + 0.5, y + 0.5, z + 0.5));
					//assign the model uniform
					shaderProgram.setMat4("model", model);

					//glm::vec3 boxColor = glm::vec3(neighbours * 0.1, neighbours * 0.15, 1);
					glm::vec3 boxColor = glm::vec3(x * mult_num, y * mult_num, z * mult_num);

					shaderProgram.setVec3("inColor", boxColor);
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