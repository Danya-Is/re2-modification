#include <iostream>
#include <algorithm>
#include <string>
#include <utility>
#include <queue>

#include "regex.h"


Regexp *Regexp::take_out_alt_under_backref() {
    /// {(a|b)}:1 == ({a}:1|{b}:1)
    if (regexp_type == backreferenceExpr && sub_regexp->regexp_type == alternationExpr) {
        auto *new_r = new Regexp(alternationExpr);
        for (auto *sub_r: sub_regexp->sub_regexps) {
            auto *new_sub_r = new Regexp(backreferenceExpr);
            new_sub_r->variable = variable;
            new_sub_r->sub_regexp = sub_r;
            new_sub_r->copy_vars(sub_r);

            new_r->sub_regexps.push_back(new_sub_r);
        }

        new_r->copy_vars(this);
        return new_r;
    }
    else {
        return this;
    }
}

Regexp *Regexp::clear_initializations_and_read(set<string> init_vars, set<string> read_vars) {
    // TODO delete epsilon in concatenation
//    if (init_vars.empty() && read_vars.empty()) {
//        return this;
//    }
    auto *new_r = new Regexp();
    if (regexp_type == epsilon) {
        new_r->regexp_type = regexp_type;
    }
    else if (regexp_type == literal) {
        new_r->regexp_type = regexp_type;
        new_r->rune = rune;
    }
    else if (regexp_type == reference) {
        if (read_vars.find(variable) != read_vars.end()) {
            new_r->regexp_type = epsilon;
            new_r->uninited_read.erase(variable);
        }
        else{
            new_r->regexp_type = regexp_type;
            new_r->reference_to = reference_to;
            new_r->variable = variable;
        }
    }
    else if (regexp_type == backreferenceExpr) {
//        if (init_vars.find(variable) != init_vars.end()) {
        if (!is_read){
            new_r->regexp_type = sub_regexp->regexp_type;
            new_r->copy_vars(this);

            // кажется что этого достаточно, тк гарантируется что эта переменная никогда не прочитается
            new_r->initialized.erase(variable);
            new_r->maybe_initialized.erase(variable);
            new_r->unread_init.erase(variable);
            new_r->definitely_unread_init.erase(variable);

            if (new_r->regexp_type == alternationExpr || new_r->regexp_type == concatenationExpr) {
                new_r->sub_regexps = sub_regexp->sub_regexps;
            }
            else if (new_r->regexp_type == kleenePlus || new_r->regexp_type == kleeneStar) {
                new_r->sub_regexp = sub_regexp->sub_regexp;
            }
            else if (new_r->regexp_type == reference) {
                new_r->variable = sub_regexp->variable;
            }
            else {
                new_r->rune = sub_regexp->rune;
            }
            new_r = new_r->clear_initializations_and_read(init_vars, read_vars);
        }
        else {
//            new_r->regexp_type = backreferenceExpr;
//            new_r->variable = variable;
            sub_regexp = sub_regexp->clear_initializations_and_read(init_vars, read_vars);
            return this;
        }
    }
    else if (regexp_type == concatenationExpr || regexp_type == alternationExpr) {
        new_r->regexp_type = regexp_type;
        for (auto *sub_r: sub_regexps) {
            if (regexp_type == concatenationExpr)
            {
                auto *new_sub_r = sub_r->clear_initializations_and_read(init_vars, read_vars);
                new_r->concat_vars(new_sub_r);
                new_r->sub_regexps.push_back(new_sub_r);
            }
            else {
                set<string> sub_init_vars_to_delete(init_vars.begin(), init_vars.end());
                intersect_sets(sub_init_vars_to_delete, sub_r->definitely_unread_init);
                set<string> sub_read_vars_to_delete(read_vars.begin(), read_vars.end());
                intersect_sets(sub_read_vars_to_delete, sub_r->uninited_read);

                auto *new_sub_r = sub_r->clear_initializations_and_read(sub_init_vars_to_delete, sub_read_vars_to_delete);
                new_r->sub_regexps.push_back(new_sub_r);
                new_r->alt_vars(new_sub_r);
            }
        }
    }
    else if (regexp_type == kleeneStar || regexp_type == kleenePlus) {
        new_r->regexp_type = regexp_type;
        new_r->sub_regexp = sub_regexp->clear_initializations_and_read(init_vars, read_vars);
        if (regexp_type == kleeneStar)
            new_r->star_kleene_vars(new_r->sub_regexp);
        else
            new_r->copy_vars(new_r->sub_regexp);
    }
    return new_r;
}


