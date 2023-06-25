/*
 * FastForward.h
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#ifndef PLANNERS_FASTFORWARD_H_
#define PLANNERS_FASTFORWARD_H_

#include "Planner.h"

class FastForward : public Planner {
private:
    const string executable_path, configuration;
protected:
    std::deque<string> parse_planner_plan(const string &plan_path) override;

public:
    FastForward(const string &executablePath, string planner_config);

    ~FastForward() override = default;

    std::deque<string>
    solve(const string &destination_path, const string &domain_path, const string &problem_path) override;
};

#endif /* PLANNERS_FASTFORWARD_H_ */
