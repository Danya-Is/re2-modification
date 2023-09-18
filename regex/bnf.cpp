#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <utility>
#include <queue>

#include "regex.h"

fstream log;


Regexp *Regexp::take_out_alt_under_backref() {
    /// {(a|b)}:1 == ({a}:1|{b}:1)
    if (regexp_type == backreferenceExpr && sub_regexp->regexp_type == alternationExpr) {
        auto *new_r = new Regexp(alternationExpr);
        for (auto *sub_r: sub_regexp->sub_regexps) {
            auto *new_sub_r = new Regexp(backreferenceExpr);
            new_sub_r->variable = variable;
            new_sub_r->sub_regexp = sub_r;
            new_sub_r->copy_vars(sub_r);
            new_sub_r->initialized[variable].push_back(new_sub_r);
            new_sub_r->maybe_initialized.insert(variable);
            new_sub_r->unread_init.insert(variable);
            new_sub_r->definitely_unread_init.insert(variable);

            new_r->sub_regexps.push_back(new_sub_r);
        }

        new_r->copy_vars(this);
        if (log.is_open())
            log << to_string() << "->" << new_r->to_string() << endl;
        return new_r;
    }
    else
        return this;
}

Regexp *Regexp::clear_initializations_and_read(set<string> init_vars, set<string> read_vars) {
    // TODO delete epsilon in concatenation
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
                new_r->reference_to = sub_regexp->reference_to;
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
                if (new_sub_r->regexp_type != epsilon)
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
        if (new_r->regexp_type == concatenationExpr && new_r->sub_regexps.empty())
            new_r->sub_regexps.push_back(new Regexp(epsilon));
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
            if (it == sub_regexps.end())
                return this;

            auto* tail = new Regexp(concatenationExpr);
            for (int i = alt_pos + 1; i < n; i++) {
                tail->push_sub_regexp((*it));
                it++;
            }

            auto *new_alt = new Regexp(alternationExpr);
            for (auto *sub_r: alt->sub_regexps) {
                auto *new_sub_r = new Regexp(concatenationExpr);
                new_sub_r->push_sub_regexp(sub_r);
                for (auto *r: tail->sub_regexps){
                    auto *copy_r = copy(r);
                    new_sub_r->push_sub_regexp(copy_r);
                }

                new_alt->push_sub_regexp(new_sub_r);
            }

            new_regexp->push_sub_regexp(new_alt);
            if (log.is_open()) {
                log << "distribute to right" << endl;
                log << to_string() << " -> " << new_alt->to_string() << endl;
            }

            new_alt = new_alt->_bnf(nullptr);

            if (new_regexp->sub_regexps.size() == 1) {
                new_regexp->regexp_type = alternationExpr;
                new_regexp->change_vars(new_regexp->sub_regexps.front());
                new_regexp->sub_regexps = new_regexp->sub_regexps.front()->sub_regexps;
            }

            return new_regexp;
        }
        else return this;
    }
    else return this;
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
                head->push_sub_regexp(*it);
                it++;
            }

            auto* alt = *it;

            if (it == sub_regexps.begin())
                return this;

            auto *new_alt = new Regexp(alternationExpr);
            for (auto *sub_r: alt->sub_regexps) {
                auto *new_sub_r = new Regexp(concatenationExpr);
                for (auto *r: head->sub_regexps) {
                    auto *copy_r = copy(r);
                    new_sub_r->push_sub_regexp(copy_r);
                }
                new_sub_r->push_sub_regexp(sub_r);
                new_alt->push_sub_regexp(new_sub_r);
            }

            if (log.is_open()) {
                log << "distribute to left" << endl;
                log << "prefix -> " << new_alt->to_string() << endl;
            }

            auto* new_r = new_alt->_bnf(nullptr);
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
        cout << "Expected concatenation in distribute()"<< endl;
    }
}

Regexp *Regexp::distribute_completely(int alt_pos) {
    if (alt_pos == 0)
        return distribute_to_right(0);
    else if (alt_pos == sub_regexps.size() - 1)
        return distribute_to_left(alt_pos);
    else {
        auto *new_r = distribute_to_right(alt_pos);
        new_r = new_r->distribute_to_left(new_r->sub_regexps.size() - 1);
        return new_r;
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
        if (log.is_open()) {
            log << "open kleene" << endl;
            log << to_string() << " -> " << new_r->to_string() << endl;
        }
        return new_r;
    }
    else if (regexp_type == kleenePlus)
        return open_kleene_plus(vars);
    else
        cout << "Open kleene: Expected kleene" << endl;
}

