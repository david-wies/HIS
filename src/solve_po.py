from __future__ import annotations
import os
import sys
from typing import Optional, List, Tuple

import defs
from utils import run


class Cost:

    def __init__(self, execution_cost: int, assumption_cost: int, information_cost: int) -> None:
        self.execution_cost = execution_cost
        self.assumption_cost = assumption_cost
        self.information_cost = information_cost
        return

    def is_better(self, cost: Cost, objective: str = 'min_ka') -> bool:
        if self.execution_cost == defs.INFINITY:
            return False
        elif cost.execution_cost == defs.INFINITY:
            return True
        if objective == 'min_ka':
            return self.information_cost < cost.information_cost
        return False


def parse_output_file(output_path: str):
    optimistic_plans = list()
    executed_plan = list()
    knowledge_acquisition = list()
    assumptions = list()
    with open(output_path) as output_file:
        output = output_file.read()
    if len(output) == 0:
        exit(-1)
    start = output.find('Starting replanning process')
    if start < 0:
        return optimistic_plans, executed_plan, knowledge_acquisition, assumptions, True
    lines = output[start:].splitlines()
    index = 0
    length = len(lines)
    while index < length:
        if lines[index] == 'Classical plan (reduced):':
            index += 1
            plan = list()
            while lines[index] != 'Assumptions:' and index < length:
                action_name = lines[index].split('.')[1]
                plan.append(action_name)
                index += 1
            optimistic_plans.append(plan)
            continue
        elif lines[index] == 'Knowledge Acquisition:':
            index += 1
            while lines[index] != 'Belief state:' and index < length:
                knowledge = lines[index].split('.')[1]
                knowledge = knowledge[20:]
                if knowledge.startswith('true-'):
                    knowledge = knowledge[5:]
                    state = True
                else:
                    knowledge = knowledge[6:]
                    state = False
                if state:
                    knowledge = f'({knowledge})'
                else:
                    knowledge = f'(not ({knowledge}))'
                knowledge_acquisition.append(knowledge)
                index += 1
            continue
        elif lines[index] == 'Assumptions:':
            index += 1
            while lines[index] != 'Knowledge Acquisition:' and index < length:
                assumption = lines[index].split('.')[1]
                assumptions.append(assumption)
                index += 1
            continue
        elif lines[index].startswith('>>> kp-action='):
            action = lines[index].split()[1]
            action = action.split('=')[1]
            executed_plan.append(action)
        index += 1
    print('\033[1mplan:\033[0m', executed_plan)
    time_out = not lines[-1].startswith('Replanning took ')
    return optimistic_plans, executed_plan, knowledge_acquisition, assumptions, time_out


def is_successful_plan(plan: List[str]) -> Tuple[List[str], bool]:
    successful_plan = False
    if len(plan) > 0 and plan[-1] == 'reach_new_goal_through_original_goal__':
        successful_plan = True
        plan = plan[:-1]
    return plan, successful_plan


def solve(domain_path: str, problem_path: str, solver_path: str, planner_type: str, compilation: Optional[str] = 'kp',
          objective: Optional[str] = None, robustness: Optional[int] = None, budget: Optional[int] = None,
          optimistic: bool = False):
    gen_path = defs.get_gen_folder_name()
    solution_file_path = os.path.join(gen_path, f'{compilation}_output.txt')
    planner_type_name = planner_type.split(':')[0].lower()
    if defs.FD.lower() in planner_type_name:
        planner_path = defs.FD_PATH
    elif defs.FF.lower() in planner_type.split(':')[0].lower():
        planner_path = defs.FF_PATH
    else:
        print('Unsupported planner', file=sys.stderr)
        exit(-1)

    args = [os.path.join(solver_path, 'Replanner'), '--domain', domain_path, '--problem', problem_path, '--planner',
            planner_type, '--planner-path', planner_path, '--compilation', compilation, '--tmpfile-path', gen_path]
    if objective is not None and len(objective) > 0:
        args += ['--objective', objective]
    if robustness is not None:
        args += ['--robustness', str(robustness)]
    if budget is not None:
        args += ['--budget', str(budget)]
    if 'rob' in compilation or optimistic:
        args += ['--no-replan']

    return_code = run(args, solution_file_path)[0]
    if return_code != 0:
        print('failed to run the command, return_code =', return_code, file=sys.stderr)
    optimistic_plans, executed_plan, knowledge_acquisition, assumptions, time_out = parse_output_file(
        solution_file_path)
    if len(optimistic_plans) > 0:
        optimistic_plan = optimistic_plans[0]
    else:
        optimistic_plan = list()
    if 'rob' in compilation or optimistic:
        optimistic_plan, successful_plan = is_successful_plan(optimistic_plan)
        plan = optimistic_plan
    else:
        plan, successful_plan = is_successful_plan(executed_plan)
    plan_cost = len(plan) if successful_plan else defs.INFINITY
    cost = Cost(plan_cost, len(assumptions), len(knowledge_acquisition))
    os.remove(solution_file_path)
    return optimistic_plan, plan, knowledge_acquisition, cost, time_out
