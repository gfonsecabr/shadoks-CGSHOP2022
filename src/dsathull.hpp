#ifndef DSATHULL
#define DSATHULL

#include "dsatur.hpp"
#include <cstdlib>
#include <unordered_set>

/**
 * @brief The DSatHull class
 */
class DSatHull : public DSatur {
    std::vector<std::vector<Point>> colorhulls; // colorhulls[c] = list of points in the convex hull of the segments with color c

    /**
     * @brief bestAvailable
     * @param si Index of a segment
     * @return Best color available
     */
    int bestAvailable(int si) {
        std::vector<int> possible;

        for (unsigned int c = 0; c < colorhulls.size(); c++)
            if (c >= neighbor_colors[si].size() || !neighbor_colors[si][c])
                possible.push_back(c);

        if (possible.empty())
        {
            std::vector<Point> vec = {segments[si].get_p(), segments[si].get_q()};
            colorhulls.push_back(vec);
            return colorhulls.size() - 1;
        }

        // Find convex hull with min area increase
        int bestc = -1;
        i64 bestdiff = LLONG_MAX;
        for (int c : possible)
        {
            std::vector<Point> ch = colorhulls[c];
            i64 a1 = polyArea2(ch);
            if (std::count(ch.begin(), ch.end(), segments[si].get_p()) == 0)
                ch.push_back(segments[si].get_p());
            if (std::count(ch.begin(), ch.end(), segments[si].get_q()) == 0)
                ch.push_back(segments[si].get_q());
            ch = convex_hull(ch);
            i64 a2 = polyArea2(ch);
            i64 diff = a2 - a1;
            if (diff < bestdiff) {
                bestc = c;
                bestdiff = diff;
            }
        }

        // Update convex hull
        if (std::count(colorhulls[bestc].begin(), colorhulls[bestc].end(), segments[si].get_p()) == 0)
            colorhulls[bestc].push_back(segments[si].get_p());
        if (std::count(colorhulls[bestc].begin(), colorhulls[bestc].end(), segments[si].get_q()) == 0)
            colorhulls[bestc].push_back(segments[si].get_q());
        colorhulls[bestc] = convex_hull(colorhulls[bestc]);

        return bestc;
    }

    int orientation(const Point &p, const Point &q, const Point &r) {
        i64 d1 = (q.y - p.y);
        i64 d2 = (r.x - q.x);
        i64 d3 = (q.x - p.x);
        i64 d4 = (r.y - q.y);
        i64 val = d1*d2 - d3*d4;

        return (val > 0) - (val < 0);
    }

    std::vector<Point> convex_hull_sorted(const std::vector<Point> &points) {
        std::vector<Point> hull;
        hull.push_back(points.back());
        for (size_t i = 0; i < points.size() - 1; i++) {
            while (hull.size() >= 2 && orientation(points[i],
                                                   hull[hull.size()-1],
                                                   hull[hull.size()-2]) >= 0) {
                hull.pop_back();
            }
            hull.push_back(points[i]);
        }

        assert(hull[0] != hull.back());

        return hull;
    }

    i64 polyArea2(const std::vector<Point> &poly)
    {
        i64 a = (poly.back().x + poly[0].x) * (poly.back().y - poly[0].y);
        for (size_t i = 1; i < poly.size(); i++)
            a += (poly[i-1].x + poly[i].x) * (poly[i-1].y - poly[i].y);

        return a;
    }

    void angular_sort(std::vector<Point> &points)
    {
        sort(points.begin(), points.end());

        Point p0 = points.back();
        points.pop_back();

        auto compare = [p0, this](Point p1, Point p2) {
            int o = orientation(p0, p1, p2);
            if (o == 0)
                return p0.l2sq(p1) < p0.l2sq(p2);
            return o == 1;
        };

        std::sort(points.begin(), points.end(), compare);
        points.push_back(p0);
    }

    std::vector<Point> convex_hull(std::vector<Point> points) {
        if (points.size() <= 2)
            return points;

        angular_sort(points);

        return convex_hull_sorted(points);
    }

public:
    DSatHull(Parameters param) : DSatur(param)
    {}

    virtual void clearSol() {
        DSatur::clearSol();
        colorhulls.clear();
    }

    /**
     * @brief color
     * Run the algorithm
     */
    virtual void color() {
        DSatur::color([this](int si){ return bestAvailable(si); });
    }

    virtual ~DSatHull() = default;
};

#endif // DSATHULL
