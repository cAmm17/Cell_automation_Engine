#include "main.h"
#include "Shader.h"
#include "Camera.h"
#include "gridManager.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void updateCurGrid();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

static const int WINDOWHEIGHT = 1200;
static const int WINDOWWIDTH = 2400;
gridManager* gameGrid;
static bool hidden = false;

static Camera *programCamera = new Camera(glm::vec3(0.0f, 0.0f, -3.0f), glm::vec3(10.0f, 10.0f, 10.0f),
	glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

int main()
{
	float update_rate = 0.5;
	//Setting shader paths
	const char* vertexShaderPath = "3.3.shader.vs";
	const char* fragmentShaderPath = "3.3.shader.fs";

	//Initiate glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Create a new glfw window
	GLFWwindow* window = glfwCreateWindow(WINDOWWIDTH, WINDOWHEIGHT, "GameOfLife", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	//Set the current window for glfw
	glfwMakeContextCurrent(window);

	//Load in Glad and make sure it loaded in 
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//Additional settings (sets opengl to check for depth and captures the mouse for mouse input)
	glEnable(GL_DEPTH_TEST);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);

	//Create the opengl viewport and use glfw to set the window
	glViewport(0, 0, WINDOWWIDTH, WINDOWHEIGHT);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


	float vertices[] = {
		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,

		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,

		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f
	};

	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//Initialize the shader
	Shader shaderProgram = Shader::Shader(vertexShaderPath, fragmentShaderPath);
	shaderProgram.use();

	//Initialize the grid manager for the game
	int gridSz = 30;

	//test for grid, remove later
	std::vector<glm::vec3> startBlock;
	//startBlock.push_back(glm::vec3(10.0f, 10.0f, 10.0f));
	//startBlock.push_back(glm::vec3(11.0f, 10.0f, 10.0f));
	//startBlock.push_back(glm::vec3(10.0f, 11.0f, 10.0f));
	//startBlock.push_back(glm::vec3(11.0f, 11.0f, 10.0f));
	//startBlock.push_back(glm::vec3(10.0f, 10.0f, 11.0f));
	//startBlock.push_back(glm::vec3(11.0f, 10.0f, 11.0f));
	//startBlock.push_back(glm::vec3(10.0f, 11.0f, 11.0f));
	//startBlock.push_back(glm::vec3(11.0f, 11.0f, 11.0f));
	//startBlock.push_back(glm::vec3(11.0f, 11.0f, 11.0f));
	//startBlock.push_back(glm::vec3(11.0f, 12.0f, 11.0f));
	//startBlock.push_back(glm::vec3(12.0f, 11.0f, 11.0f));
	//startBlock.push_back(glm::vec3(12.0f, 12.0f, 11.0f));
	startBlock.push_back(glm::vec3(int(gridSz / 2), int(gridSz / 2), int(gridSz / 2) + 1));
	startBlock.push_back(glm::vec3(int(gridSz / 2), int(gridSz / 2) + 1, int(gridSz / 2) + 1));
	startBlock.push_back(glm::vec3(int(gridSz / 2) + 1, int(gridSz / 2), int(gridSz / 2) + 1));
	startBlock.push_back(glm::vec3(int(gridSz / 2) + 1, int(gridSz / 2) + 1, int(gridSz / 2) + 1));
	/*for (int i = 0; i < 20; i++) {
		startBlock.push_back(glm::vec3(rand() % 5 + 10, rand() % 5 + 10, rand() % 5 + 10));
	}*/
	//startBlock.push_back(glm::vec3(3.0f, 3.0f, 3.0f));
	gameGrid = new gridManager(gridSz, startBlock, 6, 6, 4, 4);

	//gameGrid.initializeGrid(8, startBlock);

	//Set projection matrix once since it is not going to be reset during gameplay
	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), (float)WINDOWWIDTH / (float)WINDOWHEIGHT, 0.1f, 100.0f);
	shaderProgram.setMat4("projection", projection);
	unsigned char* gridPtr;
	std::thread updater(updateCurGrid);

	glm::mat4 gridModel = glm::mat4(1.0f);

	gridModel = glm::translate(gridModel, glm::vec3(gameGrid->gridSize / 2, gameGrid->gridSize / 2, gameGrid->gridSize / 2));
	gridModel = glm::scale(gridModel, glm::vec3(gameGrid->gridSize, gameGrid->gridSize, gameGrid->gridSize));


	float mousex, mousey, mousez;
	//Render loop
	while (!glfwWindowShouldClose(window))
	{
		//Clear color and depth each render iteration so that the next iteration is drawn correctly.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//Checks for user input
		processInput(window);

		//Game running functions here
		shaderProgram.use();

		glm::mat4 view = glm::mat4(1.0f);
		view = programCamera->setLookAt();
		shaderProgram.setMat4("view", view);


		glBindVertexArray(VAO);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		gridPtr = gameGrid->tempgrid;

		
		int y, x = 0;
		int neighbours;
		gameGrid->m.lock();
		for (int z = 0; z < gameGrid->gridSize; z++) 
		{
			y = 0;
			do {
				x = 0;
				do {
					
					while (*gridPtr == 0) {
						gridPtr++;
						if (++x >= gameGrid->gridSize) goto rowDone;
					}

					if (*gridPtr & 0x01) {
						neighbours = *gridPtr >> 1;

						glm::mat4 model = glm::mat4(1.0f);
						//translate the box to the right position. adding .5 since the boxes are centered at the passed in vector
						model = glm::translate(model, glm::vec3(x + 0.5, y + 0.5, z + 0.5));
						//assign the model uniform
						shaderProgram.setMat4("model", model);

						glm::vec3 boxColor;
						if (neighbours >= gameGrid->liveUpper && neighbours <= gameGrid->liveUpper)
							boxColor = glm::vec3(0.05f, 0.7f, 0.6f);
						else if (neighbours >= gameGrid->replicateLower && neighbours <= gameGrid->replicateUpper)
							boxColor = glm::vec3(0.3f, 0.2f, 0.9f);
						else 
							boxColor = glm::vec3(((neighbours - gameGrid->liveUpper) * (neighbours - gameGrid->liveUpper)) % 10 * 0.04,
							((neighbours - gameGrid->liveUpper) * (neighbours - gameGrid->liveUpper)) % 10 * 0.065, 
								((neighbours - gameGrid->liveUpper) * (neighbours - gameGrid->liveUpper)) % 10 * 0.1);


						shaderProgram.setVec3("inColor", boxColor);
						//draw the box
						glBindVertexArray(VAO);
						glDrawArrays(GL_TRIANGLES, 0, 36);
					}
					gridPtr++;
					x++;

				} while (x < gameGrid->gridSize);
			rowDone:;
				y++;
			} while (y < gameGrid->gridSize);
		}
		//std::cout << sizeof(unsigned int) << " " << sizeof(unsigned char) << " \n";
		gameGrid->m.unlock();
		//Sleep(10);

		//Drawing the box outline
		glm::vec3 boxColor = glm::vec3(0.9f, 0.9f, 1.0f);
		shaderProgram.setVec3("inColor", boxColor);

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		shaderProgram.setMat4("model", gridModel);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);


		mousex = int(programCamera->cameraPos.x + programCamera->cameraFront.x * 3) + .5;
		mousey = int(programCamera->cameraPos.y + programCamera->cameraFront.y * 3) + .5;
		mousez = int(programCamera->cameraPos.z + programCamera->cameraFront.z * 3) + .5;
		if (mousex > 0 && mousey > 0
			&& mousez > 0 && mousex < gameGrid->gridSize
			&& mousey < gameGrid->gridSize && mousez < gameGrid->gridSize && !gameGrid->notPaused && !hidden) 
		{
			glm::mat4 mouseoverModel = glm::mat4(1.0f);
			mouseoverModel = glm::translate(mouseoverModel, glm::vec3(mousex, mousey, mousez));

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			shaderProgram.setMat4("model", mouseoverModel);

			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		

		//Swap drawing and displayed buffer
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	gameGrid->update = false;
	//if(updater.joinable())
	updater.join();

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	//Clear program camera memory
	delete programCamera;
	delete gameGrid;

	//Clear glfw's memory
	glfwTerminate();
	return 0;
}

