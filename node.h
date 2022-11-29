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
};

#endif //COURSEWORK_NODE_H
