import json
import logging
import os
import sys
import time
from argparse import ArgumentParser
from typing import Optional, NoReturn

import defs
import solve_fo
import utils
from ppo import PPO
from search import Node, graph_search
from solve_po import solve


def process_input(arguments):
    task = arguments.task
    defs.set_task(task)
    domain_file_name = arguments.domain_path
    domain_id = utils.get_domain_id(domain_file_name)
    defs.set_domain_id(domain_id)
    template_file_name = arguments.template_path
    hyps_file_name = arguments.hyps_path
    helper_file_name = arguments.helper_path
    method_name = arguments.method
    defs.set_method(method_name)
    compilation_name = arguments.compilation
    defs.set_compilation(compilation_name)
    planner_type = arguments.planner.lower()
    defs.set_planner_type(planner_type)
    objective = arguments.objective
    defs.set_objective(objective)
    defs.set_robustness(arguments.robustness)
    log_file_results = defs.create_log_file()

    ppo_problems = list()
    with open(hyps_file_name) as hyps_file:
        lines = hyps_file.readlines()
    gen_folder_name = defs.get_gen_folder_name()
    for index, hyp in enumerate(lines):
        parsed_hyps_file_name = os.path.abspath(os.path.join(gen_folder_name, 'parsed_hyps_%d.dat' % index))
        with open(parsed_hyps_file_name, "w") as parsed_hyps_file:
            parsed_hyps_file.write(hyp)
        ppo_problem = PPO(domain_file_name, template_file_name, parsed_hyps_file_name, helper_file_name)
        ppo_problems.append(ppo_problem)

    return ppo_problems, log_file_results


