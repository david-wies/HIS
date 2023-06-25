/*
 * PDDLObject.h
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#ifndef PROBLEM_PDDLOBJECT_H_
#define PROBLEM_PDDLOBJECT_H_

#include <string>
#include <ostream>

#include "Type.h"

using std::string;

class Type;

class PDDL_Object {
private:
    const string name;
    Type *const object_type;

public:
    PDDL_Object(string name, Type *type);

    [[nodiscard]] const string &getName() const;

    [[nodiscard]] Type *getObjectType() const;

    friend std::ostream &operator<<(std::ostream &os, const PDDL_Object &object);
};

#endif /* PROBLEM_PDDLOBJECT_H_ */
