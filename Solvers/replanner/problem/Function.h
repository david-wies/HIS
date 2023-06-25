/*
 * Function.h
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#ifndef PROBLEM_FUNCTION_H_
#define PROBLEM_FUNCTION_H_

#include <ostream>
#include "PDDLObject.h"

class Function {
private:
    const std::string name;
    std::string function_metric;
    bool has_metric = false;
    size_t function_value = 0;
    const std::deque<PDDL_Object *> objects;
public:
    Function(string name, std::deque<PDDL_Object *> objects, size_t functionValue);

    Function(const Function &other) = default;

    explicit Function(const std::string &name);

    size_t getFunctionValue() const;

    void setFunctionValue(size_t functionValue);

    const string &getFunctionMetric() const;

    void setFunctionMetric(const string &metric);

    const string &getName() const;

    bool isHasMetric() const;

    void print_value(std::ostream &os) const;

    void print_metric(std::ostream &os) const;

    void increase(const size_t &value);

    friend std::ostream &operator<<(std::ostream &os, const Function &function);
};

#endif /* PROBLEM_FUNCTION_H_ */
