#ifndef CONFLICT_H
#define CONFLICT_H

#include <random>

#include "solution.hpp"

/**
 * @brief The Conflict class
 * Implementation of the conflict optimizer.
 * This also includes the DFS heuristic for reducing the queue of conflicts.
 */
class Conflict : public Solution
{

    /**
   * @brief The stack_event_t struct
   * Element in the DFS
   */
    struct stack_event_t {
        bool was_it_removed;
        bool was_it_added;
        int was_removed_from;
        int was_added_to;
        int edge;
    };

public:
    Conflict(Parameters param);

    virtual void color();
    //  inline virtual ~Conflict() = default;

private:
    void init_solution();
    void generate_intersection_map();
    bool crosses(int si, int sj) const;
    bool edge_can_be_added_to_graph(int si, int c) const;
    void remove_easy_segs(int bound);
    void add_easy_segs();
    bool shuffle(int n = 11);
    void shuffle_once();
    bool optimize();
    void reset_queue_count();
    void move_segments(unsigned c);
    void build_colorv();
    int conflict_dfs_optim_solution(bool one_shot);
    int add_data_point_to_graph_file();
    bool best_color(int seg, int &best_c, std::list<int> &conflicting_segs);
    void copy_sol(std::vector<std::list<int>> &s1, std::vector<std::list<int>> &s2);

    int dfsOptimize(std::list<int> &todo, std::list<int> &forbidden, int breadth, int depth, std::list<stack_event_t> &changes);
    void get_colors_with_small_conflict_number(int e, int breadth, std::list<int> &forbidden, std::list<std::pair<int, std::list<int>>> &conflict_colors);
    int get_intersectors(int c, int e, std::list<int> &intersectors, int max_intersections, std::list<int> &forbidden);
    bool in_list(int i, const std::list<int> &l) const;
    void remove_edges_from_color(int color, const std::list<int> &conflicts);
    int add_edges_to_color(int color, const std::list<int> &conflicts);
    int undo_changes(std::list<stack_event_t> &changes);
    int undo_change(stack_event_t &evt);
    int undo_added(stack_event_t &evt);
    int undo_removed(stack_event_t &evt);


private:
    std::vector<std::list<int>> classes; // classes[c] = list of (indices of) segments labeled as c
    std::list<long> easy_segs;
    std::vector<std::vector<unsigned int>> crossings; // data structure encoding the crossing between segments
    std::vector<int> queue_count; // queue_count[i] = number of times the i-th segment has been enqueued
    std::list<std::pair<double,int>> data_points; // data for plotting statistics

    std::default_random_engine generator;
    std::normal_distribution<double> distribution;
};

#endif // CONFLICT_H
