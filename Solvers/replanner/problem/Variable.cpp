/*
 * Variable.cpp
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#include "Variable.h"

Variable::Variable(string name, Type *const type) : name(std::move(name)), type(type) {}

bool Variable::is_compatible(PDDL_Object *object) {
    return type->is_compatible(object->getObjectType());
}