Regexp* Regexp::distribute_to_right(int alt_pos) {
    /// (a|b)c -> (ac|bc)
    if (regexp_type == concatenationExpr) {
        size_t n = sub_regexps.size();
        if (n >= alt_pos) {
            auto* new_regexp = new Regexp(concatenationExpr);
            auto it = sub_regexps.begin();
            for (int i = 0; i < alt_pos; i++) {
                new_regexp->sub_regexps.push_back(*it);
                new_regexp->concat_vars((*it));
                it++;
            }

            auto* alt = *it;
            it++;

            auto* tail = new Regexp(concatenationExpr);
            set<string> uninited_read_t;
            for (int i = alt_pos + 1; i < n; i++) {
                tail->sub_regexps.push_back(*it);
                tail->concat_vars((*it));
                it++;
            }

            auto *new_alt = new Regexp(alternationExpr);
            int k = 0;
            for (auto *sub_r: alt->sub_regexps) {
                auto *new_sub_r = new Regexp(concatenationExpr);

                new_sub_r->copy_vars(sub_r);
                if (sub_r->regexp_type == concatenationExpr){
                    for (auto *sub_sub_r: sub_r->sub_regexps)
                        new_sub_r->sub_regexps.push_back(sub_sub_r);
                }
                else
                    new_sub_r->sub_regexps.push_back(sub_r);

                for (auto *r: tail->sub_regexps){
                    auto *copy_r = copy(r);
                    new_sub_r->concat_vars(copy_r);
                    new_sub_r->sub_regexps.push_back(copy_r);
                }


                new_alt->sub_regexps.push_back(new_sub_r);
                if (k == 0) {
                    new_alt->copy_vars(new_sub_r);
                }
                else {
                    new_alt->alt_vars(new_sub_r);
                }
                k++;
            }

            auto *new_r = new_alt->_bnf();
            ::free(new_alt);
            new_regexp->concat_vars(new_r);
            new_regexp->sub_regexps.push_back(new_r);

            if (new_regexp->sub_regexps.size() == 1) {
                new_regexp->regexp_type = alternationExpr;
                new_regexp->change_vars(new_regexp->sub_regexps.front());
                new_regexp->sub_regexps = new_regexp->sub_regexps.front()->sub_regexps;
            }

            return new_regexp;
        }
        else {
            ::printf("Distribute to right: Index out of range");
        }
    }
    else {
        ::printf("Expected concatenation in distribute()");
    }
}

Regexp *Regexp::distribute_to_left(int alt_pos) {
    /// a(b|c) -> (ab|ac)
    if (regexp_type == concatenationExpr) {
        size_t n = sub_regexps.size();
        if (n >= alt_pos) {
            auto* new_regexp = new Regexp(concatenationExpr);
            auto *head = new Regexp(concatenationExpr);

            auto it = sub_regexps.begin();
            for (int i = 0; i < alt_pos; i++) {
                head->concat_vars(*it);
                head->sub_regexps.push_back(*it);
                it++;
            }

            auto* alt = *it;

            auto *new_alt = new Regexp(alternationExpr);
            int k = 0;
            for (auto *sub_r: alt->sub_regexps) {
                auto *new_sub_r = new Regexp(concatenationExpr);
                for (auto *r: head->sub_regexps) {
                    auto *copy_r = copy(r);
                    new_sub_r->concat_vars(copy_r);
                    new_sub_r->sub_regexps.push_back(copy_r);
                }

                new_sub_r->concat_vars(sub_r);
                if (sub_r->regexp_type == concatenationExpr){
                    for (auto *sub_sub_r: sub_r->sub_regexps)
                        new_sub_r->sub_regexps.push_back(sub_sub_r);
                }
                else
                    new_sub_r->sub_regexps.push_back(sub_r);

                new_alt->sub_regexps.push_back(new_sub_r);
                if (k == 0) {
                    new_alt->copy_vars(new_sub_r);
                }
                else {
                    new_alt->alt_vars(new_sub_r);
                }
                k++;
            }

            auto* new_r = new_alt->_bnf();
            ::free(new_alt);
            new_regexp->concat_vars(new_r);
            new_regexp->sub_regexps.push_back(new_r);

            it++;
            for (int i = alt_pos + 1; i < n; i++) {
                new_regexp->concat_vars(*it);
                new_regexp->sub_regexps.push_back(*it);
                it++;
            }

            if (new_regexp->sub_regexps.size() == 1) {
                new_regexp->regexp_type = alternationExpr;
                new_regexp->change_vars(new_regexp->sub_regexps.front());
                new_regexp->sub_regexps = new_regexp->sub_regexps.front()->sub_regexps;
            }

            return new_regexp;
        }
        else {
            ::printf("Distribute to right: Index out of range");
        }
    }
    else {
        ::printf("Expected concatenation in distribute()");
    }
}

