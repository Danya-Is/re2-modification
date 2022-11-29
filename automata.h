#include "list"

#include "node.h"
#include "regex.h"

#ifndef COURSEWORK_AUTOMATA_H
#define COURSEWORK_AUTOMATA_H

using namespace std;


class Automata { ;
public:
    Node* start;
    Node* finish;
    int last_idx;
    list<Node*> nodes;

    Automata() {
        start = new Node();
        finish = new Node();
        nodes.push_back(start);
        nodes.push_back(finish);
        last_idx = 0;
    }

    void makeDOTFile();

    bool draw();

    void changeFinalState(Node *new_final);
};

#endif //COURSEWORK_AUTOMATA_H
