/*
 * Problem.h
 *
 *  Created on: Feb 6, 2023
 *      Author: david
 */

#ifndef PROBLEM_PROBLEM_H_
#define PROBLEM_PROBLEM_H_

#include "Action.h"
#include "Sensor.h"
#include "Formula.h"
#include "Fact.h"
#include "Predicate.h"
#include "Invariant.h"

class Problem {
private:
    std::string problem_name, domain_name;
    std::deque<Fact *> facts, hidden, goal, unknown_facts, helper;
    std::deque<Action *> actions;
    std::deque<Sensor *> sensors;
    std::deque<Invariant *> invariants;
    std::map<std::string, Type *> types_map;
    std::map<std::string, PDDL_Object *> objects_map, constants_map;
    std::map<std::string, Predicate *> predicates_map;
    std::deque<std::string> requirements;
    std::map<std::string, std::map<std::deque<std::string>, std::map<bool, Fact *>>> fact_map;

    static std::stringstream remove_comments(const string &path);

    void types_processing(const Term *&types_term);

    void objects_processing(const Term *&objects_term);

    void constants_processing(const Term *&constants_term);

    void predicates_processing(const Term *&predicate_term);

    void init_processing(const Term *&init_term);

    void hidden_processing(const Term *&hidden_term);

    void helper_processing(const Term *&helper_term);

    void goal_processing(Term *&goal_term);

    void domain_to_pddl(const string &destination, const string &domainName);

    void problem_to_pddl(const string &destination, const string &problemName);

    void domain_to_fo_pddl(const string &destination, const string &domainName);

    void problem_to_fo_pddl(const string &destination, const string &problemName);

    Fact *get_fact(const std::string &predicate_name, const std::deque<std::string> &objects_name, bool state);

    Fact *fact_processing(Term *fact_term);

    bool is_fact_known(const Fact *fact);

public:
    Problem(const std::string &domain_path, const std::string &problem_path);

    virtual ~Problem();

    void clear();

    [[nodiscard]] const std::string &getProblemName() const;

    [[nodiscard]] const std::string &getDomainName() const;

    [[nodiscard]] const std::deque<Fact *> &getInit() const;

    [[nodiscard]] const std::deque<Fact *> &getUnknownFacts() const;

    [[nodiscard]] const std::deque<Fact *> &getHidden() const;

    [[nodiscard]] const std::deque<Fact *> &getHelper() const;

    [[nodiscard]] const std::deque<Fact *> &getGoal() const;

    [[nodiscard]] const std::deque<Action *> &getActions() const;

    [[nodiscard]] const std::deque<Sensor *> &getSensors() const;

    [[nodiscard]] const std::deque<Invariant *> &getInvariants() const;

    [[nodiscard]] const std::map<std::string, Type *> &getTypesMap() const;

    [[nodiscard]] const std::map<std::string, PDDL_Object *> &getObjectsMap() const;

    [[nodiscard]] const std::map<std::string, PDDL_Object *> &getConstantsMap() const;

    [[nodiscard]] const std::map<std::string, Predicate *> &getPredicatesMap() const;

    [[nodiscard]] const std::deque<std::string> &getRequirements() const;

    void to_pddl(const std::string &destination, const std::string &problemName, const std::string &domainName);

    void to_fo_pddl(const std::string &destination, const std::string &problemName, const std::string &domainName);
};

#endif /* PROBLEM_PROBLEM_H_ */
