import defs
import utils


class PPO:

    def __init__(self, domain_path: str, template_path: str, hyps_path: str, helper_path: str,
                 fo_plans=None, added_knowledge=None):
        self.domain_path = domain_path
        self.template_path = template_path
        self.helper_path = helper_path
        self.hyps_path = hyps_path
        self.hyps = None
        self.opop_plans = list()
        self.fo_plans = fo_plans
        self.po_plans = list()
        self.belief_states = list()
        # if fo_plans is None:
        #     fo_plans = list()
        if added_knowledge is None:
            added_knowledge = set()
        self.added_knowledge = added_knowledge
        self.helper_knowledge = utils.process_helper_knowledge(helper_path)
        return

    def get_problem_file(self):
        self.hyps, problem_file = utils.generate_problem_file(self.template_path, self.hyps_path,
                                                              defs.get_gen_folder_name(), self.helper_knowledge,
                                                              self.added_knowledge)
        return problem_file

    def create_modified_model(self, modified_elements):
        domain_path = self.domain_path
        template_path = self.template_path
        hyps_path = self.hyps_path
        helper_path = self.helper_path
        if isinstance(modified_elements, str):
            modified_elements = {modified_elements}
        added_knowledge = self.added_knowledge.union(modified_elements)
        modified_model = PPO(domain_path, template_path, hyps_path, helper_path, added_knowledge=added_knowledge)
        return modified_model

    def get_simplified_model(self):
        added_knowledge = self.added_knowledge
        if len(added_knowledge) == 0:
            return self
        helper_knowledge = self.helper_knowledge
        base_knowledge = set()
        for fact in added_knowledge:
            if fact.startswith('(not'):
                fact = fact[4:-1].strip()
            base_knowledge.add('_'.join(fact.split()[:-1]))
        simplified_added_knowledge = set()
        for fact in helper_knowledge:
            if fact.startswith('(not'):
                tmp_fact = fact[4:-1].strip()
            else:
                tmp_fact = fact
            tmp_fact = '_'.join(tmp_fact.split()[:-1])
            if tmp_fact in base_knowledge:
                simplified_added_knowledge.add(fact)
        simplified_ppo = self.create_modified_model(simplified_added_knowledge)
        return simplified_ppo

    def __str__(self) -> str:
        return '---'.join(sorted(self.added_knowledge))