def main(task: str, ppo_model: PPO, method: str, compilation: str, robustness: Optional[int], objective,
         solver_path: str, budget, log_file_results) -> NoReturn:
    instance_id = utils.get_instance_id(ppo_model.domain_path, ppo_model.hyps_path)
    msg = f'Running instance_id:{instance_id} task:{defs.TASK_TYPE} solver_path:{solver_path} planner_type:{defs.PLANNER_TYPE} '
    if task not in ('fo', 'po', 'po-optimistic'):
        msg += f'method:{method} '
        if method == 'compilation':
            msg += f'compilation:{compilation} objective:{objective}'
            if task == 'his-rob':
                msg += f'robustness:{robustness} '
        msg += f'budget:{budget}'
    else:
        msg += f'method:{task}'
    logging.info(msg)
    log_file_results.write('\n----------------------\n')
    msg = f'Domain::{ppo_model.domain_path}\ntemplate::{ppo_model.template_path}\nhyps::{ppo_model.hyps_path}\nsolver::{solver_path}\nplanner::{defs.PLANNER_TYPE}\nbudget::{budget}\nmethod::{method}\n'
    if method == 'compilation':
        msg += f'compilation::{compilation}\nobjective::{objective}\n'
        if task == 'his-rob':
            msg += f'robustness::{robustness}\n'
    log_file_results.write(msg)
    log_file_results.flush()
    start_time = time.time()
    if task == 'fo':
        problem_path = ppo_model.get_problem_file()
        cost, time_out = solve_fo.solve(ppo_model.domain_path, problem_path, solver_path, defs.PLANNER_TYPE)
        calc_time = time.time() - start_time
        node = Node(ppo_model)
        value = 0 if cost.execution_cost < defs.INFINITY or time_out else defs.INFINITY
        results = dict(
            best_value=value,
            best_node=str(node),
            knowledge_cost=cost.information_cost,
            plan_cost=cost.execution_cost,
            assumption_cost=cost.assumption_cost,
            explored=1,
            calculated=1,
            time=calc_time,
            ex_terminated=False,
            time_terminated=time_out
        )
        log_file_results.write(f'{json.dumps(results, sort_keys=True)}\n')
        msg = f'Total_best_value:{value}\tbest_node:{str(node)}\tknowledge_cost:{cost.information_cost}\tplan_cost:{cost.execution_cost}\tassumption_cost:{cost.assumption_cost}\texplored:1\tcalculated:1\ttime:{calc_time}\tex_terminated:False\ttime_terminated:{time_out}'
        log_file_results.write(msg)
        print(msg)
    elif task == 'po':
        problem_path = ppo_model.get_problem_file()
        compilation = 'kp' if len(compilation.strip()) == 0 else compilation
        cost, time_out = solve(ppo_model.domain_path, problem_path, solver_path, defs.PLANNER_TYPE, compilation,
                               objective, budget=budget)[3:]
        calc_time = time.time() - start_time
        node = Node(ppo_model)
        value = 0 if cost.execution_cost < defs.INFINITY else defs.INFINITY
        results = dict(
            best_value=value,
            best_node=str(node),
            knowledge_cost=cost.information_cost,
            plan_cost=cost.execution_cost,
            assumption_cost=cost.assumption_cost,
            explored=1,
            calculated=1,
            time=calc_time,
            ex_terminated=False,
            time_terminated=time_out
        )
        log_file_results.write(f'{json.dumps(results, sort_keys=True)}\n')
        msg = f'Total_best_value:{value}\tbest_node:{str(node)}\tknowledge_cost:{cost.information_cost}\tplan_cost:{cost.execution_cost}\tassumption_cost:{cost.assumption_cost}\texplored:1\tcalculated:1\ttime:{calc_time}\tex_terminated:False\ttime_terminated:{time_out}'
        log_file_results.write(msg)
        print(msg)
    elif task == 'po-optimistic':
        problem_path = ppo_model.get_problem_file()
        compilation = 'kp' if len(compilation.strip()) == 0 else compilation
        cost, time_out = solve(ppo_model.domain_path, problem_path, solver_path, defs.PLANNER_TYPE, compilation,
                               objective, budget=budget, robustness=robustness, optimistic=True)[3:]
        calc_time = time.time() - start_time
        node = Node(ppo_model)
        value = 0 if cost.execution_cost < defs.INFINITY else defs.INFINITY
        results = dict(
            best_value=value,
            best_node=str(node),
            knowledge_cost=cost.information_cost,
            plan_cost=cost.execution_cost,
            assumption_cost=cost.assumption_cost,
            explored=1,
            calculated=1,
            time=calc_time,
            ex_terminated=False,
            time_terminated=time_out
        )
        log_file_results.write(f'robustness::{robustness}\n')
        log_file_results.write(f'{json.dumps(results, sort_keys=True)}\n')
        msg = f'Total_best_value:{value}\tbest_node:{str(node)}\tknowledge_cost:{cost.information_cost}\tplan_cost:{cost.execution_cost}\tassumption_cost:{cost.assumption_cost}\texplored:1\tcalculated:1\ttime:{calc_time}\tex_terminated:False\ttime_terminated:{time_out}'
        log_file_results.write(msg)
        print(msg)
    elif task == 'his':
        if 'bfd' in method:
            if method == 'bfd':
                print('BFS')
                best_node, best_cost, n_explored, n_calculated, time_terminated, explored_terminated, time_out = graph_search(
                    ppo_model, solver_path, defs.PLANNER_TYPE, objective, use_simplified_function=False, budget=budget)
            elif method == 'bfd-lazy':
                print('Lazy-BFS')
                best_node, best_cost, n_explored, n_calculated, time_terminated, explored_terminated, time_out = graph_search(
                    ppo_model, solver_path, defs.PLANNER_TYPE, objective, use_simplified_function=True, budget=budget)
        elif method == 'compilation':
            explored_terminated = False
            n_explored = 1
            n_calculated = 1
            if compilation != 'kat':
                print('Unsupported compilation', file=sys.stderr)
                return
            problem_path = ppo_model.get_problem_file()
            knowledge_acquisition, best_cost, time_out = solve(ppo_model.domain_path, problem_path, solver_path,
                                                               defs.PLANNER_TYPE, compilation, objective,
                                                               budget=budget)[2:]
            if best_cost.execution_cost < defs.INFINITY:
                best_ppo = ppo_model.create_modified_model(knowledge_acquisition)
                best_node = Node(best_ppo)
            else:
                best_node = Node(ppo_model)
        else:
            print('Unsupported method', file=sys.stderr)
            return
        calc_time = time.time() - start_time
        best_value = 0 if best_cost.execution_cost < defs.INFINITY else defs.INFINITY
        results = dict(
            best_value=best_value,
            best_node=str(best_node),
            knowledge_cost=best_cost.information_cost,
            plan_cost=best_cost.execution_cost,
            assumption_cost=best_cost.assumption_cost,
            explored=n_explored,
            calculated=n_calculated,
            time=calc_time,
            ex_terminated=explored_terminated,
            time_terminated=time_out
        )
        log_file_results.write(f'{json.dumps(results, sort_keys=True)}\n')
        msg = f'Total_best_value:{best_value}\tbest_node:{str(best_node)}\tknowledge_cost:{best_cost.information_cost}\tplan_cost:{best_cost.execution_cost}\tassumption_cost:{best_cost.assumption_cost}\texplored:{n_explored}\tcalculated:{n_calculated}\ttime:{calc_time}\tex_terminated:{explored_terminated}\ttime_terminated:{time_out}'
        log_file_results.write(msg)
        print(msg)
    elif task == 'his-rob':
        if method != 'compilation':
            print('Unsupported method', file=sys.stderr)
            return
        explored_terminated = False
        if compilation != 'kat_rob':
            print('Unsupported compilation', file=sys.stderr)
            return
        problem_path = ppo_model.get_problem_file()
        solve_args = dict()
        if robustness is not None:
            solve_args['robustness'] = robustness
        knowledge_acquisition, best_cost, time_out = solve(ppo_model.domain_path, problem_path, solver_path,
                                                           defs.PLANNER_TYPE, compilation, objective, budget=budget,
                                                           robustness=robustness)[2:]
        calc_time = time.time() - start_time
        if best_cost.execution_cost < defs.INFINITY:
            best_ppo = ppo_model.create_modified_model(knowledge_acquisition)
            best_node = Node(best_ppo)
            best_value = 0
        else:
            best_node = Node(ppo_model)
            best_value = defs.INFINITY
        results = dict(
            best_value=best_value,
            best_node=str(best_node),
            knowledge_cost=best_cost.information_cost,
            plan_cost=best_cost.execution_cost,
            assumption_cost=best_cost.assumption_cost,
            explored=1,
            calculated=1,
            time=calc_time,
            ex_terminated=explored_terminated,
            time_terminated=time_out
        )
        log_file_results.write(f'{json.dumps(results, sort_keys=True)}\n')
        msg = f'Total_best_value:{best_value}\tbest_node:{str(best_node)}\tknowledge_cost:{best_cost.information_cost}\tplan_cost:{best_cost.execution_cost}\tassumption_cost:{best_cost.assumption_cost}\texplored:1\tcalculated:1\ttime:{calc_time}\tex_terminated:{explored_terminated}\ttime_terminated:{time_out}'
        log_file_results.write(msg)
        print(msg)
    log_file_results.flush()
    return


