#include "binary_tree.h"
#include <set>

using namespace std;

BinaryTree* BinaryTree::checkEpsChilds() {
    if (left == nullptr) {
        return right;
    }
    else if (right == nullptr) {
        return left;
    }
    else {
        return this;
    }
}

BinaryTree* BinaryTree::underKleene() {
    if (type == epsilon) {
        return nullptr;
    }
    else if (type == literal) {
        auto* new_t = new BinaryTree(literal);
        new_t->rune = rune;
        return new_t;
    }
    else if (type == reference) {
        auto *new_t = new BinaryTree(reference);
        new_t->variable = variable;
        return new_t;
    }
    else if (type == concatenationExpr) {
        bool left_eps = left->epsilonProducing();
        bool right_eps = right->epsilonProducing();
        if (!left_eps && !right_eps) {
            auto* new_t = new BinaryTree(concatenationExpr);
            new_t->left = left;
            new_t->right = right;
            return new_t;
        }
        else if (left_eps && right_eps) {
            auto* new_t = new BinaryTree(alternationExpr);
            new_t->left = left->underKleene();
            new_t->right = right->underKleene();
            new_t = new_t->checkEpsChilds();
            return new_t;
        }
        else {
            auto* new_t = new BinaryTree(concatenationExpr);
            new_t->left = left->toSSNF();
            new_t->right = right->toSSNF();
            return new_t;
        }
    }
    else if (type == alternationExpr) {
        auto* new_t = new BinaryTree(alternationExpr);
        new_t->left = left->toSSNF();
        new_t->right = right->toSSNF();
        new_t = new_t->checkEpsChilds();
        return new_t;
    }
    else if (type == kleeneStar or type == kleenePlus) {
        auto* new_t = child->underKleene();
        return new_t;
    }
    else if (type == backreferenceExpr) {
        auto* new_t = new BinaryTree(backreferenceExpr);
        new_t->child = child->underKleene();
        return new_t;
    }
}

BinaryTree* BinaryTree::toSSNF() {
    if (type == epsilon) {
        return this;
    }
    else if (type == literal) {
        auto* new_t = new BinaryTree(literal);
        new_t->rune = rune;
        return new_t;
    }
    else if (type == reference) {
        auto* new_t = new BinaryTree(reference);
        new_t->variable = variable;
        return new_t;
    }
    else if (type == concatenationExpr) {
        auto* new_t = new BinaryTree(concatenationExpr);
        new_t->left = left->toSSNF();
        new_t->right = right->toSSNF();
        new_t = new_t->checkEpsChilds();
        return new_t;
    }
    else if (type == alternationExpr) {
        auto* new_t = new BinaryTree(alternationExpr);
        new_t->left = left->toSSNF();
        new_t->right = right->toSSNF();
        new_t = new_t->checkEpsChilds();
        return new_t;
    }
    else if (type == kleeneStar || type == kleenePlus) {
        auto* new_t = new BinaryTree(type);
        new_t->child = child->underKleene();
        return new_t;
    }
    else if (type == backreferenceExpr) {
        return this;
    }
}