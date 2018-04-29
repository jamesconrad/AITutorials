#include <GL/gl3w.h>
#include <GLFW/glfw3.h> // GLFW helper library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp> // for glm::ortho

// IMGUI
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

#include <iostream> // Used for 'cout'
#include <stdio.h>  // Used for 'printf'
#include <SOIL.h>

#include<time.h>
#include <string>
#include <vector>
#include <unordered_map>

#include "Shaders.h"

/*---------------------------- Variables ----------------------------*/
// GLFW window
GLFWwindow* window;
int width = 1280;
int height = 720;

// Uniform locations
GLuint mvp_loc, col_loc;


// OpenGl stuff
GLuint shader_program;
GLuint quad_vbo; // vertex buffer object
GLuint quad_vao; // vertex array object

// Matrices
glm::mat4 modelMatrix;
glm::mat4 viewMatrix;
glm::mat4 projectionMatrix;


enum OPTION { ROCK, PAPER, SCISSORS, LIZARD, SPOCK };
OPTION lastpick[2] = {ROCK, ROCK};
OPTION playerLastPick = ROCK;
std::string lastwin = "Draw";
bool q1mode = false;
int winnerLookup[5][5];
int actionOrders[5][5];
std::string msg = "";
// Textures
GLuint texture;

// Functions
void DrawQuad(glm::vec2, glm::vec2, glm::vec3 = glm::vec3(1.0f));

void Initialize()
{
	srand(time(NULL));

	winnerLookup[ROCK][ROCK] = 0;
	winnerLookup[ROCK][PAPER] = 2;
	winnerLookup[ROCK][SCISSORS] = 1;
	winnerLookup[ROCK][LIZARD] = 1;
	winnerLookup[ROCK][SPOCK] = 2;

	winnerLookup[PAPER][ROCK] = 1;
	winnerLookup[PAPER][PAPER] = 0;
	winnerLookup[PAPER][SCISSORS] = 2;
	winnerLookup[PAPER][LIZARD] = 2;
	winnerLookup[PAPER][SPOCK] = 1;

	winnerLookup[SCISSORS][ROCK] = 2;
	winnerLookup[SCISSORS][PAPER] = 1;
	winnerLookup[SCISSORS][SCISSORS] = 0;
	winnerLookup[SCISSORS][LIZARD] = 1;
	winnerLookup[SCISSORS][SPOCK] = 2;

	winnerLookup[LIZARD][ROCK] = 2;
	winnerLookup[LIZARD][PAPER] = 1;
	winnerLookup[LIZARD][SCISSORS] = 2;
	winnerLookup[LIZARD][LIZARD] = 0;
	winnerLookup[LIZARD][SPOCK] = 1;

	winnerLookup[SPOCK][ROCK] = 1;
	winnerLookup[SPOCK][PAPER] = 2;
	winnerLookup[SPOCK][SCISSORS] = 1;
	winnerLookup[SPOCK][LIZARD] = 2;
	winnerLookup[SPOCK][SPOCK] = 0;

	for (int i = 0; i < 5; i++)
		for (int j = 0; j < 5; j++)
			actionOrders[i][j] = 0;

    // Create a shader for the lab
    GLuint vs = buildShader(GL_VERTEX_SHADER, ASSETS"primitive.vs");
    GLuint fs = buildShader(GL_FRAGMENT_SHADER, ASSETS"primitive.fs");
    shader_program = buildProgram(vs, fs, 0);
    dumpProgram(shader_program, "Pong shader program");

    texture = SOIL_load_OGL_texture(
        ASSETS"Images/smileyFace.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );

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

    glGenVertexArrays(1, &quad_vao);
    glBindVertexArray(quad_vao);

    glGenBuffers(1, &quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glUseProgram(shader_program);
    GLuint vPosition = glGetAttribLocation(shader_program, "vPosition");
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vPosition);

    mvp_loc = glGetUniformLocation(shader_program, "modelViewProjMat");
}

void Update(float a_deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, true);

}

void Render()
{
    glUseProgram(shader_program);
    
    viewMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-640.0f, 640.0f, -360.0f, 360.0f, -1.0f, 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    //DrawQuad(glm::vec2(0.0f), glm::vec2(100.0f, 100.0f));

    glUseProgram(GL_NONE);
}

