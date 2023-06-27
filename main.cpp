#include <iostream>
#include "regex/regex.h"
#include "bt/binary_tree.h"
#include "automata.h"
#include <cstring>

using namespace std;

int main(int argc, char *argv[]) {
    if (argc > 1 && ::strcmp(argv[1],"-match") == 0) {
        if (argc > 2 && argv[2][0] != '-'){
            run_configuration_examples(argv[2]);
        }
        else {
            bool bnf = false;
            bool reverse = false;
            bool ssnf = false;
            bool use_log = false;
            map<string, bool> settings;

            for (int i = 2; i < argc; i++) {
                settings[argv[i]] = true;
            }

            if (argc > 2 && ::strcmp(argv[2],"-all") == 0) {
                bnf = true;
                reverse = true;
                ssnf = true;
            }
            if (settings.find("-bnf") != settings.end())
                bnf = true;
            if (settings.find("-reverse") != settings.end()) {
                reverse = true;
                bnf = true;
            }

            if (settings.find("-ssnf") != settings.end())
                ssnf = true;
            if (settings.find("-log") != settings.end())
                use_log = true;

            string regex;
            cin >> regex;
            match(regex, reverse, bnf, ssnf, use_log);
        }
    }
    else if (argc > 1 && ::strcmp(argv[1],"-test-bnf") == 0) {
        run_bnf_test();
    }
    else {
        bool use_log = false;
        if (argc > 2 && ::strcmp(argv[2],"-log") == 0)
            use_log = true;

        string regexp_str;
        cin >> regexp_str;
        Regexp* regexp = Regexp::parse_regexp(regexp_str);
        regexp->is_backref_correct();

        auto *bnf_regex = regexp->bnf(use_log);
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

            bnf_regex = regexp->bnf(use_log);
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
