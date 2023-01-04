#include "automata.h"
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <map>
#include <set>
#include <vector>
#include <graphviz/gvc.h>

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
    GVC_t *gvc;
    Agraph_t *g;
    FILE *fp;
    gvc = gvContext();
    fp = fopen((filename + ".dot").c_str(), "r");
    g = agread(fp, 0);
    gvLayout(gvc, g, "dot");
    gvRender(gvc, g, "png", fopen((filename + ".png").c_str(), "w"));
    gvFreeLayout(gvc, g);
    agclose(g);
    return (gvFreeContext(gvc));
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
                             set<Node *> &new_states, set<Node *> &visited_states,
                             map<Node *, set<int>> &start_indexes, map<Node*, set<int>> &finish_indexes) {
    if (state->finish_for) {
        finish_indexes[state].insert(letter_index);
    }
    if (state->start_for) {
        start_indexes[state].insert(letter_index);
    }
    if (letter.empty() && state->edges.empty()) {
        new_states.insert(state);
    }
    else {
        for (auto *edge: state->edges) {
            if (edge->by.empty()) {
                evaluateState(edge->to, letter, letter_index, new_states, visited_states, start_indexes, finish_indexes);
            }
            else if (!letter.empty() && (edge->by == "." or edge->by == letter)) {
                new_states.insert(edge->to);
            }
        }
    }
}

void Automata::evaluateStates(string letter, int letter_index,
                              set<Node*> &states, set<Node*> &visited_states,
                              map<Node *, set<int>> &start_indexes, map<Node*, set<int>> &finish_indexes) {
    set<Node*> new_states;
    for (auto *node: states) {
        if (visited_states.find(node) == visited_states.end()) {
            visited_states.insert(node);
            evaluateState(node, letter, letter_index, new_states, visited_states, start_indexes, finish_indexes);
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
    map<Node*, set<int>> finish_indexes;
    map<Node*, set<int>> start_indexes;
    states.insert(start);
    int str_len = str.length();
    for (int i = 0; i < str_len; i++) {
        string by = str.substr(i, 1);
        set<Node*> visited_states;
        evaluateStates(by, i, states, visited_states, start_indexes, finish_indexes);
    }
    set<Node*> visited_states;
    evaluateStates("", str_len - 1, states, visited_states, start_indexes, finish_indexes);

    cout << "starts" << endl;
    for (auto it: start_indexes) {
        std::string s = std::to_string(*it.second.begin());

        std::for_each(std::next(it.second.begin()), it.second.end(), [&s] (int val) {
            s.append(", ").append(std::to_string(val));
        });
        cout << it.first->name + " - " + s << endl;
    }
    cout << "finishes" << endl;
    for (auto it: finish_indexes) {
        std::string s = std::to_string(*it.second.begin());

        std::for_each(std::next(it.second.begin()), it.second.end(), [&s] (int val) {
            s.append(", ").append(std::to_string(val));
        });
        cout << it.first->name + " - " + s << endl;
    }

    for (auto *state: states) {
        if (state == finish) {
            return true;
//            vector<int> s_indexes(start_indexes[start->start_for].begin(), start_indexes[start->start_for].end());
//            sort(s_indexes.begin(), s_indexes.end());
//            vector<int> f_indexes(finish_indexes[finish->finish_for].begin(), finish_indexes[finish->finish_for].end());
//            sort(f_indexes.begin(), f_indexes.end());
//            return make_pair(s_indexes[0], f_indexes[0]);
        }
    }
    return false;
}

bool Automata::matchGlushkov(string str) {
    return matchThomson(str);
}

void MFA::doMemoryWriteActions(std::string letter, MemoryEdge* edge) {
    // создаем новые ячейки памяти
    for (auto action: edge->memoryActions) {
        if (memory.find(action.first) == memory.end() and action.second == open) {
            memory[action.first] = Variable();
        }
    }

    // записываем в открытые ячейки памяти
    for (auto cell: memory) {
        auto var_name = cell.first;
        if (edge->memoryActions.find(var_name) != edge->memoryActions.end()) {
            auto action = edge->memoryActions[var_name];
            if (action == open) {
                memory[var_name].open();
                memory[var_name].write(letter);
            }
            else {
                memory[var_name].close();
            }
        }
        else if (memory[var_name].is_open) {
            memory[var_name].write(letter);
        }
    }
}

void MFA::evaluateState(MemoryNode *state, std::string letter, int letter_index, set<MemoryNode *> &new_states,
                        set<MemoryNode *> &visited_states, map<MemoryEdge*, int> &edge_positions) {
    if (letter.empty() && state->edges.empty()) {
        new_states.insert(state);
    }
    else {
        for (auto *edge: state->edges) {
            if (edge->by.empty() or edge->by == "ε") {
                evaluateState(edge->to, letter, letter_index, new_states, visited_states, edge_positions);
            }
            else if (!letter.empty() && (edge->by == "." or edge->by == letter)) {
                doMemoryWriteActions(letter, edge);
                new_states.insert(edge->to);
            }
            else if (!letter.empty() && memory.find(edge->by) != memory.end()) {
                auto str = memory[edge->by].read();
                int edge_len = str.length();
                if (edge_positions[edge] < edge_len) {
                    str.erase(0, edge_positions[edge]);
                    string by = substr(str, 1);
                    edge_positions[edge] += 1;
                    if (by == letter and edge_positions[edge] == edge_len) {
                        new_states.insert(edge->to);
                    }
                    else if (by == letter) {
                        new_states.insert(state);
                    }
                }

            }
        }
    }
}


void MFA::evaluateStates(std::string letter, int letter_index, set<MemoryNode *> &states,
                         set<MemoryNode *> &visited_states, map<MemoryEdge*, int> &edge_positions) {
    set<MemoryNode*> new_states;
    for (auto *node: states) {
        if (visited_states.find(node) == visited_states.end()) {
            visited_states.insert(node);
            evaluateState(node, letter, letter_index, new_states, visited_states, edge_positions);
        }
    }
    states = new_states;
}

bool MFA::matchMFA(string str) {
    // для чтения из памяти, если в ячейке лежит больше одного символа
    // (и для возможного перехода по элементам языка из несколькиз символов)
    map<MemoryEdge*, int> edge_positions;
    for (auto *node: nodes) {
        for (auto *edge: node->edges) {
            edge_positions[edge] = 0;
        }
    }

    set<MemoryNode*> states;
    states.insert(start);
    int str_len = str.length();
    for (int i = 0; i < str_len; i++) {
        string by = str.substr(i, 1);
        set<MemoryNode*> visited_states;
        evaluateStates(by, i, states, visited_states, edge_positions);
    }
    set<MemoryNode*> visited_states;
    evaluateStates("", str_len - 1, states, visited_states, edge_positions);

    for (auto *state: states) {
        if (state == finish) {
            memory.clear();
            return true;
        }
    }
    memory.clear();
    return false;
}