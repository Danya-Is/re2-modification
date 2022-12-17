#include "binary_tree.h"
#include <string>
#include <map>
#include <set>

using namespace std;


Automata *BinaryTree::toThomson() {
    auto* automata = new Automata();
    if (type == literal) {
        string s(&rune);
        automata->start->edges.push_back(new Edge(s, automata->finish));
    }
    else if (type == alternationExpr) {
        Automata* first_automata = left->toThomson();
        Automata* second_automata = right->toThomson();
        automata->start->edges.push_back(new Edge("", first_automata->start));
        automata->start->edges.push_back(new Edge("", second_automata->start));

        first_automata->finish->edges.push_back(new Edge("", automata->finish));
        second_automata->finish->edges.push_back(new Edge("", automata->finish));

        automata->nodes.merge(first_automata->nodes);
        automata->nodes.merge(second_automata->nodes);
    }
    else if (type == concatenationExpr) {
        Automata* first_automata = left->toThomson();
        Automata* second_automata = right->toThomson();

        first_automata->nodes.merge(second_automata->nodes);
        first_automata->changeFinalState(second_automata->start);

        automata = first_automata;
        automata->finish = second_automata->finish;
    }
    else if (type == kleeneStar) {
        Automata* old = child->toThomson();
        old->finish->edges.push_back(new Edge("", old->start));
        old->finish->edges.push_back(new Edge("", automata->finish));

        automata->nodes.merge(old->nodes);

        automata->start->edges.push_back(new Edge("", old->start));
        automata->start->edges.push_back(new Edge("", automata->finish));
    }
    else {
        printf("UNKNOWN BINARY TREE TYPE!!!!!");
    }
    return automata;
}

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
    else if (type == kleeneStar) {
        literals.merge(child->linearize(start_idx));
    }

    return literals;
}

bool BinaryTree::epsilonProducing() {
    if (type == literal) {
        return false;
    }
    else if (type == alternationExpr) {
        return (left->epsilonProducing() || right->epsilonProducing());
    }
    else if (type == concatenationExpr) {
        return (left->epsilonProducing() && right->epsilonProducing());
    }
    else if (type == kleeneStar) {
        return true;
    }
    else {
        printf("UNKNOWN BINARY TREE TYPE!!!!!");
        return true;
    }
}

list<string> BinaryTree::doFIRST() {
    list<string> first;
    if (type == literal) {
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
    else if (type == kleeneStar) {
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

std::string substr(std::string originalString, int maxLength)
{
    std::string resultString = originalString;

    int len = 0;
    int byteCount = 0;

    const char* aStr = originalString.c_str();

    while(*aStr)
    {
        if( (*aStr & 0xc0) != 0x80 )
            len += 1;

        if(len>maxLength)
        {
            resultString = resultString.substr(0, byteCount);
            break;
        }
        byteCount++;
        aStr++;
    }

    return resultString;
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

MFA* BinaryTree::toMFA() {
    auto* automata = new MFA();
    if (type == epsilon) {
        automata->start->edges.push_back(new MemoryEdge("", automata->finish));
    }
    else if (type == literal) {
        auto* node = new MemoryNode();
        string by(&rune);
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
                        new_edges.push_back(new MemoryEdge(edge->by + (*i)->by, (*i)->to));
                    }
                    edge->to = (*end)->to;
                    edge->by = edge->by + (*end)->by;
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
            new_edge_pair.first.first->edges.push_back(
                    new MemoryEdge(final_edge->by + start_edge->by, start_edge->to)
            );
        }
    }

    return automata;
}




