/*
 * utils.h
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <deque>
#include <map>
#include <iostream>

#include "problem/Type.h"

std::deque<std::string> get_words(const std::string &sentence);

bool is_whitespace(std::string string);

std::string join_path(const std::deque<std::string> &paths);

std::deque<std::string> split(const std::string &str, const char &delimiter);

void replaceAll(std::string &str, const std::string &from, const std::string &to);

std::ostream &bold_on(std::ostream &os);

std::ostream &bold_off(std::ostream &os);

std::pair<int, int> count_parenthesis(const std::string& str);

std::deque<std::string> get_object_combinations(const std::deque<Type *> &types);

std::deque<std::string>
get_object_combinations(const std::deque<Type *> &types, size_t index, const std::string &prefix);

std::deque<std::map<Parameter *, PDDL_Object *>> get_objects_combination(const std::deque<Parameter *> &parameters);

std::deque<std::map<Parameter *, PDDL_Object *>>
get_objects_combination(const std::deque<Parameter *> &parameters, size_t index);

inline string bool_to_string(const bool b) {
    return b ? "true" : "false";
}

#endif /* UTILS_H_ */
