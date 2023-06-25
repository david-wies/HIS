/*
 * Function.cpp
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#include "Function.h"

Function::Function(string name, std::deque<PDDL_Object *> objects, size_t functionValue) : function_value(
        functionValue), name(std::move(name)), objects(std::move(objects)) {}

Function::Function(const std::string &name) : Function(name, std::deque<PDDL_Object *>(), 0) {}

size_t Function::getFunctionValue() const {
    return function_value;
}

void Function::setFunctionValue(size_t functionValue) {
    function_value = functionValue;
}

const string &Function::getFunctionMetric() const {
    return function_metric;
}

void Function::setFunctionMetric(const string &metric) {
    if (metric.empty()) {
        has_metric = false;
        function_metric.clear();
    } else {
        function_metric = metric;
        has_metric = true;
    }
}

const string &Function::getName() const {
    return name;
}

bool Function::isHasMetric() const {
    return has_metric;
}

void Function::print_value(std::ostream &os) const {
    os << "(= " << *this << ' ' << function_value << ')';
}

void Function::print_metric(std::ostream &os) const {
    if (has_metric) {
        os << "(:metric " << function_metric << ' ' << *this << ')';
    }
}

void Function::increase(const size_t &value) {
    function_value += value;
}

std::ostream &operator<<(std::ostream &os, const Function &function) {
    os << '(' << function.name;
    for (PDDL_Object *object: function.objects) {
        os << ' ' << object->getName();
    }
    os << ')';
    return os;
}
