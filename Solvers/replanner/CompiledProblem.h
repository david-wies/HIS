/*
 * CompiledProblem.h
 *
 *  Created on: Feb 6, 2023
 *      Author: david
 */

#ifndef COMPILEDPROBLEM_H_
#define COMPILEDPROBLEM_H_

#include "problem/Action.h"
#include "problem/Predicate.h"
#include "problem/Budget.h"
#include "problem/Function.h"

#include <deque>

using namespace std;

enum FormulaType {
    always_true, always_false, changeable
};

class CompiledProblem {
private:
    const std::string problem_name, domain_name;
    std::deque<std::string> requirements;
    std::map<std::string, Predicate *> predicates_map;
    std::deque<std::pair<Formula *, std::string>> sensors_model, invariants_model;
    std::map<std::string, bool> facts_evaluations;
    std::map<std::string, Action *> actions_map;
    std::deque<std::string> goal;
    std::map<std::string, Function *> functions_map;
    std::map<string, Budget *> budget_map;

    void sense();

    bool does_formula_hold(const Formula *formula);

    void domain_to_pddl(const string &destination, const string &domainName);

    void problem_to_pddl(const string &destination, const string &problemName);

    void remove_unnecessary_inner_formulas(Formula *formula);

    FormulaType process_formula(Formula *formula);

    void apply_effect(const Formula *effect);

public:
    CompiledProblem(string problem_name, string domain_name, deque<string> requirements,
                    map<string, Predicate *> predicates_map,
                    deque<pair<Formula *, string>> sensors_model, deque<pair<Formula *, string>> invariants_model,
                    map<string, bool> facts_evaluations, deque<string> goal, const deque<Action *> &actions);

    CompiledProblem(string problem_name, string domain_name, deque<string> requirements,
                    map<string, Predicate *> predicates_map,
                    deque<pair<Formula *, string>> sensors_model, deque<pair<Formula *, string>> invariants_model,
                    map<string, bool> facts_evaluations, deque<string> goal, const deque<Action *> &actions,
                    const std::deque<Function *> &functions);

    CompiledProblem(string problem_name, string domain_name, deque<string> requirements,
                    map<string, Predicate *> predicates_map,
                    deque<pair<Formula *, string>> sensors_model, deque<pair<Formula *, string>> invariants_model,
                    map<string, bool> facts_evaluations, deque<string> goal, const deque<Action *> &actions,
                    const std::deque<Function *> &functions,
                    const std::deque<Budget *> &budgets);

    virtual ~CompiledProblem();

    void enter_state();

    bool act(const string &action_name);

    void deduce();

    bool is_goal_reached() const;

    deque<string> get_state() const;

    void optimize();

    void to_pddl(const string &destination, const string &problemName, const string &domainName);

    const Action *get_action(const std::string &action_name) const;

    ActionType get_action_type(const string &action_name) const;
};

#endif /* COMPILEDPROBLEM_H_ */
