/*
 * FastDownward.h
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#ifndef PLANNERS_FASTDOWNWARD_H_
#define PLANNERS_FASTDOWNWARD_H_

#include "Planner.h"

class FastDownward : public Planner {
private:
    const string executable_path, search;

    static string get_planner_search(const string &planner_config);

protected:
    std::deque<string> parse_planner_plan(const string &plan_path) override;

public:
    FastDownward(const string &executable_path, const string &planner_config);

    ~FastDownward() override = default;

    std::deque<string>
    solve(const string &destination_path, const string &domain_path, const string &problem_path) override;
};

#endif /* PLANNERS_FASTDOWNWARD_H_ */
