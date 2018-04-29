#define GLM_ENABLE_EXPERIMENTAL
#include <GL/gl3w.h>
#include <GLFW/glfw3.h> // GLFW helper library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp> // for glm::ortho
#include <GLM/gtx/vector_angle.hpp>
using namespace glm;

// IMGUI
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

#include <iostream> // Used for 'cout'
#include <stdio.h>  // Used for 'printf'
#include <time.h>   // Used to seed the rand
#include <vector>   // Used for std::vector<Model>

#include "Shaders.h"
#include "Perceptron.h"

struct Model
{
public:
    GLuint vao = GL_NONE;
    GLuint vbo = GL_NONE;
    int verticesCount = 0;
};

/*---------------------------- Variables ----------------------------*/
// GLFW window
GLFWwindow* window;
int width = 1280;
int height = 720;
bool mouseDownLeft = false;
bool mouseDownRight = false;

// OpenGl stuff
GLuint basic_program, bezier_program;
Model quadModel;
std::vector<Model> lineModels;
Model * currentlyBeingDrawn = nullptr;
std::vector<vec3> currentLinePoints;
float lineWidth = 3.0f;
const float linePrecision = 10.0f; // Smaller the better
glm::vec2 prevPoint = glm::vec2(0, 0);

// AI Variables
const bool USE_CUSTOM_PERCEPTRON_VALUES = true; /**  EDIT ME!!  **/
enum Features
{
	DirectionAccumulationLength,
    StartPointX,
    StartPointY,
	DirectionAccumulationX,
	DirectionAccumulationY,
    EndPointX,
    EndPointY,
	OffsetX,
	OffsetY,
	NumModels,
	AverageAngle,
    //MousePositionX,
    //MousePositionY,
	COUNT
};

enum Outputs
{
	NONE,
	Circle,
	Cross,
	Triangle
};

Perceptron perceptron(Features::COUNT);
Perceptron triangleperceptron(Features::COUNT);
Outputs result = Outputs::NONE;
float perceptronOutput = 0.0f;
float triangleOutput = 0.0f;

// Feature vector stuff
vec2 directionAccumulation = vec2(0.0f, 0.0f);
vec2 maxBounds = vec2(0.0f, 0.0f);
vec2 minBounds = vec2(0.0f, 0.0f);
vec2 startPoint = vec2(0.0f, 0.0f);
vec2 endPoint = vec2(0.0f, 0.0f);
vec2 mousePosition = vec2(0.0f, 0.0f);
float numPoints = 0.f;
int penRelease = 0;

// Functions
void DrawQuad(glm::vec2, glm::vec2, glm::vec3 = glm::vec3(1.0f));
void Cleanup();
void CleanupLines();

char* FeatureName(Features f)
{
	switch (f)
	{
	case Features::DirectionAccumulationLength: return "DirAccumLen";
	case Features::DirectionAccumulationX : return "DirAccumX";
	case Features::DirectionAccumulationY : return "DirAccumY";
	case Features::EndPointX : return "EndX";
	case Features::EndPointY : return "EndY";
	case Features::NumModels : return "NumLines";
	case Features::StartPointX : return "StartX";
	case Features::StartPointY : return "StartY";
	case Features::OffsetX : return "OffsetX";
	case Features::OffsetY : return "OffsetY";
	case Features::AverageAngle: return "AvgAngle";
	}
}

