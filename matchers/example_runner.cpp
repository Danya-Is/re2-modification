#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <pthread.h>
#include <sstream>

#include "../regex/regex.h"

using namespace std;


std::string pumped_string(int n, vector<string> pump_v) {
    int pump_count = pump_v.size() / 2 + 1;
    int del_count = pump_v.size() - pump_count;

    std::ostringstream os;
    int tmp_len = pump_v[0].length();
    string res = pump_v[0];
    while (res.length() + tmp_len < (n - del_count) / pump_count)
        res +=  pump_v[0];

    for (int i = 0; i < del_count; i++)
        os << res << pump_v[1];
    os << res;
    return os.str();
}

vector<string> split(string str, char separator) {
    vector<string> strings;
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
    return strings;
}

double match_timer(MFA* mfa, const string& input_str) {
    clock_t start = clock();
    mfa->match(input_str);
    clock_t end = clock();
    return (double)(end - start) / CLOCKS_PER_SEC;
}

double match_with_timeout(MFA* mfa, const string& input_str, int timeout = 0.5)
{
    std::mutex m;
    std::condition_variable cv;
    double result;

    std::thread t([&cv, &result, mfa, input_str]()
                  {
                      result = match_timer(mfa, input_str);
                      cv.notify_one();
                  });
    t.detach();

    {
        std::unique_lock<std::mutex> l(m);
        if(cv.wait_for(l, 0.48s) == std::cv_status::timeout)
            return 0.5;
    }

    return result;
}
void handle(MFA* mfa, const string& input_str, int len,  bool& timeouted, fstream& file) {
    if (!timeouted) {
        double seconds = match_with_timeout(mfa, input_str);
        if (seconds >= 0.5)
            timeouted = true;
        if (seconds < 1)
            file << len << " " << seconds << endl;
    }
}

void run_configuration_examples(const string& number) {
    fstream regex_file("test/example_" + number + "/regexp.txt");
    fstream pump_file("test/example_" + number + "/pump.txt");
    fstream result_file("test/example_" + number + "/diploma_results.txt", std::ofstream::out | std::ofstream::trunc);
    fstream bnf_result_file("test/example_" + number + "/diploma_bnf_results.txt", std::ofstream::out | std::ofstream::trunc);
    fstream reverse_result_file("test/example_" + number + "/diploma_reverse_results.txt", std::ofstream::out | std::ofstream::trunc);
    string regexp_str;
    string input_str;
    if (regex_file.is_open() && regex_file.is_open() && pump_file.is_open()){
        string pump_s;
        string suffix;
        string prefix;

        getline(pump_file, pump_s);
        auto pump = split(pump_s, ',');
        getline(pump_file, suffix);
        getline(pump_file, prefix);

        int pump_size = 500;

        getline(regex_file, regexp_str);
        cout << regexp_str << endl;
        Regexp* regexp = Regexp::parse_regexp(regexp_str);
        regexp->is_backref_correct();
        bool is_mfa = true;
        MFA* mfa = static_cast<MFA*>(regexp->compile(is_mfa, false, false, true, false));
        MFA* bnf_mfa = static_cast<MFA*>(regexp->compile(is_mfa, false, true, true, false));
        MFA* reverse_mfa = static_cast<MFA*>(regexp->compile(is_mfa, true, true, true, false));

        bool mfa_timeouted = false;
        bool bnf_timeouted = false;
        bool reverse_timeouted = false;

        int count = 0;
        int len_limit = INT32_MAX / 10;
        int len = prefix.length() + pump_size + suffix.length();

        while ((!reverse_timeouted || !bnf_timeouted || !mfa_timeouted) && len < len_limit) {

            input_str = prefix.append(pumped_string(pump_size, pump)).append(suffix);
            len = input_str.length();
            pump_size += pump_size;

            // without bnf and reverse
            if (!mfa_timeouted) {
                handle(mfa, input_str, len, mfa_timeouted, result_file);
            }

            // with just bnf
            if (!bnf_timeouted) {
                handle(bnf_mfa, input_str, len, bnf_timeouted, bnf_result_file);
            }
            // with reverse
            if (!reverse_timeouted) {
                handle(reverse_mfa, input_str, len, reverse_timeouted, reverse_result_file);
                count ++;
            }

            if (count % 10 == 0)
                pump_size *= 2;
        }
    }
    regex_file.close();
    pump_file.close();
    result_file.close();
    bnf_result_file.close();
    reverse_result_file.close();
}