/*
 * Parameter.h
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#ifndef PROBLEM_PARAMETER_H_
#define PROBLEM_PARAMETER_H_

#include "Type.h"
#include "PDDLObject.h"

#include <deque>

class Type;

class PDDL_Object;

class Parameter {
private:
    const Type *type;
    std::deque<PDDL_Object *> compatible_objects;
    const std::string name;
public:
    Parameter(const Type *type, std::deque<PDDL_Object *> compatibleObjects, std::string name);

    Parameter(const Parameter &parameter);

    void remove_compatible_object(PDDL_Object *object);

    void add_compatible_object(PDDL_Object *object);

    const std::deque<PDDL_Object *> &getCompatibleObjects() const;

    const Type *getType() const;

    const std::string &getName() const;

    friend std::ostream &operator<<(std::ostream &os, const Parameter &parameter);
};

#endif /* PROBLEM_PARAMETER_H_ */
