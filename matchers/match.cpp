#include <iostream>
#include <string>
#include <utility>
#include "../regex.h"
#include "../bt/binary_tree.h"
#include "../automata.h"
#include "../node.h"

using namespace std;

void match(string regexp_str) {
//    regexp_str = ".*" + regexp_str + ".*";
    Regexp* regexp = Regexp::parse_regexp(regexp_str);

    string text;
    cin >> text;
    auto *automata = regexp->compile();
    while (text != "exit") {
        bool match;
        match = automata->match(text);
        cout << match << endl;
        cin >> text;
    }
}
