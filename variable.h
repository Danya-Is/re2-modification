#include <string>

#ifndef COURSEWORK_VARIABLE_H
#define COURSEWORK_VARIABLE_H

using namespace std;

class Variable{
public:
    bool is_open = false;
    string value;

    Variable(bool is_open, string value) {
        this->is_open = is_open;
        this->value = value;
    }

    Variable() = default;

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
