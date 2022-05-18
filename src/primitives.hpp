#ifndef PRIMITIVES
#define PRIMITIVES

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <functional>
#include <string>
#include <iomanip>      // std::put_time
#include <boost/unordered_set.hpp> // hash_combine
#include <chrono>

#include "../include/rapidjson/document.h"

typedef long long int i64;

/**
 * @brief The parameters_t struct
 * Parameters of the conflict optimizer
 */
struct Parameters
{
    std::string instance_name = "";
    std::string solution_name = "";
    std::string info_name = "";
    std::string algorithm = "greedy";
    double power = 1.2;
    double noise_mean = 1;
    double noise_var = 0.15;
    long max_queue = -1;
    long max_run_time = 3600;
    bool dfs = true;
    bool easy = true;
    bool loop = false;
    int loop_time = 3600;
    std::vector<double> power_loop = {1.1, 1.2, 1.3, 1.5, 2.0};
    long loop_index = 0;

    void read(const std::string &filename)
    {
        std::ifstream in(filename, std::ifstream::in | std::ifstream::binary);
        if (!in.is_open())
        {
            std::cerr << "Error reading " << filename << std::endl;
            exit(EXIT_FAILURE);
        }

        rapidjson::IStreamWrapper isw {in};
        rapidjson::Document doc {};
        doc.ParseStream(isw);
        if (doc.HasParseError())
        {
            std::cerr << "Error  : " << doc.GetParseError()  << std::endl;
            std::cerr << "Offset : " << doc.GetErrorOffset() << std::endl;
            exit(EXIT_FAILURE);
        }

        if (doc.HasMember("instance"))
            instance_name = doc["instance"].GetString();
        else
        {
            std::clog << "Missing mandatory argument from parameters file: instance" << std::endl;
            exit(EXIT_FAILURE);
        }
        if (doc.HasMember("solution"))
            solution_name = doc["solution"].GetString();
        if (doc.HasMember("info"))
            info_name = doc["info"].GetString();
        if (doc.HasMember("algorithm"))
            algorithm = doc["algorithm"].GetString();
        if (doc.HasMember("power"))
            power = doc["power"].GetDouble();
        if (doc.HasMember("noise_mean"))
            noise_mean = doc["noise_mean"].GetDouble();
        if (doc.HasMember("noise_var"))
            noise_var = doc["noise_var"].GetDouble();
        if (doc.HasMember("max_queue"))
            max_queue = doc["max_queue"].GetInt();
        if (doc.HasMember("max_run_time"))
            max_run_time =  doc["max_run_time"].GetInt();
        if (doc.HasMember("dfs"))
            dfs = doc["dfs"].GetBool();
        if (doc.HasMember("easy"))
            easy = doc["easy"].GetBool();
        if (doc.HasMember("loop"))
        {
            loop = doc["loop"].GetBool();
            if (loop)
                power = 1.1;
        }
        if (doc.HasMember("loop_time"))
            loop_time = doc["loop_time"].GetInt();

        std::clog << "{ instance: " << instance_name << ", "
                  << "solution: " << solution_name << ", "
                  << "info: " << info_name << ", "
                  << "power: " << power << ", "
                  << "noise_mean: " << noise_mean << ", "
                  << "noise_var: " << noise_var << ", "
                  << "max_queue: " << max_queue << ", "
                  << "max_run_time: " << max_run_time << ", "
                  << "dfs: " << dfs << ", "
                  << "easy: " << easy << ", "
                  << "loop: " << loop << ", "
                  << "loop_time: " << loop_time << " }" << std::endl;
    }
};

class Point {
public:
    i64 x,y;

    Point() {
    }

    Point(i64 _x, i64 _y) : x(_x), y(_y) {
    }

    i64 l1(const Point &p = Point(0,0)) const {
        return abs(x-p.x) + abs(y-p.y);
    }

    i64 linf(const Point &p = Point(0,0)) const {
        return std::max(abs(x-p.x), abs(y-p.y));
    }

    i64 l2sq(const Point &p = Point(0,0)) const {
        return (x-p.x)*(x-p.x) + (y-p.y)*(x-p.x);
    }

    friend bool operator==(const Point &p, const Point &q) {
        return p.x == q.x && p.y == q.y;
    }

    friend bool operator!=(const Point &p, const Point &q) {
        return !(p == q);
    }

