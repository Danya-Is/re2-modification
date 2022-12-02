#include "automata.h"
#include <string>
#include <fstream>
#include <map>
#include <graphviz/gvc.h>

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
            if (edge->by.empty()) {
                edge->by = "ε";
            }
            if (!edge->drawn) {
                text += "\t" + node->name + " -> " + edge->to->name + " [label=" + edge->by + "]\n";
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
