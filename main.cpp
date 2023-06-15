#include <iostream>
#include <fstream>
#include "regex/regex.h"
#include "bt/binary_tree.h"
#include "automata.h"
#include <cstring>

using namespace std;

int main(int argc, char *argv[]) {
    if (argc > 1 && ::strcmp(argv[1],"-match") == 0) {
        if (argc > 2 && ::strcmp(argv[2],"1") == 0){
            fstream regex_file("test/example_1/regexp.txt");
            string regexp_str;
            string input_str;
            if (regex_file.is_open()){
                getline(regex_file, regexp_str);
                Regexp* regexp = Regexp::parse_regexp(regexp_str);
                regexp->is_backref_correct();
                if (!regexp->maybe_read.empty() || !regexp->maybe_initialized.empty()) {
                    auto *bnf_regex = regexp->bnf();
                    if (!bnf_regex->is_bad_bnf) {
                        auto bnf_str = bnf_regex->to_string();
                        cout << "BNF: " << bnf_str << endl;

                        auto *reverse_regex = bnf_regex->reverse();
                        auto reverse_str = reverse_regex->to_string();
                        cout << "Reverse: " << reverse_str << endl;


                    }
                }
            }
        }
        else {
            string regex;
            cin >> regex;
            match(regex);
        }
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
        if (!bnf_regex->is_bad_bnf) {
            auto bnf_str = bnf_regex->to_string();
            cout << "BNF: " << bnf_str << endl;

            auto *reverse_regex = bnf_regex->reverse();
            auto reverse_str = reverse_regex->to_string();
            cout << "Reverse: " << reverse_str << endl;
        }

        while (regexp_str != "exit") {
            cin >> regexp_str;
            regexp = Regexp::parse_regexp(regexp_str);
            regexp->is_backref_correct();

            bnf_regex = regexp->bnf();
            if (!bnf_regex->is_bad_bnf) {
                auto bnf_str =bnf_regex->to_string();
                cout << "BNF: " << bnf_str << endl;

                auto *reverse_regex = bnf_regex->reverse();
                auto reverse_str = reverse_regex->to_string();
                cout << "Reverse: " << reverse_str << endl;
            }
        }
    }
    return 0;
}
