#ifndef GREEDY
#define GREEDY

#include <cstdlib>
#include <vector>
#include "solution.hpp"

/**
 * @brief The Greedy class
 * Implementation of the greedy algorithm.
 * Segments are traversed in any order. For each segment, we assign them the label
 * of the first color that is compatible with it (which does not have a segment intersecting it)
 */
class Greedy : public Solution {
protected:
    std::vector<std::vector<int>> classes; // classes[c] = list of (indices of) segments labeled as c

    /**
     * @brief firstAvailable
     * @param si Index of a segment
     * @return The first compatible color. If there is no compatible color,
     * we create a new one
     */
    int first_available(int si)
    {
        unsigned int c;
        for (c = 0; c < classes.size(); c++)
        {
            bool valid = true;
            for (int ti : classes[c])
                if (segments[si].cross(segments[ti]))
                {
                    valid = false;
                    break;
                }
            if (valid)
                break;
        }

        if (c == classes.size())
            classes.push_back({});

        return c;
    }

    /**
     * @brief build_colorv
     * Make the map `colorv`: segments -> color
     */
    void build_colorv()
    {
        for (unsigned int c = 0; c < classes.size(); c++)
            for (int vi : classes[c])
                colorv[vi] = c;
    }


    /**
     * @brief greedy
     * Run the greedy algorithm on a list of segments
     * @param uncolored List of (indices of) uncolored segments
     */
    void greedy(std::vector<int> &uncolored)
    {
        while (uncolored.size() > 0)
        {
            const int vi = uncolored.back();
            uncolored.pop_back();
            unsigned int c = first_available(vi);
            classes[c].push_back(vi);
        }
        build_colorv();
    }

public:
    Greedy(Parameters param) : Solution(param)
    {}

    /**
     * @brief clearSol
     * Reset the solution
     */
    virtual void clearSol()
    {
        Solution::clear();
        classes.clear();
    }

    /**
     * @brief color
     * Run the algorithm
     */
    virtual void color()
    {
        clearSol();

        std::vector<int> uncolored;
        for (unsigned int si = 0; si < segments.size(); si++)
            uncolored.push_back(segments.size()-si-1);

        greedy(uncolored);
    }

    virtual ~Greedy() = default;
};

#endif // GREEDY