void Winner()
{
	switch (winnerLookup[lastpick[0]][lastpick[1]])
	{
	case 0: lastwin = "Draw"; return;
	case 1: lastwin = "P1"; return;
	case 2: lastwin = "AI"; return;
	}
}

void Select(OPTION a)
{
	if (q1mode)
	{
		OPTION ai = (OPTION)(rand() % 5);
		lastpick[0] = a;
		lastpick[1] = ai;
		return;
	}
	msg = "";
	int total = 0;
	float probs[5] = { 0,0,0,0,0 };
	OPTION next = ROCK;
	for (int i = 0; i < 5; i++)
		total += actionOrders[playerLastPick][i];
	if (total != 0)
	{
		for (int i = 0; i < 5; i++)
		{
			probs[i] = actionOrders[playerLastPick][i] / total;
			if (probs[i] > probs[next]) next = (OPTION)i;
			//msg.append(probs[i]);
			
		}
		
	}
	else
	{
		OPTION ai = (OPTION)(rand() % 5);
		lastpick[0] = a;
		lastpick[1] = ai;
		msg = "Reverting to random to avoid divide by 0.";

		actionOrders[playerLastPick][a]++;
		playerLastPick = a;
		return;
	}
	
	int index = 0;
	OPTION validChoices[2];
	for (int i = 0; i < 5; i++)
	{
		if (winnerLookup[next][i] == 2)
		{
			validChoices[index] = (OPTION)i;
			index++;
		}
	}

	lastpick[0] = a;
	lastpick[1] = validChoices[rand() % 2];

	actionOrders[playerLastPick][a]++;
	playerLastPick = a;
}

char* OptToStr(OPTION a)
{
	switch (a)
	{
	case ROCK: return "Rock";
	case PAPER: return "Paper";
	case SCISSORS: return "Scissors";
	case LIZARD: return "Liazard";
	case SPOCK: return "Spock";
	default: return "??";
	}
}

void GUI()
{
    ImGui::Begin("Settings", 0, ImVec2(100, 50), 0.4f);
    {
        // Show some basic stats in the settings window 
        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Checkbox("Q1?", &q1mode);
		if (ImGui::Button("Rock"))			Select(OPTION::ROCK);
		else if (ImGui::Button("Paper"))	Select(OPTION::PAPER);
		else if (ImGui::Button("Scissors"))	Select(OPTION::SCISSORS);
		else if (ImGui::Button("Lizard"))	Select(OPTION::LIZARD);
		else if (ImGui::Button("Spock"))	Select(OPTION::SPOCK);
		Winner();
		ImGui::Text("P1: %s\nAI: %s\nWinner: %s\n\n", OptToStr(lastpick[0]), OptToStr(lastpick[1]), lastwin.c_str());
		ImGui::Text(msg.c_str());
    }
    ImGui::End();
}

void Cleanup()
{
    glDeleteBuffers(1, &quad_vbo);
    glDeleteVertexArrays(1, &quad_vao);
    glDeleteProgram(shader_program);
}

void DrawQuad(glm::vec2 a_position, glm::vec2 a_size, glm::vec3 a_color)
{
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(a_position.x, a_position.y, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(a_size.x * 0.5f, a_size.y * 0.5f, 1.0f));

    glm::mat4 modelViewProjMat = projectionMatrix * viewMatrix * modelMatrix;

    glUniformMatrix4fv(mvp_loc, 1, 0, &modelViewProjMat[0][0]);
    glUniform3fv(col_loc, 1, &a_color[0]);

    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static void ResizeEvent(GLFWwindow* a_window, int a_width, int a_height)
{
    // Set the viewport incase the window size changed
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
}

int main()
{
    // start GL context and O/S window using the GLFW helper library
    if (!glfwInit())
    {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return 1;
    }

    window = glfwCreateWindow(width, height, "Tutorial 3 - Rock Paper Scissors", NULL, NULL);
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

    // Setup ImGui binding. This is for any parameters you want to control in runtime
    ImGui_ImplGlfwGL3_Init(window, true);
    ImGui::StyleColorsLight();

    // Get version info
    const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
    const GLubyte* version = glGetString(GL_VERSION); // version as a string
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    // tell GL to only draw onto a pixel if the shape is closer to the viewer
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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