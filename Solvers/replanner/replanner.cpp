/*
 * replanner.cpp
 *
 *  Created on: Feb 8, 2023
 *      Author: david
 */

#include "replanner.h"
#include "utils.h"
#include <iostream>

void replanner(CompiledProblem *problem, Planner *planner,
               const string &tmp_path, const bool &keep_files, const bool &optimize,
               const bool &no_replan) {
    std::cout << "Starting replanning process" << std::endl;
    const clock_t begin_time = clock();
    std::deque<std::string> running_plan, executed_plan;
    const Action *action;
    Formula *preconditions_formula;
    std::deque<Formula *> preconditions;
    std::deque<const Action *> reduced_plan, assumptions, ka, exe_plan;
    auto id = std::to_string(getpid());
    short counter = 0;
    size_t i, n;
    std::string counter_str, problem_name, domain_name, problem_path, domain_path;
    // problem->deduce();
    problem->enter_state();
    std::cout << ">>>> initial state=";
    print_state(problem);
    auto basic_problem_name = "gen-p" + id + "-";
    auto basic_domain_name = "gen-d" + id + "-";
    bool replaned = false;
    ActionType action_type;
    while ((!problem->is_goal_reached()) && !(no_replan && replaned)) {
        counter_str = std::to_string(counter++);
        problem_name = basic_problem_name + counter_str + ".pddl";
        domain_name = basic_domain_name + counter_str + ".pddl";
        if (optimize) {
            problem->optimize();
        }
        problem->to_pddl(tmp_path, problem_name, domain_name);
        problem_path = join_path({tmp_path, problem_name});
        domain_path = join_path({tmp_path, domain_name});
        std::cout << "before calling planner" << endl;
        running_plan = planner->solve(tmp_path, domain_path, problem_path);
        std::cout << "after calling planner" << endl;
        replaned = true;
        reduced_plan.clear();
        if (!keep_files) {
            remove(domain_path.c_str());
            remove(problem_path.c_str());
        }
        if ((running_plan.empty()) || (running_plan.at(0) == "Failed")) {
            std::cout << "problem has no solution!" << endl;
            std::cout << "Replanning took " << float(clock() - begin_time) / CLOCKS_PER_SEC << " seconds." << std::endl;
            return;
        }
        for (const auto &step: running_plan) {
            action = problem->get_action(step);
            switch (action->getActionType()) {
                case ActionType::exe_action:
                    exe_plan.push_back(action);
                    reduced_plan.push_back(action);
                    break;
                case ActionType::as_action:
                    assumptions.push_back(action);
                    break;
                case ActionType::ka_action:
                    ka.push_back(action);
                    break;
                default:
                    exe_plan.push_back(action);
                    break;
            }
        }
        std::cout << "Classical plan (raw):" << endl;
        n = running_plan.size();
        for (i = 0; i < n; ++i) {
            std::cout << "\tstep " << i << ": ." << running_plan.at(i) << endl;
        }
        n = reduced_plan.size();
        std::cout << "Classical plan (reduced):" << endl;
        for (i = 0; i < n; ++i) {
            std::cout << "\tstep " << i << ": ." << reduced_plan.at(i)->getName() << endl;
        }
        reduced_plan.clear();
        std::cout << "Assumptions:" << endl;
        n = assumptions.size();
        for (i = 0; i < n; ++i) {
            std::cout << "\tstep " << i << ": ." << assumptions.at(i)->getName() << endl;
        }
        assumptions.clear();
        std::cout << "Knowledge Acquisition:" << endl;
        n = ka.size();
        for (i = 0; i < n; ++i) {
            std::cout << "\tstep " << i << ": ." << ka.at(i)->getName() << endl;
        }
        ka.clear();
        n = reduced_plan.size();
        std::cout << "Belief state:" << endl;
        for (i = 0; i < n; ++i) {
            action = reduced_plan.at(i);
            std::cout << "\tstep=" << i << ": ." << reduced_plan.at(i)->getName() << ": {";
            preconditions_formula = action->getPrecondition();
            if (preconditions_formula != nullptr) {
                const auto &formula_type = preconditions_formula->getFormulaType();
                if (formula_type == "and" || formula_type == "or") {
                    preconditions = preconditions_formula->getInnerFormulas();
                } else if (formula_type == "imply") {
                    // TODO decide what to print
                } else {
                    preconditions.push_back(preconditions_formula);
                }
            }
            if (!preconditions.empty()) {
                auto condition = preconditions.begin();
                auto end = --preconditions.end();
                for (; condition != end; ++condition) {
                    std::cout << **condition << ',';
                }
                std::cout << **condition;
                preconditions.clear();
            }
            std::cout << '}' << endl;
        }
        string actionType;
        for (const auto &step: running_plan) {
            action_type = problem->get_action_type(step);
            if ((action_type == ActionType::as_action) || (action_type == ActionType::ram_action)) {
                continue;
            }
            // problem->enter_state();
            if (problem->act(step)) {
                if (action_type == ActionType::exe_action) {
                    actionType = ">>> kp-action=";
                    executed_plan.push_back(step);
                } else {
                    actionType = ">>> kat-action=";
                }
                std::cout << actionType << step << endl;
                std::cout << ">>>> state=";
                print_state(problem);
                problem->enter_state();
            } else {
                break;
            }
        }
    }
    auto need_indent = false;
    std::cout << "PLAN:";
    auto plan_n = executed_plan.size();
    for (i = 0; i < plan_n; ++i) {
        if (need_indent) {
            std::cout << "    ";
            if (i < 10) {
                std::cout << ' ';
            }
        }
        std::cout << "     " << i << " : " << executed_plan.at(i) << endl;
        need_indent = true;
    }
    std::cout << "Replanning took " << float(clock() - begin_time) / CLOCKS_PER_SEC << " seconds." << std::endl;
}

void print_state(const CompiledProblem *problem) {
    const deque<std::string> &state = problem->get_state();
    if (state.empty()) {
        std::cout << "{}" << endl;
    } else {
        size_t n = state.size() - 1, i;
        std::cout << '{';
        for (i = 0; i < n; ++i) {
            std::cout << '(' << state.at(i) << "),";
        }
        std::cout << '(' << state.at(i) << ")}" << endl;
    }
}