//Process user input
void processInput(GLFWwindow *window)
{
	programCamera->ProcessKeyboardInput(window);

	
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !gameGrid->notPaused) {
		float mousex = int(programCamera->cameraPos.x + programCamera->cameraFront.x * 3) + .5;
		float mousey = int(programCamera->cameraPos.y + programCamera->cameraFront.y * 3) + .5;
		float mousez = int(programCamera->cameraPos.z + programCamera->cameraFront.z * 3) + .5;
		gameGrid->m.lock();
		gameGrid->editBox(int(mousex), int(mousey), int(mousez), 1, true);
		gameGrid->m.unlock();
	}
	
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && !gameGrid->notPaused) {
		float mousex = int(programCamera->cameraPos.x + programCamera->cameraFront.x * 3) + .5;
		float mousey = int(programCamera->cameraPos.y + programCamera->cameraFront.y * 3) + .5;
		float mousez = int(programCamera->cameraPos.z + programCamera->cameraFront.z * 3) + .5;
		gameGrid->m.lock();
		gameGrid->editBox(int(mousex), int(mousey), int(mousez), 0, true);
		gameGrid->m.unlock();
	}

	
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
	{
		gameGrid->notPaused = !gameGrid->notPaused;
	}
		

	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
	{
		hidden = !hidden;
	}

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	programCamera->ProcessMouseInput(xpos, ypos);
}

void updateCurGrid() {
	while (gameGrid->update) {
		if (gameGrid->notPaused) {
			gameGrid->updateGrid();
			Sleep(200);
		}
	}
}

//If the window is resized by the user, then the opengl window will adjust as well
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}