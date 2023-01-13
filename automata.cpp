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
//    makeDOTFile(filename);
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

void MFA::doMemoryWriteActions(std::string letter, MemoryEdge* edge, MemoryState &state) {
    // создаем новые ячейки памяти
    for (auto action: edge->memoryActions) {
        if (state.second.second.find(action.first) == state.second.second.end() and action.second == open) {
            state.second.second[action.first] = new Variable();
        }
    }

    // записываем в открытые ячейки памяти
    for (const auto& cell: state.second.second) {
        auto var_name = cell.first;
        if (edge->memoryActions.find(var_name) != edge->memoryActions.end()) {
            auto action = edge->memoryActions[var_name];
            if (action == open) {
                state.second.second[var_name]->open();
                state.second.second[var_name]->write(letter);
            }
            else {
                state.second.second[var_name]->close();
            }
        }
        else if (state.second.second[var_name]->is_open) {
            state.second.second[var_name]->write(letter);
        }
    }
}

Memory copy_memory(Memory memory) {
    Memory new_memory;
    for (auto cell: memory) {
        new_memory[cell.first] = new Variable(cell.second->is_open, cell.second->value);
    }

    return new_memory;
}

void MFA::evaluateState(MemoryState state, std::string str, int letter_index, set<MemoryState> &new_states,
                        set<MemoryNode*> &visited_states) {
    if (state.second.first == finish and state.first == str.length()) {
        new_states.insert(state);
    }
    else {
        for (auto *edge: state.second.first->edges) {
            if (edge->by.empty() or edge->by == "ε") {
                auto new_state = make_pair(state.first,
                                           make_pair(edge->to, copy_memory(state.second.second)));
                evaluateState(new_state, str, letter_index, new_states, visited_states);
            }
            else if ("1" <= edge->by and edge->by <= "9" and
                     state.second.second.find(edge->by) == state.second.second.end()) {
                auto new_state = make_pair(state.first, make_pair(edge->to, copy_memory(state.second.second)));
                new_state.second.second[edge->by] = new Variable();
                if (edge->memoryActions.find(edge->by) == edge->memoryActions.end() or
                    edge->memoryActions[edge->by] == close) {
                    new_state.second.second[edge->by]->close();
                }
                else if (edge->memoryActions[edge->by] == open) {
                    new_state.second.second[edge->by]->open();
                }
                evaluateState(new_state, str, letter_index, new_states, visited_states);
            }
            else if (letter_index != str.length() and letter_index == state.first) {
                string by_letter = str.substr(letter_index, 1);
                auto new_state = make_pair(state.first, make_pair(edge->to, copy_memory(state.second.second)));

                // обычный переход по 1 символу, возможен  с проверкой, что выходной индекс состояние не больше
                // текущего идекса в строке
                if (edge->by == "." or edge->by == by_letter) {
                    doMemoryWriteActions(by_letter, edge, new_state);
                    new_state.first += 1;
                    new_states.insert(new_state);
                }
                else if (state.second.second.find(edge->by) != state.second.second.end()) {
                    auto by_string = state.second.second[edge->by]->read();
                    int edge_len = by_string.length();
                    if (str.length() - letter_index >= edge_len) {
                        auto sub_string = str.substr(letter_index, edge_len);
                        if (by_string == sub_string) {
                            new_state.first += edge_len;
                            new_states.insert(new_state);
                        }
                    }
                }
            }
            else if (letter_index != str.length() and letter_index < state.first) {
                new_states.insert(state);
            }
        }
    }
}


void MFA::evaluateStates(std::string str, int letter_index, set<MemoryState> &states,
                         set<MemoryNode*> &visited_states) {
    set<MemoryState> new_states;
    for (auto state: states) {
        if (visited_states.find(state.second.first) == visited_states.end()) {
            visited_states.insert(state.second.first);
            evaluateState(state, str, letter_index, new_states, visited_states);
        }
    }
    states = new_states;
}

bool MFA::matchMFA(string str) {
    // пары вида <индекс начала соответствующей подстроки для исходящих переходов, нода автомата>
    set<MemoryState> states;
    Memory empty_memory;
    states.insert(make_pair(0, make_pair(start, empty_memory)));
    int str_len = str.length();
    for (int i = 0; i < str_len; i++) {
        string by = str.substr(i, 1);
        set<MemoryNode*> visited_states;
        evaluateStates(str, i, states, visited_states);
    }
    set<MemoryNode*> visited_states;
    evaluateStates(str, str_len, states, visited_states);

    for (auto state: states) {
        if (state.second.first == finish) {
            return true;
        }
    }
    return false;
}