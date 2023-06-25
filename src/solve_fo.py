import os
import sys
from typing import Tuple, List

import defs
from solve_po import Cost
from utils import run


def parse_output_file(output_path: str) -> Tuple[bool, List[str]]:
    with open(output_path) as output_file:
        output = output_file.read()
    if len(output) == 0:
        exit(-1)
    start = output.find('after calling planner')
    plan = list()
    if start < 0:
        return False, plan
    lines = output[start:].splitlines()
    if lines[1] == 'FO problem has no solution!':
        return False, plan
    for line in lines[2:]:
        plan.append(line.split('\t')[1])
    print('\033[1mplan:\033[0m', plan)
    return True, plan


def solve(domain_path: str, problem_path: str, solver_path: str, planner_type: str):
    gen_path = defs.get_gen_folder_name()
    solution_file_path = os.path.join(gen_path, 'fo_output.txt')
    planner_type_name = planner_type.split(':')[0].lower()
    if defs.FD.lower() in planner_type_name:
        planner_path = defs.FD_PATH
    elif defs.FF.lower() in planner_type.split(':')[0].lower():
        planner_path = defs.FF_PATH
    else:
        print('Unsupported planner', file=sys.stderr)
        exit(-1)

    args = [os.path.join(solver_path, 'Replanner'), '--domain', domain_path, '--problem', problem_path, '--planner',
            planner_type, '--planner-path', planner_path, '--tmpfile-path', gen_path, "--fo-solve"]
    return_code, calc_time = run(args, solution_file_path)
    if return_code != 0:
        print('failed to run the command, return_code =', return_code, file=sys.stderr)
        os.remove(solution_file_path)
        exit(return_code)
    is_solved, plan = parse_output_file(solution_file_path)
    os.remove(solution_file_path)
    plan_cost = len(plan) if is_solved else defs.INFINITY
    cost = Cost(plan_cost, 0, 0)
    time_out = calc_time > defs.DEFAULT_TIME_LIMIT
    return cost, time_out
