#include <iostream>

#include "regex.h"

void Regexp::bind_init_to_read(map<string, list<Regexp *>> init) {
    if (regexp_type == epsilon || regexp_type == literal){}
    else if (regexp_type == reference) {
        if (init.find(variable) != init.end()){
            reference_to = init[variable].front();
            for (auto backref: init[variable]){
                backref->is_read = true;
            }
        }

    }
    else if (regexp_type == concatenationExpr || regexp_type == alternationExpr) {
        for (auto &sub_r : sub_regexps) {
            sub_r->bind_init_to_read(init);
            if (regexp_type == concatenationExpr && !sub_r->initialized.empty()) {
                for (auto el: sub_r->initialized) {
                    // интересна последняя инициализация перед возможным чтением
                    init[el.first].push_back(el.second.back());
                }
            }
        }
    }
    else if (regexp_type == kleeneStar || regexp_type == kleenePlus || regexp_type == backreferenceExpr) {
        if (regexp_type == kleeneStar || regexp_type == kleenePlus) {
            // если последняя инициализация под итерацией совпадает с последней инициализацией перед ней (w(rw)*)
            for (auto write: sub_regexp->initialized) {
                auto var_name = write.first;
                auto r = write.second.back();
                if (init.find(var_name) != init.end() &&
                        init[var_name].back()->is_equal(r))
                    init[var_name].push_back(r);
            }
        }
        sub_regexp->bind_init_to_read(init);
    }
}

Regexp *Regexp::_reverse() {
    if (regexp_type == epsilon || regexp_type == literal || regexp_type == reference) {
        return this;
    }
    else if (regexp_type == backreferenceExpr) {
        // безболезненно ли тут создавать новую регулярку (нет)
//        auto *new_r = new Regexp(backreferenceExpr);
//        new_r->variable = variable;
        sub_regexp = sub_regexp->_reverse();
        return this;
    }
    else if (regexp_type == kleeneStar || regexp_type == kleenePlus) {
        auto *new_r = new Regexp(regexp_type);
        new_r->sub_regexp = sub_regexp->_reverse();
        return new_r;
    }
    else if (regexp_type == concatenationExpr || regexp_type == alternationExpr) {
        auto *new_r = new Regexp(regexp_type);
        for (auto *sub_r: sub_regexps) {
            auto *new_sub_r = sub_r->_reverse();
            if (regexp_type == concatenationExpr)
                new_r->sub_regexps.push_front(new_sub_r);
            else
                new_r->sub_regexps.push_back(new_sub_r);
        }
        return new_r;
    }
}

Regexp *Regexp::replace_read_write(set<Regexp *> &initialized_in_reverse) {
    if (regexp_type == epsilon || regexp_type == literal) {
        return this;
    }
    else if (regexp_type == reference) {
        if (initialized_in_reverse.find(reference_to) == initialized_in_reverse.end()) {
//            auto *new_r = new Regexp(backreferenceExpr);
//            new_r->sub_regexp = reference_to->reverse(initialized_in_reverse);
//            new_r->variable = reference_to->variable;

            initialized_in_reverse.insert(reference_to);
            reference_to->sub_regexp = reference_to->sub_regexp->replace_read_write(initialized_in_reverse);

            return reference_to;
        }
        else {
            return this;
        }
    }
    else if (regexp_type == backreferenceExpr) {
        if (initialized_in_reverse.find(this) == initialized_in_reverse.end()) {
            initialized_in_reverse.insert(this);
            return this;
        }
        else{
            auto *new_r = new Regexp(reference);
            new_r->variable = variable;
            return new_r;
        }
    }
    else if (regexp_type == kleeneStar || regexp_type == kleenePlus) {
        auto *new_r = new Regexp(regexp_type);
        new_r->sub_regexp = sub_regexp->replace_read_write(initialized_in_reverse);
        return new_r;
    }
    else if (regexp_type == concatenationExpr || regexp_type == alternationExpr) {
        auto *new_r = new Regexp(regexp_type);
        for (auto *sub_r: sub_regexps) {
            auto *new_sub_r = sub_r->replace_read_write(initialized_in_reverse);
            new_r->sub_regexps.push_back(new_sub_r);
        }
        return new_r;
    }
}

Regexp *Regexp::reverse() {
    set<Regexp*> empty_set;
    auto *reversed = _reverse();

//    cout << reversed->to_string() << endl;

    return reversed->replace_read_write(empty_set);
}
