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
    PongBall(glm::vec2 position, glm::vec2 direction);

    void Move(float deltaTime, PongPaddle &leftSide, PongPaddle &rightSide);

    glm::vec2 position;
    glm::vec2 velocity;

    const float maxSpeed = 600.0f;
};

#endif