#include <iostream>
#include "regex/regex.h"
#include "bt/binary_tree.h"
#include "automata.h"

using namespace std;

int main() {

    string regex;
    cin >> regex;
    match(regex);
    return 0;
}
