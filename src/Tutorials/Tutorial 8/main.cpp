#include <GL/gl3w.h>
#include <GLFW/glfw3.h> // GLFW helper library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp> // for glm::ortho
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <SOIL.h>

#include <iostream> // Used for 'cout'
#include <stdio.h>  // Used for 'printf'
#include <string>   // Used for 'to_string'
#include "Shaders.h"

#include "FuzzyLogic.h"
#include "Car.h"

/*---------------------------- Variables ----------------------------*/
// GLFW window
GLFWwindow* window;
int width = 1280;
int height = 720;

using namespace glm;

// A struct for models
struct Model
{
    GLuint vao;
    GLuint vbo;
    GLuint vertexCount;
};
Model coin; // A model for the coin quad

// Shader
GLuint basic_program;

Car cars[2];

// Matrices
glm::mat4 modelMatrix;
glm::mat4 viewMatrix;
glm::mat4 projectionMatrix;

// --------------------------------------------------- Fuzzy Logic

enum Distances
{
    VeryClose = 0,
    Close,
    Far,
    VeryFar,
};

std::vector<FuzzySet> distanceSets(4);
FuzzyGraph distanceGraph("Distance Graph");

enum Turns
{
    HardLeft = 0,
    Left,
    Straight,
    Right,
    HardRight,
};

std::vector<FuzzySet> turnSets(5);
FuzzyGraph turnGraph("Turning Graph");

void DrawQuad(glm::vec2, glm::vec2, float);

void Initialize()
{
    // Create a shader for the lab
    GLuint vs = buildShader(GL_VERTEX_SHADER, ASSETS"primitive.vs");
    GLuint fs = buildShader(GL_FRAGMENT_SHADER, ASSETS"primitive.fs");
    basic_program = buildProgram(vs, fs, 0);
    dumpProgram(basic_program, "Simple shader program");

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

    glUseProgram(basic_program);
    GLuint vPosition = glGetAttribLocation(basic_program, "vPosition");
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vPosition);

    cars[0].texture = SOIL_load_OGL_texture
    (
        ASSETS"Images/Cars/car_green_2.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );

    cars[1].texture = SOIL_load_OGL_texture
    (
        ASSETS"Images/Cars/car_black_1.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );

    cars[1].carLocation = vec2(200.0f, 0.0f);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(basic_program, "mainTexture"), 0);

    distanceSets[0] = FuzzySet(NameID{ Distances::VeryClose, "Very Close" }, FuzzPoint{ 0.0f, Top },       FuzzPoint{ 50.0f, Bottom });
    distanceSets[1] = FuzzySet(NameID{ Distances::Close,     "Close" },      FuzzPoint{ 0.0f, Bottom },    FuzzPoint{ 100.0f, Top }, FuzzPoint{ 200.0f, Bottom });
    distanceSets[2] = FuzzySet(NameID{ Distances::Far,       "Far" },        FuzzPoint{ 100.0f, Bottom },  FuzzPoint{ 300.0f, Top }, FuzzPoint{ 500.0f, Bottom });
    distanceSets[3] = FuzzySet(NameID{ Distances::VeryFar,   "Very Far" },   FuzzPoint{ 300.0f, Bottom },  FuzzPoint{ 500.0f, Top });

    turnSets[0] = FuzzySet(NameID{ Turns::HardLeft,  "Hard Left" },  FuzzPoint{ -1.0f, Top },    FuzzPoint{ -0.25f, Bottom });
    turnSets[1] = FuzzySet(NameID{ Turns::Left,      "Left" },       FuzzPoint{ -1.0f, Bottom }, FuzzPoint{ -0.5f, Top },  FuzzPoint{ 0.0f, Bottom });
    turnSets[2] = FuzzySet(NameID{ Turns::Straight,  "Straight" },   FuzzPoint{ -0.5f, Bottom }, FuzzPoint{ 0.0f, Top },   FuzzPoint{ 0.5f, Bottom });
    turnSets[3] = FuzzySet(NameID{ Turns::Right,     "Right" },      FuzzPoint{ 0.0f, Bottom },  FuzzPoint{ 0.5f, Top },   FuzzPoint{ 1.0f, Bottom });
    turnSets[4] = FuzzySet(NameID{ Turns::HardRight, "Hard Right" }, FuzzPoint{ 0.25f, Bottom }, FuzzPoint{ 1.0f, Top });

    distanceGraph.SetFuzzySets(distanceSets);
    turnGraph.SetFuzzySets(turnSets);
}