def is_valid_path(arg: str):
    if not os.path.exists(arg):
        parser.error("The file %s does not exist!" % arg)
    else:
        return arg


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('domain_path', type=is_valid_path, help='Path to domain file')
    parser.add_argument('template_path', type=is_valid_path, help='Path to template file')
    parser.add_argument('helper_path', type=is_valid_path)
    parser.add_argument('hyps_path', type=is_valid_path, help='Path to hyps file')
    parser.add_argument('budget', type=int, help='The budget of information shaping modifications')
    parser.add_argument('solver_path', type=is_valid_path, help='Path to the solver')
    parser.add_argument('planner', help='The type of planner to use')
    parser.add_argument('task', choices=('his', 'his-rob', 'fo', 'po', 'po-optimistic'), default='his')
    parser.add_argument('-m', '--method', choices=('bfd', 'bfd-lazy', 'compilation'), default='')
    parser.add_argument('-r', '--robustness', type=int, default=None)
    parser.add_argument('-c', '--compilation', type=str, help='The compilation to use', default='',
                        choices=('kat', 'kp', 'na', 'kat_rob'))
    parser.add_argument('-o', '--objective', default='',
                        choices=('min-ka', 'min-exe-ka', 'min-ka-exe', 'min-as-ka'), help='The objective of the helper')
    parser.add_argument('-hyps', '--hyps_indices', default=None, type=int, nargs='*',
                        help='Indices of hyps to calculate')
    args = parser.parse_args()

    ppo_models, log_file = process_input(args)
    if args.hyps_indices is None:
        args.hyps_indices = list(range(len(ppo_models)))
    for i, ppo_model in enumerate(ppo_models):
        if i in args.hyps_indices:
            main(args.task, ppo_model, args.method, args.compilation, args.robustness, args.objective,
                 os.path.abspath(args.solver_path), args.budget, log_file)
            print()
