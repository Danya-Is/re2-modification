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

Regexp* Regexp::clear_initializations(set<string> vars) {
    // TODO delete needed??
    if (vars.empty()) {
        return this;
    }
    auto *new_r = new Regexp();
    if (regexp_type == epsilon) {
        new_r->regexp_type = regexp_type;
    }
    else if (regexp_type == literal) {
        new_r->regexp_type = regexp_type;
        new_r->rune = rune;
    }
    else if (regexp_type == reference) {
        new_r->regexp_type = regexp_type;
        new_r->variable = variable;
    }
    else if (regexp_type == backreferenceExpr && vars.find(variable) != vars.end()) {
        new_r->regexp_type = sub_regexp->regexp_type;
        new_r->copy_vars(this);

        // кажется что этого достаточно, тк гарантируется что эта переменная никогда не прочитается
        new_r->initialized.erase(variable);
        new_r->maybe_initialized.erase(variable);
        new_r->unread_init.erase(variable);

        if (regexp_type == alternationExpr || regexp_type == concatenationExpr) {
            new_r->sub_regexps = sub_regexp->sub_regexps;
        }
        else if (regexp_type == kleenePlus || regexp_type == kleeneStar) {
            new_r->sub_regexp = sub_regexp->sub_regexp;
        }
        else if (regexp_type == reference) {
            new_r->variable = sub_regexp->variable;
        }
        else {
            new_r->rune = sub_regexp->rune;
        }
    }
    else if (regexp_type == concatenationExpr || regexp_type == alternationExpr) {
        new_r->regexp_type = regexp_type;
        for (auto *sub_r: sub_regexps) {
            if (regexp_type == concatenationExpr)
                new_r->sub_regexps.push_back(sub_r->clear_initializations(vars));
            else {
                set<string> sub_vars_to_delete(vars.begin(), vars.end());
                intersect_sets(sub_vars_to_delete, sub_r->unread_init);
                new_r->sub_regexps.push_back(sub_r->clear_initializations(sub_vars_to_delete));
            }
        }
    }
    else if (regexp_type == kleeneStar || regexp_type == kleenePlus) {
        new_r->regexp_type = regexp_type;
        new_r->sub_regexp = sub_regexp->clear_initializations(vars);
    }
    return new_r;
}


