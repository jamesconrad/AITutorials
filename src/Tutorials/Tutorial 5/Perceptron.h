#ifndef _PERCEPTRON_H_
#define _PERCEPTRON_H_

class Perceptron
{
public:
	Perceptron(unsigned int featureVectorSize);
	Perceptron(const Perceptron& other);
	~Perceptron();

	// Naively mixes two Perceptrons together to generate a new Perceptron which is a mix of the two parents.
	// This can be used for Genetic Algorithms.
	static Perceptron Crossover(const Perceptron& parent1, const Perceptron& parent2);

	// Evaluates the full mathematical expression of the Perceptron:
	// val = activationFunc(Sum(feature * weight) +- bias)
	// FeatureVector must be a pointer to an array of floats with length 'featureVectorSize'.
	float Evaluate(const float* featureVector) const;

	// Generates random values for each weight and the bias.
	void RandomizeValues();
	void RandomizeValues(Perceptron p);

	// 'weights' must be a pointer to an array of floats with length 'featureVectorSize'.
	void SetWeights(const float* weights);

	float bias = 0.0f;

//private:
	// Dynamically allocated array of weights.
	float* weights;

	// Remembers the size of the allocated arrays.
	unsigned int featureVectorSize;
};

#endif