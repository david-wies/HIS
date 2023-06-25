/*
 * kp.cpp
 *
 *  Created on: Feb 6, 2023
 *      Author: david
 */

#include "kp.h"
#include "../utils.h"

CompiledProblem *kp_compile(const Problem &problem) {
    return kp_compile(problem, -1);
}

CompiledProblem *kp_compile(const Problem &problem, const int &robustness) {
    CompiledProblem *compiled_problem;

    auto predicates_map = compile_predicates_map(problem.getPredicatesMap());
    deque<Action *> actions, tmp_actions;
    map<string, bool> facts_evaluation = calculate_fact_evaluation(problem, predicates_map);
    bool state;
    std::deque<Budget *> budgets;
    deque<Function *> functions;

    auto fo_facts_evaluation = map<string, bool>(facts_evaluation);
    for (const auto &fact: problem.getHidden()) {
        state = fact->getState();
        fo_facts_evaluation.at(fact_to_predicate_name(fact, state)) = true;
    }

    auto *goal_action = create_goal_action(problem, predicates_map);
    goal_action->setActionType(ActionType::exe_action);
    actions.push_back(goal_action);
    deque<string> goal = {goal_action->getEffect()->getFormulaType()};
    goal.shrink_to_fit();

    deque<pair<Formula *, string>> sensors_model, invariants_model;

    Formula *effect;
    Action *action;
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
            actions.insert(actions.end(), tmp_actions.begin(), tmp_actions.end());
            ++action_it;
        }
    }

    bool add_assumption_budget = robustness >= 0;
    string budget_name = "assumptions_budget";
    auto *budget = new Budget(budget_name, robustness);
    auto budget_type = budget->getBudgetType();
    budget_name = budget->getBudgetName();
    Parameter *budget_parameter;
    Formula *budget_formula;

    for (const auto *sensor: problem.getSensors()) {
        auto tmp = compile_sensor(sensor, predicates_map, fo_facts_evaluation, facts_evaluation);
        sensors_model.insert(sensors_model.end(), tmp.second.begin(), tmp.second.end());
        if (add_assumption_budget) {
            for (auto &assumption_action: tmp.first) {
                budget_parameter = new Parameter(budget_type, {}, "?b");
                assumption_action->add_parameter(budget_parameter);
                budget_formula = new Formula({}, {budget_parameter}, budget_name, true, nullptr);
                assumption_action->getPrecondition()->add_inner_formula(budget_formula);
                budget_formula = new Formula(*budget_formula);
                budget_formula->change_is_negative();
                assumption_action->getEffect()->add_inner_formula(budget_formula);
                actions.push_back(assumption_action);
            }
        } else {
            actions.insert(actions.end(), tmp.first.begin(), tmp.first.end());
        }
    }
    sensors_model.shrink_to_fit();
    if (add_assumption_budget) {
        budgets.push_back(budget);
    } else {
        delete budget;
    }

    for (auto invariant: problem.getInvariants()) {
        auto tmp = compile_invariant(invariant, predicates_map);
        invariants_model.insert(invariants_model.end(), tmp.second.begin(), tmp.second.end());
        actions.insert(actions.end(), tmp.first.begin(), tmp.first.end());
    }
    invariants_model.shrink_to_fit();

    actions.shrink_to_fit();

    const string &problemName = problem.getProblemName();
    const string &domainName = problem.getDomainName();
    const deque<string> &requirements = problem.getRequirements();
    budgets.shrink_to_fit();
    functions.shrink_to_fit();

    compiled_problem = new CompiledProblem(problemName, domainName, requirements, predicates_map, sensors_model,
                                           invariants_model, facts_evaluation, goal, actions, functions, budgets);

    return compiled_problem;
}

std::map<string, Predicate *> compile_predicates_map(
        const std::map<string, Predicate *> &old_predicates_map) {
    map<string, Predicate *> predicate_map;
    Predicate *predicate;
    string predicate_name, full_predicate_name;
    bool is_changeable;
    for (auto &old_predicate: old_predicates_map) {
        auto combinations = get_object_combinations(old_predicate.second->getVariables());
        is_changeable = old_predicate.second->isChangeable() || old_predicate.second->isObservable() ||
                        old_predicate.second->isDeducible();
        for (const auto &combination: combinations) {
            predicate_name = old_predicate.first + combination;
            full_predicate_name = "k_" + predicate_name;
            predicate = new Predicate(full_predicate_name, is_changeable);
            old_predicate.second->add_derived_predicate(predicate);
            predicate_map[full_predicate_name] = predicate;
            full_predicate_name = "k_not_" + predicate_name;
            predicate = new Predicate(full_predicate_name, is_changeable);
            old_predicate.second->add_derived_predicate(predicate);
            predicate_map[full_predicate_name] = predicate;
        }
    }
    predicate_map["new-goal"] = new Predicate("new-goal", true);
    return predicate_map;
}

