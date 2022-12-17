#include <string>

#ifndef COURSEWORK_VARIABLE_H
#define COURSEWORK_VARIABLE_H

using namespace std;

class Variable{
public:
    bool is_open = false;
    string value;

    void open() {
        is_open = true;
    }

    void close() {
        is_open = false;
    }

    string& read() {
        return value;
    }

    void write(const string& a) {
        value += a;
    }
};

#endif //COURSEWORK_VARIABLE_H
