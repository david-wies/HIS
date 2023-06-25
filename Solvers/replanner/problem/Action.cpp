/*
 * Action.cpp
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#include "Action.h"
#include "../utils.h"


Action::Action(const Term *term, const std::map<std::string, Type *> &types_map,
               const std::map<std::string, Predicate *> &predicates_map,
               const std::map<std::string, PDDL_Object *> &constants_map) {
    auto words = get_words(term->getContent());
    words.pop_front();
    name = std::string(words.at(0));
    words.pop_front();
    auto inner_terms = term->getInnerTerms(TermType::formula);
    const Term *inner;
    short index = 0;
    if (words.at(index) == ":parameters") {
        inner = inner_terms->at(index++);
        parse_parameters(inner, types_map);
    }
    if (words.at(index) == ":precondition") {
        inner = inner_terms->at(index++);
        precondition = new Formula(inner, parameters_map, predicates_map, types_map, constants_map);
    } else
        precondition = nullptr;
    inner = inner_terms->at(index);
    effect = new Formula(inner, parameters_map, predicates_map, types_map, constants_map);
    for (Predicate *predicate: effect->getRelatedPredicates()) {
        predicate->setIsChangeable(true);
    }
}

Action::Action(std::string name,
               std::map<std::string, Parameter *> parametersMap, Formula *precondition,
               Formula *effect) : name(std::move(name)), parameters_map(std::move(parametersMap)),
                                  precondition(precondition), effect(effect) {}

Action::~Action() {
    delete precondition;
    delete effect;
    parameters_order.clear();
    for (const auto &parameter: parameters_map) {
        delete parameter.second;
    }
    parameters_map.clear();
    name.clear();
}

void Action::parse_parameters(const Term *pTerm,
                              const std::map<std::string, Type *> &types_map) {
    if (pTerm->getContent().empty()) {
        return;
    }
    Type *type;
    std::deque<PDDL_Object *> compatibleObjects;
    auto words = get_words(pTerm->getContent());
    std::deque<std::string> waiting;
    std::string word;
    while (!words.empty()) {
        word = words.front();
        words.pop_front();
        if (word == "-") {
            word = words.front();
            words.pop_front();
            type = types_map.at(word);
            for (const std::string &parameter_name: waiting) {
                parameters_order.push_back(parameter_name);
                parameters_map[parameter_name] = type->getParameter(parameter_name);
            }
            waiting.clear();
        } else {
            waiting.push_back(word);
        }
    }
    type = types_map.at("object");
    for (const std::string &parameter_name: waiting) {
        parameters_map[parameter_name] = type->getParameter(parameter_name);
    }
}

void Action::add_parameter(Parameter *parameter) {
    string parameter_name = parameter->getName();
    parameters_map[parameter_name] = parameter;
    parameters_order.push_back(parameter_name);
}

std::deque<Parameter *> Action::getParameters() const {
    std::deque<Parameter *> parameters;
    for (const auto &parameter_name: parameters_order) {
        parameters.push_back(parameters_map.at(parameter_name));
    }
    return parameters;
}

Formula *Action::getEffect() const {
    return effect;
}

void Action::setEffect(Formula *new_effect) {
    this->effect = new_effect;
}

Formula *Action::getPrecondition() const {
    return precondition;
}

const std::string &Action::getName() const {
    return name;
}

std::ostream &operator<<(std::ostream &os, const Action &action) {
    os << "(:action " << action.name << "\n";
    if (!action.parameters_map.empty()) {
        os << "\t:parameters (";
        const Parameter *parameter;
        for (auto &parameter_name: action.parameters_order) {
            parameter = action.parameters_map.at(parameter_name);
            os << *parameter << " - " << parameter->getType()->getName() << ' ';
        }
        os << ")\n";
    }
    if (action.precondition != nullptr) {
        os << "\t:precondition " << *action.precondition << '\n';
    }
    os << "\t:effect " << *action.effect << "\n)";
    return os;
}

ActionType Action::getActionType() const {
    return action_type;
}

void Action::setActionType(ActionType actionType) {
    action_type = actionType;
}

void Action::setPrecondition(Formula *new_precondition) {
    this->precondition = new_precondition;
}

void Action::add_precondition(Formula *additional_precondition) {
    this->precondition->add_inner_formula(additional_precondition);
}
