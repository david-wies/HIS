/*
 * Sensor.h
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#ifndef PROBLEM_SENSOR_H_
#define PROBLEM_SENSOR_H_

#include <ostream>
#include "Formula.h"
#include "Type.h"
#include "Parameter.h"

class Type;

class Sensor {
private:
    std::string name;
    std::map<std::string, Parameter *> parameters_map;
    Formula *condition, *sense;
    std::deque<std::string> parameters_order;

    void parse_parameters(const Term *pTerm, const std::map<std::string, Type *> &types_map);

public:
    Sensor(const Term *term, const std::map<std::string, Type *> &types_map,
           const std::map<std::string, Predicate *> &predicates_map,
           const std::map<std::string, PDDL_Object *> &constants_map);

    virtual ~Sensor();

    const std::string &getName() const;

    Formula *getCondition() const;

    Formula *getSense() const;

    std::deque<Parameter *> getParameters() const;

    friend std::ostream &operator<<(std::ostream &os, const Sensor &sensor1);
};

#endif /* PROBLEM_SENSOR_H_ */
