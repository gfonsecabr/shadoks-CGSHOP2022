#include "conflict.h"

Conflict::Conflict(Parameters param)
    : Solution(param),
      queue_count(n_segments, 0)
{
    generate_intersection_map();
}


/**
 * @brief Conflict::init_solution
 * Initialize the solution from the current solution, or make a new solution
 */
void Conflict::init_solution()
{
    if (!param.solution_name.empty()) // read the solution
    {
        classes.resize(numColors());
        for (std::size_t i = 0; i < colorv.size(); ++i)
        {
            const int c = colorv.at(i);
            classes.at(c).push_back(i);
        }
    }
    else // make a new solution
    {
        for (unsigned si = 0; si < n_segments; si++)
        {
            bool is_inserted = false;
            for (unsigned c = 0; c < classes.size(); c++)
            {
                if (edge_can_be_added_to_graph(si, c))
                {
                    classes.at(c).push_back(si);
                    is_inserted = true;
                    break;
                }
            }
            if (!is_inserted)
            {
                classes.push_back(std::list<int>());
                classes.back().push_back(si);
            }
        }
        std::clog << "We have this many colors: " << classes.size() << std::endl;
    }
}

/**
 * @brief crosses
 * @param si
 * @param sj
 * @return True if the segments with indices si and sj cross (using the precomputed crossings)
 */
bool Conflict::crosses(int si, int sj) const
{
    const int index_j = sj / 32;
    const int shift_j = sj % 32;
    return (crossings[si][index_j] & (1 << shift_j));
}

/**
 * @brief Conflict::edge_can_be_added_to_graph
 * @param si
 * @param c
 * @return True if the si-th index can be added to the
 */
bool Conflict::edge_can_be_added_to_graph(int si, int c) const
{
    for (int sj : classes.at(c))
        if (crosses(si, sj))
            return false;
    return true;
}

/**
 * @brief Conflict::remove_easy_segs
 * If we can color all the segments but one with k colors, and this uncolored
 * segment intersects less than k segments, we are sure that we can color it
 * with one of the used colors because it cannot intersect a segment from each
 * of the k colors.
 * This property can be generalized as follows: if we obtain a coloring with k
 * colors, and there are uncolored segments with less than k intersecting
 * segments, they can be colored with the same number of colors.
 * Based on this, we remove the lower degree segments because we know that we
 * can color them later. Moreover, once we remove a segment, the degree of its
 * intersecting segments decrement, so this removal of segments can be repated
 * to remove the maximum number of segments.
 * The only condition is that the segments must be later colored in the reverse
 * order.
 * @param bound We only keep segments that less than `bound` intersecting segments
 */
void Conflict::remove_easy_segs(int bound)
{
    // Compute the degree of each segment
    std::vector<long> degree(n_segments, 0); // degree[i] = number of segments intersecting the i-th segment
    for (unsigned i = 0; i < n_segments; ++i)
        for (unsigned j = i+1; j < n_segments; ++j)
            if (crosses(i, j))
            {
                degree[i]++;
                degree[j]++;
            }
    // Remove segments until all the segments have degree >= bound
    easy_segs.clear();
    bool stability = false;
    while (!stability)
    {
        stability = true;
        // Get the segment with mininimum degree
        int s = 0; // (index of) the segment to remove (with minimum degree)
        for (unsigned i = 1; i < degree.size(); ++i)
            if (degree[i] < degree[s])
                s = i;
        if (degree[s] < bound)
        {
            // Remove this segment and update the degrees
            stability = false;
            degree[s] = std::numeric_limits<long>::max(); // we set its degree to infinity so that we do not remove it again
            easy_segs.push_back(s);
            for (unsigned i = 0; i < degree.size(); ++i)
                if (degree[i] != std::numeric_limits<long>::max() && crosses(i, s))
                    degree[i]--;
        }
    }

    // Remove the easy segments from the solution
    std::set<long> set_easy_segs(easy_segs.cbegin(), easy_segs.cend());
    for (unsigned c = 0; c < classes.size(); c++)
        for (auto it = classes.at(c).begin(); it != classes.at(c).end(); )
        {
            if (set_easy_segs.count(*it) > 0) // this segment is in the list of easy segments
                it = classes.at(c).erase(it);
            else
                ++it;
        }

    std::clog << "Number of easy segments removed: " << easy_segs.size() << std::endl;
}

