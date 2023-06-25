/*
 * Fact.h
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#ifndef PROBLEM_FACT_H_
#define PROBLEM_FACT_H_

#include <ostream>
#include "Predicate.h"
#include "PDDLObject.h"

class Predicate;


class Fact {
private:
    Predicate *predicate;
    std::deque<PDDL_Object *> objects;
    bool state;
public:
    Fact(Predicate *predicate, std::deque<PDDL_Object *> objects, bool state);

    Fact(const Fact &other);

    virtual ~Fact();

    Predicate *getPredicate();

    const std::deque<PDDL_Object *> &getObjects() const;

    bool getState() const;

    bool isTheSame(const Fact &fact) const;

    bool operator==(const Fact &rhs) const;

    bool operator!=(const Fact &rhs) const;

    std::string to_string() const;

    friend std::ostream &operator<<(std::ostream &os, const Fact &fact);
};

#endif /* PROBLEM_FACT_H_ */
