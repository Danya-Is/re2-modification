#include <iostream>
#include "regex.h"
#include "binary_tree.h"
#include "automata.h"

using namespace std;

int main() {

    string regex;
    cin >> regex;
    Regexp* re = Regexp::parse_regexp(regex);
    BinaryTree* binary_tree = re->to_binary_tree();
    MFA* mfa = binary_tree->toMFA();
    mfa->draw("mfa");
//    Automata* thompson = binary_tree->toThomson();
//    thompson->draw("thompson");
//    Automata* glushkov = binary_tree->toGlushkov();
//    glushkov->draw("glushkov");
    return 0;
}