void Update(float a_deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, true);

    // Player controls for car
    cars[0].Turn(glfwGetKey(window, GLFW_KEY_LEFT) ? -1.0f : glfwGetKey(window, GLFW_KEY_RIGHT) ? 1.0f : 0.0f, a_deltaTime);
    cars[0].Accelerate(glfwGetKey(window, GLFW_KEY_DOWN) ? -1.0f : glfwGetKey(window, GLFW_KEY_UP) ? 1.0f : 0.0f, a_deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE)) cars[0].Brake();

    {   // Fuzzy logic AI
        float distance = glm::distance(cars[0].carLocation, cars[1].carLocation); // in pixels
        float rAngle = dot( vec2(cos(cars[1].carHeading - radians(90.0f)), sin(cars[1].carHeading - radians(90.0f))), normalize(cars[0].carLocation - cars[1].carLocation));
        float fAngle = dot( vec2(cos(cars[1].carHeading), sin(cars[1].carHeading)), normalize(cars[0].carLocation - cars[1].carLocation));
        float turning = fAngle > 0.0f ? rAngle : 1.0f * sign(rAngle);
    
        // Dumb AI. Just move forward, and turn right slowly.
        cars[1].Accelerate(1.0f, a_deltaTime);
        cars[1].Turn(0.4f, a_deltaTime);
    }

    // Update both cars
    cars[0].UpdateCar(a_deltaTime);
    cars[1].UpdateCar(a_deltaTime);

    cars[0].ScreenWrap(width, height);
    cars[1].ScreenWrap(width, height);
}

void Render()
{
    glUseProgram(basic_program);
    
    viewMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-(float)width, (float)width, -(float)height, (float)height, -1.0f, 1.0f);

    // Draw player car
    glBindTexture(GL_TEXTURE_2D, cars[0].texture);
    DrawQuad(cars[0].carLocation, vec2(70.0f, 130.0f), cars[0].carHeading);

    // Draw chase car
    glBindTexture(GL_TEXTURE_2D, cars[1].texture);
    DrawQuad(cars[1].carLocation, vec2(70.0f, 130.0f), cars[1].carHeading);


    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    glUseProgram(GL_NONE);
}

void GUI()
{
    distanceGraph.DrawGraph(0.0f, 500.0f);
    turnGraph.DrawGraph(-1.0f, 1.0f);
}

void Cleanup()
{
    glDeleteBuffers(1, &coin.vbo);
    glDeleteVertexArrays(1, &coin.vao);
    glDeleteProgram(basic_program);
}

void DrawQuad(glm::vec2 a_position, glm::vec2 a_size, float heading)
{
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(a_position.x, a_position.y, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, heading - radians(90.0f), vec3(0,0,1));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(a_size.x * 0.5f, a_size.y * 0.5f, 1.0f));

    glm::mat4 modelViewProjMat = projectionMatrix * viewMatrix * modelMatrix;

    glUniformMatrix4fv(glGetUniformLocation(basic_program, "modelViewProjMat"), 1, 0, &modelViewProjMat[0][0]);

    glBindVertexArray(coin.vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static void resize_event(GLFWwindow* a_window, int a_width, int a_height)
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

    window = glfwCreateWindow(width, height, "Tutorial 8 - Fuzzy Car Chase", NULL, NULL);
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

    resize_event(window, width, height);

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

    float oldTime = 0.0f, currentTime = 0.0f, deltaTime = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
        do { currentTime = (float)glfwGetTime(); } while (currentTime - oldTime < 1.0f / 120.0f);
        deltaTime = currentTime - oldTime; oldTime = currentTime; // Difference in time

        // update other events like input handling 
        glfwPollEvents();

        // Clear the screen
        glClearColor(0.23f, 0.23f, 0.23f, 1.0f);
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