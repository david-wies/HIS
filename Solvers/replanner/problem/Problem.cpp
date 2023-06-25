/*
 * Problem.cpp
 *
 *  Created on: Feb 6, 2023
 *      Author: david
 */

#include "Problem.h"
#include <fstream>
#include <sstream>
#include "../utils.h"

Problem::Problem(const std::string &domain_path,
                 const std::string &problem_path) {
    float time;
    Term problem_term, domain_term;
    std::string content, tmp;
    const Term *term;
    std::deque<Term *> *tmp_deque;
    Action *action;
    Sensor *sensor;

    std::cout << "Processing Problem \nProblem path: " << problem_path << "\nDomain path: " << domain_path << std::endl;
    const clock_t begin_time = clock();

    std::cout << "Start preprocessing" << std::endl;
    const clock_t prepocessing_begin_time = clock();
    auto stream = remove_comments(domain_path);
    stream >> domain_term;
    stream = remove_comments(problem_path);
    stream >> problem_term;
    time = float(clock() - prepocessing_begin_time) / CLOCKS_PER_SEC;
    std::cout << "Preprocessing took " << time << " seconds." << std::endl;

    term = domain_term.getInnerTerms(TermType::domain)->at(0);
    domain_name = get_words(term->getContent()).at(1);
    term = problem_term.getInnerTerms(TermType::problem)->at(0);
    problem_name = get_words(term->getContent()).at(1);
    term = problem_term.getInnerTerms(TermType::domain)->at(0);
    tmp = get_words(term->getContent()).at(1);
    if (domain_name != tmp) {
        std::cerr << "The given domain isn't the domain used by the problem!" << std::endl;
        exit(-1);
    }

    tmp_deque = domain_term.getInnerTerms(TermType::requirements);
    if (!tmp_deque->empty()) {
        term = tmp_deque->at(0);
        auto parts = get_words(term->getContent());
        if (parts.size() > 1) {
            parts.pop_front();
            requirements = parts;
        }
        requirements.shrink_to_fit();
    }

    tmp_deque = domain_term.getInnerTerms(TermType::types);
    if (!tmp_deque->empty()) {
        term = tmp_deque->at(0);
        types_processing(term);
    }

    tmp_deque = domain_term.getInnerTerms(TermType::constants);
    if (!tmp_deque->empty()) {
        term = tmp_deque->at(0);
        constants_processing(term);
    }

    tmp_deque = problem_term.getInnerTerms(TermType::objects);
    if (!tmp_deque->empty()) {
        term = tmp_deque->at(0);
        objects_processing(term);
    }

    term = domain_term.getInnerTerms(TermType::predicates)->at(0);
    predicates_processing(term);

    term = problem_term.getInnerTerms(TermType::init)->at(0);
    init_processing(term);

    term = problem_term.getInnerTerms(TermType::hidden)->at(0);
    hidden_processing(term);

    tmp_deque = problem_term.getInnerTerms(TermType::helper);
    if (!tmp_deque->empty()) {
        term = tmp_deque->at(0);
        helper_processing(term);
    }

    for (Term *action_term: *domain_term.getInnerTerms(TermType::action)) {
        action = new Action(action_term, types_map, predicates_map, constants_map);
        actions.push_back(action);
    }
    actions.shrink_to_fit();

    for (Term *sensor_term: *domain_term.getInnerTerms(TermType::sensor)) {
        sensor = new Sensor(sensor_term, types_map, predicates_map, constants_map);
        sensors.push_back(sensor);
    }
    sensors.shrink_to_fit();

    goal_processing(problem_term.getInnerTerms(TermType::goal)->at(0));

    time = float(clock() - begin_time) / CLOCKS_PER_SEC;

    std::cout << "Processing problem took " << time << " seconds." << std::endl;

}

Problem::~Problem() {
    clear();
}

