/*
 * kminrob.cpp
 *
 *  Created on: Feb 12, 2023
 *      Author: david
 */

#include "kminrob.h"
#include "kp.h"
#include "kat.h"

#include <cmath>

CompiledProblem *kminrob_compile(const Problem &problem,
                                 const std::string &objective, const int &robustness,
                                 const bool add_cost_function, int acquisition_budget) {
    CompiledProblem *compiled_problem;
    deque<Action *> actions, tmp_actions;
    deque<pair<Formula *, string>> sensors_model, invariants_model;
    Formula *effect;
    Action *action;
    bool state;
    std::deque<Action *> exe_actions, as_actions, ram_actions, ka_actions;

    auto predicates_map = compile_predicates_map(problem.getPredicatesMap());
    auto *ka_predicate = new Predicate("ka_mode",
                                       true);  // Predicate indicate whether the actor getting additional information
    predicates_map["ka_mode"] = ka_predicate;

    Formula ka_formula(deque<Formula *>(), deque<Parameter *>(), "ka_mode", true, ka_predicate);

    map<string, bool> facts_evaluation = calculate_fact_evaluation(problem, predicates_map);
    facts_evaluation.at("ka_mode") = true;

    auto fo_facts_evaluation = map<string, bool>(facts_evaluation);
    for (const auto &fact: problem.getHidden()) {
        state = fact->getState();
        fo_facts_evaluation.at(fact_to_predicate_name(fact, state)) = true;
    }

    std::deque<Budget *> budgets;

    auto *goal_action = create_goal_action(problem, predicates_map);
    goal_action->setActionType(ActionType::exe_action);
    actions.push_back(goal_action);
    deque<string> goal = {goal_action->getEffect()->getFormulaType()};
    goal.shrink_to_fit();

    auto old_actions = problem.getActions();
    for (auto action_it = old_actions.begin(); action_it != old_actions.end();) {
        action = *action_it;
        tmp_actions = compile_action(action, predicates_map, facts_evaluation);
        if (tmp_actions.empty()) {
            action_it = old_actions.erase(action_it);
            effect = action->getEffect();
            for (const auto &predicate: effect->getRelatedPredicates()) {
                predicate->setIsChangeable(false);
            }
            for (auto &running_action: old_actions) {
                effect = running_action->getEffect();
                for (const auto &predicate: effect->getRelatedPredicates()) {
                    predicate->setIsChangeable(true);
                }
            }
        } else {
            for (auto &execution_action: tmp_actions) {
                execution_action->add_precondition(new Formula(ka_formula));
                exe_actions.push_back(execution_action);
            }
            ++action_it;
        }
    }
    exe_actions.shrink_to_fit();

    Parameter *budget_parameter;
    Formula *budget_formula;

    bool add_assumption_budget = robustness >= 0;
    string budget_name = "assumptions_budget";
    auto *budget = new Budget(budget_name, robustness);
    auto budget_type = budget->getBudgetType();
    budget_name = budget->getBudgetName();

    for (const auto *sensor: problem.getSensors()) {
        auto tmp = compile_sensor(sensor, predicates_map, fo_facts_evaluation, facts_evaluation);
        sensors_model.insert(sensors_model.end(), tmp.second.begin(), tmp.second.end());
        for (auto &assumption_action: tmp.first) {
            assumption_action->add_precondition(new Formula(ka_formula));
            if (add_assumption_budget) {
                budget_parameter = new Parameter(budget_type, {}, "?b");
                assumption_action->add_parameter(budget_parameter);
                budget_formula = new Formula({}, {budget_parameter}, budget_name, true, nullptr);
                assumption_action->getPrecondition()->add_inner_formula(budget_formula);
                budget_formula = new Formula(*budget_formula);
                budget_formula->change_is_negative();
                assumption_action->getEffect()->add_inner_formula(budget_formula);
            }
            as_actions.push_back(assumption_action);
        }
    }
    as_actions.shrink_to_fit();
    if (add_assumption_budget) {
        budgets.push_back(budget);
    } else {
        delete budget;
    }

    for (auto invariant: problem.getInvariants()) {
        auto tmp = compile_invariant(invariant, predicates_map);
        invariants_model.insert(invariants_model.end(), tmp.second.begin(), tmp.second.end());
        for (auto &inference_action: tmp.first) {
            inference_action->add_precondition(new Formula(ka_formula));
            ram_actions.push_back(inference_action);
        }
    }
    invariants_model.shrink_to_fit();
    ram_actions.shrink_to_fit();

    ka_actions = create_ka_actions(problem.getHelper(), predicates_map);

    if (acquisition_budget >= 0) {
        string ka_budget_name = "ka_budget";
        auto *ka_budget = new Budget(ka_budget_name, acquisition_budget);
        Parameter *ka_budget_parameter;
        Formula *ka_budget_formula;
        auto ka_budget_type = ka_budget->getBudgetType();
        budgets.push_back(ka_budget);
        for (auto &ka_action: ka_actions) {
            if (ka_action->getName() != "switch_from_ka_mode") {
                ka_budget_parameter = new Parameter(ka_budget_type, {}, "?b");
                ka_action->add_parameter(ka_budget_parameter);
                ka_budget_formula = new Formula({}, {ka_budget_parameter}, ka_budget_name, true, nullptr);
                ka_action->getPrecondition()->add_inner_formula(ka_budget_formula);
                ka_budget_formula = new Formula(*ka_budget_formula);
                ka_budget_formula->change_is_negative();
                ka_action->getEffect()->add_inner_formula(ka_budget_formula);
            }
        }
    }

    deque<Function *> functions;

    if (add_cost_function) {
        auto *cost_function = new Function("total-cost", deque<PDDL_Object *>(), 0);
        cost_function->setFunctionMetric("minimize");
        functions.push_back(cost_function);
        functions.shrink_to_fit();
        size_t ka_cost = 1, exe_cost = 1, as_cost = 1, ram_cost = 0;
        size_t as_size = as_actions.size(), ram_size = ram_actions.size(), exe_size = exe_actions.size(), ka_size = ka_actions.size();

        if (objective == "min-ka") {
            ka_cost = exe_cost * exe_size + as_size + ram_size + 1;
        } else if (objective == "min-ka-exe") {
            exe_cost = as_size + ram_size + 1;
            ka_cost = as_size + ram_size + (exe_cost * exe_size) + 1;
        } else if (objective == "min-exe-ka") {
            ka_cost = as_size + ram_size + 1;
            exe_cost = as_size + ram_size + (ka_cost * ka_size) + 1;
        } else if (objective == "min-ram-ka") {
            ka_cost = as_size + exe_size + 1;
            ram_cost = as_size + exe_size + (ka_cost * ka_size) + 1;
        } else if (objective == "min-as-ka") {
            ka_cost = exe_size + ram_size + 1;
            as_cost = exe_size + ram_size + (ka_cost * ka_size) + 1;
        }

        FunctionFormula cost_formula(cost_function, deque<Parameter *>(), ka_cost);

        for (auto &ka_action: ka_actions) {
            ka_action->getEffect()->add_inner_function_formula(new FunctionFormula(cost_formula));
            actions.push_back(ka_action);
        }
        cost_formula = FunctionFormula(cost_function, deque<Parameter *>(), exe_cost);
        for (auto &exe_action: exe_actions) {
            exe_action->getEffect()->add_inner_function_formula(new FunctionFormula(cost_formula));
            actions.push_back(exe_action);
        }
        cost_formula = FunctionFormula(cost_function, deque<Parameter *>(), as_cost);
        for (auto &as_action: as_actions) {
            as_action->getEffect()->add_inner_function_formula(new FunctionFormula(cost_formula));
            actions.push_back(as_action);
        }
        cost_formula = FunctionFormula(cost_function, deque<Parameter *>(), ram_cost);
        for (auto &ram_action: ram_actions) {
            ram_action->getEffect()->add_inner_function_formula(new FunctionFormula(cost_formula));
            actions.push_back(ram_action);
        }
    } else {
        actions.insert(actions.end(), exe_actions.begin(), exe_actions.end());
        actions.insert(actions.end(), as_actions.begin(), as_actions.end());
        actions.insert(actions.end(), ram_actions.begin(), ram_actions.end());
        actions.insert(actions.end(), ka_actions.begin(), ka_actions.end());
    }
    actions.shrink_to_fit();
    budgets.shrink_to_fit();

    const string &problemName = problem.getProblemName();
    const string &domainName = problem.getDomainName();
    const deque<string> &requirements = problem.getRequirements();

    compiled_problem = new CompiledProblem(problemName, domainName, requirements, predicates_map, sensors_model,
                                           invariants_model, facts_evaluation, goal, actions, functions, budgets);

    return compiled_problem;
}
