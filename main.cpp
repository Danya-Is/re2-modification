#include <iostream>
#include "regex/regex.h"
#include "bt/binary_tree.h"
#include "automata.h"
#include <cstring>

using namespace std;

int main(int argc, char *argv[]) {
    if (argc > 1 && ::strcmp(argv[1],"-match") == 0) {
            string regex;
            cin >> regex;
            match(regex);
    }
    if (argc > 1 && ::strcmp(argv[1],"-test-bnf") == 0) {
            run_examples();
    }
    else {
        string regexp_str;
        cin >> regexp_str;
        Regexp* regexp = Regexp::parse_regexp(regexp_str);
        regexp->is_backref_correct();
        auto *bnf_regex = regexp->bnf();
        auto bnf_str = bnf_regex->to_string();
        cout << "BNF: " << bnf_str << endl;

        auto *reverse_regex = bnf_regex->reverse();
        auto reverse_str = reverse_regex->to_string();
        cout << "Reverse: " << reverse_str << endl;

        while (regexp_str != "exit") {
            cin >> regexp_str;
            regexp = Regexp::parse_regexp(regexp_str);
            regexp->is_backref_correct();

            bnf_regex = regexp->bnf();
            bnf_str =bnf_regex->to_string();
            cout << "BNF: " << bnf_str << endl;

            reverse_regex = bnf_regex->reverse();
            reverse_str = reverse_regex->to_string();
            cout << "Reverse: " << reverse_str << endl;
        }
    }
    return 0;
}
