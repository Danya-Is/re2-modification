#include "automata.h"
#include <string>
#include <fstream>
#include <map>
//#include <graphviz/gvc.h>

using namespace std;

void MFA::changeFinalState(MemoryNode *new_final) {
    for (auto* node: nodes) {
        for (auto* edge: node->edges) {
            if (edge->to == finish) {
                edge->to = new_final;
            }
        }
    }
    for (auto i =nodes.begin(); i!=nodes.end(); i++) {
        auto* cur_node = *i;
        if (cur_node == finish) {
            nodes.erase(i);
            break;
        }
    }
}

void MFA::makeDOTFile(const string& filename) {
    map<int, string> acts = {{0, "o"}, {1, "c"}};
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
            if (edge->by.empty()) {
                edge->by = "ε";
            }
            if (!edge->drawn) {
                string actions = "";
                for (auto action: edge->memoryActions) {
                    actions += acts[action.second] + action.first + "/";
                }
                text += "\t" + node->name + " -> " + edge->to->name +
                        " [label=\"" + edge->by + "/" + actions + "\"]\n";
            }
        }
    }
    text += "}";

    ofstream out;
    out.open(filename + ".dot");
    if (out.is_open()) {
        out << text;
    }
    out.close();
}

bool MFA::draw(const string& filename)
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
                            doMemoryWriteActions(by_string, edge, new_state);
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