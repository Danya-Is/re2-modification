#include <algorithm>
#include <string>
#include <iostream>

#include "regex.h"
#include "../bt/binary_tree.h"


bool Regexp::is_equal(Regexp *other) {
    if (!other)
        return false;
    else
        if (regexp_type != other->regexp_type)
        return false;
    else {
        if (regexp_type == epsilon)
            return true;
        else if (regexp_type == literal)
            return other->rune == rune;
        else if (regexp_type == reference)
            return other->variable == variable;
        else if (regexp_type == concatenationExpr || regexp_type == alternationExpr) {
            if (sub_regexps.size() != other->sub_regexps.size())
                return false;
            for (auto it1 = sub_regexps.begin(), it2 = sub_regexps.begin();
            it1 != sub_regexps.end() && it2 != other->sub_regexps.end(); it1++, it2++){
                if (!(*it1)->is_equal(*it2))
                    return false;
            }
            return true;
        }
        else if (regexp_type == kleeneStar || regexp_type == kleenePlus)
            return sub_regexp->is_equal(other->sub_regexp);
        else if (regexp_type == backreferenceExpr)
            return sub_regexp->is_equal(other->sub_regexp) && variable == other->variable;
    }
}

set<string> Regexp::alt_read_without_init() {
    set<string> uninit;
    for (auto *sub_r: sub_regexps) {
        union_sets(uninit,sub_r->uninited_read);
    }
    return uninit;
}

map<string, int> Regexp::alt_init() {
    map<string, int> init;
    for (auto *sub_r: sub_regexps) {
        for (const auto& var: sub_r->initialized) {
            if (init.find(var.first) == init.end()) {
                init[var.first] = 1;
            }
            else {
                init[var.first] += 1;
            }
        }
    }
    return init;
}

void union_sets(set<string> &first, set<string> &second) {
    set<string> union_set;
    set_union(first.begin(), first.end(),
              second.begin(), second.end(),
              inserter(union_set, union_set.begin()));
    first = union_set;
}

void intersect_sets(set<string> &first, set<string> &second) {
    set<string> intersection_set;
    set_intersection(first.begin(), first.end(),
              second.begin(), second.end(),
              inserter(intersection_set, intersection_set.begin()));
    first = intersection_set;
}

void concat_maps(map<string, list<Regexp*>> &first, map<string, list<Regexp*>> &second) {
    for (auto el: second) {
        if (first.find(el.first) == first.end()) {
            first[el.first] = el.second;
        }
        else {
            for (auto var: el.second) {
                first[el.first].push_back(var);
            }
        }
    }
}

void Regexp::_is_backref_correct(set<string> &initialized_vars,
                                 set<string> &read_before_init,
                                 bool& double_initialized) {
    if (regexp_type == epsilon || regexp_type == literal) {
        return;
    }
    else if (regexp_type == reference) {
        read.insert(variable);
        maybe_read.insert(variable);
        uninited_read.insert(variable);
        definitely_uninit_read.insert(variable);

//        if (initialized_vars.find(variable) == initialized_vars.end()) {
//            read_before_init.insert(variable);
//        }
    }
    else if (regexp_type == alternationExpr || regexp_type == concatenationExpr) {
        int i = 0;
        for (auto *sub_regex: sub_regexps) {
            set<string> sub_initialized;
            sub_initialized.insert(initialized_vars.begin(), initialized_vars.end());
            set<string> sub_read_before_init;
            sub_regex->_is_backref_correct(sub_initialized,
                                           sub_read_before_init,
                                           double_initialized);

            initialized_vars = sub_initialized;

            if (regexp_type == concatenationExpr) {
                concat_vars(sub_regex);
            }
            else {
                if (i == 0)
                    copy_vars(sub_regex);
                else
                    alt_vars(sub_regex);
            }
            i++;
        }
    }
    else if (regexp_type == kleeneStar || regexp_type == kleenePlus || regexp_type == backreferenceExpr) {
        sub_regexp->_is_backref_correct(initialized_vars,
                                        read_before_init,
                                        double_initialized);
        if (regexp_type != kleeneStar)
            copy_vars(sub_regexp);
        else
            star_kleene_vars(sub_regexp);

        if (regexp_type == backreferenceExpr) {
            initialized[variable].push_back(this);
            maybe_initialized.insert(variable);
            unread_init.insert(variable);
            definitely_unread_init.insert(variable);
        }
    }
}

Regexp *Regexp::simplify_conc_alt() {
    if ((regexp_type == concatenationExpr || regexp_type == alternationExpr) &&
            sub_regexps.size() == 1) {
        return sub_regexps.front();
    }
    else if ((regexp_type == concatenationExpr || regexp_type == alternationExpr) &&
             sub_regexps.empty()) {
        return new Regexp(epsilon);
    }
    else
        return this;
}

