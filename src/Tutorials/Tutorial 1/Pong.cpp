#include "Pong.h"

PongBall::PongBall(glm::vec2 position, glm::vec2 direction)
    : position (position)
{
    velocity = glm::normalize(direction) * maxSpeed;
}

void PongBall::Move(float deltaTime, PongPaddle &leftSide, PongPaddle &rightSide)
{
    const float wallPosition = 350.0f;
    const float halfWallWidth = 5.0f;

    const float paddlePosition = 630.0f;
    const float halfPaddleWidth = 5.0f;

    // Update the position
    position += velocity * deltaTime;

    // Bounce on the walls by reversing y velocity
    if (position.y > wallPosition - halfWallWidth)
    {
        velocity.y = -velocity.y;
    }
    else if (position.y < -wallPosition + halfWallWidth)
    {
        velocity.y = -velocity.y;
    }

    // Bounce on the paddles
    if (position.x > paddlePosition - halfPaddleWidth) // This means we're on the right side
    {
        if (position.y < rightSide.yPos + rightSide.paddleHeight * 0.5f &&
            position.y > rightSide.yPos - rightSide.paddleHeight * 0.5f)
        {   // There's a collision betweent the ball and the right-side paddle

            position.x = paddlePosition - halfPaddleWidth;
            velocity.x = -velocity.x; // Turn back
            velocity.y = (position.y - rightSide.yPos) / rightSide.paddleHeight * maxSpeed;

            velocity = glm::normalize(velocity) * maxSpeed;    // Normalize
        }
    }

    if (position.x < -paddlePosition + halfPaddleWidth) // This means we're on the left side
    {
        if (position.y < leftSide.yPos + leftSide.paddleHeight * 0.5f &&
            position.y > leftSide.yPos - leftSide.paddleHeight * 0.5f)
        {   // There's a collision betweent the ball and the right-side paddle

            position.x = -paddlePosition + halfPaddleWidth;
            velocity.x = -velocity.x; // Turn back
            velocity.y = (position.y - leftSide.yPos) / leftSide.paddleHeight * maxSpeed;

            velocity = glm::normalize(velocity)* maxSpeed;    // Normalize
        }
    }

    // Count score
    if (position.x > 640.0f) // Means the left side scored a point
    {
        position = glm::vec2(-620.0f, leftSide.yPos);
        velocity.x = -velocity.x;
        leftSide.score++;
    }
    if (position.x < -640.0f) // Means the right side scored a point
    {
        position = glm::vec2(620.0f, rightSide.yPos);
        velocity.x = -velocity.x;
        rightSide.score++;
    }
}

PongPaddle::PongPaddle()
    : score (0)
{
}

void PongPaddle::MoveUp(float deltaTime)
{
    yPos += paddleSpeed * deltaTime;

    // Do check for collision with wall here
    if (yPos + paddleHeight * 0.5f > 350.0f)
    {
        yPos = 350.0f - paddleHeight * 0.5f;
    }
}

void PongPaddle::MoveDown(float deltaTime)
{
    yPos -= paddleSpeed * deltaTime;

    // Do check for collision with wall here
    if (yPos - paddleHeight * 0.5f < -350.0f)
    {
        yPos = -350.0f + paddleHeight * 0.5f;
    }
}