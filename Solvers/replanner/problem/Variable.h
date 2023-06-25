/*
 * Variable.h
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#ifndef PROBLEM_VARIABLE_H_
#define PROBLEM_VARIABLE_H_

#include <string>
#include "Type.h"
#include "PDDLObject.h"

class Variable {
private:
    const std::string name;
    Type *const type;
public:
    Variable(string name, Type *type);

    bool is_compatible(PDDL_Object *object);
};

#endif /* PROBLEM_VARIABLE_H_ */
