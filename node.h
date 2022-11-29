#include <utility>

#include "edge.h"
#include "list"

#ifndef COURSEWORK_NODE_H
#define COURSEWORK_NODE_H

using namespace std;

class Node {
public:
    list<Edge*> edges;
    string name;

    Node() {
        name = "";
    }
    Node(string name) {
        this->name = std::move(name);
    }
};

#endif //COURSEWORK_NODE_H