deque<Action *> compile_action(const Action *action,
                               const map<string, Predicate *> &predicates_map,
                               const map<string, bool> &facts_evaluation) {
    deque<Action *> actions;
    Action *new_action;
    const deque<Parameter *> &parameters = action->getParameters();
    deque<map<Parameter *, PDDL_Object *>> combinations;
    if (parameters.empty()) {
        combinations.emplace_back();
    } else {
        combinations = get_objects_combination(parameters);
    }
    const string &action_name = action->getName();
    string new_action_name;
    const Formula *precondition = action->getPrecondition(), *effect = action->getEffect();
    map<string, Parameter *> parametersMap;
    Formula *new_precondition = nullptr, *new_effect;
    FormulaType formulaType;
    for (const auto &combination: combinations) {
        new_action_name = action_name;
        for (Parameter *parameter: parameters) {
            new_action_name += "_" + combination.at(parameter)->getName();
        }
        if (precondition != nullptr) {
            new_precondition = compile_formula(precondition, combination, predicates_map);
            formulaType = evaluateFormula(new_precondition, predicates_map, facts_evaluation);
            if (formulaType != changeable) {
                delete new_precondition;
                new_precondition = nullptr;
                if (formulaType == always_false)
                    continue;
            }
        }
        new_effect = compile_effect(effect, combination, predicates_map);
        new_action = new Action(new_action_name, parametersMap, new_precondition, new_effect);
        new_action->setActionType(ActionType::exe_action);
        actions.push_back(new_action);
    }
    return actions;
}

Formula *compile_effect(const Formula *old_effect,
                        const map<Parameter *, PDDL_Object *> &combination,
                        const map<string, Predicate *> &predicates_map) {
    Formula *effect;
    string formula_type = old_effect->getFormulaType();
    deque<const Formula *> effects;
    deque<Formula *> sub_effects = old_effect->getInnerFormulas(), new_effects;
    Formula *tmp_sub_effect, *new_sub_effect;
    bool is_negative = old_effect->isNegative();

    if (formula_type == "when") {
        auto effect_parts = old_effect->getInnerFormulas();
        Formula *old_condition = effect_parts.at(0), *old_cond_effect = effect_parts.at(1);
        Formula *new_condition, *new_cond_effect;
        new_condition = compile_formula(old_condition, combination, predicates_map);
        new_cond_effect = compile_formula(old_cond_effect, combination, predicates_map);
        new_sub_effect = new Formula({new_condition, new_cond_effect}, deque<Parameter *>(), "when", is_negative,
                                     nullptr);
        new_effects.push_back(new_sub_effect);
        old_condition->change_is_negative();
        new_condition = compile_formula(old_condition, combination, predicates_map);
        new_condition->change_is_negative();
        old_condition->change_is_negative();
        if (old_cond_effect->getFormulaType() == "and") {
            for (auto &inner: old_cond_effect->getInnerFormulas()) {
                inner->change_is_negative();
            }
            new_cond_effect = compile_formula(old_cond_effect, combination, predicates_map);
            for (auto &inner: new_cond_effect->getInnerFormulas()) {
                inner->change_is_negative();
            }
            for (auto &inner: old_cond_effect->getInnerFormulas()) {
                inner->change_is_negative();
            }
        } else {
            old_cond_effect->change_is_negative();
            new_cond_effect = compile_formula(old_cond_effect, combination, predicates_map);
            new_cond_effect->change_is_negative();
            old_cond_effect->change_is_negative();
        }
        new_sub_effect = new Formula({new_condition, new_cond_effect}, deque<Parameter *>(), "when", is_negative,
                                     nullptr);
        new_effects.push_back(new_sub_effect);
        auto *pFormula = new Formula(new_effects, deque<Parameter *>(), "and", false, nullptr);
        return pFormula;
    }

    if ((sub_effects.empty()))
        effects.push_back(old_effect);
    else {
        effects.insert(effects.begin(), sub_effects.begin(), sub_effects.end());
    }

    for (auto &current_effect: effects) {
        formula_type = current_effect->getFormulaType();
        if (formula_type == "forall") {
            map<Parameter *, PDDL_Object *> inner_combination(combination);
            Parameter *innerParameter = current_effect->getInnerParameter();
            auto &inner_formula = current_effect->getInnerFormulas().at(0);
            for (auto object: innerParameter->getCompatibleObjects()) {
                inner_combination[innerParameter] = object;
                effect = compile_effect(inner_formula, inner_combination, predicates_map);
                if ((effect->getFormulaType() == "and") && (effect->isNegative() == old_effect->isNegative())) {
                    for (auto &inner: effect->getInnerFormulas()) {
                        new_effects.push_back(new Formula(*inner));
                    }
                    delete effect;
                } else {
                    new_effects.push_back(effect);
                }
            }
        } else if (current_effect->getFormulaType() == "when") {
            new_sub_effect = compile_effect(current_effect, combination, predicates_map);
            if (new_sub_effect->getFormulaType() == "and") {
                for (auto &inner: new_sub_effect->getInnerFormulas()) {
                    auto new_inner = new Formula(*inner);
                    new_effects.push_back(new_inner);
                }
                delete new_sub_effect;
            } else {
                new_effects.push_back(new_sub_effect);
            }
        } else {
            new_sub_effect = compile_formula(current_effect, combination, predicates_map);
            new_effects.push_back(new_sub_effect);
            auto tmp = Formula(*current_effect);
            tmp.change_is_negative();
            tmp_sub_effect = compile_formula(&tmp, combination, predicates_map);
            tmp_sub_effect->change_is_negative();
            new_effects.push_back(tmp_sub_effect);
        }
    }
    if (new_effects.size() == 1) {
        effect = new_effects.at(0);
    } else {
        effect = new Formula(new_effects, deque<Parameter *>(), "and", false, nullptr);
    }
    return effect;
}

