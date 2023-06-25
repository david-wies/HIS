import os
from pathlib import Path

COMPILATION_TYPE = ''
DEFAULT_TIME_LIMIT = 1000
DOMAIN_ID = ''
HYPS_STRING = ';;; HYPS <hyp>'
INFINITY = 8888
METHOD_NAME = ''
OBJECTIVE = ''
PLANNER_TYPE = ''
RESULTS_LOG_NAME = 'log_results.txt'
ROBUSTNESS = 0
SEPARATOR_STRING = '---'
TASK_TYPE = ''

FD = 'fd'
if 'src' in os.getcwd():
    FD_PATH = os.path.abspath('../Solvers/Fast-Downward')
else:
    FD_PATH = os.path.abspath('./Solvers/Fast-Downward')
FF = 'ff'
if 'src' in os.getcwd():
    FF_PATH = os.path.abspath(os.path.join('../Solvers/FF-v2.3'))
else:
    FF_PATH = os.path.abspath(os.path.join('./Solvers/FF-v2.3'))


def set_task(task_type):
    global TASK_TYPE
    TASK_TYPE = task_type
    return


def set_domain_id(domain_id):
    global DOMAIN_ID
    DOMAIN_ID = domain_id
    return


def set_method(method_name):
    global METHOD_NAME
    METHOD_NAME = method_name
    return


def set_compilation(compilation_type):
    global COMPILATION_TYPE
    COMPILATION_TYPE = compilation_type
    return


def set_objective(objective):
    global OBJECTIVE
    OBJECTIVE = objective
    return


def set_robustness(robustness):
    global ROBUSTNESS
    ROBUSTNESS = robustness
    return


def set_planner_type(planner_type):
    global PLANNER_TYPE
    PLANNER_TYPE = planner_type
    return


def get_log_folder_name():
    if TASK_TYPE in ('fo', 'po', 'po-optimistic'):
        log_dir_name = f'logs--{DOMAIN_ID}--{TASK_TYPE}--{PLANNER_TYPE.replace(":", "")}'
    else:
        compilation_type = f'--{COMPILATION_TYPE}' if METHOD_NAME == 'compilation' else ''
        log_dir_name = f'logs--{DOMAIN_ID}--{TASK_TYPE}--{METHOD_NAME}{compilation_type}--{OBJECTIVE}--{PLANNER_TYPE.replace(":", "")}'
        if 'his-rob' == TASK_TYPE and ROBUSTNESS is not None:
            log_dir_name += f'--robustness={ROBUSTNESS}'
    root_path = '..' if 'src' in os.getcwd() or 'script' in os.getcwd() else '.'
    root_path = os.path.join(os.path.abspath(root_path))
    logs_folder = os.path.join(root_path, 'logs', log_dir_name, 'log')
    return logs_folder


def get_gen_folder_name():
    if TASK_TYPE in ('fo', 'po', 'po-optimistic'):
        log_dir_name = f'logs--{DOMAIN_ID}--{TASK_TYPE}--{PLANNER_TYPE.replace(":", "")}'
    else:
        compilation_type = f'--{COMPILATION_TYPE}' if METHOD_NAME == 'compilation' else ''
        log_dir_name = f'logs--{DOMAIN_ID}--{TASK_TYPE}--{METHOD_NAME}{compilation_type}--{OBJECTIVE}--{PLANNER_TYPE.replace(":", "")}'
        if 'his-rob' == TASK_TYPE and ROBUSTNESS is not None:
            log_dir_name += f'--robustness={ROBUSTNESS}'
    root_path = '..' if 'src' in os.getcwd() or 'script' in os.getcwd() else '.'
    root_path = os.path.join(os.path.abspath(root_path))
    gen_folder = os.path.join(root_path, 'logs', log_dir_name, 'gen')
    return gen_folder


def create_log_file():
    gen_path = get_gen_folder_name()
    Path(gen_path).mkdir(exist_ok=True, parents=True)
    log_path = get_log_folder_name()
    Path(log_path).mkdir(exist_ok=True, parents=True)
    log_file_results_name = os.path.join(log_path, RESULTS_LOG_NAME)
    log_file_results = open(log_file_results_name, "a")
    return log_file_results
