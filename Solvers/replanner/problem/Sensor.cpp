/*
 * Sensor.cpp
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#include "Sensor.h"
#include "../utils.h"

Sensor::Sensor(const Term *term, const std::map<std::string, Type *> &types_map,
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
    if (words.at(index) == ":condition") {
        inner = inner_terms->at(index++);
        condition = new Formula(inner, parameters_map, predicates_map, types_map, constants_map);
    } else {
        condition = nullptr;
    }
    inner = inner_terms->at(index);
    sense = new Formula(inner, parameters_map, predicates_map, types_map, constants_map);
    for (Predicate *predicate: sense->getRelatedPredicates()) {
        predicate->setIsObservable(true);
    }
}


Sensor::~Sensor() {
    name.clear();
    delete condition;
    delete sense;
    parameters_order.clear();
    for (const auto &parameter: parameters_map) {
        delete parameter.second;
    }
    parameters_map.clear();
}

void Sensor::parse_parameters(const Term *pTerm, const std::map<std::string, Type *> &types_map) {
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
        } else { waiting.push_back(word); }
    }
    type = types_map.at("object");
    for (const std::string &parameter_name: waiting) {
        parameters_map[parameter_name] = type->getParameter(parameter_name);
    }
}

std::deque<Parameter *> Sensor::getParameters() const {
    std::deque<Parameter *> parameters;
    for (const auto &parameter_name: parameters_order) {
        parameters.push_back(parameters_map.at(parameter_name));
    }
    return parameters;
}

Formula *Sensor::getCondition() const {
    return condition;
}

const std::string &Sensor::getName() const {
    return name;
}

Formula *Sensor::getSense() const {
    return sense;
}

std::ostream &operator<<(std::ostream &os, const Sensor &sensor) {
    os << "(:sensor " << sensor.name << "\n";
    if (!sensor.parameters_map.empty()) {
        os << "\t:parameters (";
        for (const auto &parameter: sensor.parameters_map) {
            os << *parameter.second << " - " << parameter.second->getType()->getName() << ' ';
        }
        os << ")\n";
    }
    if (sensor.condition != nullptr) {
        os << "\t:condition " << *sensor.condition << '\n';
    }
    os << "\t:sense " << *sensor.sense << "\n)";
    return os;
}

