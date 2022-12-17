#include "automata.h"
#include <string>
#include <fstream>
#include <map>
#include <graphviz/gvc.h>

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