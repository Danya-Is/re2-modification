#ifndef COURSEWORK_AUTOMATA_H
#define COURSEWORK_AUTOMATA_H

#include <set>
#include <map>
#include "list"

#include "node.h"
//#include "regex.h"
#include "variable.h"

#define Memory map<string, Variable*>
#define MemoryState pair<int, pair<MemoryNode*, Memory>>

using namespace std;


class Automata {
public:
    Node* start;
    Node* finish;
    int last_idx;
    list<Node*> nodes;
    bool is_reversed = false;

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

    bool match(const string& str);

    void evaluateStates(string letter, int letter_index, set<Node *> &states, set<Node *> &visited_states);

    void
    evaluateState(Node *state, string letter, int letter_index, set<Node *> &new_states, set<Node *> &visited_states);
};

class MFA : public Automata {
public:
    MemoryNode* start;
    MemoryNode* finish;
    list<MemoryNode*> nodes;
    bool is_reversed = false;

    MFA() {
        start = new MemoryNode();
        finish = new MemoryNode();
        nodes.push_back(start);
        nodes.push_back(finish);
        last_idx = 0;
    }

    void changeFinalState(MemoryNode *new_final);

    bool is_siffix_long_enough(MemoryState state, const string& str, int letter_index);

    bool match(string str);

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
