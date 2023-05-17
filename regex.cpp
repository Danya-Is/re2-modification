#include <algorithm>
#include <string>

#include "regex.h"
#include "bt/binary_tree.h"

BinaryTree *Regexp::to_binary_tree() {
    auto* tr = new BinaryTree();
    tr->type = regexp_type;
    if (regexp_type == epsilon) {
        return tr;
    }
    else if (regexp_type == literal) {
        tr->rune = rune;
    }
    else if (regexp_type == reference) {
        tr->variable = variable;
    }
    else if (regexp_type == kleeneStar || regexp_type == kleenePlus) {
        tr->child = sub_regexp->to_binary_tree();
        tr->have_backreference = tr->child->have_backreference;
    }
    else if (regexp_type == backreferenceExpr) {
        tr->have_backreference = true;
        tr->child = sub_regexp->to_binary_tree();
        tr->variable = variable;
    }
    else {
        if (sub_regexps.size() < 2) {
            printf("Error: less than two sub-regexps");
        }
        else if (sub_regexps.size() == 2) {
            tr->left = sub_regexps.front()->to_binary_tree();
            tr->right = sub_regexps.back()->to_binary_tree();;
            tr->have_backreference = tr->left->have_backreference || tr->right->have_backreference;
        }
        else {
            auto* sub_r = sub_regexps.front();
            tr->left = sub_r->to_binary_tree();
            list<Regexp*> new_sub_regexps(sub_regexps.begin(), sub_regexps.end());
            new_sub_regexps.pop_front();
            auto* re = new Regexp();
            re->regexp_type = regexp_type;
            re->sub_regexps = new_sub_regexps;
            tr->right = re->to_binary_tree();
            tr->have_backreference = tr->left->have_backreference || tr->right->have_backreference;
        }
    }

    return tr;
}

Automata* Regexp::compile() {
    auto *bt = to_binary_tree();
    bt = bt->toSSNF();

    auto *MFA = bt->toMFA();
    MFA->draw("mfa");

    if (bt->have_backreference) {
        auto *MFA = bt->toMFA();
        MFA->draw("mfa");
        return MFA;
    }
    else {
        auto *glushkov = bt->toGlushkov();
        if (glushkov->isDeterministic()) {
            return glushkov;
        }
        else {

            auto *reverse_bt = bt->reverse();
            reverse_bt = reverse_bt->toSSNF();
            auto *reverse_glushkov = reverse_bt->toGlushkov();
            reverse_glushkov->is_reversed = true;
            reverse_glushkov->draw("reverse");
            if (reverse_glushkov->isDeterministic()) {
                return reverse_glushkov;
            }
            else {
                auto *thompson = bt->toThomson();
                return thompson;
            }
        }
    }
}