Regexp* Regexp::distribute_to_right(int alt_pos) {
    if (regexp_type == concatenationExpr) {
        size_t n = sub_regexps.size();
        if (n >= alt_pos) {
            auto* new_regexp = new Regexp(concatenationExpr);
            auto it = sub_regexps.begin();
            for (int i = 0; i < alt_pos; i++) {
                new_regexp->concat_vars((*it));
                new_regexp->sub_regexps.push_back(*it);
                it++;
            }

            auto* alt = *it;
            it++;

            auto* tail = new Regexp(concatenationExpr);
            set<string> uninited_read_t;
            for (int i = alt_pos + 1; i < n; i++) {
                tail->concat_vars((*it));
                tail->sub_regexps.push_back(*it);
                it++;
            }

            auto *new_alt = new Regexp(alternationExpr);
            int k = 0;
            for (auto *sub_r: alt->sub_regexps) {
                auto *new_sub_r = new Regexp(concatenationExpr);

                new_sub_r->sub_regexps.push_back(sub_r);
                new_sub_r->sub_regexps.merge(tail->sub_regexps);

                new_sub_r->copy_vars(sub_r);
                new_sub_r->concat_vars(tail);

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
            new_regexp->sub_regexps.push_back(new_r);

            if (new_regexp->sub_regexps.size() == 1) {
                new_regexp->regexp_type = alternationExpr;
                new_regexp->change_vars(new_regexp->sub_regexps.front());
                new_regexp->sub_regexps = new_regexp->sub_regexps.front()->sub_regexps;
            }
            else {
                new_regexp->concat_vars(new_r);
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
    if (regexp_type == concatenationExpr) {
        size_t n = sub_regexps.size();
        if (n >= alt_pos) {
            auto* new_regexp = new Regexp(concatenationExpr);
            auto *head = new Regexp(concatenationExpr);

            auto it = sub_regexps.begin();
            for (int i = 0; i < alt_pos; i++) {
                head->sub_regexps.push_back(*it);
                head->concat_vars(*it);
                it++;
            }

            auto* alt = *it;

            auto *new_alt = new Regexp(alternationExpr);
            int k = 0;
            for (auto *sub_r: alt->sub_regexps) {
                auto *new_sub_r = new Regexp(concatenationExpr);
                new_sub_r->sub_regexps.merge(head->sub_regexps);
                new_sub_r->sub_regexps.push_back(sub_r);

                new_sub_r->copy_vars(head);
                new_sub_r->concat_vars(sub_r);

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
            new_regexp->sub_regexps.push_back(new_r);
            new_regexp->concat_vars(new_r);

            it++;
            for (int i = alt_pos + 1; i < n; i++) {
                new_regexp->sub_regexps.push_back(*it);
                new_regexp->concat_vars(*it);
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
    auto *new_kleene = clear_initializations(std::move(vars));
    auto *new_sub = sub_regexp;

    auto *concat = new Regexp(concatenationExpr);
    concat->sub_regexps.push_back(new_kleene);
    concat->sub_regexps.push_back(new_sub);
    concat->concat_vars(new_kleene);
    concat->concat_vars(new_sub);

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
}

Regexp *Regexp::_open_alt_under_kleene(const string& var) {
    /// (a|b)* = a*(ba*)*, b writes and a reads or do nothing
    set<Regexp*> a;
    set<Regexp*> b;

    auto *a_alt = new Regexp(alternationExpr);
    auto *b_alt = new Regexp(alternationExpr);

    for (auto *sub_r: sub_regexps) {
        if (uninited_read.find(var) == uninited_read.end() ||
        (maybe_initialized.find(var) == maybe_initialized.end()) && maybe_read.find(var) == maybe_read.find(var)) {
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

    auto *a_kleene = new Regexp(kleeneStar);
    a_kleene->sub_regexp = a_alt;
    a_kleene->star_kleene_vars(a_alt);

    auto *second_kleene = new Regexp(kleeneStar);
    auto *child_concat = new Regexp(concatenationExpr);
    child_concat->sub_regexps.push_back(b_alt);
    child_concat->sub_regexps.push_back(a_kleene);
    child_concat->concat_vars(b_alt);
    child_concat->concat_vars(a_kleene);

    second_kleene->sub_regexp = child_concat;
    second_kleene->star_kleene_vars(child_concat);

    auto *new_r = new Regexp(concatenationExpr);
    new_r->sub_regexps.push_back(a_alt);
    new_r->sub_regexps.push_back(second_kleene);
    new_r->concat_vars(a_alt);
    new_r->concat_vars(second_kleene);

    new_r = new_r->_bnf();

    return new_r;
}

Regexp *Regexp::open_alt_under_kleene(bool min_init_order) {
    if (regexp_type == alternationExpr) {
        list<string> vars;
        if (min_init_order) {
            priority_queue<pair<string, int>, vector<pair<string, int>>, MinCompare> pq;
            vars = make_var_queue(pq);
        }
        else {
            priority_queue<pair<string, int>, vector<pair<string, int>>, MaxCompare> pq;
            vars = make_var_queue(pq);
        }

        if (!vars.empty()) {
            auto *new_r = _open_alt_under_kleene(vars.front());
        }
        else {
            return this;
        }

    }
    else {
        ::printf("Open alt under kleene: Expected alternation expression");
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
        if (!sub_init.empty()) {
            union_sets(met_init, sub_init);
        }

        // ищем подходящее свободное чтение (после свободной инициализации --- met_init)
        set<string> may_read((*it)->uninited_read.begin(), (*it)->uninited_read.end());
        intersect_sets(may_read, met_init);

        // выносим подходящее свободное чтение
        // альтернатива/клини под инициализацией
        if ((*it)->regexp_type == backreferenceExpr &&
            (((*it)->sub_regexp->regexp_type == alternationExpr || (*it)->sub_regexp->regexp_type == kleeneStar)) &&
            !may_read.empty()) {
            if ((*it)->sub_regexp->regexp_type == kleeneStar) {
                (*it)->sub_regexp = (*it)->sub_regexp->open_kleene({});
            }
            (*it) = (*it)->take_out_alt_under_backref();
        }
        // если выполняется первое условие, то и это, но не наоборот
        if (((*it)->regexp_type == alternationExpr || (*it)->regexp_type == kleeneStar) &&
            !may_read.empty()) {
            // kleene star
            if ((*it)->regexp_type == kleeneStar) {
                (*it) = (*it)->open_kleene({});
            }
            // тут в любом случае будет альтернатива, которую надо раскрыть
            auto *distributed = bnf_regexp->distribute_to_left(j);
            bnf_regexp->regexp_type = distributed->regexp_type;
            bnf_regexp->sub_regexps = distributed->sub_regexps;
            // дальше происходит преобразование внутри новой альтернативы
            // получаем (<bnf>) ++ regex ++ regex
            // продолжаем прогон сначала
            j = 0;
            it = bnf_regexp->sub_regexps.begin();
        }
            // иначе двигаемся вперед
        else {
            j++;
            it++;
        }
    }
}

Regexp *Regexp::conc_handle_read_without_init() {
    auto *bnf_regexp = this;
    size_t n = bnf_regexp->sub_regexps.size();
    set<string> met_read;

    auto it = bnf_regexp->sub_regexps.end();
    int j = n - 1;
    while (j >= 0) {
        // встретили переменную(ые), которая(ые) до этого точно не инициализируется
        set<string> sub_read(bnf_regexp->uninited_read.begin(), bnf_regexp->uninited_read.end());
        intersect_sets(sub_read, (*it)->uninited_read);
        if (!sub_read.empty()) {
            met_read.insert(sub_read.begin(), sub_read.end());
        }

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
            // дальше происходит преобразование внутри новой альтернативы
            // получаем regex ++ regex ++ (<bnf>)
            // продолжаем прогон с конца
            j = n - 1;
            it = bnf_regexp->sub_regexps.end();

        }
        else {
            it--;
            j--;
        }
    }
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
        for (int i = 0; i < n; i++) {
            auto *sub_r = sub_regexps.front();
            if (regexp_type == alternationExpr)
                bnf_regexp->sub_regexps.push_back(sub_r->_bnf(false, true));
            else
                bnf_regexp->sub_regexps.push_back(sub_r->_bnf());
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
        else if (under_kleene) {
            bnf_regexp = bnf_regexp->open_alt_under_kleene();
        }

        return bnf_regexp;
    }
    else if (regexp_type == kleeneStar || regexp_type == kleenePlus) {
        auto *bnf_regexp = new Regexp(kleeneStar);
        bnf_regexp->sub_regexp = sub_regexp->_bnf(true);

        //check init under kleene
        if (!bnf_regexp->sub_regexp->unread_init.empty()) {
            bnf_regexp = bnf_regexp->open_kleene(bnf_regexp->sub_regexp->unread_init);
        }
        return bnf_regexp;
    }
    else if (regexp_type == backreferenceExpr) {
        auto *bnf_regex = new Regexp(backreferenceExpr);
        bnf_regex->variable = variable;
        bnf_regex->sub_regexp = sub_regexp->_bnf();

        return bnf_regex;
    }
}

Regexp *Regexp::bnf() {
    // TODO clear
}