#include <GL/gl3w.h>
#include <GLFW/glfw3.h> // GLFW helper library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp> // for glm::ortho

// IMGUI
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <algorithm>

#include <iostream> // Used for 'cout'
#include <stdio.h>  // Used for 'printf'
#include <string>   // Used for 'to_string'

#include <chrono>

#include "Shaders.h"
#include "ConnectFour.h"

/*---------------------------- Variables ----------------------------*/
// GLFW window
GLFWwindow* window;
int width = 1280;
int height = 720;
glm::ivec2 viewportParam;

// A struct for models
struct Model
{
    GLuint vao;
    GLuint vbo;
    GLuint vertexCount;
};
Model coin; // A model for the coin quad

// Shader
GLuint shaderProgram;

// Matrices
glm::mat4 modelMatrix;
glm::mat4 viewMatrix;
glm::mat4 projectionMatrix;

// Connect 4 board
ConnectFourBoard mainGameBoard;
bool gameOver = false;
float animationTimer = -10.0f;

// Functions
typedef glm::ivec2 ColumnScore;
const int NIL = -1; // Because NULL is 0 and we use 0

std::vector<float> times;
int AILevel = 7;
float aiScoreTotal = 0;
int aiScoreCount = 0;
float playerScoreTotal = 0;
int playerScoreCount = 0;

ColumnScore MaximizePlay(ConnectFourBoard& board, int depth);
ColumnScore MinimizePlay(ConnectFourBoard& board, int depth);


void DrawQuad(glm::vec2, glm::vec2);
bool GetMouseClicked();

std::chrono::high_resolution_clock::time_point timerStart;

void RestartTimer()
{
	std::chrono::high_resolution_clock::time_point timerStart = std::chrono::high_resolution_clock::now();
}

float StopTimer()
{
	return std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - timerStart).count();
}


void Initialize()
{
    // Create a shader for the lab
    GLuint vs = buildShader(GL_VERTEX_SHADER, ASSETS"primitive.vs");
    GLuint fs = buildShader(GL_FRAGMENT_SHADER, ASSETS"primitive.fs");
    shaderProgram = buildProgram(vs, fs, 0);
    dumpProgram(shaderProgram, "Simple shader program");

    // Create all 4 vertices of the quad
    glm::vec3 p0 = glm::vec3(-1.0f, -1.0f, 0.0f);
    glm::vec3 p1 = glm::vec3(-1.0f, 1.0f, 0.0f);
    glm::vec3 p2 = glm::vec3(1.0f, -1.0f, 0.0f);
    glm::vec3 p3 = glm::vec3(1.0f, 1.0f, 0.0f);

    // Create a list of vertices
    glm::vec3 vertices[12] =
    {
        // Bottom face
        p0, p1, p2, p3,
    };

    glGenVertexArrays(1, &coin.vao);
    glBindVertexArray(coin.vao);

    glGenBuffers(1, &coin.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, coin.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glUseProgram(shaderProgram);
    GLuint vPosition = glGetAttribLocation(shaderProgram, "vPosition");
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vPosition);

    glActiveTexture(GL_TEXTURE0);

    mainGameBoard.InitializeBoard();
}

ColumnScore MaximizePlay(ConnectFourBoard& board, int depth)
{
    // Call score of our board
    int score = board.SimpleScoring();
	aiScoreTotal += score;
	aiScoreCount++;

    // Break
    if (board.IsFinished() || depth == 0) return ColumnScore(NIL, score);

    // Column, Score
    ColumnScore max = ColumnScore(NIL, -99999);

    // For all possible moves
    for (int column = 0; column < board.COLUMNS; column++)
    {
        ConnectFourBoard newBoard = board.CreateCopy(); // Create new board

        if (newBoard.DropCoin(column, Player::AI))
        {
            ColumnScore nextMove = MinimizePlay(newBoard, depth - 1); // Recursive calling

            // Evaluate new move
            if (max[0] == NIL || nextMove[1] > max[1])
            {
                max[0] = column;
                max[1] = nextMove[1];
            }
        }
    }

    return max;
}

