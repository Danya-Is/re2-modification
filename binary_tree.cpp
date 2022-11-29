#include "binary_tree.h"
#include <string>

using namespace std;


Automata *BinaryTree::toNFA() {
    auto* automata = new Automata();
    if (type == literal) {
        string s(&rune);
        automata->start->edges.push_back(new Edge(s, automata->finish));
    }
    else if (type == alternationExpr) {
        Automata* first_automata = left->toNFA();
        Automata* second_automata = right->toNFA();
        automata->start->edges.push_back(new Edge("", first_automata->start));
        automata->start->edges.push_back(new Edge("", second_automata->start));

        first_automata->finish->edges.push_back(new Edge("", automata->finish));
        second_automata->finish->edges.push_back(new Edge("", automata->finish));

        automata->nodes.merge(first_automata->nodes);
        automata->nodes.merge(second_automata->nodes);
    }
    else if (type == concatenationExpr) {
        Automata* first_automata = left->toNFA();
        Automata* second_automata = right->toNFA();

        first_automata->nodes.merge(second_automata->nodes);
        first_automata->changeFinalState(second_automata->start);

        automata = first_automata;
        automata->finish = second_automata->finish;
    }
    else if (type == kleeneStar) {
        Automata* old = child->toNFA();
        old->finish->edges.push_back(new Edge("", old->start));
        old->finish->edges.push_back(new Edge("", automata->finish));

        automata->nodes.merge(old->nodes);

        automata->start->edges.push_back(new Edge("", old->start));
        automata->start->edges.push_back(new Edge("", automata->finish));
    }
    return automata;
}