Regexp *Regexp::open_kleene_plus(set<string> vars) {
    auto *new_kleene = this;
    bind_init_to_read({});
    new_kleene = clear_initializations_and_read(std::move(vars), {});
    auto *new_sub = copy(sub_regexp);

    auto *concat = new Regexp(concatenationExpr);
    concat->concat_vars(new_kleene);
    concat->concat_vars(new_sub);
    concat->sub_regexps.push_back(new_kleene);
    concat->sub_regexps.push_back(new_sub);
    return concat;
}

Regexp *Regexp::open_kleene(set<string> vars) {
    /// a* == (a*a|eps), a+ == a*a
    if (regexp_type == kleeneStar) {
        auto *new_r = new Regexp(alternationExpr);
        new_r->sub_regexps.push_back(new Regexp(epsilon));

        auto *concat = open_kleene_plus(vars);

        new_r->sub_regexps.push_back(concat);

        new_r->alt_vars(concat);
        return new_r;
    }
    else if (regexp_type == kleenePlus) {
        return open_kleene_plus(vars);
    }
    else {
        ::printf("Open kleene: Expected kleene");
    }
}

Regexp *Regexp::open_kleene_with_read(set<string> vars) {
    /// (a*) -> (eps|aa*); example: (&1)* -> (eps| &1&1*)
    /// (a|b)* -> a*(ba*)* -> a*(ba*(ba*)*|eps) -> (a*ba*(ba*)*|a*) where b contains read
    if (sub_regexp->regexp_type == alternationExpr) {
        auto *a_alt = new Regexp(alternationExpr);
        auto *b_alt = new Regexp(alternationExpr);

        for (auto *sub_r: sub_regexp->sub_regexps) {
            set<string> sub_read(sub_r->maybe_read.begin(), sub_r->maybe_read.end());
            intersect_sets(sub_read, vars);
            if (sub_read.empty()) {
                a_alt->sub_regexps.push_back(sub_r);
            }
            else
                b_alt->sub_regexps.push_back(sub_r);
        }
        a_alt = a_alt->simplify_conc_alt();
        b_alt = b_alt->simplify_conc_alt();

        auto *a_kleene = new Regexp(kleeneStar);
        a_kleene->sub_regexp = a_alt;
        a_kleene->star_kleene_vars(a_alt);

        auto *sub_concat = new Regexp(concatenationExpr);
        sub_concat->concat_vars(b_alt);
        sub_concat->sub_regexps.push_back(b_alt);
        auto *copy_a_kleene = copy(a_kleene);
        sub_concat->concat_vars(copy_a_kleene);
        sub_concat->sub_regexps.push_back(copy_a_kleene);

        auto *second_kleene = new Regexp(kleeneStar);
        auto *copy_sub_concat = copy(sub_concat);
        second_kleene->sub_regexp = copy_sub_concat;
        second_kleene->star_kleene_vars(copy_sub_concat);

        auto *concat = new Regexp(concatenationExpr);
        concat->concat_vars(a_kleene);
        concat->sub_regexps.push_back(a_kleene);
        concat->concat_vars(sub_concat);
        concat->sub_regexps.push_back(sub_concat);
        concat->concat_vars(second_kleene);
        concat->sub_regexps.push_back(second_kleene);

        auto *new_r = new Regexp(alternationExpr);
        auto *copy_a_kleene_2 = copy(a_kleene);
        new_r->sub_regexps.push_back(copy_a_kleene_2);
        a_alt->copy_vars(copy_a_kleene_2);
        new_r->sub_regexps.push_back(concat);
        new_r->alt_vars(concat);

        return new_r;
    }
    else
        return open_kleene(vars);
}

class MinCompare
{
public:
    bool operator() (const std::pair<string,int>& lhs, const std::pair<string,int>& rhs)
    {
        return lhs.second < rhs.second;
    }
};

class MaxCompare
{
public:
    bool operator() (const std::pair<string,int>& lhs, const std::pair<string,int>& rhs)
    {
        return lhs.second < rhs.second;
    }
};


