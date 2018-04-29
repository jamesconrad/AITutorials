#include "Pacman.h"
#include <math.h>

#define PELLET_DURATION 30.f

const static float w = 32.0f;
const static float h = 32.0f;

Map::Map(GLFWwindow* _window)
{
    for (int i = 0; i < 4; i++)
        ghosts[i] = new Ghost(this, Ghost::Name(i));

    player = new PacMan(this);

    window = _window;

    Reset();
}

void Map::SetState(Ghost::State s)
{
	for (int i = 0; i < 4; i++)
		ghosts[i]->ChangeState(s);
}

Map::~Map()
{
    for (int i = 0; i < 4; i++)
        delete ghosts[i];

    delete player;
}

void Map::Reset()
{
    int currentGhost = 0;
    for (int i = 0, ii = 12; i < 25; i++, ii--)
    {
        level[i] = layout[i];

        for (int j = 0, jj = -11; j < 23; j++, jj++)
        {
            if (layout[i][j] == 'M')
            {
                glm::vec2 startPosition = glm::vec2(jj * w, ii * h);
                ghosts[currentGhost]->targetTile = startPosition;
                ghosts[currentGhost]->currentState = Ghost::State::SCATTER; // Start in scatter mdoe
                ghosts[currentGhost++]->SetPosition(startPosition);
            }

            if (layout[i][j] == 'C')
            {
                glm::vec2 startPosition = glm::vec2(jj * w, ii * h);
                player->targetTile = startPosition;
                player->position = startPosition;
            }
        }
    }
}

void Map::Update(float a_deltaTime)
{
    for (int i = 0; i < 4; i++)
        ghosts[i]->Update(a_deltaTime);
    player->Update(a_deltaTime);
}

void Map::DrawMap(drawquad_function a_drawQuad)
{
    glm::vec2 size = glm::vec2(w, h);

    // Draw the tile map
    for (int i = 0; i < 25; i++)
    {
        for (int j = 0; j < 23; j++)
        {
            float x = (j - 11) * w; // from 0:23 range, to -11:11 range
            float y = (i - 12) * -h; // from 0:25 range, to -12:12 range

            if (level[i][j] == '#') // Draw the blue walls
            {
                a_drawQuad(glm::vec2(x, y), size, glm::vec3(0, 0, 1), 0);

                // Make the outline effect by drawing quads inside the blue quad
                a_drawQuad(glm::vec2(x, y), size * 0.8f, glm::vec3(0.0f), 0);
                if (level[i][(j + 1) % 23] == '#')     a_drawQuad(glm::vec2(x + w * 0.5f, y), size * 0.8f, glm::vec3(0), 0);
                if (level[i][(j + 22) % 23] == '#')    a_drawQuad(glm::vec2(x - w * 0.5f, y), size * 0.8f, glm::vec3(0), 0);
                if (level[(i + 1) % 25][j] == '#')     a_drawQuad(glm::vec2(x, y - h * 0.5f), size * 0.8f, glm::vec3(0), 0);
                if (level[(i + 24) % 25][j] == '#')    a_drawQuad(glm::vec2(x, y + h * 0.5f), size * 0.8f, glm::vec3(0), 0);
            }
            else if (level[i][j] == '*')    // Draw the yellow dots
                a_drawQuad(glm::vec2(x, y), size * 0.1f, glm::vec3(1, 1, 0), 0);
            else if (level[i][j] == '@')    // Draw the bigger yellow dots
                a_drawQuad(glm::vec2(x, y), size * 0.3f, glm::vec3(1, 1, 0), 0);
        }
    }

    // Draw the ghosts
    for (int i = 0; i < 4; i++)
    {
        ghosts[i]->DrawGhost(a_drawQuad);
    }

    player->DrawPacMan(a_drawQuad);
}

/* Will return the list, starting with the tile at the coordinate. The next 4 tiles are:
UP, LEFT, DOWN, RIGHT */
int Map::GetTiles(glm::vec2 a_coordinate, char(&a_tiles)[5], std::vector<glm::vec2> &centers, glm::vec2 oldTile)
{
    int xCoord = (int)round(a_coordinate.x /  w) + 11;  // from -11:11 range, to 0:23 range
    int yCoord = (int)round(a_coordinate.y / -h) + 12;  // from -12:12 range, to 0:25 range

    a_tiles[0] = level[yCoord][xCoord];

    a_tiles[1] = level[(yCoord + 1) % 25][xCoord];  // Up
    a_tiles[2] = level[yCoord][(xCoord + 22) % 23]; // Left
    a_tiles[3] = level[(yCoord + 24) % 25][xCoord]; // Down
    a_tiles[4] = level[yCoord][(xCoord + 1) % 23];  // Right

    int numPaths = 0;

    glm::vec2 coordD = a_coordinate - glm::vec2(0, h);
    glm::vec2 coordL = a_coordinate - glm::vec2(w, 0);
    glm::vec2 coordU = a_coordinate + glm::vec2(0, h);
    glm::vec2 coordR = a_coordinate + glm::vec2(w, 0);


    if (a_tiles[1] != '#' && glm::length(coordD - oldTile) > FLT_EPSILON) { numPaths++; centers.push_back(coordD); }
    if (a_tiles[2] != '#' && glm::length(coordL - oldTile) > FLT_EPSILON) { numPaths++; centers.push_back(coordL); }
    if (a_tiles[3] != '#' && glm::length(coordU - oldTile) > FLT_EPSILON) { numPaths++; centers.push_back(coordU); }
    if (a_tiles[4] != '#' && glm::length(coordR - oldTile) > FLT_EPSILON) { numPaths++; centers.push_back(coordR); }

    return numPaths;
}

