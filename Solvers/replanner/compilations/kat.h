/*
 * kat.h
 *
 *  Created on: Feb 8, 2023
 *      Author: david
 */

#ifndef COMPILATIONS_KAT_H_
#define COMPILATIONS_KAT_H_

#include "../CompiledProblem.h"
#include "../problem/Problem.h"


CompiledProblem *kat_compile(const Problem &problem);

CompiledProblem *
kat_compile(const Problem &problem, const std::string &objective, bool add_cost_function, int acquisition_budget);

deque<Action *> create_ka_actions(deque<Fact *> helper, const map<string, Predicate *> &predicates_map);


#endif /* COMPILATIONS_KAT_H_ */
