#include "regex.h"
#include "automata.h"

#include <set>

#ifndef COURSEWORK_BINARY_TREE_H
#define COURSEWORK_BINARY_TREE_H


class BinaryTree {
public:
    BinaryTree() {}

    RegexpType type;
    union {
        struct {
            BinaryTree* left;
            BinaryTree* right;
        }; // concatenation and alternation
        BinaryTree* child; // Kleene
        char rune; // literal
    };

    string name;

    bool epsilonProducing();
    list<string> linearize(int&);
    list<string> doFIRST();
    list<string> doLAST();
    set<pair<string, string>> doFOLLOW();

    Automata* toThomson();
    Automata* toGlushkov();
};


#endif //COURSEWORK_BINARY_TREE_H
