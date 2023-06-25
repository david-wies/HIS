/*
 * PDDLObject.cpp
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#include "PDDLObject.h"

PDDL_Object::PDDL_Object(string name, Type *type) : name(std::move(name)), object_type(type) {}

const string &PDDL_Object::getName() const {
    return name;
}

Type *PDDL_Object::getObjectType() const {
    return object_type;
}

std::ostream &operator<<(std::ostream &os, const PDDL_Object &object) {
    os << object.name;
    return os;
}
