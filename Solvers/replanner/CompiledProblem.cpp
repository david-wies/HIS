/*
 * CompiledProblem.cpp
 *
 *  Created on: Feb 6, 2023
 *      Author: david
 */

#include "CompiledProblem.h"
#include "utils.h"

#include <iostream>
#include <fstream>

CompiledProblem::CompiledProblem(string problem_name, string domain_name, deque<string> requirements,
                                 map<string, Predicate *> predicates_map, deque<pair<Formula *, string> > sensors_model,
                                 deque<pair<Formula *, string> > invariants_model, map<string, bool> facts_evaluations,
                                 deque<string> goal, const deque<Action *> &actions) : problem_name(
        std::move(problem_name)), domain_name(std::move(domain_name)), requirements(
        deque<string>(std::move(requirements))), predicates_map(map<string, Predicate *>(std::move(predicates_map))),
                                                                                       sensors_model(
                                                                                               deque<pair<Formula *, string>>(
                                                                                                       std::move(
                                                                                                               sensors_model))),
                                                                                       invariants_model(
                                                                                               deque<pair<Formula *, string>>(
                                                                                                       std::move(
                                                                                                               invariants_model))),
                                                                                       facts_evaluations(
                                                                                               map<string, bool>(
                                                                                                       std::move(
                                                                                                               facts_evaluations))),
                                                                                       goal(deque<string>(
                                                                                               std::move(goal))) {
    std::string name;
    for (Action *action: actions) {
        name = boost::to_lower_copy(action->getName());
        actions_map[name] = action;
    }
}

CompiledProblem::CompiledProblem(string problem_name, string domain_name,
                                 deque<string> requirements, map<string, Predicate *> predicates_map,
                                 deque<pair<Formula *, string> > sensors_model,
                                 deque<pair<Formula *, string> > invariants_model,
                                 map<string, bool> facts_evaluations, deque<string> goal,
                                 const deque<Action *> &actions, const std::deque<Function *> &functions)
        : CompiledProblem(std::move(problem_name),
                          std::move(domain_name),
                          std::move(requirements),
                          std::move(predicates_map),
                          std::move(sensors_model),
                          std::move(invariants_model),
                          std::move(facts_evaluations),
                          std::move(goal), actions) {
    std::string name;
    for (Function *function: functions) {
        name = boost::to_lower_copy(function->getName());
        functions_map[name] = function;
    }
}

CompiledProblem::CompiledProblem(string problem_name, string domain_name,
                                 deque<string> requirements, map<string, Predicate *> predicates_map,
                                 deque<pair<Formula *, string> > sensors_model,
                                 deque<pair<Formula *, string> > invariants_model,
                                 map<string, bool> facts_evaluations, deque<string> goal,
                                 const deque<Action *> &actions, const std::deque<Function *> &functions,
                                 const std::deque<Budget *> &budgets) : CompiledProblem(std::move(problem_name),
                                                                                        std::move(domain_name),
                                                                                        std::move(requirements),
                                                                                        std::move(predicates_map),
                                                                                        std::move(sensors_model),
                                                                                        std::move(invariants_model),
                                                                                        std::move(facts_evaluations),
                                                                                        std::move(goal), actions,
                                                                                        functions) {
    for (auto budget: budgets) {
        budget_map[budget->getBudgetName()] = budget;
    }
}

CompiledProblem::~CompiledProblem() {
    for (const auto &predicate: predicates_map) {
        delete predicate.second;
    }
    predicates_map.clear();

    for (const auto &action: actions_map) {
        delete action.second;
    }
    actions_map.clear();
    
    for (const auto &function: functions_map) {
        delete function.second;
    }
    functions_map.clear();
    
    for (auto &budget_it: budget_map) {
        delete budget_it.second;
    }
    budget_map.clear();

    requirements.clear();

    facts_evaluations.clear();

    goal.clear();

    for (auto &sensor: sensors_model) {
        delete sensor.first;
    }
    sensors_model.clear();

    for (const auto &invariant: invariants_model) {
        delete invariant.first;
    }
    invariants_model.clear();
}

