#ifndef FUZZYLOGIC_H
#define FUZZYLOGIC_H

#include <GLM/glm.hpp>
#include <vector>

enum TopBottom {Bottom = 0, Top = 1};

struct NameID
{
public:
    int id;
    std::string name;
};

struct FuzzPoint
{
public:
    float xPosition;
    TopBottom isClampedToTop;
};

struct EvalName
{
public:
    float eval;
    NameID nameID;
};

class FuzzySet
{
public:
    FuzzySet();
    FuzzySet(NameID a_name, FuzzPoint a_leftPoint, FuzzPoint a_rightPoint);
    FuzzySet(NameID a_name, FuzzPoint a_leftPoint, FuzzPoint a_centerPoint, FuzzPoint a_rightPoint);

    float Evaluate(float x);
    NameID nameID;

private:
    FuzzPoint left, center, right;
};

class FuzzyGraph
{
public:
    FuzzyGraph();
    FuzzyGraph(std::string a_name);

    void SetFuzzySets(std::vector<FuzzySet> a_fuzzySets);

    std::vector<EvalName> EvaluateGraph(float x);

    void DrawGraph(float min, float max);

private:
    float min = 0.0f, max = 0.0f;

    std::string name;
    std::vector<FuzzySet> fuzzySets;
};


#endif // FUZZYLOGIC_H