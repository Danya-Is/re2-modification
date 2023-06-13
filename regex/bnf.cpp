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
        if (!reference_to && read_vars.find(variable) != read_vars.end()) {
            new_r->regexp_type = epsilon;
            new_r->uninited_read.erase(variable);
            new_r->definitely_uninit_read.erase(variable);
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
                new_r->push_sub_regexp(new_sub_r);
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
        if (n - 1 > alt_pos) {
            auto* new_regexp = new Regexp(concatenationExpr);
            auto it = sub_regexps.begin();
            for (int i = 0; i < alt_pos; i++) {
                new_regexp->push_sub_regexp((*it));
                it++;
            }

            auto* alt = *it;
            it++;

            auto* tail = new Regexp(concatenationExpr);
            set<string> uninited_read_t;
            for (int i = alt_pos + 1; i < n; i++) {
                tail->push_sub_regexp((*it));
                it++;
            }

            auto *new_alt = new Regexp(alternationExpr);
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
                    new_sub_r->push_sub_regexp(copy_r);
                }


                new_alt->push_sub_regexp(new_sub_r);
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
            return this;
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
        if (alt_pos > 0) {
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

                new_alt->push_sub_regexp(new_sub_r);
            }

            auto* new_r = new_alt->_bnf();
            ::free(new_alt);
            new_regexp->push_sub_regexp(new_r);

            it++;
            for (int i = alt_pos + 1; i < n; i++) {
                new_regexp->push_sub_regexp(*it);
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
            return this;
        }
    }
    else {
        ::printf("Expected concatenation in distribute()");
    }
}

Regexp *Regexp::open_kleene_plus(set<string> vars, bool need_for_cleen) {
    auto *new_kleene = this;
    if (need_for_cleen){
        map<string, Regexp*> empty_map;
        bind_init_to_read(empty_map);
        new_kleene = clear_initializations_and_read(std::move(vars), {});
    }
    auto *new_sub = copy(sub_regexp);

    auto *concat = new Regexp(concatenationExpr);
    concat->push_sub_regexp(new_kleene);
    concat->push_sub_regexp(new_sub);
    return concat;
}

Regexp *Regexp::open_kleene(set<string> vars, bool need_for_cleen) {
    /// a* == (a*a|eps), a+ == a*a
    if (regexp_type == kleeneStar) {
        auto *new_r = new Regexp(alternationExpr);
        new_r->sub_regexps.push_back(new Regexp(epsilon));

        auto *concat = open_kleene_plus(vars, need_for_cleen);
        new_r->push_sub_regexp(concat);
        return new_r;
    }
    else if (regexp_type == kleenePlus) {
        return open_kleene_plus(vars);
    }
    else {
        ::printf("Open kleene: Expected kleene");
    }
}

