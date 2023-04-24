#include <iostream>
#include <string>
#include <regex>
#include <ctime>
#include <fstream>
#include <list>

using namespace std;

int main_() {
    std::regex pattern("((a*)b\2b)*");
    string text = "aaaa";

    cout << std::regex_match(text, pattern);

//    for (int i =0; i < 1; i++) {
//        ifstream file("input_strings.txt");
//        ofstream out("results.txt");
//        list<double> results;
//        if (file.is_open()) {
//            while (getline(file, text)) {
//                clock_t start = clock();
//
//
//                if (std::regex_match(text, pattern)) {
//                    std::cout << "Match found!" << std::endl;
//                } else {
//                    std::cout << "No match found." << std::endl;
//                }
//
//
//                clock_t end = clock();
//                double seconds = (double)(end - start) / CLOCKS_PER_SEC;
//                cout << seconds << endl;
//                results.push_back(seconds);
//
//            }
//
//            for (auto res: results) {
//                out << res << ", " << endl;
//            }
//
//            file.close();
//            out.close();
//        } else {
//            cout << "ERROR\n";
//            exit(1);
//        }
//    }


//    // Simple regular expression matching
//    const std::string words[] = {
//            std::string(10, 'a') + "b",
//            std::string(13, 'a') + "b",
//            std::string(16, 'a') + "b",
//            std::string(19, 'a') + "b",
//            std::string(22, 'a') + "b",
//            std::string(25, 'a') + "b",
//    };
//    const std::regex txt_regex("(a|a)*");
//
//    for (auto word : words) {
//        std::cout << word << "\n";
//        clock_t start = clock();
//        bool result = std::regex_match(word, txt_regex);
//        clock_t final = clock();
//        std::cout << result << ": " << (double)(final - start) / CLOCKS_PER_SEC  << " sec \n";
//    }
    return 0;
}