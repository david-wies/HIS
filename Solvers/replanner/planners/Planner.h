/*
 * Planner.h
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#ifndef PLANNERS_PLANNER_H_
#define PLANNERS_PLANNER_H_

#include <string>
#include <deque>
#include <utility>

using std::string;

class Planner {
protected:
    explicit Planner(string name) : name(std::move(name)) {}

    virtual std::deque<string> parse_planner_plan(const string &plan_path) = 0;

    const string name;
public:
    virtual ~Planner() = default;

    virtual std::deque<string>
    solve(const string &destination_path, const string &domain_path, const string &problem_path) = 0;

    string getName() const {
        return name;
    }
};

#endif /* PLANNERS_PLANNER_H_ */
