#include <algorithm>
#include <string>

#include "regex.h"
#include "../bt/binary_tree.h"


set<string> Regexp::alt_init_without_read() {
    set<string> unread;
    for (auto *sub_r: sub_regexps) {
        if (!sub_r->unread_init.empty()) {
            union_sets(unread, sub_r->unread_init);
        }
    }
    return unread;
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
            if (init.find(var) == init.end()) {
                init[var] = 1;
            }
            else {
                init[var] += 1;
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

//        if (initialized_vars.find(variable) == initialized_vars.end()) {
//            read_before_init.insert(variable);
//        }
    }
    else if (regexp_type == alternationExpr || regexp_type == concatenationExpr) {
        for (auto *sub_regex: sub_regexps) {
            set<string> sub_initialized;
            sub_initialized.insert(initialized_vars.begin(), initialized_vars.end());
            set<string> sub_read_before_init;
            sub_regex->_is_backref_correct(sub_initialized,
                                           sub_read_before_init,
                                           double_initialized);
//            if (double_initialized) {
//                break;
//            }

//            if (!read_before_init.empty()) {
//                break;
//            }

            initialized_vars = sub_initialized;

            if (regexp_type == concatenationExpr) {
                // неинициализированным в конкатенации считается чтение, которые до этого необязательно инициализировано
                auto sub_may_read = sub_regex->maybe_read;
                for (const auto& sub_may_r: sub_may_read) {
                    if (initialized.find(sub_may_r) == initialized.end()) {
                        uninited_read.insert(sub_may_r);
                    }
                }

                // добавляем новые непрочитанные переменные, а затем удаляем те, что точно прочитаны с добавлением нового элемента
                union_sets(unread_init, sub_regex->unread_init);
                for (const auto& now_read: sub_regex->read) {
                    if (unread_init.find(now_read) != unread_init.end()) {
                        unread_init.erase(now_read);
                    }
                }

                union_sets(initialized, sub_regex->initialized);
                union_sets(read, sub_regex->read);
            }
            else {
                uninited_read = alt_read_without_init();
                unread_init = alt_init_without_read();
                intersect_sets(initialized, sub_regex->initialized);
                intersect_sets(read, sub_regex->read);
            }
            union_sets(maybe_initialized, sub_regex->maybe_initialized);
            union_sets(maybe_read, sub_regex->maybe_read);
        }
    }
    else if (regexp_type == kleeneStar || regexp_type == kleenePlus || regexp_type == backreferenceExpr) {
        sub_regexp->_is_backref_correct(initialized_vars,
                                        read_before_init,
                                        double_initialized);
        if (regexp_type == backreferenceExpr) {
            initialized.insert(variable);
            maybe_initialized.insert(variable);
            unread_init.insert(variable);

            //        if (double_initialized || !read_before_init.empty()) {
            //            return;
            //        }
            //        if (initialized_vars.find(variable) == initialized_vars.end()) {
            //            initialized_vars.insert(variable);
            //        }
            //        else {
            //            double_initialized = true;
            //        }
        }
        if (regexp_type != kleeneStar) {
            initialized.insert(sub_regexp->initialized.begin(), sub_regexp->initialized.end());
            read.insert(sub_regexp->read.begin(), sub_regexp->read.end());
        }
        maybe_initialized.insert(sub_regexp->maybe_initialized.begin(), sub_regexp->maybe_initialized.end());
        maybe_read.insert(sub_regexp->maybe_read.begin(), sub_regexp->maybe_read.end());
        uninited_read.insert(sub_regexp->uninited_read.begin(), sub_regexp->uninited_read.end());
        unread_init.insert(sub_regexp->unread_init.begin(), sub_regexp->unread_init.end());
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
            printf("Error: less than two sub-regexps");
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

Automata* Regexp::compile() {
    auto *bt = to_binary_tree();
    bt = bt->toSSNF();

    auto *MFA = bt->toMFA();
    MFA->draw("mfa");

    if (have_backreference) {
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
