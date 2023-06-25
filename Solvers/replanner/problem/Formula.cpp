/*
 * Formula.cpp
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#include "Formula.h"
#include "../utils.h"
#include <sstream>

Formula::Formula(const Term *term, const std::map<std::string, Parameter *> &parameters_map,
                 const std::map<std::string, Predicate *> &predicates_map,
                 const std::map<std::string, Type *> &types_map,
                 const std::map<std::string, PDDL_Object *> &constants_map) {
    auto parts = get_words(term->getContent());
    Parameter *parameter;
    PDDL_Object *constant;
    if (parts.at(0) == "not") {
        is_negative = true;
        term = term->getInnerTerms(TermType::formula)->at(0);
        parts = get_words(term->getContent());
    } else { is_negative = false; }
    formula_type = std::string(parts.at(0));
    auto inners = term->getInnerTerms(TermType::formula);
    if (inners->empty()) {
        auto inherited_predicate = predicates_map.at(formula_type);
        related_predicates.insert(inherited_predicate);
        size_t n = parts.size();
        std::string parameter_name;
        for (size_t i = 1; i < n; ++i) {
            parameter_name = parts.at(i);
            if (parameter_name.at(0) == '?')
                parameter = parameters_map.at(parameter_name);
            else {
                constant = constants_map.at(parameter_name);
                parameter = new Parameter(constant->getObjectType(), {constant}, parameter_name);
                inner_parameters.push_back(parameter);
            }
            parameters.push_back(parameter);
        }
    } else {
        Formula *inner_formula;
        std::set<Predicate *> inner_related_predicates;
        size_t index = 0;
        auto end = inners->size();
        auto inner_parameters_map = std::map<std::string, Parameter *>(parameters_map);
        if ((parts.at(0) == "forall") || (parts.at(0) == "exists")) {
            auto parameter_term = inners->at(0);
            index++;
            auto parameter_parts = get_words(parameter_term->getContent());
            std::string parameter_name = parameter_parts.at(0);
            parameter = types_map.at(parameter_parts.at(2))->getParameter(parameter_name);
            inner_parameters_map[parameter_name] = parameter;
            inner_parameter = parameter;
            inner_parameters.push_back(parameter);
        }
        for (; index < end; index++) {
            inner_formula = new Formula(inners->at(index), inner_parameters_map, predicates_map, types_map,
                                        constants_map);
            inner_formulas.push_back(inner_formula);
            inner_related_predicates = inner_formula->related_predicates;
            related_predicates.insert(inner_related_predicates.begin(), inner_related_predicates.end());
        }
        for (auto inner: inner_formulas) {
            for (auto param: inner->parameters) {
                if (std::find(parameters.begin(), parameters.end(), param) == parameters.end()) {
                    parameters.push_back(param);
                }
            }
        }
    }
}

Formula::Formula(std::deque<Formula *> innerFormulas, std::deque<Parameter *> parameters, std::string formulaType,
                 bool isNegative, Predicate *immediate_predicate) : inner_formulas(std::move(innerFormulas)),
                                                                    parameters(std::move(parameters)),
                                                                    formula_type(std::move(formulaType)),
                                                                    is_negative(isNegative) {
    if (immediate_predicate != nullptr) {
        related_predicates.insert(immediate_predicate);
    }
    std::set<Predicate *> inner_related_predicates;
    for (const auto inner_formula: this->inner_formulas) {
        inner_related_predicates = inner_formula->related_predicates;
        related_predicates.insert(inner_related_predicates.begin(), inner_related_predicates.end());
    }
}

Formula::Formula(const Formula &other) {
    is_negative = other.is_negative;
    parameters = std::deque<Parameter *>(other.parameters);
    formula_type = std::string(other.formula_type);
    auto another_related_predicates = other.related_predicates;
    related_predicates.insert(another_related_predicates.begin(), another_related_predicates.end());
    for (auto inner: other.inner_formulas) {
        inner_formulas.push_back(new Formula(*inner));
    }
    for (auto inner: other.inner_function_formulas) {
        inner_function_formulas.push_back(new FunctionFormula(*inner));
    }
}

Formula::~Formula() {
    for (auto &inner_formula: inner_formulas) {
        delete inner_formula;
    }
    inner_formulas.clear();

    for (auto &inner_function_formula: inner_function_formulas) {
        delete inner_function_formula;
    }
    inner_function_formulas.clear();

    for (const auto *parameter: inner_parameters) {
        delete parameter;
    }
    inner_parameters.clear();

    inner_parameter = nullptr;
    parameters.clear();

    related_predicates.clear();

    formula_type.clear();
}

void Formula::change_is_negative() {
    is_negative = !is_negative;
}

void Formula::add_inner_formula(Formula *formula) {
    if ((formula_type == "and") || (formula_type == "or")) {
        inner_formulas.push_back(formula);
    } else {
        auto *copy = new Formula(*this);
        for (auto inner: inner_formulas) {
            delete inner;
        }
        inner_formulas.clear();
        inner_formulas.push_back(copy);
        inner_formulas.push_back(formula);
        parameters.clear();
        formula_type = "and";
        is_negative = false;
        auto &formula_related_predicates = formula->related_predicates;
        related_predicates.insert(formula_related_predicates.begin(), formula_related_predicates.end());
    }
}

void Formula::add_inner_function_formula(FunctionFormula *functionFormula) {
    if (formula_type != "and") {
        auto *copy = new Formula(*this);
        for (auto inner: inner_formulas) {
            delete inner;
        }
        inner_formulas.clear();
        inner_formulas.push_back(copy);
        formula_type = "and";
        is_negative = false;
        parameters.clear();
    }
    inner_function_formulas.push_back(functionFormula);
}

bool Formula::remove_inner_formula(Formula *inner_formula) {
    auto inner_it = std::find(inner_formulas.begin(), inner_formulas.end(), inner_formula);
    if (inner_it == inner_formulas.end()) {
        return false;
    }
    related_predicates.clear();
    std::set<Predicate *> inner_related_predicates;
    for (auto it = inner_formulas.begin(); it != inner_formulas.end(); ++it) {
        if (it != inner_it) {
            inner_related_predicates = (*it)->related_predicates;
            related_predicates.insert(inner_related_predicates.begin(), inner_related_predicates.end());
        }
    }
    inner_formulas.erase(inner_it);
    return true;
}

void Formula::clear_inner_formula() {
    for (auto inner: inner_formulas) {
        delete inner;
    }
    inner_formulas.clear();
    related_predicates.clear();
}

const std::string &Formula::getFormulaType() const {
    return formula_type;
}

void Formula::setFormulaType(const std::string &formulaType) {
    formula_type = formulaType;
}

const std::deque<Formula *> &Formula::getInnerFormulas() const {
    return inner_formulas;
}

const std::deque<FunctionFormula *> &Formula::getInnerFunctionFormulas() const {
    return inner_function_formulas;
}

Parameter *Formula::getInnerParameter() const {
    return inner_parameter;
}

bool Formula::isNegative() const {
    return is_negative;
}

const std::deque<Parameter *> &Formula::getParameters() const {
    return parameters;
}

const std::set<Predicate *> &Formula::getRelatedPredicates() const {
    return related_predicates;
}

std::string Formula::to_string() const {
    std::stringstream stringstream;
    stringstream << *this;
    return stringstream.str();
}

std::ostream &operator<<(std::ostream &os, const Formula &formula) {
    std::string end = ")";
    if (formula.is_negative) {
        os << "(not ";
        end = "))";
    }
    os << '(' << formula.formula_type;
    std::deque<Formula *> innerFormulas = formula.inner_formulas;
    auto &innerFunctionFormulas = formula.inner_function_formulas;
    if (innerFormulas.empty() && innerFunctionFormulas.empty()) {
        for (Parameter *parameter: formula.parameters) {
            os << ' ' << *parameter;
        }
    } else {
        Parameter *innerParameter = formula.inner_parameter;
        if (innerParameter != nullptr) {
            os << " (" << *innerParameter << " - " << innerParameter->getType()->getName() << ')';
        }
        for (auto *inner_formula: innerFormulas) {
            os << ' ' << *inner_formula;
        }
        for (auto *inner_function_formula: innerFunctionFormulas) {
            os << ' ' << *inner_function_formula;
        }
    }
    os << end;
    return os;
}
