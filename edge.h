#include <string>
#include <utility>
#include <map>

#ifndef COURSEWORK_EDGE_H
#define COURSEWORK_EDGE_H

using namespace std;

class Node;
class MemoryNode;

class Edge {
public:
    string by;
    Node* to;
    bool drawn = false;

    Edge() {
        this->drawn = false;
    };
    Edge(string by, Node* to) {
        this->drawn = false;
        this->by = std::move(by);
        this->to = to;
    }
};

enum MemoryAction {
    open,
    close
};

class MemoryEdge: public Edge {
public:
    map<string, MemoryAction> memoryActions;
    MemoryNode* to;

    MemoryEdge() = default;
    MemoryEdge(string by, MemoryNode* to) {
        this->drawn = false;
        this->by = std::move(by);
        this->to = to;
    };

    void addAction(const string& var, MemoryAction action) {
        memoryActions[var] = action;
    }
};

#endif //COURSEWORK_EDGE_H
