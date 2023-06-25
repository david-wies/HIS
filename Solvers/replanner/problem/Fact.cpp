/*
 * Fact.cpp
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#include "Fact.h"
#include <sstream>

Fact::Fact(Predicate *const predicate, std::deque<PDDL_Object *> objects, bool state) : predicate(predicate),
                                                                                        objects(std::deque<PDDL_Object *>(
                                                                                                std::move(objects))),
                                                                                        state(state) {}

Fact::Fact(const Fact &other) :
        predicate(other.predicate), objects(
        std::deque<PDDL_Object *>(other.objects)), state(other.state) {
}

Fact::~Fact() {
    objects.clear();
    predicate = nullptr;
}

bool Fact::isTheSame(const Fact &fact) const {
    return (predicate == fact.predicate) && (objects == fact.objects);
}

bool Fact::operator==(const Fact &rhs) const {
    return isTheSame(rhs) && state == rhs.state;
}

bool Fact::operator!=(const Fact &rhs) const {
    return !(*this == rhs);
}

const std::deque<PDDL_Object *> &Fact::getObjects() const {
    return objects;
}

Predicate *Fact::getPredicate() {
    return predicate;
}

bool Fact::getState() const {
    return state;
}

std::ostream &operator<<(std::ostream &os, const Fact &fact) {
    short parenthesis = 1;
    if (!fact.state) {
        os << "(not ";
        parenthesis = 2;
    }
    os << '(' << fact.predicate->getName();
    for (const auto &object: fact.objects) {
        os << ' ' << object->getName();
    }
    os << std::string(parenthesis, ')');
    return os;
}

std::string Fact::to_string() const {
    std::stringstream stringstream;
    stringstream << *this;
    return stringstream.str();
}
