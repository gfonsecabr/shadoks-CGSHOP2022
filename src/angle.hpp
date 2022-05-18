#ifndef ANGLE
#define ANGLE

#include <vector>

#include "greedy.hpp"

/**
 * @brief The Angle class
 * Specialized version of the greedy algorithm, where the segments are sorted by angle
 */
class Angle : public Greedy
{
protected:
    class Compare {
        std::vector<Segment> *p;
    public:
        Compare(std::vector<Segment> *_p) : p(_p) {}
        bool operator()(int x, int y) const {
            return std::make_tuple(p->at(x).slope(), x) < std::make_tuple(p->at(y).slope(), y);
        }
    };

    Compare cmp;

public:
    Angle(Parameters param) : Greedy(param), cmp(&segments)
    {}

    virtual void clearSol() {
        Greedy::clearSol();
    }

    virtual void color() {
        clearSol();

        std::vector<int> indices;
        for (unsigned int si = 0; si < segments.size(); si++)
            indices.push_back(si);

        std::sort(indices.begin(), indices.end(), cmp); // sort the segments by angle
        int r = rand() % indices.size(); // start at a random position
        std::vector<int> uncolored;
        for (unsigned int si = 0; si < segments.size(); si++)
            uncolored.push_back(indices[(si + r) % segments.size()]);

        greedy(uncolored);
    }

    virtual ~Angle() = default;
};

#endif // ANGLE
