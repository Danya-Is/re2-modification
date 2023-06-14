#include <fstream>
#include <iostream>
#include <cstring>

#include "../regex/regex.h"

using namespace std;

void split(vector<string>& strings, string str, char separator) {
    int startIndex = 0, endIndex = 0;
    for (int i = 0; i <= str.size(); i++) {
        if (str[i] == separator || i == str.size()) {
            endIndex = i;
            string temp;
            temp.append(str, startIndex, endIndex - startIndex);
            strings.push_back(temp);
            startIndex = endIndex + 1;
        }
    }
}

void run_examples() {
    ifstream file("/home/daria/CLionProjects/re2-modification/test/bnf_examples.txt");

    if (file.is_open()) {
        string text;
        int i = 0;
        bool is_error = false;
        while (getline(file, text)) {
            vector<string>strings;
            split(strings, text, ',');
            string regexp_str = strings[0];
            string expected_bnf = strings[1];
            string expected_reverse = strings[2];

            Regexp* regexp = Regexp::parse_regexp(regexp_str);
            regexp->is_backref_correct();
            auto *bnf_regex = regexp->bnf();
            auto bnf_str = bnf_regex->to_string();
            if (bnf_str != expected_bnf) {
                cout<< "At " << i << endl;
                cout<< "Expected bnf: " << expected_bnf << endl;
                cout << "Got: " << bnf_str << endl;
                is_error = true;
                break;
            }

            auto *reverse_regex = bnf_regex->reverse();
            string reverse_str = reverse_regex->to_string();
            if (reverse_str != expected_reverse) {
                cout<< "At " << i << endl;
                cout<< "Expected reverse: " << expected_reverse << endl;
                cout << "Got: " << reverse_str << endl;
                is_error = true;
                break;
            }
            i++;
        }

        if (!is_error)
            cout<<"OK";
        file.close();
    }
}