FormulaType evaluateFormula(Formula *formula,
                            const map<string, Predicate *> &predicates_map,
                            const map<string, bool> &facts_evaluation) {
    const string &formula_type = formula->getFormulaType();
    bool isNegative = formula->isNegative();
    const auto &inners = formula->getInnerFormulas();
    if (inners.empty()) {
        auto predicate = predicates_map.at(formula_type);
        bool isChangeable = predicate->isChangeable(), isDeducible = predicate->isDeducible(), isObservable = predicate->isObservable();
        if (!(isChangeable || isDeducible || isObservable)) {
            return facts_evaluation.at(formula_type) == isNegative ? always_false : always_true;
        }
    } else {
        FormulaType type;
        bool is_always_fixed = true;
        if (formula_type == "and") {
            for (Formula *inner_formula: inners) {
                type = evaluateFormula(inner_formula, predicates_map, facts_evaluation);
                if (type == changeable) {
                    is_always_fixed = false;
                } else if (type == always_false)
                    return always_false;
            }
            return is_always_fixed ? always_true : changeable;
        } else if (formula_type == "or") {
            for (Formula *inner_formula: inners) {
                type = evaluateFormula(inner_formula, predicates_map, facts_evaluation);
                if (type == changeable) {
                    is_always_fixed = false;
                } else if (type == always_true)
                    return always_true;
            }
            return is_always_fixed ? always_false : changeable;
        } else if ((formula_type == "imply") || (formula_type == "when")) {
            auto effect_type = evaluateFormula(inners.at(0), predicates_map, facts_evaluation);
            if (effect_type == always_true)
                return always_true;
            else {
                auto condition_type = evaluateFormula(inners.at(0), predicates_map, facts_evaluation);
                switch (condition_type) {
                    case always_true:
                        return effect_type;
                    case always_false:
                        return always_true;
                    default:
                        break;
                }
            }
        }
    }
    return changeable;
}

