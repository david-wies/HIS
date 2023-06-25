/*
 * limitedkp.cpp
 *
 *  Created on: Feb 9, 2023
 *      Author: david
 */

#include "limited_kp.h"
#include "../utils.h"

CompiledProblem *limited_kp_compile(const Problem &problem) {
    CompiledProblem *compiled_problem;

    auto predicates_map = compile_predicates_map(problem.getPredicatesMap());
    deque<Action *> actions, tmp_actions;
    map<string, bool> facts_evaluation = calculate_fact_evaluation(problem, predicates_map);
    bool state;

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

    set<string> helper_knowledge;
    string compiled_fact;
    for (auto &fact: problem.getHelper()) {
        compiled_fact = fact_to_predicate_name(fact);
        helper_knowledge.insert(compiled_fact);
    }
    for (const auto *sensor: problem.getSensors()) {
        auto tmp = compile_true_sensor(sensor, predicates_map, facts_evaluation, helper_knowledge);
        sensors_model.insert(sensors_model.end(), tmp.second.begin(), tmp.second.end());
        actions.insert(actions.end(), tmp.first.begin(), tmp.first.end());
    }
    sensors_model.shrink_to_fit();

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

    compiled_problem = new CompiledProblem(problemName, domainName, requirements, predicates_map, sensors_model,
                                           invariants_model, facts_evaluation, goal, actions);

    return compiled_problem;
}

actions_model compile_true_sensor(const Sensor *sensor, const map<string, Predicate *> &predicates_map,
                                  const map<string, bool> &facts_evaluation, set<string> &helper) {
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
    Formula *precondition, *effect, *positive_effect, *negative_effect, *conditions, *not_known_true;
    string combination_name, predicate_name, positive_predicate, negative_predicate, sense_string;
    FormulaType formulaType;
    Predicate *predicate;
    bool is_positive;
    for (const auto &combination: combinations) {
        conditions = nullptr;
        combination_name = sensor_name;
        for (const auto &object: combination) {
            combination_name += "_" + object.second->getName();
        }
        positive_effect = compile_formula(positive_sense, combination, predicates_map);
        negative_effect = compile_formula(&negative_sense, combination, predicates_map);
        positive_predicate = positive_effect->getFormulaType();
        negative_predicate = negative_effect->getFormulaType();
        if (helper.find(positive_predicate) != helper.end()) {
            effect = positive_effect;
            delete negative_effect;
            is_positive = true;
        } else {
            delete positive_effect;
            if (helper.find(negative_predicate) != helper.end()) {
                effect = negative_effect;
                is_positive = false;
            } else {
                delete negative_effect;
                continue;
            }
        }
        if (condition != nullptr) {
            precondition = compile_formula(condition, combination, predicates_map);
            predicate = predicates_map.at(positive_predicate);
            not_known_true = new Formula({}, deque<Parameter *>(), positive_predicate, true, predicate);
            predicate = predicates_map.at(negative_predicate);
            auto *not_known_false = new Formula({}, deque<Parameter *>(), negative_predicate, true, predicate);
            precondition->add_inner_formula(not_known_true);
            precondition->add_inner_formula(not_known_false);
            formulaType = evaluateFormula(precondition, predicates_map, facts_evaluation);
            if (formulaType != changeable) {
                delete precondition;
                precondition = nullptr;
                if (formulaType == always_false) {
                    delete effect;
                    continue;
                }
            }
        }
        new_action_name = is_positive ? "assume-" + combination_name + "-true" : "assume-" + combination_name +
                                                                                 "-false";
        assumption_action = new Action(new_action_name, parametersMap, precondition, effect);
        assumption_action->setActionType(ActionType::as_action);
        assumption_actions.push_back(assumption_action);
        conditions = new Formula(*precondition);
        sense_string = effect->getFormulaType();
        pair<Formula *, string> sensing = {conditions, sense_string};
        sensor_model.push_back(sensing);
    }
    return {assumption_actions, sensor_model};
}
