/*
 * Predicate.cpp
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#include "Predicate.h"
#include "../utils.h"

Predicate::Predicate(const Term &term, const std::map<std::string, Type *> &types_map) {
    auto parts = get_words(term.getContent());
    std::string word;
    auto n = parts.size();
    short type_variables_counter = 0, j;
    name = parts.at(0);
    for (size_t i = 1; i < n; ++i) {
        word = parts.at(i);
        if (word == "-") {
            word = parts.at(++i);
            for (j = 0; j < type_variables_counter; ++j) {
                variables.push_back(types_map.at(word));
            }
            type_variables_counter = 0;
        } else {
            type_variables_counter++;
        }
    }
    for (j = 0; j < type_variables_counter; ++j) {
        const string object_type = "object";
        variables.push_back(types_map.at(object_type));
    }
}

Predicate::Predicate(string name, bool isChangeable) : name(std::move(name)), is_changeable(isChangeable) {}

bool Predicate::isChangeable() const {
    return is_changeable;
}

void Predicate::setIsChangeable(bool isChangeable) {
    is_changeable = isChangeable;
}

bool Predicate::isDeducible() const {
    return is_deducible;
}

void Predicate::setIsDeducible(bool isDeducible) {
    is_deducible = isDeducible;
}

bool Predicate::isObservable() const {
    return is_observable;
}

void Predicate::setIsObservable(bool isObservable) {
    is_observable = isObservable;
}

const std::string &Predicate::getName() const {
    return name;
}

const std::deque<Type *> &Predicate::getVariables() const {
    return variables;
}

bool Predicate::isIsAcquirable() const {
    return is_acquirable;
}

void Predicate::setIsAcquirable(bool isAcquirable) {
    is_acquirable = isAcquirable;
}

void Predicate::add_derived_predicate(Predicate *predicate) {
    derived_predicates.insert(predicate);
    predicate->setIsChangeable(is_changeable);
    predicate->setIsDeducible(is_deducible);
    predicate->setIsObservable(is_observable);
    predicate->setIsAcquirable(is_acquirable);
}

void Predicate::erase_derived_predicate(Predicate *predicate) {
    auto it = derived_predicates.find(predicate);
    if (it != derived_predicates.end()) {
        derived_predicates.erase(it);
    }
}

std::ostream &operator<<(std::ostream &os, const Predicate &predicate) {
    os << '(' << predicate.getName();
    auto variables = predicate.variables;
    auto n = variables.size();
    for (size_t i = 0; i < n; ++i) {
        os << " ?p" << i << " - " << variables.at(i)->getName();
    }
    os << ')';
    return os;
}
