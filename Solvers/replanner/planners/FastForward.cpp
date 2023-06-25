/*
 * FastForward.cpp
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#include "FastForward.h"
#include "../utils.h"
#include <utility>
#include <filesystem>
#include <fstream>

std::deque<string> FastForward::parse_planner_plan(const string &plan_path) {
    string base_dir = std::filesystem::path(plan_path.c_str()).parent_path().string();
    auto tmp_1_plan = join_path({base_dir, "tmp_1_ff_output"});
    string command = "egrep -e \"[0-9]+:\" " + plan_path + " > " + tmp_1_plan;
    std::deque<string> plan;
    if (system(command.c_str()) != 0) {
        plan.emplace_back("Failed");
        remove(tmp_1_plan.c_str());
        return plan;
    }
    string line, action_name;
    size_t index;
    std::ifstream plan_file(tmp_1_plan);
    while (!plan_file.eof()) {
        getline(plan_file, line);
        if (!line.empty()) {
            index = line.find(':') + 2;
            plan.push_back(line.substr(index, line.size() - index));
        }
    }
    return plan;
}

FastForward::FastForward(const string &executablePath, string planner_config) : Planner("FF"), executable_path(
        join_path({executablePath, "ff"})), configuration(std::move(planner_config)) {}

std::deque<string> FastForward::solve(const string &destination_path,
                                      const string &domain_path, const string &problem_path) {
    auto current_path = std::filesystem::current_path();
    const string domain = std::filesystem::absolute(domain_path);
    const string problem = std::filesystem::absolute(problem_path);
    const string dest = std::filesystem::absolute(destination_path);
    std::filesystem::current_path(dest);
    auto output_path = join_path({dest, "ff_output"});
    string cmd =
            executable_path + " -o " + domain + " -f " + problem + " " + configuration + " > " + output_path;
    std::cout << bold_on << "Command: " << bold_off << cmd << std::endl;
    std::deque<string> plan;
    auto return_value = system(cmd.c_str());
    if (return_value == 0) {
        plan = parse_planner_plan(output_path);
    } else {
        plan.emplace_back("Failed");
    }
    remove(output_path.c_str());
    std::filesystem::current_path(current_path);
    return plan;
}