actions_model compile_sensor(const Sensor *sensor, const map<string, Predicate *> &predicates_map,
                             const map<string, bool> &fo_facts_evaluation, const map<string, bool> &facts_evaluation) {
    deque<Action *> assumption_actions;
    deque<pair<Formula *, string>> sensor_model;
    const deque<Parameter *> &parameters = sensor->getParameters();
    deque<map<Parameter *, PDDL_Object *>> combinations = get_objects_combination(parameters);
    const string &sensor_name = sensor->getName();
    string new_action_name;
    const Formula *condition = sensor->getCondition(), *positive_sense = sensor->getSense();
    Formula negative_sense(*positive_sense);
    negative_sense.change_is_negative();
    map<string, Parameter *> parametersMap;
    Action *assumption_action;
    Formula *precondition = nullptr, *positive_effect, *negative_effect, *conditions, *not_known_true, *not_known_false;
    string combination_name, positive_predicate, negative_predicate, sense_string;
    FormulaType formulaType;
    Predicate *predicate;
    for (const auto &combination: combinations) {
        conditions = nullptr;
        combination_name = sensor_name;
        for (const auto &object: combination) {
            combination_name += "_" + object.second->getName();
        }
        positive_effect = compile_formula(positive_sense, combination, predicates_map);
        positive_predicate = positive_effect->getFormulaType();
        negative_effect = compile_formula(&negative_sense, combination, predicates_map);
        negative_predicate = negative_effect->getFormulaType();
        if (condition != nullptr) {
            precondition = compile_formula(condition, combination, predicates_map);
            predicate = predicates_map.at(positive_predicate);
            not_known_true = new Formula({}, deque<Parameter *>(), positive_predicate, true, predicate);
            precondition->add_inner_formula(not_known_true);
            predicate = predicates_map.at(negative_predicate);
            not_known_false = new Formula({}, deque<Parameter *>(), negative_predicate, true, predicate);
            precondition->add_inner_formula(not_known_false);
            formulaType = evaluateFormula(precondition, predicates_map, facts_evaluation);
            if (formulaType != changeable) {
                delete precondition;
                precondition = nullptr;
                if (formulaType == always_false) {
                    delete positive_effect;
                    delete negative_effect;
                    continue;
                }
            }
        }

        new_action_name = "assume-" + combination_name + "-true";
        assumption_action = new Action(new_action_name, parametersMap, precondition, positive_effect);
        assumption_action->setActionType(ActionType::as_action);
        assumption_actions.push_back(assumption_action);

        precondition = new Formula(*precondition);
        new_action_name = "assume-" + combination_name + "-false";
        assumption_action = new Action(new_action_name, parametersMap, precondition, negative_effect);
        assumption_action->setActionType(ActionType::as_action);
        assumption_actions.push_back(assumption_action);

        conditions = new Formula(*precondition);
        sense_string = fo_facts_evaluation.at(positive_predicate) ? positive_predicate : negative_predicate;
        pair<Formula *, string> sensing = {conditions, sense_string};
        sensor_model.push_back(sensing);
    }
    return {assumption_actions, sensor_model};
}

Formula *compile_formula(const Formula *old_formula, const map<Parameter *, PDDL_Object *> &combination,
                         const map<string, Predicate *> &predicates_map) {
    if (old_formula == nullptr) {
        return nullptr;
    }
    deque<Formula *> inner_formulas;
    deque<Parameter *> parameters;
    bool is_negative = old_formula->isNegative();
    auto old_inner_formulas = old_formula->getInnerFormulas();
    const string old_formula_type = old_formula->getFormulaType();
    auto old_parameters = old_formula->getParameters();
    string formula_type, object_name;
    Predicate *predicate = nullptr;
    if (old_inner_formulas.empty()) {
        formula_type = is_negative ? "k_not_" : "k_";
        formula_type += old_formula_type;
        for (auto parameter: old_parameters) {
            auto it = combination.find(parameter);
            object_name = it == combination.end() ? parameter->getName() : it->second->getName();
            formula_type += "_" + object_name;
        }
        predicate = predicates_map.at(formula_type);
    } else {
        if (old_formula_type == "forall" || old_formula_type == "exists") {
            formula_type = old_formula_type == "forall" ? "and" : "or";
            map<Parameter *, PDDL_Object *> inner_combination(combination);
            Parameter *innerParameter = old_formula->getInnerParameter();
            auto &inner_formula = old_formula->getInnerFormulas().at(0);
            for (auto object: innerParameter->getCompatibleObjects()) {
                inner_combination[innerParameter] = object;
                inner_formulas.push_back(compile_formula(inner_formula, inner_combination, predicates_map));
            }
        } else {
            for (const auto old_inner_formula: old_inner_formulas) {
                inner_formulas.push_back(compile_formula(old_inner_formula, combination, predicates_map));
            }
            formula_type = old_formula_type;
        }
    }
    return new Formula(inner_formulas, parameters, formula_type, false, predicate);
}