    friend bool operator<(const Point &p, const Point &q) {
        return p.x < q.x || (p.x == q.x && p.y < q.y);
    }

    //  auto operator<=>(const Point& p) const = default;

    Point operator-(const Point &p) const {
        return Point(x-p.x, y-p.y);
    }

    Point operator+(const Point &p) const {
        return Point(x+p.x, y+p.y);
    }

    // Dot product
    i64 operator*(const Point &p) const {
        return x*p.x + y*p.y;
    }

    std::string toString() const {
        std::string s = "(" + std::to_string(x) + "," + std::to_string(y) + ")";
        return s;
    }

    bool inside(const Point &p, const Point &q) const {
        i64 minx = std::min(p.x, q.x);
        i64 miny = std::min(p.y, q.y);
        i64 maxx = std::max(p.x, q.x);
        i64 maxy = std::max(p.y, q.y);
        return x >= minx && x <= maxx && y >= miny && y <= maxy;
    }

    friend std::ostream& operator<<(std::ostream& os, const Point& p) {
        os << p.toString();
        return os;
    }
};


class Segment {
    Point p,q;
public:
    Segment() {
    }

    Segment(const Point  &_p, const Point  &_q) {
        if (_p < _q) {
            p = _p;
            q = _q;
        }
        else {
            p = _q;
            q = _p;
        }
    }

    const Point &get_p() const {
        return p;
    }

    const Point &get_q() const{
        return q;
    }

    i64 l1() const {
        return p.l1(q);
    }

    i64 linf() const {
        return p.linf(q);
    }

    i64 l2sq() const {
        return p.l2sq(q);
    }

    double slope() const {
        Point d = q - p;
        return (double) d.y / d.x;
    }

    friend bool operator==(const Segment &s, const Segment &t) {
        return s.p == t.p && s.q == t.q;
    }

    int orientation(const Point &r) const {
        i64 d1 = (q.y - p.y);
        i64 d2 = (r.x - q.x);
        i64 d3 = (q.x - p.x);
        i64 d4 = (r.y - q.y);
        i64 val = d1*d2 - d3*d4;

        return (val > 0) - (val < 0);
    }

    bool cross(const Segment &s) const {
        int o1 = orientation(s.p);
        int o2 = orientation(s.q);
        int o3 = s.orientation(p);
        int o4 = s.orientation(q);

        // No 3 colinear points
        if (o1 != 0 && o2 != 0 && o3 != 0 && o4 != 0) {
            return o1 != o2 && o3 != o4;
        }

        // Colinear but 4 distinct points
        if (s.p != p && s.q != q && s.p != q && s.q != p)
            return s.p.inside(p, q) || s.q.inside(p, q) || p.inside(s.p, s.q) || q.inside(s.p, s.q);

        // Same segment twice, return false for convinience
        if (*this == s)
            return false;

        // 3 points among 4 vertices, not all colinear
        if (o1 != 0 || o2 != 0 || o3 != 0 || o4 != 0)
            return false;

        // 3 points among 4 vertices, all colinear
        if (s.p == p)
            return q.inside(s.p, s.q) || s.q.inside(p, q);
        if (s.q == q)
            return p.inside(s.p, s.q) || s.p.inside(p, q);
        if (s.p == q)
            return p.inside(s.p, s.q) || s.q.inside(p, q);
        //      if (s.q == p)
        return q.inside(s.p, s.q) || s.p.inside(p, q);
    }

    std::string toString() const {
        std::string s = p.toString() + "-" + q.toString();
        return s;
    }

    friend std::ostream& operator<<(std::ostream& os, const Segment& s) {
        os << s.toString();
        return os;
    }
};

namespace std {

template <> struct hash<Point> {
    size_t operator()(const Point& p) const {
        size_t seed = 0;
        boost::hash_combine(seed, p.x);
        boost::hash_combine(seed, p.y);
        return seed;
    }
};

template <> struct hash<Segment> {
    size_t operator()(const Segment& s) const {
        size_t seed = 0;
        boost::hash_combine(seed, s.get_p().x);
        boost::hash_combine(seed, s.get_p().y);
        boost::hash_combine(seed, s.get_q().x);
        boost::hash_combine(seed, s.get_q().y);
        return seed;
    }
};

}


#endif // PRIMITIVES
