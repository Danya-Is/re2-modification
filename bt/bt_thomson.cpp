#include "binary_tree.h"
#include <string>
#include <set>

using namespace std;


Automata *BinaryTree::toThomson() {
    auto* automata = new Automata();
    if (type == literal) {
        string s(&rune);
        automata->start->edges.push_back(new Edge(s, automata->finish));

        automata->finish->finish_for = automata->start;
        automata->start->start_for = automata->finish;
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

        automata->finish->finish_for = automata->start;
        automata->start->start_for = automata->finish;
    }
    else if (type == concatenationExpr) {
        Automata* first_automata = left->toThomson();
        Automata* second_automata = right->toThomson();

        second_automata->start->finish_for = first_automata->finish->finish_for;
        first_automata->changeFinalState(second_automata->start);

        automata = first_automata;
        automata->nodes.merge(second_automata->nodes);
        automata->finish = second_automata->finish;
    }
    else if (type == kleeneStar) {
        Automata* old = child->toThomson();
        old->finish->edges.push_back(new Edge("", old->start));
        old->finish->edges.push_back(new Edge("", automata->finish));

        automata->nodes.merge(old->nodes);

        automata->start->edges.push_back(new Edge("", old->start));
        automata->start->edges.push_back(new Edge("", automata->finish));

        automata->finish->finish_for = automata->start;
        automata->start->start_for = automata->finish;
    }
    else {
        printf("UNKNOWN BINARY TREE TYPE!!!!!");
    }

    return automata;
}