/*
 * Budget.cpp
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#include "Budget.h"

Budget::Budget(std::string budgetName, int budgetValue) : budget_name(std::move(budgetName)),
                                                            budget_type_name(budget_name + "_type"),
                                                            budget_value(budgetValue) {
    budget_type = new Type(budget_type_name);
}

Budget::~Budget() {
    delete budget_type;
    budget_type = nullptr;
    budget_value = 0;
}

std::string Budget::get_objects() const {
    std::string objects;
    if (budget_value > 0) {
        for (int i = 1; i <= budget_value; ++i) {
            objects += budget_name + '_' + std::to_string(i) + ' ';
        }
        objects += "- " + budget_type_name;
    }
    return objects;
}

std::string Budget::get_predicate() const {
    return "(" + budget_name + " ?b - " + budget_type_name + ')';
}

bool Budget::decrease() {
    return budget_value-- >= 0;
}

const std::string &Budget::getBudgetName() const {
    return budget_name;
}

const Type *Budget::getBudgetType() const {
    return budget_type;
}

bool Budget::does_reached_maximum() const {
    return budget_value < 0;
}
