/*
 * FunctionFormula.h
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#ifndef PROBLEM_FUNCTIONFORMULA_H_
#define PROBLEM_FUNCTIONFORMULA_H_


#include "Function.h"
#include "Parameter.h"

class FunctionFormula {
private:
    const Function *function;
    std::deque<Parameter *> parameters;
    size_t value;

    void clear();

public:
    FunctionFormula(const Function *function, std::deque<Parameter *> parameters, size_t value);

    FunctionFormula(const FunctionFormula &other);

    virtual ~FunctionFormula();

    const Function *getFunction() const;

    size_t getValue() const;

    FunctionFormula &operator=(const FunctionFormula &other);

    FunctionFormula &operator=(const FunctionFormula &&other);

    friend std::ostream &operator<<(std::ostream &os, const FunctionFormula &functionFormula);
};

#endif /* PROBLEM_FUNCTIONFORMULA_H_ */