template<class Compare>
list<string> Regexp::make_var_queue(priority_queue<pair<string, int>, vector<pair<string, int>>, Compare> pq) {
    auto init_vars = alt_init();
    auto read_without_init_vars = alt_read_without_init();

    if (!read_without_init_vars.empty()) {
        for (const auto& var_pair: init_vars) {
            // нас не интересуют переменные, у которых не встречаются свободные чтения
            if (read_without_init_vars.find(var_pair.first) != read_without_init_vars.end()) {
                pq.emplace(var_pair);
            }
        }
    }

    list<string> vars;
    for (; !pq.empty(); pq.pop())
        vars.push_back(pq.top().first);
    return vars;
}

Regexp *Regexp::_open_alt_under_kleene(const string& var) {
    /// (a|b)* = a*(ba*)*, b writes and a reads or do nothing
    auto *a_alt = new Regexp(alternationExpr);
    auto *b_alt = new Regexp(alternationExpr);

    for (auto *sub_r: sub_regexp->sub_regexps) {
        if (sub_r->uninited_read.find(var) != sub_r->uninited_read.end() ||
        (sub_r->maybe_initialized.find(var) == sub_r->maybe_initialized.end() && sub_r->maybe_read.find(var) == sub_r->maybe_read.end())) {
            if (a_alt->sub_regexps.empty()) {
                a_alt->copy_vars(sub_r);
            }
            else {
                a_alt->alt_vars(sub_r);
            }
            a_alt->sub_regexps.push_back(sub_r);
        }
        else {
            if (b_alt->sub_regexps.empty()) {
                b_alt->copy_vars(sub_r);
            }
            else {
                b_alt->alt_vars(sub_r);
            }
            b_alt->sub_regexps.push_back(sub_r);
        }
    }
    a_alt = a_alt->simplify_conc_alt();
    b_alt = b_alt->simplify_conc_alt();

    auto *a_kleene = new Regexp(kleeneStar);
    a_kleene->sub_regexp = a_alt;
    a_kleene->star_kleene_vars(a_alt);

    auto *second_kleene = new Regexp(kleeneStar);
    auto *child_concat = new Regexp(concatenationExpr);
    auto *copy_a_kleene = copy(a_kleene);
    child_concat->concat_vars(b_alt);
    child_concat->concat_vars(copy_a_kleene);
    child_concat->sub_regexps.push_back(b_alt);
    child_concat->sub_regexps.push_back(copy_a_kleene);

    second_kleene->sub_regexp = child_concat;
    second_kleene->star_kleene_vars(child_concat);

    auto *new_r = new Regexp(concatenationExpr);
    new_r->concat_vars(a_kleene);
    new_r->concat_vars(second_kleene);
    new_r->sub_regexps.push_back(a_kleene);
    new_r->sub_regexps.push_back(second_kleene);

    new_r = new_r->_bnf();

    return new_r;
}

Regexp *Regexp::open_alt_under_kleene(bool min_init_order) {
    if (sub_regexp->regexp_type == alternationExpr) {
        list<string> vars;
        if (min_init_order) {
            priority_queue<pair<string, int>, vector<pair<string, int>>, MinCompare> pq;
            vars = sub_regexp->make_var_queue(pq);
        }
        else {
            priority_queue<pair<string, int>, vector<pair<string, int>>, MaxCompare> pq;
            vars = sub_regexp->make_var_queue(pq);
        }

        if (!vars.empty()) {
            auto *new_r = _open_alt_under_kleene(vars.front());
            return new_r;
        }
        else {
            return this;
        }

    }
    else {
        ::printf("Open alt under kleene: Expected child alternation expression");
    }
}

