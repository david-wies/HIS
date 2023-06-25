/*
 * Term.cpp
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#include "Term.h"
#include "../utils.h"
#include <regex>

Term::Term() {
    TermType termType;
    for (int i = action; i <= types; ++i) {
        termType = (TermType) i;
        inners_terms[termType] = new std::deque<Term *>;
    }
    type = formula;
}

Term::Term(std::string content, TermType type,
           std::map<TermType, std::deque<Term *> *> inners) : content(std::move(
        content)), type(type), inners_terms(std::move(inners)) {}

Term::Term(const std::string &full_content) : Term() {
    std::string sub_term, tmp;
    Term *inner_term;
    TermType termType;
    size_t starting_index = 1, index_start_sub;
    int count_opening, count_closing;
    while ((index_start_sub = full_content.find('(', starting_index)) != std::string::npos) {
        content += full_content.substr(starting_index, index_start_sub - starting_index) + " ";
        do {
            starting_index = full_content.find(')', starting_index + 1);
            sub_term = full_content.substr(index_start_sub, starting_index - index_start_sub + 1);
            auto count = count_parenthesis(sub_term);
            count_opening = count.first;
            count_closing = count.second;
        } while (count_opening != count_closing);
        starting_index++;
        inner_term = new Term(sub_term);
        termType = inner_term->type;
        inners_terms.at(termType)->push_back(inner_term);
    }
    content += full_content.substr(starting_index, full_content.size() - starting_index - 1);
    boost::trim(content);
    calculate_type();
}

Term::~Term() {
    for (auto &inner: inners_terms) {
        for (auto &term: *inner.second) {
            delete term;
        }
        inner.second->clear();
        delete inner.second;
    }
    inners_terms.clear();
    type = formula;
    content.clear();
}

void Term::clear() {
    for (auto &inner: inners_terms) {
        for (auto &term: *inner.second) {
            delete term;
        }
        inner.second->clear();
    }
    type = formula;
    content.clear();
}

std::istream &operator>>(std::istream &input, Term &term) {
    term.clear();
    static const std::regex space_pattern("\\s+");

    std::string sub_content, tmp_full_content = "(", full_content, content, tmp;
    Term *inner_term;
    TermType termType;
    std::getline(input, tmp, '(');
    tmp = "";
    int count_opening, count_closing;
    do {
        std::getline(input, tmp, ')');
        tmp_full_content += tmp + ")";
        auto count = count_parenthesis(tmp_full_content);
        count_opening = count.first;
        count_closing = count.second;
    } while (count_closing != count_opening);

    std::regex_replace(std::back_inserter(full_content), tmp_full_content.begin(), tmp_full_content.end(),
                       space_pattern, " ");
    size_t starting_index = 1, index_start_sub;
    while ((index_start_sub = full_content.find('(', starting_index)) != std::string::npos) {
        content += full_content.substr(starting_index, index_start_sub - starting_index) + " ";
        do {
            starting_index = full_content.find(')', starting_index + 1);
            sub_content = full_content.substr(index_start_sub, starting_index - index_start_sub + 1);
            auto count = count_parenthesis(sub_content);
            count_opening = count.first;
            count_closing = count.second;
        } while (count_opening != count_closing);
        starting_index++;
        inner_term = new Term(sub_content);
        termType = inner_term->type;
        term.inners_terms.at(termType)->push_back(inner_term);
    }
    boost::trim(content);
    term.content = content;
    term.calculate_type();
    return input;
}

const std::string &Term::getContent() const {
    return content;
}

std::deque<Term *> *Term::getInnerTerms(const TermType &termType) const {
    return inners_terms.at(termType);
}

TermType Term::getType() const {
    return type;
}

void Term::calculate_type() {
    std::string type_word = get_words(content).at(0);
    if (type_word.empty()) {
        type = formula;
        return;
    }
    if (type_word.at(0) == ':') {
        type_word = type_word.substr(1);
    }
    if (type_word == "domain") {
        type = domain;
    } else if (type_word == "problem") {
        type = problem;
    } else if (type_word == "define") {
        type = define;
    } else if (type_word == "action") {
        type = action;
    } else if (type_word == "sensor")
        type = sensor;
    else if (type_word == "requirements") {
        type = requirements;
    } else if (type_word == "types") {
        type = types;
    } else if (type_word == "predicates") {
        type = predicates;
    } else if (type_word == "objects") {
        type = objects;
    } else if (type_word == "constants") {
        type = constants;
    } else if (type_word == "init") {
        type = init;
    } else if (type_word == "goal") {
        type = goal;
    } else if (type_word == "hidden") {
        type = hidden;
    } else if (type_word == "helper") {
        type = helper;
    } else {
        type = formula;
    }
}
