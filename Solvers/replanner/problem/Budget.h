/*
 * Budget.h
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#ifndef PROBLEM_BUDGET_H_
#define PROBLEM_BUDGET_H_

#include <string>
#include "Type.h"

class Budget {
private:
    const std::string budget_name, budget_type_name;
    int budget_value;
    Type *budget_type;
public:
    Budget(std::string budgetName, int budgetValue);

    virtual ~Budget();

    const std::string &getBudgetName() const;

    std::string get_objects() const;

    std::string get_predicate() const;

    const Type *getBudgetType() const;

    bool decrease();

    bool does_reached_maximum() const;
};

#endif /* PROBLEM_BUDGET_H_ */
