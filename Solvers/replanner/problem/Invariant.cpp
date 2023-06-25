/*
 * Invariant.cpp
 *
 *  Created on: Feb 6, 2023
 *      Author: david
 */

#include "Invariant.h"
#include <sstream>

Invariant::Invariant(std::deque<Fact *> &facts) : facts(facts) {
    for (auto fact: facts) {
        fact->getPredicate()->setIsDeducible(true);
    }
}

Invariant::~Invariant() {
    facts.clear();
}

const std::deque<Fact *> &Invariant::getFacts() const {
    return facts;
}

std::string Invariant::to_string() const {
    std::stringstream stringstream;
    stringstream << *this;
    return stringstream.str();
}

std::ostream &operator<<(std::ostream &os, const Invariant &invariant) {
    os << "(invariant";
    for (Fact *fact: invariant.facts) {
        os << ' ' << *fact;
    }
    os << ')';
    return os;
}