string Regexp::to_string() {
    if (regexp_type == epsilon) {
        return "ε";
    }
    else if (regexp_type == literal) {
        return string(1, rune);
    }
    else if (regexp_type == reference) {
        return "&" + variable;
    }
    else if (regexp_type == concatenationExpr) {
        string res = "";
        for (auto *sub_r: sub_regexps) {
            res += sub_r->to_string();
        }
        return res;
    }
    else if (regexp_type == alternationExpr) {
        string res = "(";
        for (auto *sub_r: sub_regexps) {
            res += sub_r->to_string() + "|";
        }
        if (res == "(")
            return "(ε)";
        res.pop_back();

        res += ")";
        return res;
    }
    else if (regexp_type == backreferenceExpr) {
        return "{" + sub_regexp->to_string() +  "}:" + variable;
    }
    else if (regexp_type == kleeneStar) {
        if (sub_regexp->regexp_type == alternationExpr || sub_regexp->regexp_type == literal ||
        sub_regexp->regexp_type == epsilon || sub_regexp->regexp_type == regexp_type)
            return sub_regexp->to_string() + "*";
        else
            return "(" + sub_regexp->to_string() +  ")*";
    }
    else if (regexp_type == kleenePlus) {
        if (sub_regexp->regexp_type == alternationExpr)
            return sub_regexp->to_string() + "+";
        else
            return "(" + sub_regexp->to_string() +  ")+";
    }
}

bool Regexp::is_backref_correct() {
    set<string> initialized_vars;
    set<string> read_before_init;
    bool double_initialized;
    _is_backref_correct(initialized_vars, read_before_init, double_initialized);

    if (double_initialized || !read_before_init.empty()) {
        return false;
    }
    else {
        return true;
    }
}

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
    }
    else if (regexp_type == backreferenceExpr) {
        tr->child = sub_regexp->to_binary_tree();
        tr->variable = variable;
    }
    else {
        if (sub_regexps.size() < 2) {
            tr->type = sub_regexps.front()->regexp_type;
            tr = sub_regexps.front()->to_binary_tree();
        }
        else if (sub_regexps.size() == 2) {
            tr->left = sub_regexps.front()->to_binary_tree();
            tr->right = sub_regexps.back()->to_binary_tree();;
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
        }
    }

    return tr;
}

Automata * Regexp::compile(bool &is_mfa, bool use_reverse, bool use_bnf, bool use_ssnf) {
    is_backref_correct();
    auto *bt = to_binary_tree();
//    bt = bt->toSSNF();

    if (!maybe_initialized.empty() || !maybe_read.empty()) {
        cout << "Используется память" << endl;
        is_mfa = true;
        bool is_one_unam = bt->is_one_unambiguity();
        if (!is_one_unam && use_reverse) {
            auto* bnf_regexp = bnf();

            if (!bnf_regexp->is_bad_bnf) {
                cout << "BNF: " << bnf_regexp->to_string() << endl;
                auto *reverse_bnf = bnf_regexp->reverse();
                cout << "Reverse: " << reverse_bnf->to_string() << endl;
                auto *reverse_bt = reverse_bnf->to_binary_tree();
                if (use_ssnf)
                    reverse_bt->toSSNF();
                auto *MFA = reverse_bt->toMFA();
                MFA->is_reversed = true;
                MFA->draw("reverse_mfa");
                return MFA;
            }
            else {
                if (use_ssnf)
                    bt->toSSNF();
                auto *MFA = bt->toMFA();
                MFA->draw("mfa");
                return MFA;
            }

        }
        else {
            if (is_one_unam){
                cout << "1-однозначность" << endl;
                this->is_one_unamb = true;
            }
            if (use_bnf) {
                auto* bnf_regex = bnf();
                bt = bnf_regex->to_binary_tree();
            }
            if (use_ssnf)
                bt->toSSNF();
            auto *MFA = bt->toMFA();
            MFA->draw("mfa");
            return MFA;
        }
    }
    else {
        is_mfa = false;
        cout << "Без использования памяти" << endl;
        if (bt->is_one_unambiguity()) {
            cout << "1-однозначность" << endl;
            if (use_ssnf)
                bt->toSSNF();
            auto *glushkov = bt->toGlushkov();
            return glushkov;
        }
        else {
            auto *reverse_bt = reverse()->to_binary_tree();
            if (use_ssnf)
                reverse_bt = reverse_bt->toSSNF();
            auto *reverse_glushkov = reverse_bt->toGlushkov();
            reverse_glushkov->is_reversed = true;
            reverse_glushkov->draw("reverse");
            if (reverse_glushkov->isDeterministic()) {
                return reverse_glushkov;
            }
            else {
                if (use_ssnf)
                    bt->toSSNF();
                auto *thompson = bt->toThomson();
                return thompson;
            }
        }
    }
}
