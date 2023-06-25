/*
 * Term.h
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#ifndef TERM_H_
#define TERM_H_

#include <deque>
#include <string>
#include <map>
#include <boost/algorithm/string.hpp>

enum TermType {
    action,
    constants,
    define,
    domain,
    formula,
    goal,
    hidden,
    helper,
    init,
    objects,
    predicates,
    problem,
    requirements,
    sensor,
    types,
};

class Term {
private:
    std::string content;
    TermType type;
    std::map<TermType, std::deque<Term *> *> inners_terms;

    void calculate_type();

public:
    Term();

    Term(std::string content, TermType type,
         std::map<TermType, std::deque<Term *> *> inners);

    explicit Term(const std::string &full_content);

    virtual ~Term();

    void clear();

    const std::string &getContent() const;

    std::deque<Term *> *getInnerTerms(const TermType &termType) const;

    TermType getType() const;

    friend std::istream &operator>>(std::istream &input, Term &term);
};

#endif /* TERM_H_ */
