/*
 * utils.cpp
 *
 *  Created on: Feb 2, 2023
 *      Author: david
 */

#include "utils.h"
#include <boost/filesystem.hpp>
#include <sstream>
#include <boost/algorithm/string/trim.hpp>

using namespace std;


std::deque<std::string> get_words(const std::string &sentence) {
    std::deque<std::string> words;
    std::stringstream stream;
    std::string word;
    stream << sentence;
    while (!stream.eof()) {
        stream >> word;
        words.push_back(word);
    }
    words.shrink_to_fit();
    return words;
}

bool is_whitespace(std::string string) {
    boost::trim(string);
    return string.empty();
}

std::string join_path(const std::deque<std::string> &paths) {
    boost::filesystem::path path(paths.at(0));
    size_t n = paths.size();
    for (int i = 1; i < n; ++i) {
        boost::filesystem::path tmp_path(paths.at(i));
        path = path / tmp_path;
    }
    return path.string();
}

std::deque<std::string> split(const std::string &str, const char &delimiter) {
    std::deque<std::string> result;
    std::string segment;
    std::stringstream stream(str);
    while (std::getline(stream, segment, delimiter)) {
        result.push_back(segment);
    }
    return result;
}

void replaceAll(string &str, const string &from, const string &to) {
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}


std::pair<int, int> count_parenthesis(const std::string& str) {
    int open = 0, close = 0;
    for (auto &c: str) {
        switch (c) {
            case '(':
                open++;
                break;
            case ')':
                close++;
                break;
        }
    }
    return std::make_pair(open, close);
}

std::ostream &bold_on(std::ostream &os) {
    return os << "\e[1m";
}

std::ostream &bold_off(std::ostream &os) {
    return os << "\e[0m";
}

std::deque<std::string> get_object_combinations(
        const std::deque<Type *> &types) {
    return get_object_combinations(types, 0, "");
}

std::deque<std::string> get_object_combinations(const std::deque<Type *> &types,
                                                size_t index, const std::string &prefix) {
    std::deque<std::string> combinations, inner_combinations;
    if (index < types.size()) {
        Type *type = types.at(index++);
        auto objects = type->getObjects();
        std::deque<PDDL_Object *> compatible_objects(objects.begin(), objects.end());;
        for (PDDL_Object *object: compatible_objects) {
            inner_combinations = get_object_combinations(types, index, prefix + '_' + object->getName());
            combinations.insert(combinations.end(), inner_combinations.begin(), inner_combinations.end());
        }
    } else {
        combinations.push_back(prefix);
    }
    return combinations;
}

std::deque<std::map<Parameter *, PDDL_Object *> > get_objects_combination(
        const std::deque<Parameter *> &parameters) {
    return get_objects_combination(parameters, 0);
}

std::deque<std::map<Parameter *, PDDL_Object *> > get_objects_combination(
        const std::deque<Parameter *> &parameters, size_t index) {
    Parameter *parameter = parameters.at(index);
    std::deque<PDDL_Object *> compatible_objects = parameter->getCompatibleObjects();
    std::deque<std::map<Parameter *, PDDL_Object *>> combinations;
    size_t n = parameters.size();
    if (index >= n - 1) {
        for (PDDL_Object *object: compatible_objects) {
            std::map<Parameter *, PDDL_Object *> combination;
            combination[parameter] = object;
            combinations.push_back(combination);
        }
    } else {
        auto inner_combinations = get_objects_combination(parameters, index + 1);
        for (auto inner_combination: inner_combinations) {
            for (PDDL_Object *object: compatible_objects) {
                std::map<Parameter *, PDDL_Object *> combination;
                combination[parameter] = object;
                combination.insert(inner_combination.begin(), inner_combination.end());
                combinations.push_back(combination);
            }
        }
    }
    return combinations;
}
