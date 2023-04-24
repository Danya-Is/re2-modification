#include <set>
#include <map>
#include "list"

#include "node.h"
#include "regex.h"
#include "variable.h"

#ifndef COURSEWORK_AUTOMATA_H
#define COURSEWORK_AUTOMATA_H

#define Memory map<string, Variable*>
#define MemoryState pair<int, pair<MemoryNode*, Memory>>

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

    void evaluateStates(string letter, int letter_index, set<Node *> &states, set<Node *> &visited_states);

    void
    evaluateState(Node *state, string letter, int letter_index, set<Node *> &new_states, set<Node *> &visited_states);
};

class MFA : public Automata {
public:
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
    evaluateStates(string str, int letter_index, set<MemoryState> &states,
                   set<MemoryNode*> &visited_states);

    void
    evaluateState(MemoryState state, string str, int letter_index, set<MemoryState> &new_states,
                  set<MemoryNode*> &visited_states);

    static void doMemoryWriteActions(std::string letter, MemoryEdge* edge, MemoryState &state);

    void makeDOTFile(const string &filename);

    bool draw(const string &filename);
};

#endif //COURSEWORK_AUTOMATA_H
