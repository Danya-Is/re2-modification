#include <string>

#include "list"

#ifndef COURSEWORK_REGEX_H
#define COURSEWORK_REGEX_H

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

    void doKleene();

    void doBackreference(string name);

    BinaryTree* to_binary_tree();
};


#endif //COURSEWORK_REGEX_H
