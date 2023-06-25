/*
 * Formula.h
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#ifndef PROBLEM_FORMULA_H_
#define PROBLEM_FORMULA_H_

#include <ostream>
#include "FunctionFormula.h"
#include "Term.h"
#include "Parameter.h"
#include "Predicate.h"

class Formula {
private:
    std::deque<Formula *> inner_formulas;
    std::deque<FunctionFormula *> inner_function_formulas;
    std::deque<Parameter *> parameters, inner_parameters;
    Parameter *inner_parameter = nullptr;
    std::string formula_type;
    bool is_negative;
    std::set<Predicate *> related_predicates;
public:
    Formula(const Term *term, const std::map<std::string, Parameter *> &parameters_map,
            const std::map<std::string, Predicate *> &predicates_map, const std::map<std::string, Type *> &types_map,
            const std::map<std::string, PDDL_Object *> &constants_map);

    Formula(std::deque<Formula *> innerFormulas, std::deque<Parameter *> parameters,
            std::string formulaType, bool isNegative, Predicate *immediate_predicate);

    Formula(const Formula &other);

    virtual ~Formula();

    [[nodiscard]] const string &getFormulaType() const;

    void setFormulaType(const string &formulaType);

    [[nodiscard]] const std::deque<Formula *> &getInnerFormulas() const;

    bool remove_inner_formula(Formula *inner_formula);

    void clear_inner_formula();

    [[nodiscard]] const std::deque<FunctionFormula *> &getInnerFunctionFormulas() const;

    [[nodiscard]] const std::deque<Parameter *> &getParameters() const;

    [[nodiscard]] Parameter *getInnerParameter() const;

    [[nodiscard]] bool isNegative() const;

    void change_is_negative();

    [[nodiscard]] const std::set<Predicate *> &getRelatedPredicates() const;

    void add_inner_formula(Formula *formula);

    void add_inner_function_formula(FunctionFormula *functionFormula);

    [[nodiscard]] std::string to_string() const;

    friend std::ostream &operator<<(std::ostream &os, const Formula &formula1);
};

#endif /* PROBLEM_FORMULA_H_ */
