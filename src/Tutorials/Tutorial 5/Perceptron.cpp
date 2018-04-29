#include "Perceptron.h"

#include <cmath>
#include <stdlib.h>

float SigmoidFunction(float val)
{
	const float e = 2.71828182845904523536f;

	return 1.0f / (1.0f + std::powf(e, -val));
}

float RandomRange(float min, float max)
{
	return min + ((max - min) * rand()) / (RAND_MAX + 1.0f);
}

Perceptron::Perceptron(unsigned int _featureVectorSize)
	: featureVectorSize(_featureVectorSize)
{
	weights = new float[featureVectorSize];

	for (unsigned int i = 0; i < featureVectorSize; ++i)
	{
		weights[i] = 0.0f;
	}
}

Perceptron::Perceptron(const Perceptron& other)
	: featureVectorSize(other.featureVectorSize)
	, bias(other.bias)
{
	weights = new float[featureVectorSize];

	// Copy the values from the other Perceptron.
	for (unsigned int i = 0; i < featureVectorSize; ++i)
	{
		weights[i] = other.weights[i];
	}
}

Perceptron::~Perceptron()
{
	delete[] weights;
}

Perceptron Perceptron::Crossover(const Perceptron& parent1, const Perceptron& parent2)
{
	// Naturally, we are assuming parent1.featureVectorSize == parent2.featureVectorSize.
	Perceptron result(parent1.featureVectorSize);

	// Average all the values between the parents.
	for (unsigned int i = 0; i < result.featureVectorSize; ++i)
	{
		result.weights[i] = (parent1.weights[i] + parent2.weights[i]) / 2.0f;
	}

	result.bias = (parent1.bias + parent2.bias) / 2.0f;

	return result;
}

float Perceptron::Evaluate(const float* featureVector) const
{
	float result = 0.0f;
	for (unsigned int i = 0; i < featureVectorSize; ++i)
	{
		result += featureVector[i] * weights[i];
	}

	result += bias;

	return SigmoidFunction(result);
}

void Perceptron::SetWeights(const float* _weights)
{
	for (unsigned int i = 0; i < featureVectorSize; ++i)
	{
		weights[i] = _weights[i];
	}
}

void Perceptron::RandomizeValues()
{
	for (unsigned int i = 0; i < featureVectorSize; ++i)
	{
		weights[i] = RandomRange(-2.0f, 2.0f); // This range of [-2, 2] is arbitrary.
	}

	bias = RandomRange(-2.0f, 2.0f);
}
