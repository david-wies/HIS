/*
 * FastDownward.cpp
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#include "FastDownward.h"
#include "../utils.h"
#include <iostream>
#include <fstream>
#include <filesystem>

std::deque<string> FastDownward::parse_planner_plan(const string &plan_path) {
    std::deque<string> plan;
    std::ifstream plan_file(plan_path);
    if (!plan_file.good()) {
        return plan;
    }
    string line, step;
    while (!plan_file.eof()) {
        getline(plan_file, line);
        if (line.at(0) != ';') {
            step = line.substr(1, line.size() - 3);
            step = get_words(step).at(0);
            plan.push_back(step);
        } else {
            break;
        }
    }
    plan_file.close();
    return plan;
}

FastDownward::FastDownward(const string &executable_path,
                           const string &planner_config) : Planner("FD"), executable_path(
        join_path({executable_path, "fast-downward.py"})), search(get_planner_search(planner_config)) {}

string FastDownward::get_planner_search(const string &planner_config) {
    string tmp_search;
    if (planner_config.find("hmaxnocachetrans") != std::string::npos) {
        tmp_search = "--search \"astar(hmax(transform=adapt_costs(NORMAL), cache_estimates=false))\"";
    } else if (planner_config.find("hmaxcachetrans") != std::string::npos) {
        tmp_search = "--search \"astar(hmax(transform=adapt_costs(NORMAL), cache_estimates=true))\"";
    } else if (planner_config.find("hmaxnocache") != std::string::npos) {
        tmp_search = "--search \"astar(hmax(transform=no_transform(), cache_estimates=false))\"";
    } else if (planner_config.find("hmax") != std::string::npos) {
        tmp_search = "--search \"astar(hmax(transform=no_transform(), cache_estimates=true))\"";
    } else if (planner_config.find("fftransnocache") != std::string::npos) {
        tmp_search = " --search \"astar(ff(transform=adapt_costs(NORMAL), cache_estimates=false))\"";
    } else if (planner_config.find("ff") != std::string::npos) {
        tmp_search = " --search \"astar(ff(transform=adapt_costs(NORMAL), cache_estimates=true))\"";
    } else if (planner_config.find("lmcut") != std::string::npos) {
        tmp_search = " --search \"astar(lmcut())\"";
    } else if (planner_config.find("fftrans") != std::string::npos) {
        tmp_search = " --search \"astar(ff(transform=adapt_costs(NORMAL), cache_estimates=true))\"";
    } else {
        tmp_search = " --search \"astar(hmax(transform=adapt_costs(NORMAL), cache_estimates=true))\"";
    }
    return tmp_search;
}

std::deque<string> FastDownward::solve(const string &destination_path,
                                       const string &domain_path, const string &problem_path) {
    auto current_path = std::filesystem::current_path();
    const string fd_output_path = "fd_output";
    const string plan_path = "sas_plan";
    std::cout << bold_on << "Path: " << bold_off << current_path << std::endl;
    const string domain = std::filesystem::absolute(domain_path);
    const string problem = std::filesystem::absolute(problem_path);
    std::filesystem::current_path(destination_path);
    string cmd = executable_path + " " + domain + " " + problem + " " + search + " > " + fd_output_path;
    std::cout << bold_on << "Command: " << bold_off << cmd << std::endl;
    auto return_value = system(cmd.c_str());
    std::deque<string> plan;
    if (return_value == 0) {
        plan = parse_planner_plan(plan_path);
    } else {
        plan.emplace_back("Failed");
    }
    remove(plan_path.c_str());
    std::filesystem::current_path(current_path);
    return plan;
}
