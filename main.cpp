#include <iostream>
#include <fstream>
#include "regex/regex.h"
#include "bt/binary_tree.h"
#include "automata.h"
#include <cstring>
#include <ctime>
#include <cstdlib>

using namespace std;

void run_example(string number) {
    fstream regex_file("test/example_" + number + "/regexp.txt");
    fstream input_string_file("test/example_" + number + "/input_strings.txt");
    fstream result_file("test/example_" + number + "/diploma_results.txt", std::ofstream::out | std::ofstream::trunc);
    fstream reverse_result_file("test/example_" + number + "/diploma_reverse_results.txt", std::ofstream::out | std::ofstream::trunc);
    string regexp_str;
    string input_str;
    if (regex_file.is_open() && input_string_file.is_open() && regex_file.is_open()){
        getline(regex_file, regexp_str);
        cout << regexp_str << endl;
        Regexp* regexp = Regexp::parse_regexp(regexp_str);
        regexp->is_backref_correct();
        bool is_mfa = true;
        MFA* mfa = static_cast<MFA*>(regexp->compile(is_mfa, false));
        MFA* reverse_mfa = nullptr;
        if (!regexp->is_one_unamb)
            reverse_mfa = static_cast<MFA*>(regexp->compile(is_mfa, true));

        while (getline(input_string_file, input_str)) {
            clock_t start = clock();
            bool match = mfa->match(input_str);
            clock_t end = clock();
            double seconds = (double)(end - start) / CLOCKS_PER_SEC;
//            cout << match << endl;

            result_file << seconds << endl;

            if (reverse_mfa) {
                start = clock();
                match = reverse_mfa->match(input_str);
                end = clock();
                seconds = (double)(end - start) / CLOCKS_PER_SEC;
//                cout << match << endl;

                reverse_result_file << seconds << endl;
            }
        }
    }
    regex_file.close();
    input_string_file.close();
    result_file.close();
    reverse_result_file.close();

//    string cmd = "python3.10 python_re/matcher.py " + number;
//    ::system(cmd.c_str());
}

int main(int argc, char *argv[]) {
    if (argc > 1 && ::strcmp(argv[1],"-match") == 0) {
        if (argc > 2){
            run_example(argv[2]);
        }
        else {
            string regex;
            cin >> regex;
            match(regex);
        }
    }
    else if (argc > 1 && ::strcmp(argv[1],"-test-bnf") == 0) {
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
