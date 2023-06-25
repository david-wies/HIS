/*
 * Invariant.h
 *
 *  Created on: Feb 6, 2023
 *      Author: david
 */

#ifndef PROBLEM_INVARIANT_H_
#define PROBLEM_INVARIANT_H_

#include <ostream>
#include "Fact.h"

class Invariant {
private:
    std::deque<Fact *> facts;
public:
    explicit Invariant(std::deque<Fact *> &facts);

    virtual ~Invariant();

    const std::deque<Fact *> &getFacts() const;

    std::string to_string() const;

    friend std::ostream &operator<<(std::ostream &os, const Invariant &invariant);
};

#endif /* PROBLEM_INVARIANT_H_ */
