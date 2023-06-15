#include "binary_tree.h"
#include <string>
#include <map>
#include <set>

using namespace std;

list<string> BinaryTree::linearize(int &start_idx) {
    list<string> literals;
    if (type == literal) {
        string r(&rune);
        name = r + to_string(start_idx);
        literals.push_back(name);
        start_idx++;
    }
    else if (type == alternationExpr || type == concatenationExpr) {
        literals.merge(left->linearize(start_idx));
        literals.merge(right->linearize(start_idx));
    }
    else if (type == kleeneStar || type == kleenePlus) {
        literals.merge(child->linearize(start_idx));
    }

    return literals;
}

list<string> BinaryTree::doFIRST() {
    list<string> first;
    if (type == epsilon) {}
    else if (type == reference)
        first.push_back(variable);
    else if (type == literal) {
        first.push_back(name);
    }
    else if (type == alternationExpr) {
        first.merge(left->doFIRST());
        first.merge(right->doFIRST());
    }
    else if (type == concatenationExpr) {
        first.merge(left->doFIRST());
        if (left->epsilonProducing()) {
            first.merge(right->doFIRST());
        }
    }
    else if (type == kleeneStar || type == kleenePlus || type == backreferenceExpr) {
        first.merge(child->doFIRST());
    }
    return first;
}

list<string> BinaryTree::doLAST() {
    list<string> first;
    if (type == literal) {
        first.push_back(name);
    }
    else if (type == alternationExpr) {
        first.merge(left->doLAST());
        first.merge(right->doLAST());
    }
    else if (type == concatenationExpr) {
        first.merge(right->doLAST());
        if (right->epsilonProducing()) {
            first.merge(left->doLAST());
        }
    }
    else if (type == kleeneStar) {
        first.merge(child->doLAST());
    }
    return first;
}

set<pair<string, string>> BinaryTree::doFOLLOW() {
    set<pair<string, string>> follow;
    if (type == alternationExpr) {
        auto left_follow = left->doFOLLOW();
        auto right_follow = right->doFOLLOW();
        follow.insert(left_follow.begin(), left_follow.end());
        follow.insert(right_follow.begin(), right_follow.end());
    }
    else if (type == concatenationExpr) {
        auto first = left->doLAST();
        auto second = right->doFIRST();
        for (const auto& f: first) {
            for (const auto& s: second) {
                follow.insert(make_pair(f, s));
            }
        }
        auto left_follow = left->doFOLLOW();
        auto right_follow = right->doFOLLOW();
        follow.insert(left_follow.begin(), left_follow.end());
        follow.insert(right_follow.begin(), right_follow.end());
    }
    else if (type == kleeneStar) {
        auto first = child->doLAST();
        auto second = child->doFIRST();
        for (const auto& f: first) {
            for (const auto& s: second) {
                follow.insert(make_pair(f, s));
            }
        }
        auto child_follow = child->doFOLLOW();
        follow.insert(child_follow.begin(), child_follow.end());
    }
    return follow;
}

Automata *BinaryTree::toGlushkov() {
    auto* automata = new Automata();

    int start_p = 0;
    auto literals = linearize(start_p);
    map<string, Node*> named_nodes;
    for (const auto& lit: literals) {
        named_nodes[lit] = new Node(lit);
        automata->nodes.push_back(named_nodes[lit]);
    }

    auto first = doFIRST();
    auto last = doLAST();
    auto follow = doFOLLOW();

    for(auto name: first) {
        string by = substr(name, 1);
        automata->start->edges.push_back(new Edge(by, named_nodes[name]));
    }

    if (last.size() == 1) {
        automata->finish = named_nodes[last.back()];
    }
    else {
        for(auto name: last) {
            named_nodes[name]->edges.push_back(new Edge("", automata->finish));
        }
    }

    for (auto p: follow) {
        string by = substr(p.second, 1);
        named_nodes[p.first]->edges.push_back(new Edge(by, named_nodes[p.second]));
    }

    return automata;
}
