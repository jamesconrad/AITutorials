#include "FuzzyLogic.h"

#include <algorithm>
#include <imgui.h>

FuzzyGraph::FuzzyGraph()
{
    name = "Fuzzy Graph";
}

FuzzyGraph::FuzzyGraph(std::string a_name)
{
    name = a_name;
}

void FuzzyGraph::SetFuzzySets(std::vector<FuzzySet> a_fuzzySets)
{
    fuzzySets = a_fuzzySets;
}

std::vector<EvalName> FuzzyGraph::EvaluateGraph(float x)
{
    std::vector<EvalName> values;

    for (auto itr = fuzzySets.begin(); itr != fuzzySets.end(); itr++)
    {
        values.push_back(EvalName{ (*itr).Evaluate(x), (*itr).nameID });
    }

    std::sort(values.begin(), values.end(),
        [](const EvalName& a, const EvalName& b)
        {
            return a.eval > b.eval;
        });

    return values;
}

void FuzzyGraph::DrawGraph(float min, float max)
{
    ImGui::Begin(("Fuzzy " + name).c_str());
    {
        for (auto itr = fuzzySets.begin(); itr != fuzzySets.end(); itr++)
        {
            auto fs = (*itr);

            float points[120];
            for (int i = 0; i < 120; i++) points[i] = fs.Evaluate(glm::mix(min, max, i / 120.0f));

            ImGui::PlotLines(fs.nameID.name.c_str(), points, 120, 0, 0, 0.0f, 1.0f);
        }
    }
    ImGui::End();
}

FuzzySet::FuzzySet()
{
    nameID = NameID{ 0, "Fuzzy Set" };
    left = FuzzPoint{ 0, Bottom };
    center = FuzzPoint{ 1, Top };
    right = FuzzPoint{ 2, Bottom };
}

FuzzySet::FuzzySet(NameID a_name, FuzzPoint a_leftPoint, FuzzPoint a_rightPoint)
{
    nameID = a_name;
    left = a_leftPoint;
    center = a_leftPoint.isClampedToTop ? a_leftPoint : a_rightPoint;
    right = a_rightPoint;
}

FuzzySet::FuzzySet(NameID a_name, FuzzPoint a_leftPoint, FuzzPoint a_centerPoint, FuzzPoint a_rightPoint)
{
    nameID = a_name;
    left = a_leftPoint;
    center = a_centerPoint;
    right = a_rightPoint;
}

float FuzzySet::Evaluate(float x)
{
    // This won't evaluate correctly for a curve which dips down in the center, that curve will return 0 always
    float evaluatedA = glm::mix((float)left.isClampedToTop, (float)center.isClampedToTop, glm::smoothstep(left.xPosition, center.xPosition + 0.0001f, x));
    float evaluatedB = glm::mix((float)center.isClampedToTop, (float)right.isClampedToTop, glm::smoothstep(center.xPosition, right.xPosition + 0.0001f, x));
    return (evaluatedA * evaluatedB);
}