void Problem::clear() {
    problem_name.clear();
    domain_name.clear();
    facts.clear();
    hidden.clear();
    helper.clear();
    unknown_facts.clear();
    goal.clear();
    requirements.clear();

    for (auto &predicate_objects_map: fact_map) {
        for (auto objects_state_map: predicate_objects_map.second) {
            for (auto state_fact_map: objects_state_map.second) {
                delete state_fact_map.second;
            }
            objects_state_map.second.clear();
        }
        predicate_objects_map.second.clear();
    }
    fact_map.clear();

    for (auto action: actions) {
        delete action;
    }
    actions.clear();

    for (auto sensor: sensors) {
        delete sensor;
    }
    sensors.clear();

    for (auto invariant: invariants) {
        delete invariant;
    }
    invariants.clear();

    for (const auto &object: objects_map) {
        delete object.second;
    }
    objects_map.clear();

    for (const auto &object: constants_map) {
        delete object.second;
    }
    constants_map.clear();

    for (const auto &type: types_map) {
        delete type.second;
    }
    types_map.clear();

    for (auto predicate_it: predicates_map) {
        delete predicate_it.second;
    }
    predicates_map.clear();
}

const std::string &Problem::getProblemName() const {
    return problem_name;
}

const std::string &Problem::getDomainName() const {
    return domain_name;
}

const std::deque<Fact *> &Problem::getInit() const {
    return facts;
}

const std::deque<Fact *> &Problem::getUnknownFacts() const {
    return unknown_facts;
}

const std::deque<Fact *> &Problem::getHidden() const {
    return hidden;
}

const std::deque<Invariant *> &Problem::getInvariants() const {
    return invariants;
}

const std::deque<Fact *> &Problem::getHelper() const {
    return helper;
}

const std::deque<Fact *> &Problem::getGoal() const {
    return goal;
}

const std::deque<Action *> &Problem::getActions() const {
    return actions;
}

const std::deque<Sensor *> &Problem::getSensors() const {
    return sensors;
}

const std::map<std::string, Type *> &Problem::getTypesMap() const {
    return types_map;
}


const std::map<std::string, PDDL_Object *> &Problem::getConstantsMap() const {
    return constants_map;
}

const std::map<std::string, PDDL_Object *> &Problem::getObjectsMap() const {
    return objects_map;
}

const std::map<std::string, Predicate *> &Problem::getPredicatesMap() const {
    return predicates_map;
}

const std::deque<std::string> &Problem::getRequirements() const {
    return requirements;
}

std::stringstream Problem::remove_comments(const string &path) {
    std::ifstream input(path);
    std::stringstream output;
    std::string line, new_line;
    size_t semicolon_index;
    while (std::getline(input, line)) {
        semicolon_index = line.find(';');
        if (semicolon_index != std::string::npos) {
            new_line = line.substr(0, semicolon_index);
        } else {
            new_line = line;
        }
        if (!is_whitespace(new_line)) {
            output << new_line << '\n';
        }
    }
    input.close();
    return output;
}

void Problem::types_processing(const Term *&types_term) {
    std::set<string> type_names;
    std::deque<std::string> types_order, waiting_types;
    auto parts = get_words(types_term->getContent());
    parts.pop_front();
    size_t n = parts.size();
    string word;
    std::map<string, string> parent_name;
    std::set<string>::iterator it;
    for (size_t i = 0; i < n; ++i) {
        word = parts.at(i);
        if (word == "-") {
            word = parts.at(++i);
            if (word != "object") {
                it = type_names.find(word);
                if (it == type_names.end()) {
                    types_order.push_back(word);
                    type_names.insert(word);
                }
            }
            for (const string &name: waiting_types) {
                parent_name[name] = word;
                if (name != "object") {
                    it = type_names.find(name);
                    if (it == type_names.end()) {
                        types_order.push_back(name);
                        type_names.insert(name);
                    }
                }
            }
            if ((word != "object") && (type_names.find(word) == type_names.end())) {
                parent_name[word] = "object";
            }
            waiting_types.clear();
        } else {
            waiting_types.push_back(word);
        }
    }
    if (!waiting_types.empty()) {
        for (const std::string &name: waiting_types) {
            parent_name[name] = "object";
            if (name != "object") {
                it = type_names.find(name);
                if (it == type_names.end()) {
                    types_order.push_back(name);
                    type_names.insert(name);
                }
            }
        }
    }
    Type *type = new Type("object", nullptr);
    Type *parent;
    types_map["object"] = type;
    while (!types_order.empty()) {
        word = types_order.front();
        parent = types_map.at(parent_name.at(word));
        types_map[word] = new Type(word, parent);
        types_order.pop_front();
    }
}

