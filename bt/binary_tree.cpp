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