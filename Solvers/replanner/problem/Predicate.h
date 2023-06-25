/*
 * Predicate.h
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#ifndef PROBLEM_PREDICATE_H_
#define PROBLEM_PREDICATE_H_

#include <ostream>

#include "Type.h"
#include "Fact.h"
#include "Parameter.h"
#include "Term.h"

class Predicate {
private:
    std::set<Predicate *> derived_predicates;
protected:
    std::deque<Type *> variables;
    std::string name;
    bool is_changeable = false, is_observable = false, is_deducible = false, is_acquirable = false;
public:
    Predicate(const Term &term, const std::map<std::string, Type *> &types_map);

    Predicate(string name, bool isChangeable);

    [[nodiscard]] bool isChangeable() const;

    void setIsChangeable(bool isChangeable);

    [[nodiscard]] bool isDeducible() const;

    void setIsDeducible(bool isDeducible);

    [[nodiscard]] bool isObservable() const;

    void setIsObservable(bool isObservable);

    [[nodiscard]] bool isIsAcquirable() const;

    void setIsAcquirable(bool isAcquirable);

    [[nodiscard]] const std::deque<Type *> &getVariables() const;

    [[nodiscard]] const std::string &getName() const;

    void add_derived_predicate(Predicate *predicate);

    void erase_derived_predicate(Predicate *predicate);

    friend std::ostream &operator<<(std::ostream &os, const Predicate &predicate);
};

#endif /* PROBLEM_PREDICATE_H_ */