/**
 * @brief Conflict::generate_intersection_map
 * Precompute all the intersections
 */
void Conflict::generate_intersection_map()
{
    if (!dimacs)
    {
        const int m = segments.size();
        int size = ((m - 1) / 32) + 1;
        crossings.resize(m);
        for (int i = 0; i < m; ++i)
            crossings[i].resize(size);

        for (int i = 0; i < m; ++i)
        {
            const int index_i = i / 32;
            const int shift_i = i % 32;
            for (int j = i + 1; j < m; ++j) {

                if (segments[i].cross(segments[j]))
                {
                    const int index_j = j / 32;
                    const int shift_j = j % 32;
                    crossings[i][index_j] = crossings[i][index_j] | (1 << shift_j);
                    crossings[j][index_i] = crossings[j][index_i] | (1 << shift_i);
                }
            }
        }
    }
    else
    {
        rapidjson::Document doc = read_json(param.instance_name);
        const int m = doc["edges"].GetInt();
        int size = ((m - 1) / 32) + 1;
        crossings.resize(m);
        for (int i = 0; i < m; ++i)
            crossings[i].resize(size);

        for (auto &x : doc["pairs"].GetArray())
        {
            const int i = x[0].GetInt() - 1;
            const int j = x[1].GetInt() - 1;
            const int index_i = i / 32;
            const int shift_i = i % 32;
            const int index_j = j / 32;
            const int shift_j = j % 32;
            crossings[i][index_j] = crossings[i][index_j] | (1 << shift_j);
            crossings[j][index_i] = crossings[j][index_i] | (1 << shift_i);
        }
    }

}


void Conflict::color()
{
    if (!param.solution_name.empty())
        Solution::read(param.solution_name);
    init_solution();

    if (!clique.empty() && clique.size() == classes.size())
    {
        std::cout << "File is optimal" << std::endl;
        return;
    }

    while (elapsed_sec() < param.max_run_time)
    {
        if (optimize())
        {
            build_colorv();
            std::cout << "Writing solution of size " << classes.size() << std::endl;
            write_sol("conflict");
            //Also write the new data point for the paper graph
            add_data_point_to_graph_file();
        }
    }
}

/**
 * @brief Conflict::optimize
 * Try to optimize the solution using the conflict optimizer and the DFS technique.
 * This handles the shuffling and multistart aspect of the algorithm
 */
bool Conflict::optimize()
{
    distribution = std::normal_distribution<double>(param.noise_mean, param.noise_var);

    if (param.easy)
        remove_easy_segs(classes.size() - 1);

    while (true)
    {
        shuffle(); // shuffle the solution (and improve it maybe)

        long old_size = classes.size();
        long new_size = 0;
        while (new_size != old_size) // @todo Does this make any sense? There is a return later
        {
            old_size = classes.size();
            conflict_dfs_optim_solution(false);
            new_size = classes.size();
            std::clog << "sol size is: " << new_size << std::endl;
            if (old_size != new_size)
            {
                add_easy_segs();
                return true;
            }
            // If we are running for more than MAX_RUN_TIME: we are out
            if (elapsed_sec() > param.max_run_time)
                return false;
        }
    }
    return false;
}

/**
 * @brief Conflict::shuffle
 * Move the segments between the colors a number of times.
 * If this procedure reduces the number of colors, we restart the counter
 * @param n Number of trials
 * @return True if the number of colors is improved
 */
