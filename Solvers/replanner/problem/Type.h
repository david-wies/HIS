/*
 * Type.h
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#ifndef PROBLEM_TYPE_H_
#define PROBLEM_TYPE_H_

#include <string>
#include <set>
#include <ostream>

#include "Parameter.h"
#include "PDDLObject.h"

using std::string;

class Parameter;

class PDDL_Object;

class Type {
private:
    const string name;
    Type *const parent;
    std::set<Type *> descendants;
    std::set<PDDL_Object *> objects;
public:
    Type(string name, Type *parent);

    explicit Type(string name);

    void add_descendant(Type *descendant);

    void add_object(PDDL_Object *object);

    bool is_compatible(Type *type);

    [[nodiscard]] std::set<PDDL_Object *> getObjects() const;

    [[nodiscard]] const string &getName() const;

    [[nodiscard]] Parameter *getParameter(const string &parameter_name) const;

    friend std::ostream &operator<<(std::ostream &os, const Type &type);
};

#endif /* PROBLEM_TYPE_H_ */