Regexp *Regexp::open_kleene_with_read(set<string> vars, Regexp* parent, list<Regexp*>::iterator prefix_index) {
    /// (a*) -> (eps|aa*); example: (&1)* -> (eps| &1&1*)
    /// (a|b)* -> a*(ba*)* -> a*(ba*(ba*)*|eps) -> (a*ba*(ba*)*|a*) where b contains read
    if (sub_regexp->regexp_type == alternationExpr) {
        auto *a_alt = new Regexp(alternationExpr);
        auto *b_alt = new Regexp(alternationExpr);

        for (auto *sub_r: sub_regexp->sub_regexps) {
            set<string> sub_read;
            for (const auto& refs: sub_r->maybe_read) {
                sub_read.insert(refs.first);
            }
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

Regexp *Regexp::denesting(Regexp *a_alt, Regexp *b_alt) {
    /// (a|b)* = a*(ba*)*
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

    if (log.is_open()) {
        log << "denesting" << endl;
        log << to_string() << " -> " << new_r->to_string() << endl;
    }

    new_r = new_r->_bnf(nullptr);

    return new_r;
}

Regexp *Regexp::_open_alt_under_kleene(const string& var) {
    /// (a|b)* = a*(ba*)*
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

    // (rw | rw | rw | f | f)* case
    if (b_alt->regexp_type == epsilon)
        return rw_in_alt_under_kleene();

    return denesting(a_alt, b_alt);
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
        else return this;

    }
    else return this;
}

Regexp *Regexp::rw_with_bad_init_read(set<string> not_amb) {
    /// denesting + sliding (ab)* = (a(ba)*b | eps), where a contains read and b contains init
    /// so regex looks like (rwrw...rw)*, and we need the last one w
    set<string> last_init(not_amb.begin(), not_amb.end());

    auto *bnf_r = new Regexp();
    auto *new_kleene = new Regexp(kleeneStar);

    auto *a_conc = new Regexp(concatenationExpr);
    auto *b_conc = new Regexp(concatenationExpr);

    auto it = sub_regexp->sub_regexps.end();
    it--;
    size_t init_pos = sub_regexp->sub_regexps.size() - 1;
    list<Regexp*>::iterator init_it;
    while (it != sub_regexp->sub_regexps.end()) {
        b_conc->push_sub_regexp(*it, true);
        last_init.clear();
        last_init.insert(not_amb.begin(), not_amb.end());
        intersect_sets(last_init, (*it)->maybe_initialized);
        it--;
        // нашли последнюю инициализацию, все что до нее пойдет в a_conc
        if (!last_init.empty()) {
            init_it = it;
            init_it++;
            break;
        }
        init_pos--;
    }

    while (it != sub_regexp->sub_regexps.end()) {
        a_conc->push_sub_regexp(*it, true);
        it--;
    }

    a_conc = a_conc->simplify_conc_alt();
    b_conc = b_conc->simplify_conc_alt();

    auto *new_r = new Regexp(alternationExpr);
    new_r->sub_regexps.push_back(new Regexp(epsilon));

    auto *new_concat = new Regexp(concatenationExpr);

    auto *sub_concat = new Regexp(concatenationExpr);
    auto *copy_b = copy(b_conc);
    auto *copy_a = copy(a_conc);
    sub_concat->push_sub_regexp(copy_b);
    sub_concat->push_sub_regexp(copy_a);
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
    if (!new_kleene->sub_regexp->rw_vars.empty()) {
        new_kleene = new_kleene->open_kleene({}, false);
        new_kleene->is_slided = true;
    }
    else {
        new_kleene = new_kleene->_bnf(nullptr, true);
        new_kleene->is_slided = true;
    }

    if (log.is_open()) {
        log << "denesting + sliding" << endl;
        log << to_string() << " -> " << bnf_r->to_string() << endl;
    }

    return bnf_r;
}

Regexp *Regexp::rw_in_conc_under_kleene(Regexp* parent, list<Regexp*>::iterator current_index) {
    /// (r_1w_1r_2w_2)* -> sliding (r_1w_1r_2(w_2r_1w_1r_2)*w_2 | eps)
    /// -> open kleene (r_1w_1r_2 ((w_2r_1w_1r_2)*w_2r_1w_1r_2 | eps) w_2 | eps)

    if (sub_regexp->is_cross_references()) {
        this->is_bad_bnf = true;
        cout << this->to_string() << endl;
        cout << "Обращение регулярок типа ({&j}:i {}:j &i)* не поддерживается, так как требует введение вспомогательных ячеек" << endl;
        return this;
    }

    set<string> read_before_init(sub_regexp->definitely_uninit_read.begin(),
                                 sub_regexp->definitely_uninit_read.end());
    intersect_sets(read_before_init, sub_regexp->maybe_initialized);

    // remove reads for which last_init of prefix equal to last_init of ab
    set<string> not_amb;
    if (parent && current_index != parent->sub_regexps.begin()) {
        for (const auto& var: sub_regexp->rw_vars) {
            if (sub_regexp->last_init(var) &&
            !sub_regexp->last_init(var)->is_equal(parent->prefix_last_init(var, current_index--))) {
                not_amb.insert(var);
            }
        }
    }
    else {
        not_amb.insert(sub_regexp->rw_vars.begin(), sub_regexp->rw_vars.end());
    }

    auto *bnf_r = new Regexp();
    if (!not_amb.empty()) {
        return rw_with_bad_init_read(not_amb);
    }
    else if (!sub_regexp->rw_vars.empty()){
        bnf_r = open_kleene({}, false);
        // (eps | a*a)
        bnf_r->sub_regexps.back()->sub_regexps.front()->is_slided = true;
    }
    else if (!read_before_init.empty()) {
        // значит инициализация там же где и то самое чтение (c(&1|{a*}:1&1)b)*
        // нужно раскрыть эту альтернативу

        auto it = sub_regexp->sub_regexps.end();
        it--;
        size_t init_pos = sub_regexp->sub_regexps.size() - 1;
        list<Regexp*>::iterator init_it;
        while (it != sub_regexp->sub_regexps.end()) {
            set<string> needed_var(read_before_init.begin(), read_before_init.end());
            intersect_sets(needed_var, (*it)->definitely_uninit_read);
            intersect_sets(needed_var, (*it)->maybe_initialized);
            // нашли нужную регулярку инициализацию
            if (!needed_var.empty()) {
                init_it = it;
                break;
            }
            it--;
            init_pos--;
        }

        if (init_pos == 0)
            init_it = sub_regexp->sub_regexps.begin();

        if ((*init_it)->regexp_type == alternationExpr)
            sub_regexp = sub_regexp->distribute_completely(init_pos);

        if (log.is_open()) {
            log << "open alt in conc under kleene" << endl;
            log << " -> " << to_string() << endl;
        }
        return this;
    }

    else bnf_r = this;
    return bnf_r;
}

Regexp *Regexp::rw_in_alt_under_kleene() {
    auto *first_alt = new Regexp(alternationExpr);
    auto *other_alt = new Regexp(alternationExpr);
    for (auto *sub_r: sub_regexp->sub_regexps) {
        set<string> read_before_init(sub_r->definitely_uninit_read.begin(),
                                     sub_r->definitely_uninit_read.end());
        intersect_sets(read_before_init, sub_r->maybe_initialized);

        if (!read_before_init.empty() && first_alt->sub_regexps.empty()) {
            first_alt = sub_r;
        }
        else
            other_alt->sub_regexps.push_back(sub_r);
    }

    other_alt = other_alt->simplify_conc_alt();

    return denesting(other_alt, first_alt);
}

Regexp *Regexp::handle_rw_under_kleene(Regexp* parent, list<Regexp*>::iterator current_index) {
    if (sub_regexp->regexp_type == concatenationExpr) {
        return rw_in_conc_under_kleene(parent, current_index);
    }
    else if (sub_regexp->regexp_type == alternationExpr) {
        return rw_in_alt_under_kleene();
    }
    // todo backreference
    else return this;
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
                if (log.is_open()) {
                    log << "init without read " << to_string() << endl;
                }

                // альтернатива, которую надо раскрыть
                auto *distributed = bnf_regexp->distribute_to_left(j);
                bnf_regexp->regexp_type = distributed->regexp_type;
                bnf_regexp->sub_regexps = distributed->sub_regexps;
                bnf_regexp->change_vars(distributed);

//                if (bnf_regexp->regexp_type == alternationExpr)
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
                if (log.is_open()) {
                    log << "read without init" << to_string() << endl;
                }

                // альтернатива, которую надо раскрыть
                bnf_regexp = bnf_regexp->distribute_to_right(j);

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

Regexp *Regexp::_bnf(Regexp* parent, bool under_kleene, list<Regexp*>::iterator cur_position) {
    if (regexp_type == epsilon ||  regexp_type == literal ||
    regexp_type == reference || is_bad_bnf) {
        return this;
    }
    else if (regexp_type == alternationExpr || regexp_type == concatenationExpr) {

        // приводим к бнф вложенные регулярки
        size_t n = sub_regexps.size();
        auto *bnf_regexp = new Regexp(regexp_type);
        for (auto it = sub_regexps.begin(); it != sub_regexps.end(); it++) {
            auto *new_sub_r = (*it)->_bnf(this, false, it);
            bnf_regexp->push_sub_regexp(new_sub_r);
        }

        if (bnf_regexp->is_bad_bnf)
            return bnf_regexp;

        if (bnf_regexp->regexp_type == concatenationExpr) {

            // check unread init under kleene or under alternation ({}:1(&1|a))*
            if (!bnf_regexp->unread_init.empty()) {
                bnf_regexp = bnf_regexp->conc_handle_init_without_read();
            }
            // check read that may be uninitialized ({}:1&1|a)&1
            else if (!bnf_regexp->uninited_read.empty()) {
                bnf_regexp = bnf_regexp->conc_handle_read_without_init();
            }

//            if (bnf_regexp->regexp_type == concatenationExpr) {
//                //check init in alternations
//                int i = 0;
//                for (auto it = bnf_regexp->sub_regexps.begin(); it != bnf_regexp->sub_regexps.end(); it++) {
//                    auto sub_r = *it;
//                    if (sub_r->regexp_type == backreferenceExpr && sub_r->sub_regexp->regexp_type == alternationExpr &&
//                        !sub_r->sub_regexp->alt_init_without_read().empty()) {
//                        (*it) = sub_r->take_out_alt_under_backref();
//                    }
//                    sub_r = *it;
//                    if (sub_r->regexp_type == alternationExpr && !sub_r->alt_init_without_read().empty()) {
//                        bnf_regexp = bnf_regexp->distribute_to_right(i);
//                        break;
//                    }
//                    i++;
//                }
//            }
        }

        return bnf_regexp;
    }
    else if (regexp_type == kleeneStar || regexp_type == kleenePlus) {
        if (is_slided)
            return this;
        auto *bnf_regexp = new Regexp(regexp_type);
        auto *new_sub_r = sub_regexp->_bnf(this, true);
        bnf_regexp->sub_regexp = new_sub_r;
        if (regexp_type == kleeneStar)
            bnf_regexp->star_kleene_vars(new_sub_r);
        else
            bnf_regexp->copy_vars(new_sub_r);

        if (bnf_regexp->sub_regexp->regexp_type == alternationExpr) {
            bnf_regexp = bnf_regexp->open_alt_under_kleene();
        }
        else {
            auto old_r = copy(bnf_regexp);
            bnf_regexp = bnf_regexp->handle_rw_under_kleene(parent, cur_position);
            bool changed = !old_r->is_equal(bnf_regexp);

            if (changed && !bnf_regexp->is_bad_bnf) {
                bnf_regexp = bnf_regexp->_bnf(parent, false, cur_position);
            }
        }
        return bnf_regexp;
    }
    else if (regexp_type == backreferenceExpr) {
        auto *bnf_regex = new Regexp(backreferenceExpr);
        bnf_regex->variable = variable;
        bnf_regex->sub_regexp = sub_regexp->_bnf(this);
        bnf_regex->copy_vars(bnf_regex->sub_regexp);
        bnf_regex->initialized[variable].push_back(bnf_regex);
        bnf_regex->maybe_initialized.insert(variable);
        bnf_regex->unread_init.insert(variable);
        bnf_regex->definitely_unread_init.insert(variable);

        return bnf_regex;
    }
}

Regexp *Regexp::bnf(bool is_log) {
    if (is_log) {
        log.open("log.txt", std::ofstream::out | std::ofstream::trunc);
    }

    if (!is_acreg()){
        cout << "Регулярное выражение не удовлетворяет условию ацикличности и не может быть нормализовано" << endl;
        is_bad_bnf = true;
        return this;
    }

    auto *new_r = _bnf(nullptr);

    if (new_r->is_bad_bnf)
        return new_r;

//    cout << new_r->to_string() << endl;

    map<string, Regexp*> empty_map;
    new_r->bind_init_to_read(empty_map);
    new_r = new_r->clear_initializations_and_read(new_r->definitely_unread_init, new_r->uninited_read);

    log.close();

    return new_r;
}