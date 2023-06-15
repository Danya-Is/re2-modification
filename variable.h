#include <string>

#ifndef COURSEWORK_VARIABLE_H
#define COURSEWORK_VARIABLE_H

using namespace std;

class Variable{
public:
    bool is_open = false;
    /// была ли ячейка прочинатана после инициализации
    bool is_read = false;
    string value;

    Variable(bool is_open, string value, bool is_read = false) {
        this->is_open = is_open;
        this->value = value;
        this->is_read = is_read;
    }

    Variable() = default;

    void open() {
        is_open = true;
        is_read = false;
        this->value = "";
    }

    void close() {
        is_open = false;
    }

    string& read() {
        is_read = true;
        return value;
    }

    void write(const string& a) {
        value += a;
    }
};

#endif //COURSEWORK_VARIABLE_H
