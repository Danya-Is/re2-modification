#include "regex.h"


void Regexp::_push_sub_regexp(Regexp *sub_r, bool front) {
    if (front)
        sub_regexps.push_front(sub_r);
    else
        sub_regexps.push_back(sub_r);
}

void Regexp::push_concat(Regexp *sub_r, bool front) {
    if (regexp_type == concatenationExpr) {
        if (front) {
            auto it = sub_r->sub_regexps.end();
            it--;
            while (it != sub_r->sub_regexps.end()){
                _push_sub_regexp(*it, front);
                it--;
            }
        }
        for (auto *sub_sub_r: sub_r->sub_regexps)
            _push_sub_regexp(sub_sub_r, front);
    }
    else
        _push_sub_regexp(sub_r, front);
}

void Regexp::push_alt(Regexp *sub_r, bool front) {
    if (regexp_type == alternationExpr) {
        for (auto *sub_sub_r: sub_r->sub_regexps)
            _push_sub_regexp(sub_sub_r);
    }
    else
        _push_sub_regexp(sub_r, front);
}

void Regexp::push_sub_regexp(Regexp *sub_r, bool front) {
    if (regexp_type == concatenationExpr)
        concat_vars(sub_r);
    else if (regexp_type == alternationExpr) {
        if (sub_regexps.empty())
            copy_vars(sub_r);
        else
            alt_vars(sub_r);
    }
    if (sub_r->regexp_type == concatenationExpr)
        push_concat(sub_r, front);
    else if (sub_r->regexp_type == alternationExpr)
        push_alt(sub_r, front);
    else
        _push_sub_regexp(sub_r, front);
}

void Regexp::concat_vars(Regexp *regexp) {
    if (sub_regexps.empty())
        copy_vars(regexp);
    else {
        set<string> new_uninited_read(regexp->uninited_read.begin(), regexp->uninited_read.end());
        for (const auto& now_init: initialized) {
            if (new_uninited_read.find(now_init.first) != new_uninited_read.end()) {
                new_uninited_read.erase(now_init.first);
            }
        }
        union_sets(uninited_read, new_uninited_read);

        union_sets(unread_init, regexp->unread_init);
        for (const auto& now_read: regexp->read) {
            if (unread_init.find(now_read) != unread_init.end()) {
                unread_init.erase(now_read);
            }
        }

        set<string> new_definitely_uninited_read(regexp->uninited_read.begin(), regexp->uninited_read.end());
        for (const auto& now_maybe_init: maybe_initialized) {
            if (new_definitely_uninited_read.find(now_maybe_init) != new_definitely_uninited_read.end()) {
                new_definitely_uninited_read.erase(now_maybe_init);
            }
        }
        union_sets(definitely_uninit_read, new_definitely_uninited_read);

        for (const auto& now_maybe_read: regexp->maybe_read) {
            if (definitely_unread_init.find(now_maybe_read) != definitely_unread_init.end()) {
                definitely_unread_init.erase(now_maybe_read);
            }
        }
        union_sets(definitely_unread_init, regexp->definitely_unread_init);

        concat_maps(initialized, regexp->initialized);
        union_sets(read, regexp->read);
        union_sets(maybe_read, regexp->maybe_read);
        union_sets(maybe_initialized, regexp->maybe_initialized);
    }
}

void Regexp::alt_vars(Regexp *regexp) {
    union_sets(uninited_read, regexp->uninited_read);
    union_sets(unread_init, regexp->unread_init);
    union_sets(definitely_unread_init, regexp->definitely_unread_init);
    union_sets(definitely_uninit_read, regexp->definitely_uninit_read);

    intersect_sets(read, regexp->read);
    union_sets(maybe_read, regexp->maybe_read);
    union_sets(maybe_initialized, regexp->maybe_initialized);
}

void Regexp::star_kleene_vars(Regexp *regexp) {
    maybe_initialized = regexp->maybe_initialized;
    maybe_read = regexp->maybe_read;
    uninited_read = regexp->uninited_read;
    unread_init = regexp->unread_init;
    definitely_unread_init = regexp->definitely_unread_init;
    definitely_uninit_read = regexp->definitely_uninit_read;
}

void Regexp::copy_vars(Regexp *regexp) {
    union_sets(uninited_read, regexp->uninited_read);
    union_sets(definitely_unread_init, regexp->definitely_unread_init);
    union_sets(unread_init, regexp->unread_init);
    if (regexp_type != alternationExpr)
        concat_maps(initialized, regexp->initialized);
    union_sets(read, regexp->read);
    union_sets(maybe_read, regexp->maybe_read);
    union_sets(maybe_initialized, regexp->maybe_initialized);
    union_sets(definitely_uninit_read, regexp->definitely_uninit_read);
}

void Regexp::change_vars(Regexp *regexp) {
    initialized = regexp->initialized;
    read = regexp->read;
    star_kleene_vars(regexp);
}

Regexp *Regexp::copy(Regexp *regexp) {
    if (regexp->regexp_type == epsilon || regexp->regexp_type == literal) {
        return regexp;
    }
    else if (regexp->regexp_type == reference) {
        auto *new_r = new Regexp(reference);
        new_r->variable = regexp->variable;
        new_r->reference_to = regexp->reference_to;
        new_r->copy_vars(regexp);
        return new_r;
    }
    else if (regexp->regexp_type == backreferenceExpr) {
        auto *new_r = new Regexp(backreferenceExpr);
        new_r->variable = regexp->variable;
        new_r->sub_regexp = copy(regexp->sub_regexp);

        new_r->copy_vars(new_r->sub_regexp);
        new_r->definitely_unread_init.insert(regexp->variable);
        new_r->unread_init.insert(regexp->variable);
        new_r->maybe_initialized.insert(regexp->variable);
        new_r->initialized[regexp->variable].push_back(new_r);
        return new_r;
    }
    else if (regexp->regexp_type == kleeneStar || regexp->regexp_type == kleenePlus) {
        auto *new_r = new Regexp(regexp->regexp_type);
        new_r->sub_regexp = copy(regexp->sub_regexp);
        return new_r;
    }
    else if (regexp->regexp_type == alternationExpr || regexp->regexp_type == concatenationExpr) {
        auto *new_r = new Regexp(regexp->regexp_type);
        int i = 0;
        for (auto *sub_r: regexp->sub_regexps) {
            auto *copy_sub_r = copy(sub_r);

            if (regexp->regexp_type == concatenationExpr) {
                new_r->concat_vars(copy_sub_r);
            }
            else {
                if (i == 0)
                    new_r->copy_vars(copy_sub_r);
                else
                    new_r->alt_vars(copy_sub_r);
            }
            new_r->sub_regexps.push_back(copy_sub_r);
            i++;
        }
        return new_r;
    }
}