ColumnScore MinimizePlay(ConnectFourBoard& board, int depth)
{
    int score = board.SimpleScoring();
	playerScoreTotal += score;
	playerScoreCount++;

    if (board.IsFinished() || depth == 0) return ColumnScore(NIL, score);

    ColumnScore min = ColumnScore(NIL, 99999);

    for (int column = 0; column < board.COLUMNS; column++)
    {
        ConnectFourBoard newBoard = board.CreateCopy();

        if (newBoard.DropCoin(column, Player::HUMAN))
        {
            ColumnScore nextMove = MaximizePlay(newBoard, depth - 1);

            if (min[0] == NIL || nextMove[1] < min[1])
            {
                min[0] = column;
                min[1] = nextMove[1];
            }
        }
    }
    return min;
}

int GenerateComputerDecision()
{
	RestartTimer();

    ConnectFourBoard tempBoard = mainGameBoard.CreateCopy();

    ColumnScore aiMove = MaximizePlay(tempBoard, AILevel);
	
	printf("AI:%f\nP1:%f\n", aiScoreTotal / aiScoreCount, playerScoreTotal / playerScoreCount);
	printf("AI:%f, %i\nP1:%f, %i\n", aiScoreTotal, aiScoreCount, playerScoreTotal, playerScoreCount);
	if (aiScoreTotal / aiScoreCount < playerScoreTotal / playerScoreCount)
		AILevel = 7;
	else
		AILevel = 5;

	aiScoreTotal = 0;
	aiScoreCount = 0;
	playerScoreTotal = 0;
	playerScoreCount = 0;

	times.push_back(StopTimer());

    return aiMove[0];
}

void Update(float a_deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_R))
        mainGameBoard.ResetBoard();

    // Don't make moves while things are animating
    if (glfwGetTime() - animationTimer < 1.0f) return;

    // Simple function to get if the mouse was clicked
    bool leftMouseWasClicked = GetMouseClicked();

    if (leftMouseWasClicked && gameOver)
    {
        gameOver = false;
        mainGameBoard.ResetBoard();
    }

    if (mainGameBoard.GetPlayerTurn() == Player::HUMAN) // Player Turn
    {
        if (leftMouseWasClicked)
        {
            glm::dvec2 pos;
            glfwGetCursorPos(window, &pos[0], &pos[1]);

            float coinSize = viewportParam[1] / (float)mainGameBoard.COLUMNS;
            int column = (int)floor((pos.x - viewportParam[0]) / coinSize);

            if (column < 0 || column >= mainGameBoard.COLUMNS) return;

            if (mainGameBoard.DropCoin(column, Player::HUMAN))
            {
                mainGameBoard.SetPlayerTurn(Player::AI);
                animationTimer = glfwGetTime();
            }

            // Now check if the we won
            if (mainGameBoard.SimpleScoring() == -mainGameBoard.MAX_SCORE)
            {
                std::cout << "You beat the AI" << std::endl;
                gameOver = true;
            }
        }
    }
    else // board.PlayerTurn() == Player::AI
    {
        if (gameOver) return;   // Don't do AI if the game is over

        do // Do the AI logic ...
        {
#ifdef RANDOM_AI
            // Pick a random slot to drop the coin in
            if (mainGameBoard.DropCoin(rand() % 7, Player::AI))
            {
                mainGameBoard.SetPlayerTurn(Player::HUMAN);
            }
#else
            if (mainGameBoard.DropCoin(GenerateComputerDecision(), Player::AI))
            {
                mainGameBoard.SetPlayerTurn(Player::HUMAN);
                animationTimer = glfwGetTime();
            }
#endif
        } while (mainGameBoard.GetPlayerTurn() != Player::HUMAN); // ... while it's not the humans turn

        // Now check if the AI won
        if (mainGameBoard.SimpleScoring() == mainGameBoard.MAX_SCORE)
        {
            std::cout << "You lost to the AI" << std::endl;
            gameOver = true;
        }
    }
}

