#include <string>
#include <utility>

#ifndef COURSEWORK_EDGE_H
#define COURSEWORK_EDGE_H

using namespace std;

class Node;

class Edge {
public:
    string by;
    Node* to;
    bool drawn;

    Edge() {
        this->drawn = false;
    };
    Edge(string by, Node* to) {
        this->drawn = false;
        this->by = std::move(by);
        this->to = to;
    }
};

#endif //COURSEWORK_EDGE_H
