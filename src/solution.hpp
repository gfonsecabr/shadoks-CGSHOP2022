#ifndef SOLUTION
#define SOLUTION

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <map>
#include <algorithm>
#include <stack>
#include <unistd.h>
#include <string>

#include "../include/rapidjson/document.h"
#include "../include/rapidjson/istreamwrapper.h"
#include "../include/rapidjson/writer.h"
#include "../include/rapidjson/stringbuffer.h"
#include "../include/rapidjson/ostreamwrapper.h"

#include "instance.hpp"

/**
 * @brief The Solution class
 * A solution is a vector that maps each index (of a segment) to (the label of) its color
 */
class Solution : public Instance
{

protected:
    std::list<long> clique;
    std::vector<int> colorv; // colorv[i] is the label of the i-th segment. -1 means unlabeled

    Solution(Parameters param) : Instance(param) {
        clear();
        process_parameters();
    }

public:
    virtual void color() = 0;

    /**
     * @brief clear
     * Clear, or reset a solution. Each segments has the label -1
     */
    virtual void clear()
    {
        colorv.clear();
        for (size_t i = 0; i < segments.size(); i++)
            colorv.push_back(-1);
    }

    /**
     * @brief write_sol
     * Write the solution
     * @param basefn
     * @param quiet Output logging information if true
     */
    void write_sol(bool quiet = false) const
    {
        std::string filename = instance_id + "." + param.algorithm + "." + timeString() + ".sol.json";

        if (!quiet)
            std::cout << "->" << filename << std::endl;

        std::ofstream file(filename, std::fstream::out | std::ifstream::binary);

        file << "{" << std::endl;
        file << "\t\"type\": \"Solution_CGSHOP2022\"," << std::endl;
        file << "\t\"instance\": \"" << instance_id << "\"," << std::endl;
        file << "\t\"num_colors\": " << numColors() << "," << std::endl;

        file << "\t\"meta\": {" << std::endl;
        file << "\t\t\"input\": \"" << param.instance_name << "\"," << std::endl;
        file << "\t\t\"author\": \"" << author << "\"," << std::endl;
        file << "\t\t\"start_time\": \"" << timeString(start_time) << "\"," << std::endl;
        file << "\t\t\"host\": \"" << host << "\"," << std::endl;
        file << "\t\t\"save_time\": \"" << timeString() << "\"," << std::endl;
        file << "\t\t\"elapsed_time\": " << elapsed_sec() << "," << std::endl;
        file << "\t\t\"" << "last_meta\": \"\"" << std::endl;
        file << "\t}," << std::endl;

        file << "\t\"colors\": [";
        for (size_t i=0; i<colorv.size(); i++)
        {
            file << colorv[i];
            if (i != colorv.size() - 1)
                file << ", ";
        }
        file << "]" << std::endl;
        file << "}" << std::endl;
        file.close();
    }

    /**
     * @brief read
     * Read a solution file. We read a solution in @class Conflict
     * @param fn Filename of the solution
     */
    void read(std::string fn)
    {
        rapidjson::Document doc = read_json(fn);
//        const long num_colors = doc["num_colors"].GetInt();

        colorv.resize(segments.size());
        for (unsigned i = 0; i < segments.size(); i++)
            colorv[i] = doc["colors"][i].GetInt();
    }

    int numColors() const
    {
        return 1 + *std::max_element(colorv.begin(), colorv.end());
    }

    std::string timeString(std::chrono::system_clock::time_point tp = std::chrono::system_clock::now()) const
    {
        std::time_t tt = std::chrono::system_clock::to_time_t(tp);
        struct std::tm * ptm = std::localtime(&tt);
        std::ostringstream oss;
        oss << std::put_time(ptm, "%Y%m%d-%H%M%S");
        std::string s = oss.str();
        return s;
    }

    /**
     * @brief Conflict::load_parameters
     * Process the parameters
     */
    void process_parameters()
    {
        if (!param.solution_name.empty())
            Solution::read(param.solution_name);
        if (!param.info_name.empty())
            parse_info_file();
        if (param.max_queue < 1)
        {
            long card = segments.size();
            double nb_repetition = 75000.0 / card;
            nb_repetition = nb_repetition * nb_repetition;
            nb_repetition = nb_repetition * 2000;
            param.max_queue = nb_repetition;
        }
    }

    /**
     * @brief parse_info_file
     * Read the clique in the info file
     */
    void parse_info_file()
    {
        rapidjson::Document doc = read_json(param.info_name);
        const rapidjson::Value& json_easy = doc["clique"];
        for (auto &ob : json_easy.GetArray()) {
            clique.push_back(ob.GetInt());
        }
    }

    virtual ~Solution() = default;
};

#endif // SOLUTION