actions_model compile_invariant(const Invariant *invariant, const map<string, Predicate *> &predicates_map) {
    deque<pair<Formula *, string>> invariant_model;
    deque<Action *> invariant_actions;
    map<string, Parameter *> action_parameters;
    const auto &invariant_facts = invariant->getFacts();
    auto n = invariant_facts.size();
    Predicate *predicate;
    Formula *precondition, *effect;
    deque<Parameter *> parameters;
    string predicate_name;
    bool state, goal_state;
    Action *action;
    for (size_t i = 0; i < n; ++i) {
        const auto &fact = invariant_facts.at(i);
        deque<Formula *> inner_formulas;
        for (size_t j = 0; j < n; ++j) {
            auto another_fact = Fact(*invariant_facts.at(j));
            state = !another_fact.getState();
            if (j != i) {
                goal_state = true;
            } else {
                goal_state = false;
            }
            predicate_name = fact_to_predicate_name(&another_fact, state);
            predicate = predicates_map.at(predicate_name);
            inner_formulas.push_back(
                    new Formula(deque<Formula *>(), parameters, predicate_name, !goal_state, predicate));
        }
        precondition = new Formula(inner_formulas, parameters, "and", false, nullptr);
        state = fact->getState();
        predicate_name = fact_to_predicate_name(fact, state);
        predicate = predicates_map.at(predicate_name);
        effect = new Formula(deque<Formula *>(), parameters, predicate_name, false, predicate);
        static size_t actions_counter = 0;
        string name = "invariant-at-least-one-" + to_string(actions_counter++);
        action = new Action(name, action_parameters, precondition, effect);
        action->setActionType(ActionType::ram_action);
        invariant_actions.push_back(action);
        pair<Formula *, string> deducing = {new Formula(*precondition), predicate_name};
        invariant_model.push_back(deducing);
    }
    return {invariant_actions, invariant_model};
}

map<string, bool> calculate_fact_evaluation(const Problem &problem,
                                            const map<string, Predicate *> &predicates_map) {
    map<string, bool> facts_evaluation;
    bool state;
    std::string tmp;

    for (const auto &predicate: predicates_map) {
        facts_evaluation[predicate.first] = predicate.first.find("k_not") == 0;
    }

    for (Fact *fact: problem.getUnknownFacts()) {
        state = fact->getState();
        tmp = fact_to_predicate_name(fact, state);
        facts_evaluation.at(fact_to_predicate_name(fact, state)) = false;
        state = !state;
        facts_evaluation.at(fact_to_predicate_name(fact, state)) = false;
    }

    for (const auto &fact: problem.getInit()) {
        state = fact->getState();
        facts_evaluation.at(fact_to_predicate_name(fact, state)) = true;
        state = !state;
        facts_evaluation.at(fact_to_predicate_name(fact, state)) = false;
    }

    return facts_evaluation;
}

Action *create_goal_action(const Problem &problem,
                           const map<string, Predicate *> &predicates_map) {
    deque<Formula *> goal_precondition_inner_formulas, inner_formulas;
    deque<Parameter *> formulas_parameters;
    Formula *inner_formula, *precondition, *effect;
    string predicate_name, goal_action_name = "reach_new_goal_through_original_goal__";
    Predicate *predicate;
    bool state;
    for (auto sub_goal: problem.getGoal()) {
        state = sub_goal->getState();
        predicate_name = fact_to_predicate_name(sub_goal, state);
        predicate = predicates_map.at(predicate_name);
        inner_formula = new Formula(inner_formulas, formulas_parameters, predicate_name, false, predicate);
        goal_precondition_inner_formulas.push_back(inner_formula);
    }
    precondition = new Formula(goal_precondition_inner_formulas, formulas_parameters, "and", false, nullptr);
    predicate_name = "new-goal";
    predicate = predicates_map.at(predicate_name);
    effect = new Formula(inner_formulas, formulas_parameters, predicate_name, false, predicate);
    return new Action(goal_action_name, map<std::string, Parameter *>(), precondition, effect);
}