void Initialize()
{
	if (USE_CUSTOM_PERCEPTRON_VALUES)
	{
		float weightVector[Features::COUNT] = { 0.0f };

		weightVector[Features::DirectionAccumulationLength] = -1.0f;
		weightVector[Features::DirectionAccumulationX] = -1.0f;
		weightVector[Features::DirectionAccumulationY] = -1.0f;
        weightVector[Features::StartPointX] = 0.0f;
        weightVector[Features::StartPointY] = 0.0f;
        weightVector[Features::EndPointX] = 0.0f;
        weightVector[Features::EndPointY] = 0.0f;
		weightVector[Features::NumModels] = 0.0f;
		weightVector[Features::OffsetX] = -1.0f;
		weightVector[Features::OffsetY] = -1.0f;
		weightVector[Features::AverageAngle] = 0.0f;
        //weightVector[Features::MousePositionX] = 1.0f;
        //weightVector[Features::MousePositionY] = 1.0f;
		
        perceptron.bias = 0.0f;

		perceptron.SetWeights(weightVector);

		weightVector[Features::DirectionAccumulationLength] = 0.0f;
		weightVector[Features::DirectionAccumulationX] = 1.0f;
		weightVector[Features::DirectionAccumulationY] = 0.0f;
		weightVector[Features::StartPointX] = 0.0f;
		weightVector[Features::StartPointY] = 0.0f;
		weightVector[Features::EndPointX] = 0.0f;
		weightVector[Features::EndPointY] = 0.0f;
		weightVector[Features::NumModels] = -1.0f;
		weightVector[Features::OffsetX] = 0.0f;
		weightVector[Features::OffsetY] = 0.0f;
		weightVector[Features::AverageAngle] = 0.0f;

		triangleperceptron.SetWeights(weightVector);
	}
	else
	{
		perceptron.RandomizeValues();
		triangleperceptron.RandomizeValues();
	}

    {   // Create a shader for the quad
        GLuint vs = buildShader(GL_VERTEX_SHADER, ASSETS"primitive.vert");
        GLuint fs = buildShader(GL_FRAGMENT_SHADER, ASSETS"primitive.frag");
        basic_program = buildProgram(vs, fs, 0);
        dumpProgram(basic_program, "Simple shader program");
    }
    
    {   // Create a shader for the line
        GLuint vs = buildShader(GL_VERTEX_SHADER, ASSETS"line.vert");
        GLuint gs = buildShader(GL_GEOMETRY_SHADER, ASSETS"line.geom");
        GLuint fs = buildShader(GL_FRAGMENT_SHADER, ASSETS"line.frag");
        bezier_program = buildProgram(vs, gs, fs, 0);
        dumpProgram(bezier_program, "Bezier shader program");
    }

    #pragma region QUAD OBJECT
    {
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

        glGenVertexArrays(1, &quadModel.vao);
        glBindVertexArray(quadModel.vao);

        glGenBuffers(1, &quadModel.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, quadModel.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        quadModel.verticesCount = 4;
    }
    #pragma endregion
}

void Update(float a_deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, true);
    
    // Mouse position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    mousePosition = vec2(xpos, ypos);

    // Draw the line by adding points
    if (mouseDownLeft)
    {
        if (currentLinePoints.size() == 0)
        {
            // Push back 2 for an EP curve
            currentLinePoints.push_back(vec3((float)xpos, (float)ypos, 0.0f));
            currentLinePoints.push_back(vec3((float)xpos, (float)ypos, 0.0f));

			maxBounds = vec2(xpos, ypos);
			minBounds = vec2(xpos, ypos);

            startPoint = mousePosition;
        }

        vec3 previousPosition = currentLinePoints[currentLinePoints.size() - 1];
        vec3 newPosition = vec3((float)xpos, (float)ypos, 0.0f);
		vec3 delta = previousPosition - newPosition;

        if (length(delta) > linePrecision)
		{
			maxBounds.x = max(maxBounds.x, newPosition.x);
			maxBounds.x = max(maxBounds.y, newPosition.y);

			minBounds.x = min(minBounds.x, newPosition.x);
			minBounds.y = min(minBounds.y, newPosition.y);

			currentLinePoints.push_back(newPosition);

			directionAccumulation += vec2(delta);
		}
    }

    // Mouse input
    int mouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if (mouseState == GLFW_PRESS)
        mouseDownLeft = true;
    else if (mouseState == GLFW_RELEASE)
    {
        if (mouseDownLeft)
        {
            endPoint = mousePosition;

            Model line;
            currentLinePoints.push_back(currentLinePoints[currentLinePoints.size() - 1]);

            glGenVertexArrays(1, &line.vao);
            glBindVertexArray(line.vao);

            glGenBuffers(1, &line.vbo);
            glBindBuffer(GL_ARRAY_BUFFER, line.vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * currentLinePoints.size(), NULL, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * currentLinePoints.size(), &currentLinePoints[0][0]);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(0);

            line.verticesCount = (int)currentLinePoints.size();

            lineModels.push_back(line);
            currentLinePoints.clear();

			mouseDownLeft = false;
			penRelease++;
        }
    }

	mouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
	if (mouseState == GLFW_PRESS)
	{
		mouseDownRight = true;
	}
	else if (mouseState == GLFW_RELEASE)
	{
		if (mouseDownRight)
		{
			CleanupLines();

			mouseDownRight = false;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) && !lineModels.empty())
	{
		// Normalize scale, or else the values will explode
		float normalization = 1.0f / abs(length(maxBounds - minBounds));

		float featureVector[Features::COUNT] = { 0.0f };
		featureVector[Features::DirectionAccumulationLength] = length(directionAccumulation) * normalization;
		featureVector[Features::DirectionAccumulationX] = abs(directionAccumulation.x) * normalization;
		featureVector[Features::DirectionAccumulationY] = abs(directionAccumulation.y) * normalization;
        featureVector[Features::StartPointX] = startPoint.x;
        featureVector[Features::StartPointY] = startPoint.y;
        featureVector[Features::EndPointX] = endPoint.x;
		featureVector[Features::EndPointY] = endPoint.y;
		featureVector[Features::NumModels] = penRelease;
		featureVector[Features::OffsetX] = endPoint.x - startPoint.x;
		featureVector[Features::OffsetY] = endPoint.y - startPoint.y;
		//vec3 prev = currentLinePoints[0];
		//float totalAngle = 0;
		//int count = 0;
		//for (int i = 1; i < currentLinePoints.size(); i++)
		//{
		//	float a = angle(glm::normalize(prev), glm::normalize(currentLinePoints[i]));
		//	totalAngle += a;
		//	prev = currentLinePoints[i];
		//	count++;
		//}
		//float avgA = totalAngle / count;
		//prev = currentLinePoints[0];
		//count = 0;
		//totalAngle = 0;
		//for (int i = 1; i < currentLinePoints.size(); i++)
		//{
		//	float a = angle(glm::normalize(prev), glm::normalize(currentLinePoints[i])) * 57.2958;
		//
		//	totalAngle += a - avgA;
		//
		//	prev = currentLinePoints[i];
		//	count++;
		//}
		//
		//featureVector[Features::AverageAngle] = totalAngle/count;
        //featureVector[Features::MousePositionX] = mousePosition.x;
        //featureVector[Features::MousePositionY] = mousePosition.y;

		
		perceptronOutput = perceptron.Evaluate(featureVector);
		triangleOutput = triangleperceptron.Evaluate(featureVector);

		if (perceptronOutput >= triangleOutput && perceptronOutput > 0.5)
			result = Circle;
		else if (triangleOutput > perceptronOutput && triangleOutput > 0.5)
			result = Triangle;
		else
			result = Cross;
		

		CleanupLines();
	}
}

