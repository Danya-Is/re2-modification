#include "regex.h"
#include "binary_tree.h"

BinaryTree *Regexp::to_binary_tree() {
    auto* tr = new BinaryTree();
    tr->type = regexp_type;
    if (regexp_type == literal) {
        tr->rune = rune;
    }
    else if (regexp_type == kleeneStar) {
        tr->child = sub_regexp->to_binary_tree();
    }
    else {
        if (sub_regexps.size() < 2) {
            printf("Error: less than two sub-regexps");
        }
        else if (sub_regexps.size() == 2) {
            tr->left = sub_regexps.front()->to_binary_tree();
            tr->right = sub_regexps.back()->to_binary_tree();;
        }
        else {
            tr->left = sub_regexps.front()->to_binary_tree();
            list<Regexp*> new_sub_regexps(sub_regexps.begin(), sub_regexps.end());
            new_sub_regexps.pop_front();
            auto* re = new Regexp();
            re->regexp_type = regexp_type;
            re->sub_regexps = new_sub_regexps;
            tr->right = re->to_binary_tree();
        }
    }

    return tr;
}
