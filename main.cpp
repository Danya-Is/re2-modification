#include <iostream>
#include "regex/regex.h"
#include "bt/binary_tree.h"
#include "automata.h"

using namespace std;

int main() {

//    string regex;
//    cin >> regex;
//    match(regex);

    string regexp_str;
    cin >> regexp_str;
    Regexp* regexp = Regexp::parse_regexp(regexp_str);
    regexp->is_backref_correct();
    auto bnf_regex = regexp->bnf();
    auto bnf_str = bnf_regex->to_string();
    cout << bnf_str;

    while (regexp_str != "exit") {
        cin >> regexp_str;
        regexp = Regexp::parse_regexp(regexp_str);
        regexp->is_backref_correct();
        cout << regexp->bnf()->to_string();
    }
    return 0;
}
