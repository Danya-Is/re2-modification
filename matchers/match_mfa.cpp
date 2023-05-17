#include <iostream>
#include <string>
#include <utility>
#include "../regex.h"
#include "../bt/binary_tree.h"
#include "../automata.h"
#include "../node.h"
#include <ctime>
#include <fstream>

using namespace std;

void match_gt(string regexp_str) {
//    regexp_str = ".*" + regexp_str + ".*";
    Regexp* regexp = Regexp::parse_regexp(regexp_str);
    BinaryTree* bt = regexp->to_binary_tree();
    Automata* glushkov = bt->toGlushkov();
    Automata* thomson = bt->toThomson();
    thomson->draw("thomson");
    glushkov->draw("glushkov");


    string text;

    for (int i =0; i < 1; i++) {
        ifstream file("input_strings.txt");
        ofstream out("results.txt");
        if (file.is_open()) {
            while (getline(file, text)) {
                bool match;
                clock_t start = clock();
//                match = thomson->match(text);
                match = glushkov->match(text);
                clock_t end = clock();
                double seconds = (double)(end - start) / CLOCKS_PER_SEC;
//    if (!glushkov->isDeterministic()) {
//        match = thomson->match(std::move(text));
//    }
//    else {
//        match = glushkov->match(text);
//    }
//                cout << match << endl;
                cout << seconds << endl;
                out << seconds << ",";

            }

            file.close();
        } else {
            cout << "ERROR\n";
            exit(1);
        }
    }


}

void match_mfa(string regexp_str) {
//    regexp_str = ".*" + regexp_str + ".*";
    Regexp* regexp = Regexp::parse_regexp(regexp_str);
    BinaryTree* bt = regexp->to_binary_tree();
    MFA* mfa = bt->toMFA();
    mfa->draw("mfa");

    string text;

    for (int i =0; i < 1; i++) {
        ifstream file("mfa_str.txt");
        ofstream out("results7.txt");
        list<double> results;
        if (file.is_open()) {
            while (getline(file, text)) {
                bool match;
                clock_t start = clock();
                match = mfa->match(text);
                clock_t end = clock();
                double seconds = (double)(end - start) / CLOCKS_PER_SEC;
                cout << seconds << endl;
                cout << match << endl;
                results.push_back(seconds);

            }

            for (auto res: results) {
                out << res << ", " << endl;
            }

            file.close();
            out.close();
        } else {
            cout << "ERROR\n";
            exit(1);
        }
    }


}
