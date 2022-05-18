#ifndef DSATUR
#define DSATUR

#include <unordered_set>
#include <cstdlib>

#include "solution.hpp"

/**
 * @brief The DSatur class
 * Implementation of the DSatur algorithm.
 * This is similar to the greedy algorithm, but the order of the segments is computed dynamically.
 * In this class, we first color segments that cross segments of many different colors
 */
class DSatur : public Solution {
protected:
    std::vector<std::vector<bool>> neighbor_colors;
    std::vector<int> dsat; // dsat[i] = number of different colors of the segments crossing the i-th segment
    std::vector<int> degree; // degree[i] = number of segments crossing the i-th segment

    /**
     * @brief build_deg
     * Compute the degree of each segment
     */
    void build_deg()
    {
        for (const Segment &s : segments)
        {
            int isec = 0;
            for (const Segment &t : segments)
                if (s.cross(t))
                    isec++;
            degree.push_back(isec);
        }
    }

    // First color available
    int first_available(int si) const
    {
        unsigned int c;
        for (c = 0; c < neighbor_colors[si].size() && neighbor_colors[si][c]; c++) {}
        return c;
    }

    /**
     * @brief color
     * Abstract dsat coloring routine that calls a function colorChoice to choose
     * the color for each vertex
     * @param colorChoice
     */
    void color(auto&& colorChoice)
    {
        clearSol();
        std::unordered_set<int> uncolored;
        for (unsigned int si = 0; si < segments.size(); si++) // initialize uncolored segments
            uncolored.insert(si);

        while (uncolored.size())
        {
            int maxdsat = -1;
            for (unsigned int si: uncolored)
                maxdsat = std::max(maxdsat, dsat[si]);

            std::vector<std::tuple<int,int,int>> candidates;
            for (int si : uncolored) {
                if (dsat[si] == maxdsat)
                    candidates.push_back(std::make_tuple(-dsat[si], -degree[si], si));
            }

            sort(candidates.begin(), candidates.end());
            int r = rand() % (std::min((int)candidates.size(), 8));
            int vi = std::get<2>(candidates[r]);
            unsigned int c = colorChoice(vi);
            colorv[vi] = c;
            uncolored.erase(vi);
            neighbor_colors[vi].clear();

            for (int si : uncolored)
            {
                if (segments[si].cross(segments[vi]))
                {
                    if (neighbor_colors[si].size() < c+1)
                    {
                        neighbor_colors[si].resize(c+1);
                    }
                    if (!neighbor_colors[si][c])
                    {
                        neighbor_colors[si][c] = true;
                        dsat[si]++;
                    }
                }
            }
        }
    }

public:
    DSatur(Parameters param) : Solution(param)
    {
        neighbor_colors = std::vector<std::vector<bool>>(segments.size());
        dsat = std::vector<int>(segments.size());
        build_deg();
    }

    virtual void clearSol() {
        Solution::clear();
        for (int &x : dsat)
            x = 0;
        for (auto &x : neighbor_colors)
            x.clear();
    }

    /**
     * @brief color
     * Concrete implementation calling abstract one
     */
    virtual void color() {
        color([this](int si){ return first_available(si); });
    }

    virtual ~DSatur() = default;
};

#endif // DSATUR
