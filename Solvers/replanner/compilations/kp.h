/*
 * kp.h
 *
 *  Created on: Feb 6, 2023
 *      Author: david
 */

#ifndef COMPILATIONS_KP_H_
#define COMPILATIONS_KP_H_

#include "../CompiledProblem.h"
#include "../problem/Problem.h"

using namespace std;

#define actions_model pair<deque<Action *>, deque<pair<Formula *, string>>>

CompiledProblem *kp_compile(const Problem &problem);

CompiledProblem *kp_compile(const Problem &problem, const int &robustness);

std::map<string, Predicate *> compile_predicates_map(const std::map<string, Predicate *> &old_predicates_map);

deque<Action *> compile_action(const Action *action, const map<string, Predicate *> &predicates_map,
                               const map<string, bool> &facts_evaluation);

Formula *compile_effect(const Formula *old_effect, const map<Parameter *, PDDL_Object *> &combination,
                        const map<string, Predicate *> &predicates_map);

FormulaType evaluateFormula(Formula *formula, const map<string, Predicate *> &predicates_map,
                            const map<string, bool> &facts_evaluation);

actions_model compile_sensor(const Sensor *sensor, const map<string, Predicate *> &predicates_map,
                             const map<string, bool> &fo_facts_evaluation, const map<string, bool> &facts_evaluation);

Formula *compile_formula(const Formula *old_formula, const map<Parameter *, PDDL_Object *> &combination,
                         const map<string, Predicate *> &predicates_map);

actions_model compile_invariant(const Invariant *invariant, const map<string, Predicate *> &predicates_map);

map<string, bool> calculate_fact_evaluation(const Problem &problem, const map<string, Predicate *> &predicates_map);

Action *create_goal_action(const Problem &problem, const map<string, Predicate *> &predicates_map);

inline string
predicate_to_predicate_name(const Predicate *predicate, const deque<PDDL_Object *> &objects, bool &state) {
    string name = state ? "k_" : "k_not_";
    name += predicate->getName();
    for (const auto &object: objects) {
        name += "_" + object->getName();
    }
    return name;
}

inline string fact_to_predicate_name(Fact *fact, bool &state) {
    return predicate_to_predicate_name(fact->getPredicate(), fact->getObjects(), state);
}

inline string fact_to_predicate_name(Fact *fact) {
    bool state = fact->getState();
    return fact_to_predicate_name(fact, state);
}

#endif /* COMPILATIONS_KP_H_ */