void Problem::objects_processing(const Term *&objects_term) {
    Type *type;
    PDDL_Object *object;
    std::map<std::string, Type *>::iterator iterator;
    std::string object_name;
    std::deque<std::string> parts = get_words(objects_term->getContent()), waiting_objects;
    auto n = parts.size();
    for (size_t i = 1; i < n; ++i) {
        object_name = parts.at(i);
        if (object_name == "-") {
            object_name = parts.at(++i);
            iterator = types_map.find(object_name);
            if (iterator != types_map.end()) {
                type = iterator->second;
            } else {
                std::string msg = "Type " + object_name + "hasn't declared!";
                std::cerr << msg << std::endl;
                exit(-1);
            }
            for (const std::string &name: waiting_objects) {
                object = new PDDL_Object(name, type);
                objects_map[name] = object;
                type->add_object(object);
            }
            waiting_objects.clear();
        } else {
            waiting_objects.push_back(object_name);
        }
    }
    if (!waiting_objects.empty()) {
        type = types_map["object"];
        for (const std::string &name: waiting_objects) {
            object = new PDDL_Object(name, type);
            objects_map[name] = object;
            type->add_object(object);
        }
    }
}

void Problem::constants_processing(const Term *&constants_term) {
    Type *type;
    PDDL_Object *object;
    std::map<std::string, Type *>::iterator iterator;
    std::string object_name;
    std::deque<std::string> parts = get_words(constants_term->getContent()), waiting_objects;
    auto n = parts.size();
    for (size_t i = 1; i < n; ++i) {
        object_name = parts.at(i);
        if (object_name == "-") {
            object_name = parts.at(++i);
            iterator = types_map.find(object_name);
            if (iterator != types_map.end()) {
                type = iterator->second;
            } else {
                std::string msg = "Type " + object_name + "hasn't declared!";
                std::cerr << msg << std::endl;
                exit(-1);
            }
            for (const std::string &name: waiting_objects) {
                object = new PDDL_Object(name, type);
                constants_map[name] = object;
                type->add_object(object);
            }
            waiting_objects.clear();
        } else {
            waiting_objects.push_back(object_name);
        }
    }
    if (!waiting_objects.empty()) {
        type = types_map["object"];
        for (const std::string &name: waiting_objects) {
            object = new PDDL_Object(name, type);
            constants_map[name] = object;
            type->add_object(object);
        }
    }
}

void Problem::predicates_processing(const Term *&predicate_term) {
    std::deque<std::string> parts;
    std::string name;
    Predicate *predicate;
    for (auto term: *predicate_term->getInnerTerms(TermType::formula)) {
        predicate = new Predicate(*term, types_map);
        predicates_map[predicate->getName()] = predicate;
    }
}

void Problem::init_processing(const Term *&init_term) {
    Fact *fact;
    Invariant *invariant;
    std::string type;
    std::deque<std::string> parts;
    for (auto term: *init_term->getInnerTerms(TermType::formula)) {
        parts = get_words(term->getContent());
        type = parts.at(0);
        if (type == "invariant") {
            std::deque<Fact *> inner_facts;
            for (auto inner_term: *term->getInnerTerms(TermType::formula)) {
                fact = fact_processing(inner_term);
                fact->getPredicate()->setIsDeducible(true);
                inner_facts.push_back(fact);
            }
            if (inner_facts.size() == 2 && inner_facts.at(0)->isTheSame(*inner_facts.at(1))) {
                unknown_facts.push_back(inner_facts.at(0));
            } else {
                invariant = new Invariant(inner_facts);
                invariants.push_back(invariant);
            }
        } else {
            fact = fact_processing(term);
            facts.push_back(fact);
        }
    }
    for (auto unknownFact = unknown_facts.begin(); unknownFact != unknown_facts.end();) {
        auto index = std::find(facts.begin(), facts.end(), *unknownFact);
        if (index == facts.end()) {
            unknownFact++;
        } else {
            unknown_facts.erase(unknownFact);
        }
    }
    invariants.shrink_to_fit();
    facts.shrink_to_fit();
    unknown_facts.shrink_to_fit();
}

void Problem::hidden_processing(const Term *&hidden_term) {
    for (auto term: *hidden_term->getInnerTerms(TermType::formula)) {
        Fact *fact = fact_processing(term);
        hidden.push_back(fact);
    }
    hidden.shrink_to_fit();
}

