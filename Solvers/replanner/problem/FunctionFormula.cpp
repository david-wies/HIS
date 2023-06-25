/*
 * FunctionFormula.cpp
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#include "FunctionFormula.h"

FunctionFormula::FunctionFormula(const Function *function, std::deque<Parameter *> parameters, size_t value) : function(
        function), parameters(std::move(parameters)), value(value) {}

FunctionFormula::FunctionFormula(const FunctionFormula &other) : FunctionFormula(other.function, other.parameters,
                                                                                 other.value) {}

const Function *FunctionFormula::getFunction() const {
    return function;
}

FunctionFormula::~FunctionFormula() {
    clear();
}

void FunctionFormula::clear() {
    function = nullptr;
    for (const auto *parameter: parameters) {
        delete parameter;
    }
    parameters.clear();
    value = 0;
}

size_t FunctionFormula::getValue() const {
    return value;
}

FunctionFormula &FunctionFormula::operator=(const FunctionFormula &other) {
    clear();
    function = other.function;
    for (auto *parameter: other.parameters) {
        parameters.push_back(new Parameter(*parameter));
    }
    value = other.value;
    return *this;
}

FunctionFormula &FunctionFormula::operator=(const FunctionFormula &&other) {
    clear();
    function = other.function;
    value = other.value;
    parameters = other.parameters;
    return *this;
}

std::ostream &operator<<(std::ostream &os, const FunctionFormula &functionFormula) {
    os << "(increase " << *functionFormula.function << ' ' << functionFormula.value << ')';
    return os;
}