Regexp *Regexp::conc_handle_init_without_read() {
    auto *bnf_regexp = this;
    set<string> met_init;

    size_t n = bnf_regexp->sub_regexps.size();
    int j = 0;
    auto it = bnf_regexp->sub_regexps.begin();
    while (j < n) {
        // встретили переменную(ые), которая(ые) потом возможно не читаются
        set<string> sub_init((*it)->unread_init.begin(), (*it)->unread_init.end());
        intersect_sets(sub_init, bnf_regexp->unread_init);


        // ищем подходящее свободное чтение (после свободной инициализации --- met_init)
        set<string> may_read((*it)->uninited_read.begin(), (*it)->uninited_read.end());
        intersect_sets(may_read, met_init);

        // выносим подходящее свободное чтение
        // альтернатива/клини под инициализацией
        if ((*it)->regexp_type == backreferenceExpr &&
            ((*it)->sub_regexp->regexp_type == alternationExpr || (*it)->regexp_type == kleeneStar) &&
            !may_read.empty()) {
            if ((*it)->sub_regexp->regexp_type == kleeneStar) {
                (*it)->sub_regexp = (*it)->sub_regexp->open_kleene_with_read(may_read);
            }
            (*it) = (*it)->take_out_alt_under_backref();
        }
        // если выполняется первое условие, то и это, но не наоборот
        if (((*it)->regexp_type == alternationExpr || (*it)->regexp_type == kleeneStar) &&
            !may_read.empty()) {
            // kleene star
            if ((*it)->regexp_type == kleeneStar) {
                (*it) = (*it)->open_kleene_with_read(may_read);
            }
            // тут в любом случае будет альтернатива, которую надо раскрыть
            auto *distributed = bnf_regexp->distribute_to_left(j);
            bnf_regexp->regexp_type = distributed->regexp_type;
            bnf_regexp->sub_regexps = distributed->sub_regexps;
            bnf_regexp->change_vars(distributed);

            if (bnf_regexp->regexp_type == alternationExpr)
                break;
            // дальше происходит преобразование внутри новой альтернативы
            // получаем (<bnf>) ++ regex ++ regex
            // продолжаем прогон сначала
            j = 0;
            it = bnf_regexp->sub_regexps.begin();
            n = bnf_regexp->sub_regexps.size();
            met_init.clear();
        }
            // иначе двигаемся вперед
        else {
            j++;
            it++;
        }

        // добавляем инициализацию из этой регулярки в конце чтобы не зациклиться
        if (!sub_init.empty()) {
            union_sets(met_init, sub_init);
        }
    }
    return bnf_regexp;
}

Regexp *Regexp::conc_handle_read_without_init() {
    auto *bnf_regexp = this;
    size_t n = bnf_regexp->sub_regexps.size();
    set<string> met_read;

    auto it = bnf_regexp->sub_regexps.end();
    it--;
    int j = n - 1;
    while (j >= 0) {
        // встретили переменную(ые), которая(ые) до этого точно не инициализируется
        set<string> sub_read(bnf_regexp->uninited_read.begin(), bnf_regexp->uninited_read.end());
        intersect_sets(sub_read, (*it)->uninited_read);


        // ищем инициализацию, которую можно вытащить
        set<string> maybe_init((*it)->maybe_initialized.begin(), (*it)->maybe_initialized.end());
        intersect_sets(maybe_init, met_read);

        // альтернатива под инициализацией
        if ((*it)->regexp_type == backreferenceExpr &&
            (((*it)->sub_regexp->regexp_type == alternationExpr || (*it)->sub_regexp->regexp_type == kleeneStar)) &&
            !maybe_init.empty()) {
            if ((*it)->sub_regexp->regexp_type == kleeneStar) {
                (*it)->sub_regexp = (*it)->sub_regexp->open_kleene({});
            }
            (*it) = (*it)->take_out_alt_under_backref();
        }
        // раскрываем альтернативу(клини)
        if (((*it)->regexp_type == alternationExpr || (*it)->regexp_type == kleeneStar) &&
            !maybe_init.empty()) {
            // kleene star
            if ((*it)->regexp_type == kleeneStar) {
                (*it) = (*it)->open_kleene({});
            }
            // тут в любом случае будет альтернатива, которую надо раскрыть
            auto *distributed = bnf_regexp->distribute_to_right(j);
            bnf_regexp->regexp_type = distributed->regexp_type;
            bnf_regexp->sub_regexps = distributed->sub_regexps;
            bnf_regexp->change_vars(distributed);

            if (bnf_regexp->regexp_type == alternationExpr)
                break;
            // дальше происходит преобразование внутри новой альтернативы
            // получаем regex ++ regex ++ (<bnf>)
            // продолжаем прогон с конца
            n = bnf_regexp->sub_regexps.size();
            j = n - 1;
            it = bnf_regexp->sub_regexps.end();
            it--;
            met_read.clear();
        }
        else {
            it--;
            j--;
        }

        // добавляем чтение из этой регулярки в конце чтобы не зациклиться
        if (!sub_read.empty()) {
            met_read.insert(sub_read.begin(), sub_read.end());
        }
    }
    return bnf_regexp;
}