Regexp *Regexp::open_kleene_with_read(set<string> vars, Regexp* parent, list<Regexp*>::iterator prefix_index) {
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
        sub_concat->push_sub_regexp(b_alt);
        auto *copy_a_kleene = copy(a_kleene);
        sub_concat->push_sub_regexp(copy_a_kleene);

        auto *second_kleene = new Regexp(kleeneStar);
        auto *copy_sub_concat = copy(sub_concat);
        second_kleene->sub_regexp = copy_sub_concat;
        second_kleene->star_kleene_vars(copy_sub_concat);

        auto *concat = new Regexp(concatenationExpr);
        concat->push_sub_regexp(a_kleene);
        concat->push_sub_regexp(sub_concat);
        concat->push_sub_regexp(second_kleene);

        auto *new_r = new Regexp(alternationExpr);
        auto *copy_a_kleene_2 = copy(a_kleene);
        new_r->sub_regexps.push_back(copy_a_kleene_2);
        a_alt->copy_vars(copy_a_kleene_2);
        new_r->sub_regexps.push_back(concat);
        new_r->alt_vars(concat);

        return new_r;
    }
    else {
        set<string> read_before_init(sub_regexp->definitely_uninit_read.begin(),
                                     sub_regexp->definitely_uninit_read.end());
        intersect_sets(read_before_init, sub_regexp->maybe_initialized);
        if (read_before_init.empty())
            return open_kleene(vars);
        else
            return rw_in_conc_under_kleene(parent, prefix_index);
    }
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
            a_alt->push_sub_regexp(sub_r);
        }
        else {
            b_alt->push_sub_regexp(sub_r);
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
    child_concat->push_sub_regexp(b_alt);
    child_concat->push_sub_regexp(copy_a_kleene);

    second_kleene->sub_regexp = child_concat;
    second_kleene->star_kleene_vars(child_concat);

    auto *new_r = new Regexp(concatenationExpr);
    new_r->push_sub_regexp(a_kleene);
    new_r->push_sub_regexp(second_kleene);

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

Regexp *Regexp::rw_in_conc_under_kleene(Regexp* parent, list<Regexp*>::iterator prefix_index) {
    /// (r_1w_1r_2w_2)* -> sliding (r_1w_1r_2(w_2r_1w_1r_2)*w_2 | eps)
    /// -> open kleene (r_1w_1r_2 ((w_2r_1w_1r_2)*w_2r_1w_1r_2 | eps) w_2 | eps)

    set<string> read_before_init(sub_regexp->definitely_uninit_read.begin(),
                                 sub_regexp->definitely_uninit_read.end());
    intersect_sets(read_before_init, sub_regexp->maybe_initialized);

    // remove reads for which last_init of prefix equal to last_init of ab
    set<string> not_amb;
    for (const auto& var: read_before_init) {
        if (!sub_regexp->last_init(var)->is_equal(parent->prefix_last_init(var, prefix_index--))) {
            not_amb.insert(var);
        }
    }

    auto *bnf_r = new Regexp();
    // для новой итерации, возможно ее потом еще надо будет раскрывать
    auto *new_kleene = new Regexp(kleeneStar);
    /// sliding (ab)* = (a(ba)*b | eps), where a contains read and b contains init
    if (!not_amb.empty()) {
        // so regex looks like (rwrw...rw)*, and we need the last one w
        string var;

        auto *a_conc = new Regexp(concatenationExpr);
        auto *b_conc = new Regexp(concatenationExpr);

        auto it = sub_regexp->sub_regexps.end();
        it--;
        while (it != sub_regexp->sub_regexps.end()) {
            b_conc->push_sub_regexp(*it);
            set<string> last_init(not_amb.begin(), not_amb.end());
            intersect_sets(last_init, (*it)->maybe_initialized);
            // нашли последнюю инициализацию, все что до нее пойдет в a_conc
            if (!last_init.empty()) {
                break;
            }
            it--;
        }

        while (it != sub_regexp->sub_regexps.end()) {
            a_conc->push_sub_regexp(*it);
            it--;
        }

        a_conc = a_conc->simplify_conc_alt();
        b_conc = b_conc->simplify_conc_alt();

        auto *new_r = new Regexp(alternationExpr);
        new_r->sub_regexps.push_back(new Regexp(epsilon));

        auto *new_concat = new Regexp(concatenationExpr);

        auto *sub_concat = new Regexp(concatenationExpr);
        auto *copy_b = copy(b_conc);
        sub_concat->push_sub_regexp(copy_b);
        sub_concat->push_sub_regexp(a_conc);
        new_kleene->sub_regexp = sub_concat;
        new_kleene->star_kleene_vars(sub_concat);

        new_concat->push_sub_regexp(a_conc);
        new_concat->push_sub_regexp(new_kleene);
        new_concat->push_sub_regexp(b_conc);

        if (regexp_type == kleeneStar) {
            new_r->push_sub_regexp(new_concat);
            bnf_r = new_r;
        }
        else {
            bnf_r = new_concat;
        }

        // произошел слайдинг
        // теперь нужно раскрыть итерацию, если внутри нее еще остались rw
        // из за того что слайдинг был по последней инициализации, все они точно удовлетворяют lastinit условию
        read_before_init.clear();
        read_before_init.insert(new_kleene->definitely_uninit_read.begin(),
                                new_kleene->definitely_uninit_read.end());
        intersect_sets(read_before_init, new_kleene->maybe_initialized);

        if (!read_before_init.empty()) {
            new_kleene = new_kleene->open_kleene({}, false);
            new_kleene->is_slided = true;
        }
    }
    else if (!read_before_init.empty()){
        bnf_r = open_kleene({}, false);
        // (eps | a*a)
        bnf_r->sub_regexps.back()->sub_regexps.front()->is_slided = true;
    }

    else bnf_r = this;
    return bnf_r;
}

Regexp *Regexp::handle_rw_under_kleene(Regexp* parent, list<Regexp*>::iterator prefix_index) {
    if (sub_regexp->regexp_type == concatenationExpr) {
        return rw_in_conc_under_kleene(parent, prefix_index);
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
            if ((*it)->sub_regexp->regexp_type == kleeneStar && !(*it)->sub_regexp->is_slided) {
                (*it)->sub_regexp = (*it)->sub_regexp->open_kleene_with_read(may_read, bnf_regexp, it);
            }
            (*it) = (*it)->take_out_alt_under_backref();
        }
        // если выполняется первое условие, то и это, но не наоборот
        if (((*it)->regexp_type == alternationExpr || (*it)->regexp_type == kleeneStar) &&
            !may_read.empty()) {
            // kleene star
            if ((*it)->regexp_type == kleeneStar && !(*it)->is_slided) {
                (*it) = (*it)->open_kleene_with_read(may_read, bnf_regexp, it);
            }

            if ((*it)->regexp_type == alternationExpr) {
                // альтернатива, которую надо раскрыть
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
            else {
                j++;
                it++;
            }
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
            if ((*it)->sub_regexp->regexp_type == kleeneStar && !(*it)->sub_regexp->is_slided) {
                (*it)->sub_regexp = (*it)->sub_regexp->open_kleene({});
            }
            (*it) = (*it)->take_out_alt_under_backref();
        }
        // раскрываем альтернативу(клини)
        if (((*it)->regexp_type == alternationExpr || (*it)->regexp_type == kleeneStar) &&
            !maybe_init.empty()) {
            // kleene star
            if ((*it)->regexp_type == kleeneStar && !(*it)->is_slided) {
                (*it) = (*it)->open_kleene({});
            }
            if ((*it)->regexp_type == alternationExpr) {
                // альтернатива, которую надо раскрыть
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
            if (!bnf_regexp->unread_init.empty()) {
                bnf_regexp = bnf_regexp->conc_handle_init_without_read();
            }
            // check read that may be uninitialized ({}:1&1|a)&1
            else if (!bnf_regexp->uninited_read.empty()) {
                bnf_regexp = bnf_regexp->conc_handle_read_without_init();
            }

            if (bnf_regexp->regexp_type == concatenationExpr) {
                //check rw under kleene
                bool changed = false;
                for (auto it = bnf_regexp->sub_regexps.begin(); it != bnf_regexp->sub_regexps.end(); it++) {
                    if ((*it)->regexp_type == kleeneStar && !(*it)->is_slided) {
                        auto old_r = *it;
                        *it = (*it)->handle_rw_under_kleene(bnf_regexp, it);
                        changed = !old_r->is_equal(*it);
                    }
                }
                if (changed)
                    bnf_regexp = bnf_regexp->_bnf();
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
        if (is_slided)
            return this;
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

    cout << new_r->to_string() << endl;

    map<string, Regexp*> empty_map;
    new_r->bind_init_to_read(empty_map);
    new_r = new_r->clear_initializations_and_read(new_r->definitely_unread_init, new_r->uninited_read);

    return new_r;
}