void CompiledProblem::deduce() {
    bool had_changed;
    do {
        had_changed = false;
        for (auto invariant = invariants_model.begin(); invariant != invariants_model.end();) {
            if (does_formula_hold(invariant->first)) {
                facts_evaluations.at(invariant->second) = true;
                delete invariant->first;
                invariant = invariants_model.erase(invariant);
                had_changed = true;
            } else {
                ++invariant;
            }
        }
    } while (had_changed);
}

void CompiledProblem::enter_state() {
    sense();
    deduce();
}

bool CompiledProblem::act(const string &action_name) {
    Action *action = actions_map.at(action_name);
    if (does_formula_hold(action->getPrecondition())) {
        auto effect = action->getEffect();
        apply_effect(effect);
        return true;
    }
    return false;
}

bool CompiledProblem::is_goal_reached() const {
    for (const auto &fact: goal) {
        if (!facts_evaluations.at(fact))
            return false;
    }
    return true;
}

deque<string> CompiledProblem::get_state() const {
    std::deque<std::string> state;
    for (const auto &fact: facts_evaluations) {
        if (fact.second) {
            state.push_back(fact.first);
        }
    }
    return state;
}

void CompiledProblem::optimize() {
    std::cout << "Starting optimize process" << std::endl;
    const clock_t begin_time = clock();
    Action *action;
    FormulaType type;
    Formula *formula;
    bool is_changed, had_erased;
    size_t rounds = -1;
    do {
        rounds++;
        for (auto &predicate_iter: predicates_map) {
            predicate_iter.second->setIsChangeable(false);
        }
        for (auto &action_iter: actions_map) {
            formula = action_iter.second->getEffect();
            for (auto predicate: formula->getRelatedPredicates()) {
                predicate->setIsChangeable(true);
            }
        }
        is_changed = false;
        for (auto it = actions_map.begin(); it != actions_map.end();) {
            had_erased = false;
            action = it->second;
            formula = action->getPrecondition();
            if (formula == nullptr) {
                ++it;
                continue;
            }
            type = process_formula(formula);
            if (type == always_true) {
                action->setPrecondition(nullptr);
                delete formula;
                is_changed = true;
            } else if (type == always_false) {
                delete action;
                it = actions_map.erase(it);
                had_erased = true;
                is_changed = true;
            }
            if (!had_erased)
                ++it;
        }
    } while (is_changed);

    for (auto it = sensors_model.begin(); it != sensors_model.end();) {
        formula = it->first;
        type = process_formula(formula);
        if (type == always_false) {
            delete formula;
            it = sensors_model.erase(it);
        } else
            ++it;
    }

    for (auto it = invariants_model.begin(); it != invariants_model.end();) {
        formula = it->first;
        type = process_formula(formula);
        if (type == changeable) {}
        if (type == always_false) {
            delete formula;
            it = invariants_model.erase(it);
        } else {
            if (type == always_true) {
                it->first = nullptr;
            }
            ++it;
        }
    }

    std::map<std::string, bool> is_useable_predicate;
    for (auto &iter: predicates_map) {
        is_useable_predicate[iter.first] = false;
    }
    for (auto &action_iter: actions_map) {
        auto precondition = action_iter.second->getPrecondition();
        if (precondition != nullptr) {
            for (auto &predicate_iter: precondition->getRelatedPredicates()) {
                is_useable_predicate.at(predicate_iter->getName()) = true;
            }
        }
        for (auto predicate_iter: action_iter.second->getEffect()->getRelatedPredicates()) {
            is_useable_predicate.at(predicate_iter->getName()) = true;
        }
    }
    for (const auto &fact: goal) {
        is_useable_predicate.at(fact) = true;
    }
    for (auto it = predicates_map.begin(); it != predicates_map.end();) {
        if (!is_useable_predicate.at(it->first)) {
            delete it->second;
            it = predicates_map.erase(it);
        } else
            ++it;
    }
    for (auto it = facts_evaluations.begin(); it != facts_evaluations.end();) {
        is_useable_predicate.at(it->first) ? ++it : (it = facts_evaluations.erase(it));
    }
    for (auto it = budget_map.begin(); it != budget_map.end();) {
        if (it->second->does_reached_maximum()) {
            delete it->second;
            it = budget_map.erase(it);
        } else
            ++it;
    }
    std::cout << "Optimizing took " << float(clock() - begin_time) / CLOCKS_PER_SEC << " seconds." << std::endl;
}

