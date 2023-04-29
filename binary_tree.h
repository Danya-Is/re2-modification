#include "regex.h"
#include "automata.h"

#include <set>

#ifndef COURSEWORK_BINARY_TREE_H
#define COURSEWORK_BINARY_TREE_H


class BinaryTree {
public:
    BinaryTree() {}
    BinaryTree(RegexpType type) {
        this->type = type;
    }

    RegexpType type;
    union {
        struct {
            BinaryTree* left;
            BinaryTree* right;
        }; // concatenation and alternation
        BinaryTree* child; // Kleene
        char rune; // literal
    };
    string variable; // reference and backreferenceExpr
    string name;

    // using for constructing ssnf
    bool epsilon_producing = false;

    bool epsilonProducing();
    list<string> linearize(int&);
    list<string> doFIRST();
    list<string> doLAST();
    set<pair<string, string>> doFOLLOW();

    BinaryTree* toSSNF();
    BinaryTree* toSSNF_();
    BinaryTree* underKleene();
    BinaryTree* checkEpsChilds();

    Automata* toThomson();
    Automata* toGlushkov();
    MFA* toMFA();
};

std::string substr(std::string originalString, int maxLength);


#endif //COURSEWORK_BINARY_TREE_H