bool Conflict::shuffle(int n)
{
    int count = 0;
    long old_size = 0;
    long size = classes.size();
    while (count < n)
    {
        std::clog << "size of solution: " << size << "\t(count=" << count << ")" << std::endl;
        shuffle_once();
        old_size = size;
        size = classes.size();
        // If we improve the solution, we restart the counter
        if (old_size != size)
        {
            count = -1;
            /*
            name = "./optimized/" + G.instance + "_sol_" + std::to_string(sol_cpt++) + "_size_" + std::to_string(sol.graphs.size());
            if (is_using_easy) {
                name = "./optimized/" + G.instance + "_sol_partial_from_easy_coloring" + std::to_string(sol_cpt++) + "_size_" + std::to_string(sol.graphs.size());
            }
            write_solution(G, sol, name);
            */
            if (param.easy)
            {
                add_easy_segs(); // @todo Why? I should just remove more segments
                return true;
            }
        }
        count++;
    }
    return false;
}

/**
 * @brief Conflict::shuffle_once
 * Try to optimize a solution by moving the edges from color to color.
 * As a result, the solution is shuffled even when no optimization is done
 */
void Conflict::shuffle_once()
{
    // order classes from smallest to largest
    sort(classes.begin(), classes.end(), [](const std::list<int> & c1, const std::list<int> & c2) -> bool
    {
        return c1.size() < c2.size();
    });

    // for each class, for each of its segments, we try to move the segment to another class
    for (unsigned c = 0; c < classes.size(); c++)
    {
        move_segments(c);
        if (classes.at(c).empty())
        {
            classes.erase(std::next(classes.begin(), c));
            c--;
//            std::clog << "one color removed; current size = " << classes.size() << std::endl;
        }
    }
}

/**
 * @brief Conflict::reset_queue_count
 */
void Conflict::reset_queue_count()
{
    queue_count.clear();
    queue_count.resize(n_segments, 0);
    // Put clique segments weight to infty
    for (int si : clique)
        queue_count.at(si) = std::numeric_limits<int>::max();
}

/**
 * @brief Conflict::add_easy_segs
 * Add the easy segments to the solution. We use a greedy approach, inserting
 * the segments in the reverse order of their removal.
 */