void Render()
{    
    float ratio = width / (float)height;
    mat4 mvp = ortho(0.0f, (float)width, (float)height,  0.0f, -100.0f, 100.0f);

    // Start drawing the lines 
    glUseProgram(bezier_program);
    glUniformMatrix4fv(glGetUniformLocation(bezier_program, "mvp"), 1, GL_FALSE, &mvp[0][0]);
    glUniform1f(glGetUniformLocation(bezier_program, "width"), lineWidth);

    for (auto itr = lineModels.begin(); itr != lineModels.end(); itr++)
    {
        Model line = (*itr);

        glBindVertexArray(line.vao);
        glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, line.verticesCount);
    }

    if (mouseDownLeft)
    {
        if (currentLinePoints.size() >= 4) // Lines adjacency needs 4 points
        {
            Model line;

            glGenVertexArrays(1, &line.vao);
            glBindVertexArray(line.vao);

            glGenBuffers(1, &line.vbo);
            glBindBuffer(GL_ARRAY_BUFFER, line.vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * currentLinePoints.size(), NULL, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * currentLinePoints.size(), &currentLinePoints[0][0]);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(0);

            line.verticesCount = (int)currentLinePoints.size();

            glBindVertexArray(line.vao);
            glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, line.verticesCount);
        }
    }

    glUseProgram(GL_NONE);
}

