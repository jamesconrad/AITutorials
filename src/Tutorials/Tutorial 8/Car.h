#ifndef CAR_H
#define CAR_H

#include "GLM/glm.hpp"

using namespace glm;

class Car
{
public:
    Car();

public:
    // Amount ranges from -1 (left) to 1 (right)
    void Turn(float a_amount, float a_deltaTime);

    // Amount ranges from -1 (reverse) to 1 (accelerate), where 0 is brake
    void Accelerate(float a_amount, float a_deltaTime);

    // Brakes hard
    void Brake();

    // Updates the position and values of the car
    void UpdateCar(float a_deltaTime);

    // Screen wraps the car
    void ScreenWrap(int width, int height);

    vec2 carLocation = vec2(0.0f);
    float carHeading = 0.0f;
    float carSpeed = 0.0f;
    float steerAngle = 0.0f;
    unsigned int texture = 0; // OGL texture

private:
    const float maxSpeed = 700.0f;
    const float wheelBase = 50.0f;
};


#endif // CAR_H