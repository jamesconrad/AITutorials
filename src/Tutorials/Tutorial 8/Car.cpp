#include "Car.h"

Car::Car()
{
    carLocation = vec2(0.0f);
    carHeading = 0.0f;
    carSpeed = 0.0f;
    steerAngle = 0.0f;
    texture = 0;
}

void Car::Turn(float a_amount, float a_deltaTime)
{
    steerAngle -= a_amount * a_deltaTime;

    steerAngle *= mix(0.96f, 1.0f, abs(a_amount)); // Add friction if there's no input
    steerAngle = clamp(steerAngle, -1.0f, 1.0f);
}

void Car::Accelerate(float a_amount, float a_deltaTime)
{
    if (a_amount > 0.0f) carSpeed += a_amount * 400.0f * a_deltaTime; // Forward speed
    if (a_amount <= 0.0f) carSpeed += a_amount * 200.0f * a_deltaTime; // Reverse speed

    if (carSpeed > maxSpeed) carSpeed *= 0.98f; // Friction to cap speed forward
    else if (carSpeed < -maxSpeed * 0.3f) carSpeed *= 0.98f; // Friction cap reverse speed
    else carSpeed *= 0.998f; // Friction Coast
}

void Car::Brake()
{
    carSpeed *= 0.97f;
}

void Car::UpdateCar(float a_deltaTime)
{
    float speedControlledSteer = steerAngle * mix(1.2f, 0.6f, max(0.0f, carSpeed / maxSpeed));

    vec2 frontWheel = carLocation + wheelBase / 2 * vec2(cos(carHeading), sin(carHeading));
    vec2 backWheel = carLocation - wheelBase / 2 * vec2(cos(carHeading), sin(carHeading));

    backWheel += carSpeed * a_deltaTime * vec2(cos(carHeading), sin(carHeading));
    frontWheel += carSpeed * a_deltaTime * vec2(cos(carHeading + speedControlledSteer), sin(carHeading + speedControlledSteer));

    carLocation = (frontWheel + backWheel) / 2.0f;
    carHeading = atan2(frontWheel.y - backWheel.y, frontWheel.x - backWheel.x);
}

void Car::ScreenWrap(int width, int height)
{
    const float border = 50.0f;
    if (carLocation.x > width + border || carLocation.x < -width - border) carLocation.x *= -1.0f;
    if (carLocation.y > height + border || carLocation.y < -height - border) carLocation.y *= -1.0f;
}