void Conflict::add_easy_segs()
{
    if (!param.easy) // Test whether or not we use easy
        return;

    if (crossings.empty())
        generate_intersection_map();

    std::clog << "There are " << easy_segs.size() << " edges to greedy color" << std::endl;

    for (auto it = easy_segs.rbegin(); it != easy_segs.rend(); ++it)
    {
        // greedy coloring
        bool colored = false;
        for (unsigned c = 0; c < classes.size() && !colored; c++)
        {
            if (edge_can_be_added_to_graph(*it, c))
            {
                classes.at(c).push_back(*it);
                colored = true;
            }
        }
        if (!colored)
        {
            std::cerr << "ERROR: could not greedy color the graph" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

int Conflict::conflict_dfs_optim_solution(bool one_shot)
{
    int DEBUG_COUNT = 0;

    // order classes from smallest to largest
    std::sort(classes.begin(), classes.end(), [](const std::list<int> & c1, const std::list<int> & c2) -> bool
    {
        return c1.size() < c2.size();
    });

    //for each color class, for each of its segments, we try to move the edge to another color class
    for (int c = 0; c < classes.size(); c++)
    {
        move_segments(c);
        if (classes.at(c).empty())
        {
            classes.erase(std::next(classes.begin(), c));
            c--;
        }
        else
        {
            std::clog << "entering conflict solver for the " << ++DEBUG_COUNT << " time" << std::endl;
            // We did not manage to move every segment. We start the conflict solver
            std::list<int> queue;
            std::vector<std::list<int>> temp_sol; // This is a save. In case we do not succeed to improve, we'll restore the save
            copy_sol(classes, temp_sol);

            // We move the remaining edges to the queue, and delete the color
            for (int si : classes.at(c))
                queue.push_back(si);
//            long first_edge_id_of_it = classes.at(c).front(); // @todo Why?
            classes.erase(std::next(classes.begin(), c));

            // Now, for each segment in the queue, we move it to the color class with
            // least conflict, and move the conflicting segments to the queue.
            // Repeat until the queue is empty, or until we cannot find a color class
            // such that we would re-queue a segment (put into the queue a segment
            // that already went into the queue this generation)
            bool successfull_removal = true;
            reset_queue_count();
            std::list<int> dfs_queue;
            while (!queue.empty() || !dfs_queue.empty())
            {
                // test for stopping running
                if (elapsed_sec() > param.max_run_time)
                {
                    exit(0);
                }
                // Test for switching params
                if (param.loop && elapsed_sec() > param.loop_time * (param.loop_index + 1))
                {
                    std::clog << "Switching param" << std::endl;
                    for (int power : param.power_loop)
                        std::clog << power << std::endl;
                    param.loop_index++;
                    param.power = param.power_loop.at(param.loop_index % 5);
                    std::clog << "New power is: " << param.power << std::endl;
                }

                int cur_seg;
                if (!dfs_queue.empty()) // @todo  I do not understand this well
                {
                    cur_seg = dfs_queue.front();
                    std::list<int> solo;
                    solo.push_back(cur_seg);
                    std::list<stack_event_t> stack;

                    //We try to put it, if we succeed good, else it goes to the conflict solver
                    std::list<int> forbidden;
                    int depth = 3;
                    if (queue.size() < 3)
                        depth = (queue.size() == 1) ? 5 : 7;
                    if (param.dfs && dfsOptimize(solo, forbidden, 3, depth, stack) == 0)
                    {
                        // we succeeded
                        dfs_queue.pop_front();
                        continue;
                    }
                    else
                    {
                        queue.push_back(dfs_queue.front());
                        dfs_queue.pop_front();
                        continue;
                    }
                }
                else
                {
                    cur_seg = queue.front();
                    queue.pop_front();
                }

                // Find the color of least conflict
                int best_c;
                std::list<int> conflicting_segs;
                if (best_color(cur_seg, best_c, conflicting_segs))
                {
                    // move the conflicting segments from the color class to the DFS queue
                    for (int si : conflicting_segs)
                    {
                        dfs_queue.push_back(si);
                        classes.at(best_c).remove(si);
                    }
                    // add cur_seg to the best color class
                    classes.at(best_c).push_back(cur_seg);

                    //flag cur_seg as untouchable from now on
                    queue_count[cur_seg]++;// = queue_count[cur_seg.id] + 1;
                }
                else
                {
                    // all graphs have conflicts with an old queued segment
                    // Stop computation, and restore to temp_sol
                    std::cout << "MAX RUN TIME (" << param.max_queue << ") REACHED" << std::endl;

                    copy_sol(temp_sol, classes);
                    successfull_removal = false;
                    break;
                }
            }
            if (!successfull_removal)
            {
                if (one_shot)
                    return 0;
                else
                    std::cout << "No one shot" << std::endl;
            }
            else
            {
                std::cout << "REMOVED a color" << std::endl;
                return 0;
            }
            if (elapsed_sec()> param.max_run_time)
                return 0;
        }
    }
    return 0;
}

/**
 * @brief Conflict::move_segments
 * Move the segments with color `c` to other colors
 * @param c
 */
void Conflict::move_segments(unsigned c)
{
    std::list<std::pair<int, int>> moving_segs; // list of pairs (segment, color)
    for (int si : classes.at(c))
    {
        bool is_moved = false;
        for (unsigned c2 = 0; c2 < classes.size() && !is_moved; c2++)
        {
            if (c != c2 && edge_can_be_added_to_graph(si, c2))
            {
                moving_segs.push_back(std::make_pair(si, c2));
//                std::clog << "s_" << si << ": c_" << c << " -> c_" << c2 << std::endl;
                is_moved = true;
            }
        }
    }
    for (const auto &pair : moving_segs)
    {
        classes.at(c).remove(pair.first);
        classes.at(pair.second).push_back(pair.first);
    }
}


/**
 * @brief Conflict::dfsOptimize
 * Try a limited Depth First Search in the tree of possibilities in order to
 * add a segment (or several during recursive calls)
 * to a color without having to queue any other edges.
 * If we failed, the solution remains unchanged, otherwise the colors are changed
 * @param todo List of segments that need to find a color
 * @param forbidden List of segments that we cannot intersect and move to the recursive todo list
 * @param breadth Maximum number of intersection within a color allowed
 * @param depth Maximum depth in the tree of possibility to reach
 * @param changes Stack of all the changes that have been made to reach the current state of the solution
 */
int Conflict::dfsOptimize(std::list<int> &todo, std::list<int> &forbidden, int breadth, int depth, std::list<stack_event_t> &changes)
{
    std::list<int> copy_forbidden = forbidden;

    if (todo.size() == 0)
        return 0;

    if (depth == 1)
        breadth = 0;

    // All edges in todo must be colored
    for (int e : todo)
    {
        std::list<std::pair<int, std::list<int>>> conflict_colors; // list of pairs (color, list of segments)
        get_colors_with_small_conflict_number(e, breadth, forbidden, conflict_colors);
        bool has_been_breaked = false;
        for (const auto &conflicts_c : conflict_colors)
        {
            classes.at(conflicts_c.first).push_back(e); // @todo Loic did a push_front

            remove_edges_from_color(conflicts_c.first, conflicts_c.second);
            std::list<int> subtodo = conflicts_c.second;

            std::list<stack_event_t> status;
            copy_forbidden.push_back(e);
            int ret = dfsOptimize(subtodo, copy_forbidden, breadth, depth - 1, status);
            copy_forbidden.pop_back();

            if (ret == -1)
            {
                std::list<int> edge_to_remove;
                edge_to_remove.push_back(e);
                remove_edges_from_color(conflicts_c.first, edge_to_remove);
                add_edges_to_color(conflicts_c.first, conflicts_c.second);
            }
            else
            {
                stack_event_t change;
                change.was_it_removed = false;
                change.was_it_added = true;
                change.was_added_to = conflicts_c.first;
                change.edge = e;
                changes.push_back(change);

                for (int cf: conflicts_c.second) {
                    stack_event_t change;
                    change.was_it_removed = true;
                    change.was_it_added = false;
                    change.was_removed_from = conflicts_c.first;
                    change.edge = cf;
                    changes.push_back(change);
                }
                for (stack_event_t &st: status) {
                    changes.push_back(st);
                }
                has_been_breaked = true;
                break;
            }
        }
        // No possibility, abort
        if (!has_been_breaked)
        {
            undo_changes(changes);
            return -1;
        }
    }
    return 0;
}


/**
 * @brief Conflict::get_colors_with_small_conflict_number
 * Return all the colors that have a number of conflict less or equal than breadth.
 * The colors are returned in conflict_colors, that stores, the color, and the
 * list of segments that intersects
 * @param e The segment
 * @param breadth Max number of conflicts
 * @param forbidden List of forbidden edges, we can't intersect those
 * @param conflict_colors List of the colors that cross e less than breadth time
 */
void Conflict::get_colors_with_small_conflict_number(int e, int breadth, std::list<int> &forbidden, std::list<std::pair<int, std::list<int> > > &conflict_colors)
{
    for (unsigned c = 0; c < classes.size(); c++)
    {
        std::list<int> intersectors;
        if (get_intersectors(c, e, intersectors, breadth, forbidden) != -1)
            conflict_colors.push_back(std::make_pair(c, intersectors));
    }
}

/**
 * @brief Conflict::get_intersectors
 * Compute the list of edge intersecting e in color.
 * @param c The color
 * @param e The segment
 * @param intersectors The list of edges from color that intersect e
 * @param max_intersections Maximum number of allowed intersections before
 * stopping computation and returning -1
 * @param forbidden List of forbidden edges that we are not allowed to intersect
 * @return -1 if there is more than max_intersections intersections or
 * if at least 1 forbidden edge is intersected by e in color. 0 otherwise
 */
int Conflict::get_intersectors(int c, int e, std::list<int> &intersectors, int max_intersections, std::list<int> &forbidden)
{
    int count = 0;
    for (int si : classes.at(c))
    {
        if (crosses(e, si))
        {
            count++;
            if (count > max_intersections)
                return -1;
            if (in_list(si, forbidden))
                return -1;
            intersectors.push_back(si);
        }
    }
    return 0;
}

/**
 * @brief Conflict::in_list
 * @param i
 * @param l
 * @return True if `i` appears in `l`
 */
bool Conflict::in_list(int i, const std::list<int> &l) const
{
    for (int j : l)
        if (i == j)
            return true;
    return false;
}

/**
 * @brief Conflict::remove_edges_from_color
 * Remove all the edges in conflicts from color
 * @param color
 * @param conflicts
 * @return
 * @todo I SHOULD BETTER USE A SET
 */
void Conflict::remove_edges_from_color(int color, const std::list<int> &conflicts)
{
    for (int seg :conflicts)
        classes.at(color).remove(seg);
}


int Conflict::add_edges_to_color(int color, const std::list<int> &conflicts)
{
    for (int si : conflicts) {
        classes.at(color).push_back(si);
        // std::clog << "Pushing: " << si << std::endl;
    }
    return 0;
}

/**
 * @brief Conflict::undo_changes
 * Undo all stacked events and pop them out of the stack
 * @param changes
 * @return
 */
int Conflict::undo_changes(std::list<stack_event_t> &changes)
{
    while (changes.size() != 0) {
        undo_change(changes.back());
        changes.pop_back();
    }
    return 0;
}

int Conflict::undo_change(stack_event_t &evt)
{
    if (evt.was_it_added) {
        //cout << "was added" << endl;
        undo_added(evt);
    } else {
        //cout << "was removed" << endl;
        undo_removed(evt);
    }
    return 0;
}



int Conflict::undo_added(stack_event_t &evt)
{
    for (auto it = classes.at(evt.was_added_to).begin(); it != classes.at(evt.was_added_to).end(); ++it)
        if (*it == evt.edge)
        {
            classes.at(evt.was_added_to).erase(it);
            return 0;
        }
    return -1;
}


int Conflict::undo_removed(stack_event_t &evt)
{
    //we have to add it back
    classes.at(evt.was_removed_from).push_back(evt.edge);
    return 0;
}

/**
 * @brief Conflict::build_colorv
 * Build the solution
 */
void Conflict::build_colorv()
{
    for (unsigned int c = 0; c < classes.size(); c++)
        for (int vi : classes[c])
            colorv[vi] = c;
}


int Conflict::add_data_point_to_graph_file()
{
    std::pair<double, int> data_point(elapsed_sec(), classes.size());
    data_points.push_back(data_point);

    std::string info = (param.info_name.empty()) ? "1" : "0";

    std::string fn;
    if (param.loop == 0)
    {
        if (param.solution_name.empty())
        {
            fn = "./graphs/" + instance_id +
                "_info-" + info +
                "_power-" + std::to_string(param.power) +
                "_noise_mean-" + std::to_string(param.noise_mean) +
                "_noise_var-" + std::to_string(param.noise_var) +
                "_max_queue-" + std::to_string(param.max_queue) +
                "_max_run_time-" + std::to_string(param.max_run_time) +
                "_dfs-" + std::to_string(param.dfs) +
                "_easy-" + std::to_string(param.easy) +
                "_loop-" + std::to_string(param.loop) + ':' + std::to_string(param.loop_time);
        }
        else
        {
            fn = "./graphs/" + instance_id +
                "_solution-" + param.solution_name +
                "_info-" + info +
                "_power-" + std::to_string(param.power) +
                "_noise_mean-" + std::to_string(param.noise_mean) +
                "_noise_var-" + std::to_string(param.noise_var) +
                "_max_queue-" + std::to_string(param.max_queue) +
                "_max_run_time-" + std::to_string(param.max_run_time) +
                "_dfs-" + std::to_string(param.dfs) +
                "_easy-" + std::to_string(param.easy) +
                "_loop-" + std::to_string(param.loop) + ':' + std::to_string(param.loop_time);
        }
    } else {
        if (param.solution_name.empty()) {
            fn = "./graphs/" + instance_id +
                "_info-" + info +
                "_power-" + "loop" +
                "_noise_mean-" + std::to_string(param.noise_mean) +
                "_noise_var-" + std::to_string(param.noise_var) +
                "_max_queue-" + std::to_string(param.max_queue) +
                "_max_run_time-" + std::to_string(param.max_run_time) +
                "_dfs-" + std::to_string(param.dfs) +
                "_easy-" + std::to_string(param.easy) +
                "_loop-" + std::to_string(param.loop) + ':' + std::to_string(param.loop_time);
        } else {
            fn = "./graphs/" + instance_id +
                "_solution-" + param.solution_name +
                "_info-" + info +
                "_power-" + "loop" +
                "_noise_mean-" + std::to_string(param.noise_mean) +
                "_noise_var-" + std::to_string(param.noise_var) +
                "_max_queue-" + std::to_string(param.max_queue) +
                "_max_run_time-" + std::to_string(param.max_run_time) +
                "_dfs-" + std::to_string(param.dfs) +
                "_easy-" + std::to_string(param.easy) +
                "_loop-" + std::to_string(param.loop) + ':' + std::to_string(param.loop_time);
        }
    }

    std::ofstream file(fn);
    for (const auto &pr : data_points)
        file << pr.first << " " << pr.second << std::endl;

    file.close();
    return 0;
}

/**
 * @brief Conflict::best_color
 * Find the color with the smallest score. This is the color in which the edge
 * will be added by the conflict optimizer.
 * @param seg The segment that we want to insert in the solution
 * @param queue_count The number of times that each edge has already been
 * in the queue. (Used to compute the score)
 * @param best_c
 * @param conflicting_segs The chosen color the put the edge_to_place
 * @return False if all subgraphs have an intersection with an already been
 * queued edge
 * @todo We put the queue_count vector in the class
 */
bool Conflict::best_color(int seg, int &best_c, std::list<int> &conflicting_segs)
{
    double min_conflict = param.max_queue * n_segments;

    for (unsigned c = 0; c < classes.size(); c++)
    {
        double conflict_count = 0;
        std::list<int> candidate_edges;
        // some gaussian noise, but not stupidly low noise, or even worse: negative noise
        double noise = distribution(generator);
        while (noise < 0.001)
            noise = distribution(generator);
        double min_conflict_noised = min_conflict / noise;
        for (int si : classes.at(c))
        {
            if (crosses(seg, si))
            {
                if (queue_count[si] >= param.max_queue)
                {
                    //It s no good
                    conflict_count = min_conflict_noised + 1;
                    break;
                }
                else
                {
                    conflict_count = conflict_count + pow(queue_count[si], param.power) + 1;
                    if (conflict_count >= min_conflict_noised)
                        break;
                    candidate_edges.push_back(si);
                }
            }
        }

        if (conflict_count * noise < min_conflict)
        {
            min_conflict = conflict_count * noise;
            best_c = c;
            conflicting_segs.clear();
            for (int si : candidate_edges)
                conflicting_segs.push_back(si);
        }

    }
    return (min_conflict < param.max_queue * n_segments);
}


void Conflict::copy_sol(std::vector<std::list<int>> &s1, std::vector<std::list<int>> &s2)
{
    s2.clear();
    for (const auto &cur_class : s1)
        s2.push_back(cur_class);
}
