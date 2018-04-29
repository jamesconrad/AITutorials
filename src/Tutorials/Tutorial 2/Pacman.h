#ifndef _PACMAN_H_
#define _PACMAN_H_

#include <GLM\glm.hpp>
#include <string>
#include <vector>
#include <GLFW\glfw3.h>

typedef void(*drawquad_function)(glm::vec2, glm::vec2, glm::vec3, int t);

class Map;
class Ghost
{
public:
    enum Name
    {
        BLINKY = 0,
        INKY = 1,
        PINKY = 2,
        CLYDE = 3
    };

    enum State
    {
        SCATTER = 0,
        CHASE = 1,
        FRIGHTENED = 2
    };

public:
    Ghost(Map* map, Name ghostName);

    void DrawGhost(drawquad_function a_drawQuad);
    void Update(float a_deltaTime);
    void MoveTowardsTarget(float a_deltaTime);

    glm::vec2 MakeDecision(std::vector<glm::vec2> centers);

    void SetPosition(glm::vec2 a_position);
	void ChangeState(State s);
public:
    glm::vec2 oldTile;
    glm::vec2 targetTile;
    float mixPercent = 0.0f;

    State currentState;
	int timingPhase = 1;
	float stageTimer = 0.f;
	State prevState;

    Name name;
    Map* map;
    glm::vec2 position;
    float speed = 8.0f;
};

class PacMan
{
public:
    PacMan(Map* map);

    void DrawPacMan(drawquad_function a_drawQuad);
    void Update(float a_deltaTime);
    void MoveTowardsTarget(float a_deltaTime);

public:
    glm::vec2 oldTile;
    glm::vec2 targetTile;
    float mixPercent = 0.0f;	

    Map* map;
    glm::vec2 position;
    float speed = 8.0f;
};

class Map
{
public:
    Map(GLFWwindow* _window);
    ~Map();

    // Resets everything
    void Reset();

    // Updates the ghosts basically
    void Update(float a_deltaTime);

    // A simple way to draw the map out using quads
    void DrawMap(drawquad_function a_function);

    int GetTiles(glm::vec2 a_coordinate, char(&a_tiles)[5], std::vector<glm::vec2> &centers, glm::vec2 oldTile);
    int GetIndex(glm::vec2 a_coordinate);
	void SetState(Ghost::State s);
public:
    Ghost* ghosts[4];
    PacMan* player;

    // A pacman level - 25 characters wide, 25 tall
    const std::string layout[25] =
    {
        "#######################",
        "#          #          #",
        "#@### #### # #### ###@#",
        "# ### #### # #### ### #",
        "#                     #",
        "# ### # ####### # ### #",
        "#     # ####### #     #",
        "# ### #    #    # ### #",
        "#   # #### # #### #   #",
        "### # #    M    # # ###",
        "#   # # ### ### # #   #",
        "# ###   ##MMM##   ### #",
        "#   # # ### ### # #   #",
        "### # #         # # ###",
        "#   # # ####### # #   #",
        "# ### # ####### # ### #",
        "#          #          #",
        "# ### #### # #### ### #",
        "#@  #      C      #  @#",
        "### # # ####### # # ###",
        "#     #    #    #     #",
        "# ######## # ######## #",
        "# ######## # ######## #",
        "#                     #",
        "#######################"
    };

    std::string level[25];

public:
    GLFWwindow* window;
};

#endif