int Map::GetIndex(glm::vec2 a_coordinate)
{
    int xCoord = (int)round(a_coordinate.x / w) + 11; // from -11:11 range, to 0:23 range
    int yCoord = (int)round(a_coordinate.y / -h) + 12; // from -12:12 range, to 0:25 range

    return yCoord * 25 + xCoord;
}

Ghost::Ghost(Map* map, Name ghostName)
    : map(map), name(ghostName)
{
}

void Ghost::DrawGhost(drawquad_function a_drawQuad)
{
    if(name == Name::BLINKY)
        a_drawQuad(position, glm::vec2(24), glm::vec3(235, 27, 36) / 255.0f, 0);
    else if (name == Name::PINKY)
        a_drawQuad(position, glm::vec2(24), glm::vec3(251, 181, 250) / 255.0f, 0);
    else if (name == Name::INKY)
        a_drawQuad(position, glm::vec2(24), glm::vec3(17, 255, 254) / 255.0f, 0);
    else if (name == Name::CLYDE)
        a_drawQuad(position, glm::vec2(24), glm::vec3(245, 179, 82) / 255.0f, 0);
}

void Ghost::ChangeState(State s)
{
	prevState = currentState;
	currentState = s;
	stageTimer = 0.f;
}

void Ghost::Update(float a_deltaTime)
{
    mixPercent += a_deltaTime * speed;
	stageTimer += a_deltaTime;
    {
        // You can make state changes here
		if (timingPhase == 1 || timingPhase == 2)
		{
			//7/20
			if (currentState == State::SCATTER && stageTimer >= 7.f)
				ChangeState(State::CHASE);
			else if (currentState == State::CHASE && stageTimer >= 20.f)
				ChangeState(State::SCATTER);
		}
		else if (timingPhase == 3)
		{
			//5/20
			if (currentState == State::SCATTER && stageTimer >= 5.f)
				ChangeState(State::CHASE);
			else if (currentState == State::CHASE && stageTimer >= 20.f)
				ChangeState(State::SCATTER);
		}
		else if (timingPhase == 4)
		{
			//5/inf
			if (currentState == State::SCATTER && stageTimer >= 5.f)
				ChangeState(State::CHASE);
		}
		else if (currentState == State::FRIGHTENED && stageTimer >= PELLET_DURATION)
		{
			ChangeState(prevState);
			timingPhase += 1;
		}
    }

    if (mixPercent >= 1.0f)
    {
        mixPercent = 0.0f;
        position = targetTile;

        glm::vec2 nearestCenterTile = glm::round(position / w) * w;
        int currentTileIndex = map->GetIndex(nearestCenterTile);

        char tiles[5];
        std::vector<glm::vec2> centers;
        int numPaths = map->GetTiles(nearestCenterTile, tiles, centers, oldTile);

        if (numPaths > 1) // We're at an intersection
        {
            position = nearestCenterTile;
            targetTile = MakeDecision(centers);
            oldTile = position;
        }
        else
        {
            position = nearestCenterTile;
            if (numPaths == 1)
                targetTile = centers[0];
            else // Shouldn't ever happen that we have no paths to go down
                targetTile = position;
            oldTile = position;
        }
    }

    MoveTowardsTarget(a_deltaTime);
}

void Ghost::MoveTowardsTarget(float a_deltaTime)
{
    position = glm::mix(oldTile, targetTile, mixPercent);
}
    //return centers[rand() % centers.size()];

