/*
 * replanner.h
 *
 *  Created on: Feb 8, 2023
 *      Author: david
 */

#ifndef REPLANNER_H_
#define REPLANNER_H_

#include "CompiledProblem.h"
#include "planners/Planner.h"

void replanner(CompiledProblem *problem, Planner *planner, const string &tmp_path, const bool &keep_files,
               const bool &optimize, const bool &no_replan);

void print_state(const CompiledProblem *problem);

#endif /* REPLANNER_H_ */