void CompiledProblem::to_pddl(const string &destination,
                              const string &problemName, const string &domainName) {
    domain_to_pddl(destination, domainName);
    problem_to_pddl(destination, problemName);
}

void CompiledProblem::sense() {
    for (auto sensor = sensors_model.begin(); sensor != sensors_model.end();) {
        if (does_formula_hold(sensor->first)) {
            facts_evaluations.at(sensor->second) = true;
            delete sensor->first;
            sensor = sensors_model.erase(sensor);
        } else {
            ++sensor;
        }
    }
}

bool CompiledProblem::does_formula_hold(const Formula *formula) {
    if (formula == nullptr) {
        return true;
    }
    const auto &inner = formula->getInnerFormulas();
    const std::string &formulaType = formula->getFormulaType();
    auto budget_it = budget_map.find(formulaType);
    if (budget_it != budget_map.end()) {
        return !budget_it->second->does_reached_maximum();
    }
    if (inner.empty()) {
        return facts_evaluations.at(formulaType) != formula->isNegative();
    } else {
        if (formulaType == "or") {
            for (Formula *inner_formula: inner) {
                if (does_formula_hold(inner_formula)) {
                    return true;
                }
            }
            return false;
        } else if (formulaType == "and") {
            for (Formula *inner_formula: inner) {
                if (!does_formula_hold(inner_formula)) {
                    return false;
                }
            }
            return true;
        } else if ((formulaType == "when") || (formulaType == "imply")) {
            return (!does_formula_hold(inner.at(0))) || (does_formula_hold(inner.at(1)));
        }
    }
    return false;
}

void CompiledProblem::remove_unnecessary_inner_formulas(Formula *formula) {
    auto &inner = formula->getInnerFormulas();
    if (inner.empty()) {
        return;
    }
    const std::string &formulaType = formula->getFormulaType();
    FormulaType inner_formula_type;
    if (formulaType == "or") {
        bool is_always_false = true;
        for (Formula *inner_formula: inner) {
            inner_formula_type = process_formula(inner_formula);
            if (inner_formula_type == always_true) {
                formula->setFormulaType("true");
                formula->clear_inner_formula();
                return;
            } else if (inner_formula_type != always_false) {
                is_always_false = false;
            } else {
                formula->remove_inner_formula(inner_formula);
            }
        }
        if (is_always_false) {
            formula->setFormulaType("false");
            formula->clear_inner_formula();
        }
    } else if (formulaType == "and") {
        bool is_always_true = true;
        for (Formula *inner_formula: inner) {
            inner_formula_type = process_formula(inner_formula);
            if (inner_formula_type == always_false) {
                formula->setFormulaType("false");
                formula->clear_inner_formula();
                return;
            } else if (inner_formula_type != always_true) {
                is_always_true = false;
            } else {
                formula->remove_inner_formula(inner_formula);
            }
        }
        if (is_always_true) {
            formula->setFormulaType("true");
            formula->clear_inner_formula();
        }
    }
}

FormulaType CompiledProblem::process_formula(Formula *formula) {
    if (formula == nullptr) {
        return always_true;
    }
    remove_unnecessary_inner_formulas(formula);
    auto inners = formula->getInnerFormulas();
    auto formula_type = formula->getFormulaType();
    if (inners.empty()) {
        if (formula_type == "true") {
            return always_true;
        } else if (formula_type == "false") {
            return always_false;
        } else {
            auto budget_it = budget_map.find(formula_type);
            if (budget_it != budget_map.end()) {
                return budget_it->second->does_reached_maximum() ? always_false : changeable;
            } else if (!formula->getParameters().empty()) { // todo change to go over all options
                return changeable;
            } else {
                auto predicate = predicates_map.at(formula_type);
                if (!predicate->isChangeable()) {
                    return facts_evaluations.at(formula_type) != formula->isNegative() ? always_true : always_false;
                }
            }
        }
    }

    return changeable;
}

