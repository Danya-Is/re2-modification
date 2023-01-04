#include <set>
#include <map>
#include "list"

#include "node.h"
#include "regex.h"
#include "variable.h"

#ifndef COURSEWORK_AUTOMATA_H
#define COURSEWORK_AUTOMATA_H

using namespace std;


class Automata {
public:
    Node* start;
    Node* finish;
    int last_idx;
    list<Node*> nodes;

    Automata() {
        start = new Node();
        finish = new Node();
        nodes.push_back(start);
        nodes.push_back(finish);
        last_idx = 0;
    }

    void makeDOTFile(const string& filename);

    bool draw(const string& filename);

    void changeFinalState(Node *new_final);

    bool isDeterministic();

    bool matchThomson(string str);

    bool matchGlushkov(string str);

    void evaluateStates(string letter, int letter_index, set<Node *> &states, set<Node *> &visited_states,
                        map<Node *, set<int>> &start_indexes, map<Node *, set<int>> &finish_indexes);

    void
    evaluateState(Node *state, string letter, int letter_index, set<Node *> &new_states, set<Node *> &visited_states,
                  map<Node *, set<int>> &start_indexes, map<Node *, set<int>> &finish_indexes);
};

class MFA : public Automata {
public:
    map<string, Variable> memory;

    MemoryNode* start;
    MemoryNode* finish;
    list<MemoryNode*> nodes;

    MFA() {
        start = new MemoryNode();
        finish = new MemoryNode();
        nodes.push_back(start);
        nodes.push_back(finish);
        last_idx = 0;
    }

    void changeFinalState(MemoryNode *new_final);

    bool matchMFA(string str);

    void
    evaluateStates(string letter, int letter_index, set<MemoryNode *> &states, set<MemoryNode *> &visited_states,
                        map<MemoryEdge*, int> &edge_positions);

    void
    evaluateState(MemoryNode *state, string letter, int letter_index, set<MemoryNode *> &new_states, set<MemoryNode *> &visited_states,
                  map<MemoryEdge*, int> &edge_positions);

    void doMemoryWriteActions(std::string letter, MemoryEdge* edge);

    void makeDOTFile(const string &filename);

    bool draw(const string &filename);
};

#endif //COURSEWORK_AUTOMATA_H
