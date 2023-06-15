#include <iostream>
#include <string>
#include "../regex/regex.h"
#include "../bt/binary_tree.h"
#include "../automata.h"
#include "../node.h"

using namespace std;

void match(string regexp_str) {
//    regexp_str = ".*" + regexp_str + ".*";
    Regexp* regexp = Regexp::parse_regexp(regexp_str);

    bool is_mfa = false;
    auto *automata = regexp->compile(is_mfa);
    MFA* new_automata = nullptr;
    if (is_mfa) {
        new_automata = static_cast<MFA*>(automata);
    }

    string text;
    cin >> text;
    while (text != "exit") {
        bool match;
        if (new_automata)
            match = new_automata->match(text);
        else
            match = automata->match(text);
        cout << match << endl;
        cin >> text;
    }
}
