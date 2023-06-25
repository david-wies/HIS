/*
 * Parameter.cpp
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#include "Parameter.h"

Parameter::Parameter(const Type *type,
                     std::deque<PDDL_Object *> compatibleObjects, std::string name) :
        type(type), compatible_objects(std::move(compatibleObjects)), name(
        std::move(name)) {
}

Parameter::Parameter(const Parameter &parameter) = default;

void Parameter::remove_compatible_object(PDDL_Object *object) {
    auto end = compatible_objects.end();
    for (auto it = compatible_objects.begin(); it != end;) {
        if (*it == object) {
            compatible_objects.erase(it);
            break;
        } else {
            it++;
        }
    }
}

void Parameter::add_compatible_object(PDDL_Object *object) {
    compatible_objects.push_back(object);
}

const Type *Parameter::getType() const {
    return type;
}

const std::deque<PDDL_Object *> &Parameter::getCompatibleObjects() const {
    return compatible_objects;
}

const std::string &Parameter::getName() const {
    return name;
}

std::ostream &operator<<(std::ostream &os, const Parameter &parameter) {
    os << parameter.name;
    return os;
}
