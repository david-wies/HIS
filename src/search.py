from __future__ import annotations

import time
from typing import Optional, Tuple

import defs
from ppo import PPO
from solve_po import Cost, solve


class Node:

    def __init__(self, ppo_model: PPO, parent: Optional[Node] = None) -> None:
        self.ppo_model = ppo_model
        self.parent = parent
        if parent:
            self.depth = parent.depth + 1
        else:
            self.depth = 0
        return


def get_evaluate_function(solver_path: str, planner_type: str, objective: str = 'min_ka'):
    cost_table = dict()

    def evaluate(ppo_model: PPO, use_simplified_function: bool = False) -> Tuple[Cost, float, int]:
        desc = str(ppo_model)
        cur_cost = cost_table.get(desc)
        calculated = 0
        if cur_cost is not None:
            return cur_cost, False, 0
        elif use_simplified_function:
            simplified_model = ppo_model.get_simplified_model()
            simplified_desc = str(simplified_model)
            simplified_cost = cost_table.get(simplified_desc)
            time_out = False
            if simplified_cost is None:
                simplified_cost, time_out, inner_calculated = evaluate(simplified_model)
                cost_table[simplified_desc] = simplified_cost
                calculated = 1
            if simplified_cost.execution_cost < defs.INFINITY and not time_out:
                cur_cost, time_out, inner_calculated = evaluate(ppo_model)
                cost_table[desc] = cur_cost
                calculated += 1
            else:
                cur_cost = simplified_cost
            return cur_cost, time_out, calculated
        else:
            cur_cost, time_out = solve(ppo_model.domain_path, ppo_model.get_problem_file(), solver_path, planner_type,
                                       'kp_limited')[3:]
            cost_table[desc] = cur_cost
            return cur_cost, time_out, 1

    return evaluate


def graph_search(ppo_model: PPO, solver_path: str, planner_type: str, objective: str = 'min_ka',
                 time_limit: int = defs.DEFAULT_TIME_LIMIT, max_calculate: float = 1000, max_explore: float = 3000,
                 use_simplified_function: bool = False, budget: Optional[int] = None):
    root_node = Node(ppo_model)
    explored = set()
    n_explored = 0
    n_calculated = 0
    queue = [root_node]
    best_cost = Cost(defs.INFINITY, defs.INFINITY, defs.INFINITY)
    time_terminated = False
    explored_terminated = False
    best_node = root_node
    time_out = False
    evaluate_function = get_evaluate_function(solver_path, planner_type, objective)
    start_time = time.time()
    last_depth = 0
    while len(queue) > 0:
        if n_calculated >= max_explore or n_explored >= max_calculate:
            explored_terminated = True
            break
        time_elapsed = time.time() - start_time
        if time_elapsed > time_limit:
            time_terminated = True
            break
        node = queue.pop(0)
        if last_depth < node.depth:
            last_depth = node.depth
        n_explored += 1
        node_description = str(node)
        if node_description in explored:
            continue
        else:
            explored.add(node_description)
        cur_cost, time_out, calculated = evaluate_function(node.ppo_model, use_simplified_function)
        if not isinstance(calculated, int):
            explored_terminated = True
            return best_node, best_cost, n_explored, n_calculated, time_terminated, explored_terminated, time_out
        n_calculated += calculated
        cur_cost.information_cost = node.depth
        if time_out:
            time_terminated = True
            break
        if cur_cost.is_better(best_cost, objective):
            best_node = node
            best_cost = cur_cost
            if objective == 'min-ka':
                break
        if node.depth < budget:
            queue.extend(node.get_successors())

    return best_node, best_cost, n_explored, n_calculated, time_terminated, explored_terminated, time_out