glm::vec2 Ghost::MakeDecision(std::vector<glm::vec2> centers)
{
    switch (currentState)
	{
	case State::SCATTER:
	{
		// Move toward the fixed point (see tutorial doc)
		glm::vec2 home;
		switch (name)
		{
		case Ghost::PINKY:	home = glm::vec2(-368.0f, 400.0f); break;
		case Ghost::BLINKY:	home = glm::vec2(368.0f, 400.0f); break;
		case Ghost::INKY:	home = glm::vec2(368.0f, -400.0f); break;
		case Ghost::CLYDE:	home = glm::vec2(-368.0f, -400.0f); break;
		}

		glm::vec2 shortestDist = centers[0];

		for (int i = 0; i < centers.size(); i++)
		{
			float currShortest = glm::length(shortestDist - home);
			float dist = glm::length(centers[i] - home);

			if (dist < currShortest)
			{
				shortestDist = centers[i];
			}
		}
		return shortestDist;
		
	} break;
	case State::CHASE:
	{
		// Based on a distance to the player
		//glm::vec2 targetPos = glm::mix(pacmanPosition, glm::vec2(368.0f, 400.0f), factor);

		glm::vec2 shortestDist = centers[0];
		glm::vec2 targetPosition = map->player->position;

		switch (name)
		{
		case Ghost::PINKY: targetPosition = map->player->targetTile; break;
		case Ghost::BLINKY: targetPosition = map->player->position; break;
		case Ghost::INKY: targetPosition = map->player->position; break;
		case Ghost::CLYDE:
			float dst = glm::length(position - map->player->position);
			if (dst > 32 * 8)
				targetPosition = map->player->position;
			else
				targetPosition = glm::vec2(-368.0f, -400.0f);
			break;
		}

		for (int i = 0; i < centers.size(); i++)
		{
			float currShortest = glm::length(shortestDist - targetPosition);
			float dist = glm::length(centers[i] - targetPosition);

			if (dist < currShortest)
			{
				shortestDist = centers[i];
			}

		}
		return shortestDist;


	} break;
	case State::FRIGHTENED:
	{
		// Random decision
		return centers[rand() % centers.size()];
	} break;
	}

    return centers[0]; // This means the ghosts will eventually get stuck in an infinite loop
}

void Ghost::SetPosition(glm::vec2 a_position)
{
    position = a_position;
}

PacMan::PacMan(Map* map)
    : map(map)
{
}

void PacMan::DrawPacMan(drawquad_function a_drawQuad)
{
    a_drawQuad(position, glm::vec2(24), glm::vec3(1,1,0), 1);
}

void PacMan::Update(float a_deltaTime)
{
    mixPercent += a_deltaTime * speed;
	
    if (mixPercent >= 1.0f)
    {
        position = targetTile;

        glm::vec2 nearestCenterTile = glm::round(position / w) * w;
        int currentTileIndex = map->GetIndex(nearestCenterTile);

        char tiles[5];
        std::vector<glm::vec2> centers;
        int numPaths = map->GetTiles(nearestCenterTile, tiles, centers, oldTile);

        if (numPaths > 1) // We're at an intersection
        {
            position = nearestCenterTile;

            if (glfwGetKey(map->window, GLFW_KEY_UP))
            {
                mixPercent = 0.0f;
                for each (glm::vec2 tile in centers)
                    if (tile.y > position.y) targetTile = tile;
            }
            else if (glfwGetKey(map->window, GLFW_KEY_DOWN))
            {
                mixPercent = 0.0f;
                for each (glm::vec2 tile in centers)
                    if (tile.y < position.y) targetTile = tile;
            }
            else if (glfwGetKey(map->window, GLFW_KEY_LEFT))
            {
                mixPercent = 0.0f;
                for each (glm::vec2 tile in centers)
                    if (tile.x < position.x) targetTile = tile;
            }
            else if (glfwGetKey(map->window, GLFW_KEY_RIGHT))
            {
                mixPercent = 0.0f;
                for each (glm::vec2 tile in centers)
                    if (tile.x > position.x) targetTile = tile;
            }

            oldTile = position;
        }
        else
        {
            position = nearestCenterTile;
            if (numPaths == 1)
            {
                // We can change path
                if (glfwGetKey(map->window, GLFW_KEY_UP)) {
                    mixPercent = 0.0f;
                    if (centers[0].y > position.y) targetTile = centers[0];
                }
                else if (glfwGetKey(map->window, GLFW_KEY_DOWN)) {
                    mixPercent = 0.0f;
                    if (centers[0].y < position.y) targetTile = centers[0];
                }
                else if (glfwGetKey(map->window, GLFW_KEY_LEFT)) {
                    mixPercent = 0.0f;
                    if (centers[0].x < position.x) targetTile = centers[0];
                }
                else if (glfwGetKey(map->window, GLFW_KEY_RIGHT)) {
                    mixPercent = 0.0f;
                    if (centers[0].x > position.x) targetTile = centers[0];
                }
                else {
                    mixPercent = 0.0f;
                    targetTile = centers[0];
                }
            }
            else // Shouldn't ever happen that we have no paths to go down
                targetTile = position;
            oldTile = position;
        }
    }

    MoveTowardsTarget(a_deltaTime);
}

void PacMan::MoveTowardsTarget(float a_deltaTime)
{
    position = glm::mix(oldTile, targetTile, mixPercent);
}
