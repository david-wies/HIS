/*
 * Type.cpp
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#include "Type.h"

Type::Type(string name, Type *const parent) : name(std::move(name)), parent(parent) {
    if (parent != nullptr) {
        parent->add_descendant(this);
    }
}

Type::Type(string name) : Type(std::move(name), nullptr) {}

void Type::add_descendant(Type *descendant) {
    descendants.insert(descendant);
}

void Type::add_object(PDDL_Object *object) {
    objects.insert(object);
}

bool Type::is_compatible(Type *type) {
    return descendants.find(type) != descendants.end();
}

const string &Type::getName() const {
    return name;
}

std::set<PDDL_Object *> Type::getObjects() const {
    std::set<PDDL_Object *> all_objects(objects);
    for (auto descendant: descendants) {
        auto inner_descendants = descendant->getObjects();
        all_objects.insert(inner_descendants.begin(), inner_descendants.end());
    }
    return all_objects;
}

Parameter *Type::getParameter(const string &parameter_name) const {
    return new Parameter(this, std::deque<PDDL_Object *>(objects.begin(), objects.end()), parameter_name);
}

std::ostream &operator<<(std::ostream &os, const Type &type) {
    os << type.name;
    if (type.parent != nullptr) {
        os << " - " << type.parent->name;
    }
    return os;
}
