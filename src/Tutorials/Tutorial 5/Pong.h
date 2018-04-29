#ifndef _PONG_H_
#define _PONG_H_

#include <GLM\glm.hpp>

class PongPaddle
{
public:
    PongPaddle();

    void MoveUp(float deltaTime);
    void MoveDown(float deltaTime);

    float yPos;

    const float paddleSpeed = 400.0f;
    const float paddleHeight = 75.0f;
    
    int score;
};

class PongBall
{
public:
    enum BouncedOnSide
    {
        LEFT = -1,
        DIDNTBOUNCE = 0,
        RIGHT = 1,
    };

public:
    PongBall(glm::vec2 position, glm::vec2 direction);

    // This function has been modified to return an int if it bounces
    BouncedOnSide Move(float deltaTime, PongPaddle &leftSide, PongPaddle &rightSide);

    glm::vec2 position;
    glm::vec2 velocity;

    const float maxSpeed = 600.0f;
};

#endif