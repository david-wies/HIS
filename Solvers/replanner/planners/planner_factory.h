/*
 * planner_factory.h
 *
 *  Created on: Feb 5, 2023
 *      Author: david
 */

#ifndef PLANNERS_PLANNER_FACTORY_H_
#define PLANNERS_PLANNER_FACTORY_H_

#include <string>
#include <boost/algorithm/string.hpp>
#include "Planner.h"
#include "FastDownward.h"
#include "FastForward.h"
#include "../utils.h"

Planner *planner_factory(string name, const string &planner_path) {
    Planner *planner;
    boost::to_lower(name);
    auto parts = split(name, ':');
    std::string planner_name = parts.at(0), configuration = parts.size() > 1 ? parts.at(1) : "";
    if (planner_name.find("fd") == 0) {
        planner = new FastDownward(planner_path, configuration);
    } else if (planner_name.find("ff") == 0) {
        planner = new FastForward(planner_path, configuration);
    } else {
        throw std::invalid_argument(planner_name + "is unsupported planner!");
    }
    return planner;
}

#endif /* PLANNERS_PLANNER_FACTORY_H_ */
