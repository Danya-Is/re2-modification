#include "automata.h"
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <map>
#include <set>
#include <vector>
//#include <graphviz/gvc.h>

#include "binary_tree.h"

using namespace std;

void Automata::changeFinalState(Node *new_final) {
    for (auto* node: nodes) {
        for (auto* edge: node->edges) {
            if (edge->to == finish) {
                edge->to = new_final;
            }
        }
    }
    for (auto i =nodes.begin(); i!=nodes.end(); i++) {
        Node* cur_node = *i;
        if (cur_node == finish) {
            nodes.erase(i);
            break;
        }
    }
    start->start_for = new_final;
}

void Automata::makeDOTFile(const string& filename) {
    string text = "digraph g {\n";
    int i = 0;
    for (auto *node: nodes) {
        if (node->name.empty()) {
            node->name = to_string(last_idx++);
        }
        for (auto *edge: node->edges) {
            if (edge->to->name.empty()) {
                edge->to->name = to_string(last_idx++);
            }
            string by = edge->by;
            if (edge->by.empty()) {
                by = "ε";
            }
            if (edge->by == ".") {
                by = "dot";
            }
            if (!edge->drawn) {
                text += "\t" + node->name + " -> " + edge->to->name + " [label=" + by + "]\n";
            }
        }
    }
    text += "}";
//    cout << text << "\n";

    ofstream out;
    out.open(filename + ".dot");
    if (out.is_open()) {
        out << text;
    }
    out.close();
}

bool Automata::draw(const string& filename)
{
    makeDOTFile(filename);
//    GVC_t *gvc;
//    Agraph_t *g;
//    FILE *fp;
//    gvc = gvContext();
//    fp = fopen((filename + ".dot").c_str(), "r");
//    g = agread(fp, 0);
//    gvLayout(gvc, g, "dot");
//    gvRender(gvc, g, "png", fopen((filename + ".png").c_str(), "w"));
//    gvFreeLayout(gvc, g);
//    agclose(g);
//    return (gvFreeContext(gvc));
}

bool Automata::isDeterministic() {
    map<pair<Node*, string>, int> transition_amount;
    for (auto *node: nodes) {
        for (auto *edge: node->edges) {
            auto transition = make_pair(node, edge->by);
            transition_amount[transition] += 1;
            if (transition_amount[transition] > 1) {
                return false;
            }
        }
    }
    return true;
}

void Automata::evaluateState(Node* state, std::string letter, int letter_index,
                             set<Node *> &new_states, set<Node *> &visited_states) {
    visited_states.insert(state);
    if (letter.empty() && state == finish) {
        new_states.insert(state);
    }
    else {
        for (auto *edge: state->edges) {
            if (visited_states.find(edge->to) != visited_states.end()) {
                continue;
            }
            if (edge->by.empty()) {
                evaluateState(edge->to, letter, letter_index, new_states, visited_states);
            }
            else if (!letter.empty() && (edge->by == "." or edge->by == letter)) {
                new_states.insert(edge->to);
            }
        }
    }
}

void Automata::evaluateStates(string letter, int letter_index,
                              set<Node*> &states, set<Node*> &visited_states) {
    set<Node*> new_states;
    for (auto *node: states) {
        if (visited_states.find(node) == visited_states.end()) {
            evaluateState(node, letter, letter_index, new_states, visited_states);
        }
    }
    states = new_states;
}



//pair<int, int> Automata::matchThomson(string str) {
//    set<Node*> states;
//    map<Node*, set<int>> finish_indexes;
//    map<Node*, set<int>> start_indexes;
//    states.insert(start);
//    int str_len = str.length();
//    for (int i = 0; i < str_len; i++) {
//        string by = str.substr(i, 1);
//        set<Node*> visited_states;
//        evaluateStates(by, i, states, visited_states, start_indexes, finish_indexes);
//    }
//    set<Node*> visited_states;
//    evaluateStates("", str_len - 1, states, visited_states, start_indexes, finish_indexes);
//
//    cout << "starts" << endl;
//    for (auto it: start_indexes) {
//        std::string s = std::to_string(*it.second.begin());
//
//        std::for_each(std::next(it.second.begin()), it.second.end(), [&s] (int val) {
//            s.append(", ").append(std::to_string(val));
//        });
//        cout << it.first->name + " - " + s << endl;
//    }
//    cout << "finishes" << endl;
//    for (auto it: finish_indexes) {
//        std::string s = std::to_string(*it.second.begin());
//
//        std::for_each(std::next(it.second.begin()), it.second.end(), [&s] (int val) {
//            s.append(", ").append(std::to_string(val));
//        });
//        cout << it.first->name + " - " + s << endl;
//    }
//
//    for (auto *state: states) {
//        if (state == finish) {
//            vector<int> s_indexes(start_indexes[start->start_for].begin(), start_indexes[start->start_for].end());
//            sort(s_indexes.begin(), s_indexes.end());
//            vector<int> f_indexes(finish_indexes[finish->finish_for].begin(), finish_indexes[finish->finish_for].end());
//            sort(f_indexes.begin(), f_indexes.end());
//            return make_pair(s_indexes[0], f_indexes[0]);
//        }
//    }
//    return make_pair(-1, -1);
//}

bool Automata::matchThomson(string str) {
    set<Node*> states;
    states.insert(start);
    int str_len = str.length();
    for (int i = 0; i < str_len; i++) {
        string by = str.substr(i, 1);
        set<Node*> visited_states;
        evaluateStates(by, i, states, visited_states);
        if (states.empty()) {
            break;
        }
    }
    set<Node*> visited_states;
    evaluateStates("", str_len - 1, states, visited_states);

    for (auto *state: states) {
        if (state == finish) {
            return true;
        }
    }
    return false;
}

bool Automata::matchGlushkov(string str) {
    return matchThomson(str);
}

