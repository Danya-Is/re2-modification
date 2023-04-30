#ifndef COURSEWORK_REGEX_H
#define COURSEWORK_REGEX_H

#include <string>

#include "list"

#include "automata.h"

using namespace std;

class BinaryTree;

enum RegexpType {
    // '('
    leftParenthesis,

    // ')'
    rightParenthesis,

    // '{'
    leftBrace,

    // '}'
    rightBrace,

    // '['
    leftSquareBr,

    // ']'
    rightSquareBr,

    // '-'
    dash,

    // '*'
    kleeneStar,

    // '+'
    kleenePlus,

    // '|'
    alternation,

    // one symbol
    literal,

    // empty string
    epsilon,

    //(regex | regex | ...)
    alternationExpr,

    // regex ++ regex ++ regex
    concatenationExpr,

    // backreference expression
    backreferenceExpr,

    // reference to expression
    reference,

    // root regex
    rootExpr
};

class Regexp {
public:
    RegexpType regexp_type;
    string regexp_str;


    char rune; // literal
    Regexp* sub_regexp; // kleene
    list<Regexp*> sub_regexps; // alternative and concatination
    string variable; // backreferenceExpr expression

    Regexp() {
    };

    Regexp(RegexpType type) {
        regexp_type = type;
    }

    static Regexp* parse_regexp(string &s);

    void doConcatenation();

    void doCollapse();

    void doKleene(char c);

    void doBackreference(string name);

    void doEnumeration();

    BinaryTree* to_binary_tree();

    Automata* compile();

    bool match(const string& input_str);
};

void match(string regexp_str);

void match_gt(string regexp_str);

void match_mfa(string regexp_str);

#endif //COURSEWORK_REGEX_H