void Problem::helper_processing(const Term *&helper_term) {
    for (auto term: *helper_term->getInnerTerms(TermType::formula)) {
        Fact *fact = fact_processing(term);
        if (!is_fact_known(fact)) {
            helper.push_back(fact);
        }
    }
    helper.shrink_to_fit();
}

Fact *Problem::fact_processing(Term *fact_term) {
    Fact *fact;
    bool state;
    std::string predicate_name;
    auto parts = get_words(fact_term->getContent());
    if (parts.at(0) == "not") {
        fact_term = fact_term->getInnerTerms(formula)->at(0);
        parts = get_words(fact_term->getContent());
        state = false;
    } else {
        state = true;
    }
    predicate_name = parts.at(0);
    parts.pop_front();
    fact = get_fact(predicate_name, parts, state);
    return fact;
}

bool Problem::is_fact_known(const Fact *fact) {
    auto end = facts.end();
    return std::find(facts.begin(), end, fact) != end;
}

void Problem::goal_processing(Term *&goal_term) {
    Fact *fact;
    goal_term = goal_term->getInnerTerms(TermType::formula)->at(0);
    auto parts = get_words(goal_term->getContent());
    auto formula_type = parts.at(0);
    if ((formula_type == "and") || (formula_type == "or") || (formula_type == "imply")) {
        for (auto &term: *goal_term->getInnerTerms(TermType::formula)) {
            fact = fact_processing(term);
            goal.push_back(fact);
        }
    } else {
        fact = fact_processing(goal_term);
        goal.push_back(fact);
    }
    goal.shrink_to_fit();
}

Fact *Problem::get_fact(const std::string &predicate_name,
                        const std::deque<std::string> &objects_name, bool state) {
    Fact *fact;

    auto predicate = predicates_map.at(predicate_name);
    std::deque<PDDL_Object *> objects;
    auto n = objects_name.size();
    PDDL_Object *object;
    std::string name;
    for (size_t i = 0; i < n; ++i) {
        name = objects_name.at(i);
        auto it_obj = objects_map.find(name);
        object = it_obj == objects_map.end() ? constants_map.at(name) : it_obj->second;
        objects.push_back(object);
    }

    auto predicate_it = fact_map.find(predicate_name);
    if (predicate_it == fact_map.end()) {
        predicate_it = fact_map.emplace(predicate_name,
                                        std::map<std::deque<std::string>, std::map<bool, Fact *>>()).first;
    }
    auto &predicate_objects_map = predicate_it->second;

    auto objects_it = predicate_objects_map.find(objects_name);
    if (objects_it == predicate_objects_map.end()) {
        objects_it = predicate_objects_map.emplace(objects_name, std::map<bool, Fact *>()).first;
    }
    auto &state_fact_map = objects_it->second;

    auto fact_it = state_fact_map.find(state);
    if (fact_it == state_fact_map.end()) {
        fact = new Fact(predicate, objects, state);
        state_fact_map[state] = fact;
    } else {
        fact = fact_it->second;
    }

    return fact;
}

void Problem::domain_to_pddl(const string &destination,
                             const string &domainName) {
    std::ofstream os(join_path({destination, domainName}));
    os << "(define (domain " << domain_name << ")\n\n(:requirements ";
    for (const string &requirement: requirements) {
        os << requirement << ' ';
    }
    os << ")\n\n(:types\n";
    for (auto &types_it: types_map) {
        if (types_it.second->getName() != "object") {
            os << '\t' << *types_it.second << '\n';
        }
    }
    if (!constants_map.empty()) {
        PDDL_Object *object;
        os << ")\n\n(:constants ";
        Type *type = nullptr, *new_type;
        for (auto &object_it: constants_map) {
            object = object_it.second;
            new_type = object->getObjectType();
            if (type != new_type) {
                if (type != nullptr) {
                    os << "- " << type->getName();
                }
                os << "\n\t";
                type = new_type;
            }
            os << *object << ' ';
        }
    }
    os << ")\n\n(:predicates" << std::endl;
    for (auto &predicate_it: predicates_map) {
        os << '\t' << *predicate_it.second << '\n';
    }
    os << ")\n" << std::endl;
    for (Action *action: actions) {
        os << *action << "\n\n";
    }
    os << std::endl;
    for (Sensor *sensor: sensors) {
        os << *sensor << "\n\n";
    }
    os << ')';
    os.close();
}

