#include <iostream>
#include <string>
#include <utility>
#include "regex.h"
#include "binary_tree.h"
#include "automata.h"
#include "node.h"

using namespace std;

void match(string regexp_str) {
//    regexp_str = ".*" + regexp_str + ".*";
    Regexp* regexp = Regexp::parse_regexp(regexp_str);
    BinaryTree* bt = regexp->to_binary_tree();

    bt = bt->toSSNF();
    Automata* glushkov = bt->toGlushkov();
    Automata* thomson = bt->toThomson();
    thomson->draw("thomson");
    glushkov->draw("glushkov");

    MemoryState old_state;
    auto new_state1 = make_pair(1, make_pair(old_state.second.first, old_state.second.second));

//    MFA* mfa = bt->toMFA();
//    mfa->draw("mfa");

    string text;
    cin >> text;
    while (text != "exit") {
        bool match;
//    if (!glushkov->isDeterministic()) {
//        match = thomson->matchThomson(std::move(text));
//    }
//    else {
        match = glushkov->matchThomson(text);
//    }
//        match = mfa->matchMFA(std::move(text));
        cout << match << endl;
        cin >> text;
    }
}
