import os
import re
import resource
import subprocess
import sys
import time
from typing import Set, Optional, List

import defs

FACT_PATTERN = re.compile('(\(\s*not\s*\([^()]+\)\s*\))|(\([^()]+\))')


def get_domain_id(domain_path: str) -> str:
    split_domain_path = domain_path.split('/')
    domain_id = split_domain_path[-5: -2]
    sep = defs.SEPARATOR_STRING
    domain_id = sep.join(domain_id)
    return domain_id


def get_instance_id(domain_path: str, hyp_path: str) -> str:
    split_domain_path = domain_path.split('/')
    domain_id = split_domain_path[-5: -1]
    sep = defs.SEPARATOR_STRING
    domain_id = sep.join(domain_id)
    hyp_id = os.path.basename(hyp_path)
    instance_id = domain_id + '-' + hyp_id
    return instance_id


def process_helper_knowledge(helper_path) -> Set:
    if not os.path.exists(helper_path):
        print(f"Error in process_rec_knowledge : File {helper_path} does not exist")
        return set()
    with open(helper_path) as helper_file:
        content = helper_file.read()
    helper_knowledge = {match.group(0) for match in FACT_PATTERN.finditer(content)}
    return helper_knowledge


def generate_problem_file(template_path: str, hyps_path: str, destination_path: str,
                          helper_knowledge: Optional[Set[str]] = None, added_knowledge: Optional[Set[str]] = None):
    if not os.path.exists(template_path):
        print(f'Error in generate_problem_files : File {template_path} does not exist')
        return None
    if not os.path.exists(hyps_path):
        print(f'Error in generate_problem_files : File {hyps_path} does not exist')
        return None
    if not os.path.exists(destination_path):
        os.mkdir(destination_path)

    with open(template_path) as template_file:
        template_code = template_file.read()
    if helper_knowledge is not None and len(helper_knowledge) > 0:
        goal_index = template_code.find('(:goal')
        template_code = template_code[:goal_index] + '(:helper\n\t\t' + '\n\t\t'.join(
            helper_knowledge) + '\n\t)\n\n\t' + template_code[goal_index:]
    if added_knowledge is not None:
        init_index = template_code.find('(:init') + 7
        template_code = template_code[:init_index] + '\n\t\t' + ' '.join(added_knowledge) + '\n\n\t\t' + template_code[
                                                                                                         init_index:]

    with open(hyps_path) as hyps_file:
        lines = hyps_file.readlines()
    hyp = lines[0].strip()
    hyps_number = os.path.splitext(os.path.basename(hyps_path))[0].split('_')[-1]
    problem_file_name = os.path.join(destination_path, f'problem_{hyps_number}.pddl')
    problem_code = template_code.replace(defs.HYPS_STRING, hyp)
    with open(problem_file_name, "w") as problem_file:
        problem_file.write(problem_code)
    print(problem_file_name)

    return hyp, problem_file_name


class SilentWriter:
    def write(self, string):
        pass


class Log:
    SILENT = 0
    FILE = 0x1
    SCREEN = 0x2
    BOTH = FILE | SCREEN

    def __init__(self, filename=None):
        self.name = filename
        self.has_file = filename is not None
        if self.has_file:
            self.file = open(filename, "w")

    def write(self, string):
        sys.stdout.write(string)
        if self.has_file:
            self.file.write(string)

    def suspend(self):
        if self.has_file:
            self.file.close()
            del self.file
        sys.stdout.flush()

    def resume(self):
        if self.has_file:
            self.file = open(self.name, "a")

    def __call__(self, mode):
        if mode == Log.SILENT:
            return SilentWriter()
        elif mode == Log.SCREEN or not self.has_file:
            return sys.stdout
        elif mode == Log.FILE:
            return self.file
        else:
            return self


def run(cmd: List[str], output: str = '', running_dir=None, timeout=defs.DEFAULT_TIME_LIMIT, memory=2548, log=None,
        verbose=True):
    """
    Runs a command using subprocess.Popen(), restricting time and space resources, preventing core dumps and redirecting
    the output (both stdout and stderr) into a log file (if log is not None).
    :param cmd: bash command to be executed
    :param output: Path to file to save the output of running the command
    :param running_dir: The dir from where to run the command
    :param timeout: timeout in CPU seconds
    :param memory: maximum heap size allowed in Megabytes
    :param log: the log file (of class benchmark.Log)
    :param verbose: If true, also print the heap and time restrictions, the return code of the program and elapsed time.
                    If false, this info is logged if there is a log, but not printed.
    :return: (signal, time), where:
        signal  - 0 if the program terminated properly, non-zero otherwise.
        time    - time spent for executing the program in seconds.
                Note that this is *not* CPU time but usertime and might thus exceed the timeout threshold.
    """
    log_mode = Log.SILENT
    if verbose:
        log_mode |= Log.SCREEN
    if log:
        cmd = ['('] + cmd + [')', '>>', '%s 2>&1' % log.name]
        log_mode |= Log.FILE
    else:
        log = Log()

    if verbose:
        print(log(log_mode), "\033[1mTimeout:\033[0m %d seconds" % timeout)
        print(log(log_mode), "\033[1mHeap restriction:\033[0m %d MB" % memory)
        print(log(log_mode), "\033[1mCommand:\033[0m %s" % ' '.join(cmd))

    memory *= 1024 * 1024

    def setlimits():
        resource.setrlimit(resource.RLIMIT_AS, (memory, memory))
        resource.setrlimit(resource.RLIMIT_CPU, (timeout, timeout))
        resource.setrlimit(resource.RLIMIT_RSS, (memory, memory))
        resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
        return

    with open(output, 'w+') as output_file:
        start_time = time.time()
        return_code = subprocess.Popen(cmd, stdout=output_file, preexec_fn=setlimits, cwd=running_dir).wait()
    time_passed = time.time() - start_time
    log.resume()
    if return_code == 0:
        if verbose:
            print(log(log_mode), "\n\033[1mTime spent:\033[0m %.3f seconds" % time_passed)
    else:
        if verbose:
            print(log(log_mode), "\nFailed! [returncode %d, Time %.3f seconds]" % (return_code, time_passed))

    return return_code, time_passed
