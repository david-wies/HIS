//============================================================================
// Name        : replanner.cpp
// Author      : David
// Version     : 2
// Description : TKA replanner
//============================================================================

#include <iostream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include "planners/planner_factory.h"
#include "problem/Problem.h"
#include "compilations/compiler.h"
#include "replanner.h"

using namespace std;
namespace po = boost::program_options;

int main(int argc, char *argv[]) {
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,H", "produce help message")
            ("domain", po::value<string>()->required(), "Path to the domain file to solve")
            ("problem", po::value<string>()->required(), "Path to the problem file to solve")
            ("planner", po::value<string>()->default_value("fd"), "Set planner to use")
            ("planner-path", po::value<string>()->required(), "Path to directory of the planner to use")
            ("compilation", po::value<string>()->default_value("kp"), "The compilation to use")
            ("objective", po::value<string>()->default_value(""), "The objective of the agent")
            ("robustness", po::value<int>()->default_value(-1), "Required minimum level of robustness")
            ("budget", po::value<int>()->default_value(-1), "Budget of knowledge acquisition")
            ("keep-intermediate-files", po::bool_switch(), "Keep the compiled files after solving")
            ("no-replan", po::bool_switch(), "Plain only once for the compiled problem")
            ("fo-solve", po::bool_switch(), "Solve the FO problem")
            ("tmpfile-path", po::value<string>()->default_value(boost::filesystem::current_path().string()),
             "Path for tmp files");
    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return 1;
        }
    } catch (const boost::program_options::required_option &e) {
        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return 1;
        } else {
            throw e;
        }
    }
    if (vm.count("help") || argc == 0) {
        cout << desc << "\n";
        return 0;
    }
    auto &tmp_path = vm["tmpfile-path"].as<string>();
    auto &keep_files = vm["keep-intermediate-files"].as<bool>();

    Planner *planner = planner_factory(vm["planner"].as<string>(), vm["planner-path"].as<string>());

    Problem problem(vm["domain"].as<string>(), vm["problem"].as<string>());

    if (vm["fo-solve"].as<bool>()) {
        string problem_name = "fo-problem.pddl", domain_name = "fo-domain.pddl", problem_path, domain_path;
        problem.to_fo_pddl(tmp_path, problem_name, domain_name);
        problem_path = join_path({tmp_path, problem_name});
        domain_path = join_path({tmp_path, domain_name});
        cout << "before calling planner" << endl;
        auto plan = planner->solve(tmp_path, domain_path, problem_path);
        cout << "after calling planner" << endl;
        if ((!plan.empty()) && (plan.at(0) == "Failed")) {
            std::cout << "FO problem has no solution!" << endl;
        } else {
            std::cout << "FO problem plan:" << endl;
            auto n = plan.size();
            for (int i = 0; i < n; ++i) {
                std::cout << i << ".\t" << plan.at(i) << std::endl;
            }
        }
        if (!keep_files) {
            remove(domain_path.c_str());
            remove(problem_path.c_str());
        }
        plan.clear();
        delete planner;
        return 0;
    }

    auto &robustness = vm["robustness"].as<int>();
    auto &budget = vm["budget"].as<int>();
    std::map<string, string> options = {{"robustness", std::to_string(robustness)},
                                        {"objective",  vm["objective"].as<string>()},
                                        {"budget",     std::to_string(budget)}};

    auto &compilation = vm["compilation"].as<std::string>();
    bool add_cost_function = planner->getName() != "FF";

    auto compiled_problem = compiler(problem, compilation, options, add_cost_function);
    options.clear();

    problem.clear();

    compiled_problem->optimize();

    auto &no_replan = vm["no-replan"].as<bool>();
    bool optimize = false;
    replanner(compiled_problem, planner, tmp_path, keep_files, optimize, no_replan);

    delete planner;
    delete compiled_problem;

    return 0;
}