void GUI()
{
	ImGui::Begin("Settings", 0, ImVec2(0, 0), 0.4f, ImGuiWindowFlags_AlwaysAutoResize);
    {
        // Show some basic stats in the settings window 
		ImGui::Text("Circle output = %.3f", perceptronOutput);
		ImGui::Text("Smiley output = %.3f", triangleOutput);

		if (result == Outputs::Circle)
		{
			ImGui::Spacing();
			ImGui::Text("Circle!");
		}
		else if (result == Outputs::Cross)
		{
			ImGui::Spacing();
			ImGui::Text("Cross!");
		}
		else if (result == Outputs::Triangle)
		{
			ImGui::Spacing();
			ImGui::Text("Smiley!");
		}

        ImGui::Spacing();
        ImGui::SliderFloat("Line Width", &lineWidth, 1.0f, 100.0f);

		ImGui::Spacing();
		ImGui::Text("Circle Perceptron Weights:");
		for (int i = 0; i < Features::COUNT; i++)
		{
			char* str = FeatureName((Features)i);
			int l = strlen(str);
			char* fin = (char*)malloc(l + 1);
			memcpy(fin + 1, str, l);
			fin[0] = 'C';
			ImGui::SliderFloat(fin, &perceptron.weights[i], -1, 1);
			free(fin);
		}
		ImGui::SliderFloat("CBias", &perceptron.bias, -1, 1);
		ImGui::Spacing();
		ImGui::Text("Smiley Perceptron Weights:");
		for (int i = 0; i < Features::COUNT; i++)
		{
			char* str = FeatureName((Features)i);
			int l = strlen(str);
			char* fin = (char*)malloc(l + 1);
			memcpy(fin + 1, str, l);
			fin[0] = 'T';
			ImGui::SliderFloat(fin, &triangleperceptron.weights[i], -1, 1);
			free(fin);
		}
		ImGui::SliderFloat("TBias", &triangleperceptron.bias, -1, 1);
    }
    ImGui::End();
}

void Cleanup()
{
    glDeleteBuffers(1, &quadModel.vbo);
    glDeleteVertexArrays(1, &quadModel.vao);

	CleanupLines();
    
	glDeleteProgram(basic_program);
    glDeleteProgram(bezier_program);
}

void CleanupLines()
{
	for (auto itr = lineModels.begin(); itr != lineModels.end(); itr++)
	{
		Model line = (*itr);
		glDeleteBuffers(1, &line.vbo);
		glDeleteVertexArrays(1, &line.vao);
	}
	penRelease = 0;
	lineModels.clear();
	currentLinePoints.clear();
	directionAccumulation = vec2(0.0f, 0.0f);
}

void DrawQuad(glm::vec2 a_position, glm::vec2 a_size, glm::vec3 a_color)
{
    mat4 modelMatrix = glm::mat4(1.0f);
    mat4 viewMatrix = glm::mat4(1.0f);
    mat4 projectionMatrix = ortho(0.0f, (float)width, 0.0f, (float)height, -100.0f, 100.0f);

    modelMatrix = glm::translate(modelMatrix, glm::vec3(a_position.x, a_position.y, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(a_size.x * 0.5f, a_size.y * 0.5f, 1.0f));

    glm::mat4 modelViewProjMat = projectionMatrix * viewMatrix * modelMatrix;

    glUniformMatrix4fv(glGetUniformLocation(basic_program, "mvp"), 1, 0, &modelViewProjMat[0][0]);
    glUniform3fv(glGetUniformLocation(basic_program, "color"), 1, &a_color[0]);

    glBindVertexArray(quadModel.vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, quadModel.verticesCount);
}

static void resize_event(GLFWwindow* a_window, int a_width, int a_height)
{
    // Set the viewport incase the window size changed
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
}

int main()
{
    srand((unsigned int)time(NULL));
    // start GL context and O/S window using the GLFW helper library
    if (!glfwInit())
    {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return 1;
    }

    window = glfwCreateWindow(width, height, "Tutorial 7 - Tic Tac Toe (kinda)", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeCallback(window, resize_event);
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
        glClearColor(0.96f, 0.97f, 1.0f, 1.0f);
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