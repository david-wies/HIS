/*
 * Action.h
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#ifndef PROBLEM_ACTION_H_
#define PROBLEM_ACTION_H_

#include <ostream>
#include "Formula.h"
#include "Term.h"
#include "Parameter.h"
#include "Type.h"
#include "Predicate.h"

class Type;

class Formula;

enum ActionType {
    exe_action, as_action, ram_action, ka_action, special_action
};

class Action {
private:
    std::string name;
    std::map<std::string, Parameter *> parameters_map;
    Formula *precondition, *effect;
    std::deque<std::string> parameters_order;
    ActionType action_type = exe_action;

    void parse_parameters(const Term *pTerm, const std::map<std::string, Type *> &types_map);

public:
    Action(const Term *term, const std::map<std::string, Type *> &types_map,
           const std::map<std::string, Predicate *> &predicates_map,
           const std::map<std::string, PDDL_Object *> &constants_map);

    Action(std::string name, std::map<std::string, Parameter *> parametersMap, Formula *precondition,
           Formula *effect);

    virtual ~Action();

    [[nodiscard]] const std::string &getName() const;

    [[nodiscard]] Formula *getPrecondition() const;

    void setPrecondition(Formula *new_precondition);

    void add_precondition(Formula *additional_precondition);

    [[nodiscard]] Formula *getEffect() const;

    void setEffect(Formula *new_effect);

    [[nodiscard]] std::deque<Parameter *> getParameters() const;

    void add_parameter(Parameter *parameter);

    [[nodiscard]] ActionType getActionType() const;

    void setActionType(ActionType actionType);

    friend std::ostream &operator<<(std::ostream &os, const Action &action1);
};

#endif /* PROBLEM_ACTION_H_ */