void Problem::problem_to_pddl(const string &destination,
                              const string &problemName) {
    std::ofstream os(join_path({destination, problemName}));
    os << "(define (problem " << problem_name << ")\n\n(:domain " << domain_name << ")\n\n(:objects";
    Type *type = nullptr, *new_type;
    PDDL_Object *object;
    for (auto &object_it: objects_map) {
        object = object_it.second;
        new_type = object->getObjectType();
        if (type != new_type) {
            if (type != nullptr) {
                os << "- " << type->getName();
            }
            os << "\n\t";
            type = new_type;
        }
        os << *object << ' ';
    }
    os << "- " << object->getObjectType()->getName() << "\n)\n\n(:init" << std::endl;
    for (Fact *fact: facts) {
        os << '\t' << *fact << '\n';
    }
    os << std::endl;
    for (Invariant *invariant: invariants) {
        os << '\t' << *invariant << '\n';
    }
    for (Fact *fact: unknown_facts) {
        os << "\t (invariant " << *fact << "(not " << *fact << "))\n";
    }
    if (!hidden.empty()) {
        os << ")\n\n(:hidden" << std::endl;
        for (Fact *fact: hidden) {
            os << '\t' << *fact << '\n';
        }
    }
    if (!helper.empty()) {
        os << ")\n\n(:helper" << std::endl;
        for (Fact *fact: helper) {
            os << '\t' << *fact << '\n';
        }
    }
    os << ")\n\n(:goal (and";
    for (const auto fact: goal) {
        os << ' ' << *fact;
    }
    os << "))\n\n)";
    os.close();
}

void Problem::to_pddl(const std::string &destination,
                      const std::string &problemName, const std::string &domainName) {
    domain_to_pddl(destination, domainName);
    problem_to_pddl(destination, problemName);
}

void Problem::domain_to_fo_pddl(const string &destination, const string &domainName) {
    std::ofstream os(join_path({destination, domainName}));
    os << "(define (domain " << domain_name << ")\n\n(:requirements ";
    for (const string &requirement: requirements) {
        os << requirement << ' ';
    }
    os << ")\n\n(:types\n";
    for (auto &types_it: types_map) {
        if (types_it.second->getName() != "object") {
            os << '\t' << *types_it.second << '\n'; // todo: change to work with hierarchy
        }
    }
    if (!constants_map.empty()) {
        PDDL_Object *object;
        os << ")\n\n(:constants ";
        Type *type = nullptr, *new_type;
        for (auto &object_it: constants_map) {
            object = object_it.second;
            new_type = object->getObjectType();
            if (type != new_type) {
                if (type != nullptr) {
                    os << "- " << type->getName();
                }
                os << "\n\t";
                type = new_type;
            }
            os << *object << ' ';
        }
    }
    os << ")\n\n(:predicates" << std::endl;
    for (auto &predicate_it: predicates_map) {
        os << '\t' << *predicate_it.second << '\n';
    }
    os << ")\n" << std::endl;
    for (Action *action: actions) {
        os << *action << "\n\n";
    }
    os << "\n)";
    os.close();
}

void Problem::problem_to_fo_pddl(const string &destination, const string &problemName) {
    std::ofstream os(join_path({destination, problemName}));
    os << "(define (problem " << problem_name << ")\n\n(:domain " << domain_name << ")\n\n(:objects";
    Type *type = nullptr, *new_type;
    PDDL_Object *object;
    for (auto &object_it: objects_map) {
        object = object_it.second;
        new_type = object->getObjectType();
        if (type != new_type) {
            if (type != nullptr) {
                os << "- " << type->getName();
            }
            os << "\n\t";
            type = new_type;
        }
        os << *object << ' ';
    }
    os << "- " << object->getObjectType()->getName() << "\n)\n\n(:init" << std::endl;
    for (Fact *fact: facts) {
        os << '\t' << *fact << '\n';
    }
    os << std::endl;
    if (!hidden.empty()) {
        for (Fact *fact: hidden) {
            os << '\t' << *fact << '\n';
        }
    }
    os << ")\n\n(:goal (and";
    for (const auto fact: goal) {
        os << ' ' << *fact;
    }
    os << "))\n\n)";
    os.close();
}

void Problem::to_fo_pddl(const string &destination, const string &problemName, const string &domainName) {
    domain_to_fo_pddl(destination, domainName);
    problem_to_fo_pddl(destination, problemName);
}
