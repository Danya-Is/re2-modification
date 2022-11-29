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

    // '*'
    kleeneStar,

    // '|'
    alternation,

    // one symbol
    literal,

    //(regex | regex | ...)
    alternationExpr,

    // regex ++ regex ++ regex
    concatenationExpr,

    // root regex
    rootExpr
};

class Regexp {
public:
    RegexpType regexp_type;

//    union {
//
//    };

    char rune; // literal
    Regexp* sub_regexp; // kleene
    list<Regexp*> sub_regexps; // alternative and concatination

    Regexp() {
    };

    Regexp(RegexpType type) {
        regexp_type = type;
    }

    static Regexp* parse_regexp(string &s);

    void doConcatenation();

    void doCollapse();

    void doKleene();

    BinaryTree* to_binary_tree();
};


#endif //COURSEWORK_REGEX_H
