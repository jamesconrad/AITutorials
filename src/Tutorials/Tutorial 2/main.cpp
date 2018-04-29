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
#include "Pacman.h"

/*---------------------------- Variables ----------------------------*/
// GLFW window
GLFWwindow* window;
int width = 736;
int height = 800;

// Uniform locations
GLuint mvp_loc, col_loc;

// OpenGl stuff
GLuint shader_program, pacman_shader_program;
GLuint quad_vbo; // vertex buffer object
GLuint quad_vao; // vertex array object

// Matrices
glm::mat4 modelMatrix;
glm::mat4 viewMatrix;
glm::mat4 projectionMatrix;

// Global Variables
Map* mainLevel;

// Functions
void DrawQuad(glm::vec2, glm::vec2, glm::vec3 = glm::vec3(1.0f), int t = 0);

void Initialize()
{
    mainLevel = new Map(window);

    // Create a shader for the lab
    GLuint vs = buildShader(GL_VERTEX_SHADER, ASSETS"passthrough.vs");
    GLuint fs = buildShader(GL_FRAGMENT_SHADER, ASSETS"passthrough.fs");
    GLuint fs2 = buildShader(GL_FRAGMENT_SHADER, ASSETS"pacman.fs");
    shader_program = buildProgram(vs, fs, 0);
    dumpProgram(shader_program, "Default shader program");
    pacman_shader_program = buildProgram(vs, fs2, 0);
    dumpProgram(pacman_shader_program, "Pacman shader program");

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

    if (glfwGetKey(window, GLFW_KEY_R))
        mainLevel->Reset();

    mainLevel->Update(a_deltaTime);
}

void Render()
{
    glUseProgram(shader_program);
    
    viewMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-368.0f, 368.0f, -400.0f, 400.0f, -1.0f, 1.0f);
    
    mainLevel->DrawMap(DrawQuad); // reuse the draw quad function

    glUseProgram(GL_NONE);
}

void GUI()
{
    ImGui::Begin("Settings", 0, ImVec2(100, 50), 0.4f);
    {
        // Show some basic stats in the settings window 
        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        if (ImGui::Button("Eat Pellet"))
        {
            // Set all ghosts to frightened state for a specific amount of time
        }
    }
    ImGui::End();
}

void Cleanup()
{
    glDeleteBuffers(1, &quad_vbo);
    glDeleteVertexArrays(1, &quad_vao);
    glDeleteProgram(shader_program);

    delete mainLevel;
}

void DrawQuad(glm::vec2 a_position, glm::vec2 a_size, glm::vec3 a_color, int t)
{
    glUseProgram(t == 0 ? shader_program : pacman_shader_program);
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

    window = glfwCreateWindow(width, height, "Tutorial 2 - Pac-Man State Machine", NULL, NULL);
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