void CompiledProblem::apply_effect(const Formula *effect) {
    std::string function_name;
    Function *function;
    auto &inners = effect->getInnerFormulas();
    auto &formulaType = effect->getFormulaType();
    if (formulaType == "and") {
        for (auto inner: inners) {
            apply_effect(inner);
        }
    } else if (formulaType == "when") {
        if (does_formula_hold(inners.at(0))) {
            apply_effect(inners.at(1));
        }
    } else {
        auto budget_it = budget_map.find(formulaType);
        if (budget_it == budget_map.end()) {
            facts_evaluations.at(formulaType) = !effect->isNegative();
        } else {
            budget_it->second->decrease();
        }
    }
    for (auto &function_formula: effect->getInnerFunctionFormulas()) {
        function_name = function_formula->getFunction()->getName();
        function = functions_map.at(function_name);
        function->increase(function_formula->getValue());
    }
}

void CompiledProblem::domain_to_pddl(const string &destination,
                                     const string &domainName) {
    std::ofstream os(join_path({destination, domainName}));
    os << "(define (domain " << domain_name << ")\n\n(:requirements";
    for (const std::string &requirement: requirements) {
        os << ' ' << requirement;
    }

    if (!budget_map.empty()) {
        os << ")\n\n(:types\n";
        for (const auto &budget: budget_map) {
            os << '\t' << *budget.second->getBudgetType() << '\n';
        }
    }

    os << ")\n\n(:predicates" << std::endl;
    for (auto &predicate_it: predicates_map) {
        os << '\t' << *predicate_it.second << '\n';
    }
    for (const auto &budget: budget_map) {
        os << '\t' << budget.second->get_predicate() << '\n';
    }
    os << ")\n" << std::endl;

    if (!functions_map.empty()) {
        os << "\n(:functions";
        for (const auto &function: functions_map) {
            os << ' ' << *function.second;
        }
        os << ")\n" << std::endl;
    }

    auto action_end = actions_map.end();
    for (auto action = actions_map.begin(); action != action_end; action++) {
        os << *action->second << '\n' << std::endl;
    }

    os << "\n)";
    os.close();
}

void CompiledProblem::problem_to_pddl(const string &destination,
                                      const string &problemName) {
    std::ofstream os(join_path({destination, problemName}));

    os << "(define (problem " << problem_name << ")\n\n(:domain " << domain_name << ")\n" << std::endl;

    if (!budget_map.empty()) {
        os << "(:objects\n";
        for (const auto &budget: budget_map) {
            os << '\t' << budget.second->get_objects() << '\n';
        }
        os << ")\n" << std::endl;
    }

    os << "(:init" << std::endl;
    for (const auto &fact: facts_evaluations) {
        if (fact.second) {
            os << "\t(" << fact.first << ")\n";
        }
    }
    if (!functions_map.empty()) {
        for (const auto &function: functions_map) {
            os << '\t';
            function.second->print_value(os);
            os << std::endl;
        }
    }

    os << ")\n\n(:goal (and";
    for (const auto &fact: goal) {
        os << " (" << fact << ')';
    }
    os << "))\n";

    if (!functions_map.empty()) {
        for (const auto &function: functions_map) {
            if (function.second->isHasMetric()) {
                os << '\n';
                function.second->print_metric(os);
                os << std::endl;
            }
        }
    }

    os << "\n)";
    os.close();
}

const Action *CompiledProblem::get_action(
        const std::string &action_name) const {
    string name = boost::to_lower_copy(action_name);
    auto iterator = actions_map.find(name);
    if (iterator != actions_map.end()) {
        return iterator->second;
    }
    return nullptr;
}

ActionType CompiledProblem::get_action_type(
        const string &action_name) const {
    ActionType action_type = ActionType::exe_action;
    auto action = get_action(action_name);
    if (action != nullptr) {
        action_type = action->getActionType();
    }
    return action_type;
}
