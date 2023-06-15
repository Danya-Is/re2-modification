#include "binary_tree.h"
#include <string>
#include <set>

using namespace std;

MFA* BinaryTree::toMFA() {
    auto* automata = new MFA();
    if (type == epsilon) {
        automata->start->edges.push_back(new MemoryEdge("", automata->finish));
    }
    else if (type == literal) {
        auto* node = new MemoryNode();
        string by = string (1, rune);
        automata->start->edges.push_back(new MemoryEdge(by, node));
        node->edges.push_back(new MemoryEdge("", automata->finish));
        automata->nodes.push_back(node);
    }
    else if (type == reference) {
        auto* node = new MemoryNode();
        auto* readEdge = new MemoryEdge(variable, node);
        readEdge->addAction(variable, close);
        automata->start->edges.push_back(readEdge);
        node->edges.push_back(new MemoryEdge("", automata->finish));
        automata->nodes.push_back(node);
    }
    else if (type == backreferenceExpr) {
        auto* new_a = child->toMFA();
        for (auto* edge : new_a->start->edges) {
            edge->addAction(variable, open);
        }

        for (auto* node: new_a->nodes) {
            for (auto* edge: node->edges) {
                if (edge->to == new_a->finish) {
                    edge->addAction(variable, close);
                }
            }
        }

        automata = new_a;
    }
    else if (type == alternationExpr) {
        automata = left->toMFA();
        auto* second_a = right->toMFA();

        for (auto* edge : second_a->start->edges) {
            automata->start->edges.push_back(edge);
        }
        second_a->nodes.erase(second_a->nodes.begin());
        automata->nodes.merge(second_a->nodes);

        automata->changeFinalState(second_a->finish);
        automata->finish = second_a->finish;
    }
    else if (type == concatenationExpr) {
        automata = left->toMFA();
        auto* second_a = right->toMFA();

        auto final_edges = list<MemoryEdge*>();
        auto start_edges = list<MemoryEdge*>();

        for (auto* edge : second_a->start->edges) {
            start_edges.push_back(edge);
        }

        for (auto* node: automata->nodes) {
            auto new_edges = list<MemoryEdge*>();
            for (auto* edge: node->edges) {
                if (edge->to == automata->finish) {
                    auto end = start_edges.end();
                    end--;
                    for (auto i = start_edges.begin(); i != end; i++) {
                        auto new_edge = new MemoryEdge(edge->by + (*i)->by, (*i)->to);
                        for (const auto& action: (*i)->memoryActions) {
                            new_edge->memoryActions[action.first] = action.second;
                        }
                        new_edges.push_back(new_edge);
                    }
                    edge->to = (*end)->to;
                    edge->by = edge->by + (*end)->by;
                    for (const auto& action: (*end)->memoryActions) {
                        edge->memoryActions[action.first] = action.second;
                    }
                }
            }
            node->edges.merge(new_edges);
        }

        second_a->nodes.erase(second_a->nodes.begin());
        automata->nodes.merge(second_a->nodes);
        automata->changeFinalState(second_a->finish);
        automata->finish = second_a->finish;
    }
    else if (type == kleeneStar) {
        auto* new_t = new BinaryTree(alternationExpr);

        auto* plus = new BinaryTree(kleenePlus);
        plus->child = child;
        new_t->left = plus;
        new_t->right = new BinaryTree(epsilon);

        automata = new_t->toMFA();
    }
    else if (type == kleenePlus) {
        automata = child->toMFA();

        auto final_edges = list<pair<MemoryNode*, MemoryEdge*>>();
        auto start_edges = list<MemoryEdge*>();

        for (auto* edge : automata->start->edges) {
            start_edges.push_back(edge);
        }
        for (auto* node: automata->nodes) {
            for (auto* edge: node->edges) {
                if (edge->to == automata->finish) {
                    final_edges.emplace_back(node, edge);
                }
            }
        }

        auto new_edges = set<pair<pair<MemoryNode*, MemoryEdge*>, MemoryEdge*>>();
        for (auto* start: start_edges) {
            for (auto final: final_edges) {
                new_edges.insert(make_pair(final, start));
            }
        }

        for (auto new_edge_pair: new_edges) {
            auto* final_edge = new_edge_pair.first.second;
            auto* start_edge = new_edge_pair.second;
            auto* new_edge = new MemoryEdge(final_edge->by + start_edge->by, start_edge->to);
            for (const auto& action: final_edge->memoryActions) {
                new_edge->memoryActions[action.first] = action.second;
            }
            for (const auto& action: start_edge->memoryActions) {
                new_edge->memoryActions[action.first] = action.second;
            }
            new_edge_pair.first.first->edges.push_back(new_edge);
        }
    }

    return automata;
}