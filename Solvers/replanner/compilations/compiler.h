/*
 * compiler.h
 *
 *  Created on: Feb 9, 2023
 *      Author: david
 */

#ifndef COMPILATIONS_COMPILER_H_
#define COMPILATIONS_COMPILER_H_

#include "kp.h"
#include "limited_kp.h"
#include "kat.h"
#include "kminrob.h"

#include <iostream>

CompiledProblem *
compiler(const Problem &problem, string compilation, map<string, string> &options, const bool add_cost_function) {
    cout << "Compiling problem, using ";
    const clock_t begin_time = clock();
    CompiledProblem *compiled_problem;
    boost::to_lower(compilation);
    string &objective = options.at("objective");
    int budget = stoi(options.at("budget"));
    if (compilation == "kat") {
        cout << "kat compilation" << endl;
        compiled_problem = kat_compile(problem, objective, add_cost_function, budget);
    } else if (compilation == "kp_limited") {
        cout << "limited kp compilation" << endl;
        compiled_problem = limited_kp_compile(problem);
    } else if (compilation == "kat_rob") {
        cout << "kminrob compilation" << endl;
        int robustness = stoi(options.at("robustness"));
        compiled_problem = kminrob_compile(problem, objective, robustness, add_cost_function, budget);
    } else {
        cout << "kp compilation" << endl;
        int robustness = stoi(options.at("robustness"));
        compiled_problem = kp_compile(problem, robustness);
    }
    std::cout << "Compiling problem took " << float(clock() - begin_time) / CLOCKS_PER_SEC << " seconds." << std::endl;
    return compiled_problem;
}

#endif /* COMPILATIONS_COMPILER_H_ */
