#include <GL/gl3w.h>
#include <GLFW/glfw3.h> // GLFW helper library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp> // for glm::ortho

// IMGUI
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

#include <iostream> // Used for 'cout'
#include <stdio.h>  // Used for 'printf'

#include "Shaders.h"
#include "Pong.h"

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

// Global variables
PongPaddle leftPaddle, rightPaddle;
PongBall ball(glm::vec2(0.0f), glm::vec2(1.0f, 1.0f));

// Functions
void DrawQuad(glm::vec2, glm::vec2, glm::vec3 = glm::vec3(1.0f));

void Initialize()
{
    // Create a shader for the lab
    GLuint vs = buildShader(GL_VERTEX_SHADER, ASSETS"primitive.vs");
    GLuint fs = buildShader(GL_FRAGMENT_SHADER, ASSETS"primitive.fs");
    shader_program = buildProgram(vs, fs, 0);
    dumpProgram(shader_program, "Pong shader program");

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
    col_loc = glGetUniformLocation(shader_program, "boxColor");
}

void Update(float a_deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, true);

    /*  When creating the Pong AI, it needs to follow the same rules as the player.
    Instead of explicitly setting the y position to follow the ball, use AI logic, and
    PongPaddle::MoveUp() and PongPaddle::MoveDown() functions to control the paddle. */

    // Left side player
    if (glfwGetKey(window, GLFW_KEY_W))
        leftPaddle.MoveUp(a_deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S))
        leftPaddle.MoveDown(a_deltaTime);

    // Right side player
    if (glfwGetKey(window, GLFW_KEY_UP))
        rightPaddle.MoveUp(a_deltaTime);
    if (glfwGetKey(window, GLFW_KEY_DOWN))
        rightPaddle.MoveDown(a_deltaTime);

    // You can look in this ball class to see how the original ball moves. You need to
    // in order to implement the 'predict the ball' or 'invisible ball' methods.
    ball.Move(a_deltaTime, leftPaddle, rightPaddle);
}

void Render()
{
    glUseProgram(shader_program);
    
    viewMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-640.0f, 640.0f, -360.0f, 360.0f, -1.0f, 1.0f);

    ////// Draw the wall ///////
    DrawQuad(glm::vec2(0.0f, 350.0f), glm::vec2(1280.0f, 10.0f)); // bottom
    DrawQuad(glm::vec2(0.0f, -350.0f), glm::vec2(1280.0f, 10.0f)); // top

    ////// Draw the dots in the middle ///////
    for (float i = -350.0f; i < 350.0f; i+= 20.0f)
    {
        DrawQuad(glm::vec2(0.0f, i), glm::vec2(10.0f, 10.0f));
    }

    ////// Draw the Paddles ///////
    DrawQuad(glm::vec2(-630.0f, leftPaddle.yPos), glm::vec2(10.0f, leftPaddle.paddleHeight));
    DrawQuad(glm::vec2(630.0f, rightPaddle.yPos), glm::vec2(10.0f, rightPaddle.paddleHeight));

    ////// Draw the Ball ///////
    DrawQuad(ball.position, glm::vec2(5.0f, 5.0f));

    glUseProgram(GL_NONE);
}

void GUI()
{
    ImGui::Begin("Settings", 0, ImVec2(100, 50), 0.4f);
    {
        // Show some basic stats in the settings window 
        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("Score: %i : %i", leftPaddle.score, rightPaddle.score);
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

    window = glfwCreateWindow(width, height, "Tutorial 1 - Pong", NULL, NULL);
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