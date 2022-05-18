// g++ -Wall -std=c++20 -Ofast -o solver solver.cpp
#include <iostream>
#include <limits>
#include "../include/cxxopts.hpp"
#include "greedy.hpp"
#include "angle.hpp"
#include "bad.hpp"
#include "dsatur.hpp"
#include "dsathull.hpp"
#include "conflict.h"

cxxopts::Options options("Shadoks CG:SHOP 2022 solver", "Partition into plane subgraphs");
cxxopts::ParseResult par;

void parse(int argc, char **argv) {
  options.add_options()
  ("help", "Print help")
  ("i,instance", "Instance file name (required)", cxxopts::value<std::string>())
  ("s,solution", "Solution file name", cxxopts::value<std::string>())
  ("a,algorithm", "Algorithm name (required: greedy, angle, bad, dsatur, dsathull, conflict)", cxxopts::value<std::string>())
  ("t,time", "Maximum time to start a new repetition in seconds", cxxopts::value<int>()->default_value("-1"))
  ("r,repetitions", "Maximum number of repetitions", cxxopts::value<int>()->default_value("100"))
  ("p,parameters", "Parameters file name", cxxopts::value<std::string>())
  ;
  
  par = options.parse(argc, argv);
}

Parameters parse_parameters()
{
    Parameters param;
    if (par.count("instance"))
        param.instance_name = par["instance"].as<std::string>();
    if (par.count("algorithm"))
        param.algorithm = par["algorithm"].as<std::string>();
    if (par.count("parameters"))
        param.read(par["parameters"].as<std::string>());
    return param;
}


int main(int argc, char **argv) {
    parse(argc, argv);

    if (par.count("help") /*|| !par.count("instance")  || !par.count("algorithm")*/)
    {
        std::cout << options.help() << std::endl;
        return 1;
    }

    const Parameters param = parse_parameters();

    int repetitions = par["repetitions"].as<int>();
    if (repetitions < 0)
        repetitions = std::numeric_limits<int>::max();

    double maxSec = par["time"].as<int>();
    if (maxSec < 0)
        maxSec = std::numeric_limits<double>::infinity();

    Solution *solver;

    // Compute initial solution
    if (param.algorithm != "conflict")
    {
        if (param.algorithm == "greedy")
        {
            solver = new Greedy(param);
            repetitions = 1;
        }
        else if (param.algorithm == "angle")
            solver = new Angle(param);
        else if (param.algorithm == "bad")
            solver = new Bad(param);
        else if (param.algorithm == "dsatur")
            solver = new DSatur(param);
        else if (param.algorithm == "dsathull")
            solver = new DSatHull(param);
        else
        {
            std::cerr << "Unknown algorithm: " << param.algorithm << std::endl;
            std::cerr << options.help() << std::endl;
            return 2;
        }

        // Run the algorithms for initial solutions
        int best = std::numeric_limits<int>::max();
        for (int rep = 0; rep < repetitions && solver->elapsed_sec() < maxSec; rep++)
        {
            solver->color();
            std::cout << "Colors: " << solver->numColors();
            if (solver->numColors() < best)
            {
                solver->write_sol();
                best = solver->numColors();
            }
            else
                std::cout << std::endl;
        }
    }
    else // Run the conflict optimizer
    {
        solver = new Conflict(param);
        solver->color();
    }

    delete solver;

    return 0;
}
