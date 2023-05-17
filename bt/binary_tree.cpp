#include "binary_tree.h"
#include <string>
#include <set>

using namespace std;

bool BinaryTree::epsilonProducing() {
    if (type == epsilon || type == kleeneStar) {
        return true;
    }
    else if (type == literal || type == reference) {
        return false;
    }
    else if (type == kleenePlus || type == backreferenceExpr) {
        return child->epsilonProducing();
    }
    else if (type == alternationExpr) {
        return (left->epsilonProducing() || right->epsilonProducing());
    }
    else if (type == concatenationExpr) {
        return (left->epsilonProducing() && right->epsilonProducing());
    }
    else {
        printf("UNKNOWN BINARY TREE TYPE!!!!!");
        return true;
    }
}

BinaryTree* BinaryTree::reverse() {
    auto *new_t = new BinaryTree(type);
    if (type == epsilon || type == literal || type == reference) {
        return this;
    }
    else if (type == kleeneStar || type == kleenePlus || type == backreferenceExpr) {
        new_t->child = child->reverse();
    }
    else if (type == alternationExpr) {
        new_t->left = left->reverse();
        new_t->right = right->reverse();
    }
    else if (type == concatenationExpr) {
        new_t->left = right->reverse();
        new_t->right = left->reverse();
    }
    return new_t;
}

std::string substr(std::string originalString, int maxLength)
{
    std::string resultString = originalString;

    int len = 0;
    int byteCount = 0;

    const char* aStr = originalString.c_str();

    while(*aStr)
    {
        if( (*aStr & 0xc0) != 0x80 )
            len += 1;

        if(len>maxLength)
        {
            resultString = resultString.substr(0, byteCount);
            break;
        }
        byteCount++;
        aStr++;
    }

    return resultString;
}