void Render()
{
    glUseProgram(shaderProgram);

    if      (mainGameBoard.SimpleScoring() ==  mainGameBoard.MAX_SCORE) glClearColor(0.3f, 0.0f, 0.0f, 1.0f);
    else if (mainGameBoard.SimpleScoring() == -mainGameBoard.MAX_SCORE) glClearColor(0.0f, 0.3f, 0.0f, 1.0f);
    else     glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    viewMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(0.0f, (float)mainGameBoard.PIXEL_WIDTH, 0.0f, (float)mainGameBoard.PIXEL_HEIGHT, -1.0f, 1.0f);

    mainGameBoard.DrawBoard(DrawQuad);

    glUseProgram(GL_NONE);
}

void GUI()
{
    // Uncomment this when you want to draw the AI compute time history

    ImGui::Begin("History", 0, ImVec2(100, 50), 0.4f);
    {
		if (times.size() > 0)
		{
			// Get the min and max values
			float min = (*std::min_element(std::begin(times), std::end(times))) * 0.99f;
			float max = (*std::max_element(std::begin(times), std::end(times))) * 1.01f;
			// And plot it using ImGui
			ImGui::PlotHistogram("AI Decision Times", &times[0],
				times.size(), 0, NULL, min, max, ImVec2(0, 80));
			ImGui::Text("AIDepth: %i", AILevel);
		}
    }
    ImGui::End();
}

void Cleanup()
{
    glDeleteBuffers(1, &coin.vbo);
    glDeleteVertexArrays(1, &coin.vao);
    glDeleteProgram(shaderProgram);
}

void DrawQuad(glm::vec2 a_position, glm::vec2 a_size)
{
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(a_position.x, a_position.y, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(a_size.x * 0.5f, a_size.y * 0.5f, 1.0f));

    glm::mat4 modelViewProjMat = projectionMatrix * viewMatrix * modelMatrix;

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelViewProjMat"), 1, 0, &modelViewProjMat[0][0]);

    glBindVertexArray(coin.vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

int previousState;
bool GetMouseClicked()
{
    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    bool mouseWasClicked = state == GLFW_PRESS && previousState != GLFW_PRESS;
    previousState = state;
    return mouseWasClicked;
}

static void ResizeEvent(GLFWwindow* a_window, int a_width, int a_height)
{
    // Set the viewport incase the window size changed
    glfwGetFramebufferSize(window, &width, &height);

    float windowRatio   = a_width / (float)a_height;
    float screenRatio   = mainGameBoard.PIXEL_WIDTH / (float)mainGameBoard.PIXEL_HEIGHT;
    float screenRatioV  = mainGameBoard.PIXEL_HEIGHT / (float)mainGameBoard.PIXEL_WIDTH;

    if (windowRatio > screenRatio)
    {
        int w = screenRatio * a_height;
        int x = (a_width - w) * 0.5f;

        viewportParam = glm::ivec2(x, w);

        glViewport(x, 0, w, a_height);
    }
    else
    {
        int h = screenRatioV * a_width;
        int y = (a_height - h) * 0.5f;

        viewportParam = glm::ivec2(0, a_width);

        glViewport(0, y, a_width, h);
    }
}

int main()
{
    // start GL context and O/S window using the GLFW helper library
    if (!glfwInit())
    {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return 1;
    }

    window = glfwCreateWindow(width, height, "Tutorial 4 - Connect 4", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeCallback(window, ResizeEvent);
    glfwSwapInterval(0);

    // start GL3W
    gl3wInit();

    ResizeEvent(window, width, height);

    // Setup ImGui binding. This is for any parameters you want to control in runtime
    ImGui_ImplGlfwGL3_Init(window, true);
    ImGui::StyleColorsLight();

    // Get version info
    const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
    const GLubyte* version = glGetString(GL_VERSION); // version as a string
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    glDisable(GL_DEPTH_TEST); // disable depth-testing

    Initialize();

    float oldTime = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - oldTime;
        oldTime = currentTime;

        // update other events like input handling 
        glfwPollEvents();

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplGlfwGL3_NewFrame();

        // Call the helper functions
        Update(deltaTime);
        Render();
        GUI();

        // Finish by drawing the GUI on top of everything
        ImGui::Render();
        glfwSwapBuffers(window);
    }

    // close GL context and any other GLFW resources
    glfwTerminate();
    ImGui_ImplGlfwGL3_Shutdown();
    Cleanup();
    return 0;
}