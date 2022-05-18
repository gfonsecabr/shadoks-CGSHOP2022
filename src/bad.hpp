#ifndef BAD
#define BAD

#include "angle.hpp"
#include <set>
#include <cstdlib>

class Bad : public Angle {
    std::set<int, Compare> good, bad;

public:
    Bad(Parameters param) : Angle(param), good(cmp), bad(cmp) {
    }

    void clearGoodBad() {
        Angle::clearSol();
        good.clear();
        bad.clear();
    }

    virtual void color() {
        clearSol();

        if (good.empty() && bad.empty())
            for (unsigned int si=0; si<segments.size(); si++)
                good.insert(si);

        std::clog << "Bad=" << bad.size() << " Good=" << good.size() << " ";

        std::vector<int> uncolored;
        for (int vi : good)
            uncolored.push_back(vi);
        for (int vi : bad)
            uncolored.push_back(vi);

        greedy(uncolored);

        for (int vi : classes.back()) {
            bad.insert(vi);
            good.erase(vi);
        }
    }

    virtual ~Bad() = default;
};

#endif // BAD
