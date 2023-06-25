/*
 * kminrob.h
 *
 *  Created on: Feb 12, 2023
 *      Author: david
 */

#ifndef COMPILATIONS_KMINROB_H_
#define COMPILATIONS_KMINROB_H_

#include "../CompiledProblem.h"
#include "../problem/Problem.h"

CompiledProblem *kminrob_compile(const Problem &problem, const std::string &objective, const int &robustness,
                                 bool add_cost_function, int acquisition_budget);

#endif /* COMPILATIONS_KMINROB_H_ */