Regexp *Regexp::_bnf(bool under_kleene, bool under_alt) {
    if (regexp_type == epsilon ||  regexp_type == literal ||
    regexp_type == reference) {
        return this;
    }
    else if (regexp_type == alternationExpr || regexp_type == concatenationExpr) {

        // приводим к бнф вложенные регулярки
        size_t n = sub_regexps.size();
        auto *bnf_regexp = new Regexp(regexp_type);
        for (auto sub_r : sub_regexps) {
            if (regexp_type == alternationExpr){
                auto *new_sub_r = sub_r->_bnf(false, true);
                if (bnf_regexp->sub_regexps.empty())
                    bnf_regexp->copy_vars(new_sub_r);
                else
                    bnf_regexp->alt_vars(new_sub_r);
                if (new_sub_r->regexp_type == alternationExpr) {
                    for (auto *sub_sub_r:new_sub_r->sub_regexps)
                        bnf_regexp->sub_regexps.push_back(sub_sub_r);
                }
                else
                    bnf_regexp->sub_regexps.push_back(new_sub_r);
            }
            else{
                auto *new_sub_r = sub_r->_bnf();
                bnf_regexp->concat_vars(new_sub_r);
                if (sub_r->regexp_type == concatenationExpr){
                    for (auto *sub_sub_r: new_sub_r->sub_regexps)
                        bnf_regexp->sub_regexps.push_back(sub_sub_r);
                }
                else
                    bnf_regexp->sub_regexps.push_back(new_sub_r);
            }
        }

        if (bnf_regexp->regexp_type == concatenationExpr) {

            // check unread init under kleene or under alternation ({}:1(&1|a))*
            n = bnf_regexp->sub_regexps.size();
            if ((under_kleene || under_alt) && !bnf_regexp->unread_init.empty()) {
                bnf_regexp = bnf_regexp->conc_handle_init_without_read();
            }
            // check read that may be uninitialized ({}:1&1|a)&1
            else if (!bnf_regexp->uninited_read.empty()) {
                bnf_regexp = bnf_regexp->conc_handle_read_without_init();
            }

            if (bnf_regexp->regexp_type == concatenationExpr) {
                //check init in alternations
                int i = 0;
                for (auto it = bnf_regexp->sub_regexps.begin(); it != bnf_regexp->sub_regexps.end(); it++) {
                    auto sub_r = *it;
                    if (sub_r->regexp_type == backreferenceExpr && sub_r->sub_regexp->regexp_type == alternationExpr &&
                        !sub_r->sub_regexp->alt_init_without_read().empty()) {
                        (*it) = sub_r->take_out_alt_under_backref();
                    }
                    sub_r = *it;
                    if (sub_r->regexp_type == alternationExpr && !sub_r->alt_init_without_read().empty()) {
                        auto *distributed = bnf_regexp->distribute_to_right(i);
                        bnf_regexp->regexp_type = distributed->regexp_type;
                        bnf_regexp->sub_regexps = distributed->sub_regexps;
                        break;
                    }
                    i++;
                }
            }

//            if (bnf_regexp->regexp_type == alternationExpr)
//                bnf_regexp = bnf_regexp->_bnf(false, true);
//            else
//                bnf_regexp = bnf_regexp->_bnf();
        }

        return bnf_regexp;
    }
    else if (regexp_type == kleeneStar || regexp_type == kleenePlus) {
        auto *bnf_regexp = new Regexp(regexp_type);
        auto *new_sub_r = sub_regexp->_bnf(true);
        bnf_regexp->sub_regexp = new_sub_r;
        if (regexp_type == kleeneStar)
            bnf_regexp->star_kleene_vars(new_sub_r);
        else
            bnf_regexp->copy_vars(new_sub_r);

        if (bnf_regexp->sub_regexp->regexp_type == alternationExpr) {
            bnf_regexp = bnf_regexp->open_alt_under_kleene();
        }
        return bnf_regexp;
    }
    else if (regexp_type == backreferenceExpr) {
        auto *bnf_regex = new Regexp(backreferenceExpr);
        bnf_regex->variable = variable;
        bnf_regex->sub_regexp = sub_regexp->_bnf();
        bnf_regex->copy_vars(bnf_regex->sub_regexp);
        bnf_regex->initialized[variable].push_back(bnf_regex);
        bnf_regex->maybe_initialized.insert(variable);
        bnf_regex->unread_init.insert(variable);
        bnf_regex->definitely_unread_init.insert(variable);

        return bnf_regex;
    }
}

Regexp *Regexp::bnf() {
    auto *new_r = _bnf();

//    cout << new_r->to_string() << endl;

    new_r->bind_init_to_read({});
    new_r = new_r->clear_initializations_and_read(new_r->definitely_unread_init, new_r->uninited_read);

    return new_r;
}