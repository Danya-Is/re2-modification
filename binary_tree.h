#include "regex.h"
#include "automata.h"

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

    Automata* toNFA();
};


#endif //COURSEWORK_BINARY_TREE_H
