/*
 * limitedkp.h
 *
 *  Created on: Feb 9, 2023
 *      Author: david
 */

#ifndef COMPILATIONS_LIMITED_KP_H_
#define COMPILATIONS_LIMITED_KP_H_

#include "../CompiledProblem.h"
#include "../problem/Problem.h"
#include "kp.h"

using namespace std;

/**
 * Like KP except that the sensor can reveal only information the helper know.
 */
CompiledProblem *limited_kp_compile(const Problem &problem);

actions_model compile_true_sensor(const Sensor *sensor, const map<string, Predicate *> &predicates_map,
                                  const map<string, bool> &facts_evaluation, set<string> &helper);

#endif /* COMPILATIONS_LIMITED_KP_H_ */
