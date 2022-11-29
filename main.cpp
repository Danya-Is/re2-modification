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
    Automata* automata = binary_tree->toThomson();
    automata->makeDOTFile();
    automata->draw();
    return 0;
}
