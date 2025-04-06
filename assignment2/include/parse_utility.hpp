
#ifndef UTILITY_H
#define UTILITY_H
#include <iostream>
#include <string>
#include <utility>
#include <regex>

using namespace std;

inline pair<long, long> parseRange(const string &rangeStr) {
    regex pattern(R"(^\s*(\d+)\s*-\s*(\d+)\s*$)");
    smatch match;

    if (!regex_match(rangeStr, match, pattern)) {
        cerr << "Not valid range format: " << rangeStr << endl;
        exit(EXIT_FAILURE);
    }
    long start = stol(match[1].str());
    long end = stol(match[2].str());
    return {start, end};
}

inline int parse_int(const char *arg, const string &option) {
    try {
        return stol(arg);
    } catch (const invalid_argument &e) {
        cerr << "Invalid argument for " << option << ": must be an integer." << endl;
        exit(EXIT_FAILURE);
    } catch (const out_of_range &e) {
        cerr << "Out of range value for " << option << ": too big for long." << endl;
        exit(EXIT_FAILURE);
    }
}

#endif //UTILITY_H
