#ifndef INSTANCE
#define INSTANCE

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <string>

#include "primitives.hpp"

/**
 * @brief The Instance class
 * Class encoding an instance, that is a list of segments and metadata
 */
class Instance
{

protected:
    Parameters param;
    std::vector<Segment> segments;
    std::size_t n_segments;
    std::string instance_id; // id of the instance
    std::string author; // name of the author of this solution
    std::string host; // machine computing this solution
    bool dimacs = false; // it is a DIMACS instance
    const std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();

    /**
     * @brief read_json
     * @param filename
     * @return A rapidjson document with the data of the instance
     */
    rapidjson::Document read_json(std::string filename) const
    {
        std::ifstream in(filename, std::ifstream::in | std::ifstream::binary);
        if (!in.is_open())
        {
            std::cerr << "Error reading " << filename << std::endl;
            assert(false);
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
        return doc;
    }

    /**
     * @brief json_int_vec
     * @param values
     * @return The vector of values in this vector
     */
    std::vector<int> json_int_vec(rapidjson::Value &values) {
        std::vector<int> v;
        for (auto &x : values.GetArray())
            v.push_back((int)x.GetDouble());
        return v;
    }


    /**
     * @brief Instance Read an instance file
     * @param filename The filename of the sintance
     */
    Instance(const Parameters _param) : param(_param)
    {
        rapidjson::Document doc = read_json(param.instance_name);

        if (doc["type"].GetString() == "Instance_CGSHOP2022")
        {
            const std::vector<int> x_vec = json_int_vec(doc["x"]);
            const std::vector<int> y_vec = json_int_vec(doc["y"]);
            const std::vector<int> i_vec = json_int_vec(doc["edge_i"]);
            const std::vector<int> j_vec = json_int_vec(doc["edge_j"]);
            for (size_t k = 0; k < i_vec.size(); k++)
            {
                Point p(x_vec[i_vec[k]], y_vec[i_vec[k]]);
                Point q(x_vec[j_vec[k]], y_vec[j_vec[k]]);
                segments.push_back(Segment(p, q));
            }
            n_segments = segments.size();
        }
        else
        {
            dimacs = true;
            n_segments = doc["edges"].GetInt();
        }

        instance_id = doc["id"].GetString();
        author = "shadoks";
        char hn[80];
        gethostname(hn, 80);
        host = std::string(hn);
    }

    virtual ~Instance() = default;


public:
    /**
     * @brief elapsed_sec
     * @return Elapsed seconds since we read this instance
     */
    double elapsed_sec() const
    {
        auto cur_time = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = cur_time - start_time;
        return elapsed_seconds.count();
    }
};

#endif // INSTANCE
