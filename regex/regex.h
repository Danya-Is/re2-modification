#ifndef COURSEWORK_REGEX_H
#define COURSEWORK_REGEX_H

#include <string>
#include <queue>

#include "list"

#include "../automata.h"

using namespace std;

class BinaryTree;

enum RegexpType {
    // '('
    leftParenthesis,

    // ')'
    rightParenthesis,

    // '{'
    leftBrace,

    // '}'
    rightBrace,

    // '['
    leftSquareBr,

    // ']'
    rightSquareBr,

    // '-'
    dash,

    // '*'
    kleeneStar,

    // '+'
    kleenePlus,

    // '|'
    alternation,

    // one symbol
    literal,

    // empty string
    epsilon,

    //(regex | regex | ...)
    alternationExpr,

    // regex ++ regex ++ regex
    concatenationExpr,

    // backreference expression
    backreferenceExpr,

    // reference to expression
    reference,

    // root regex
    rootExpr
};

class Regexp;

void union_sets(set<string> &first, set<string> &second);
void intersect_sets(set<string> &first, set<string> &second);
void concat_maps(map<string, list<Regexp*>> &first, map<string, list<Regexp*>> &second);

class Regexp {
public:
    RegexpType regexp_type;
    string regexp_str;


    char rune; // literal
    Regexp* sub_regexp; // kleene
    list<Regexp*> sub_regexps; // alternative and concatination
    string variable; // backreferenceExpr expression
    bool is_read; // backreferenceExpr
    Regexp* reference_to; // reference

    //    TODO
    bool have_backreference;

    /// точно инициализированные
    map<string, list<Regexp*>> initialized;
    /// точно прочитанные
    set<string> read;

    /// возможно инициализированные
    set<string> maybe_initialized;
    /// возможно прочитанные
    set<string> maybe_read;

    /// есть ли в регулярка (конкатенации) путь при котором переменная может оказать неинициализированной `({}:1&1|a)&1`
    set<string> uninited_read;
    /// нужно внутри альтернатив и под итерациями, возможно непрочтенные
    set<string> unread_init;
    /// нужно для очистки итоговой регулярки
    set<string> definitely_unread_init;

    void concat_vars(Regexp* regexp) {
        if (sub_regexps.empty())
            copy_vars(regexp);
        else {
            union_sets(uninited_read, regexp->uninited_read);
            for (const auto& now_init: initialized) {
                if (uninited_read.find(now_init.first) != uninited_read.end()) {
                    uninited_read.erase(now_init.first);
                }
            }
            union_sets(unread_init, regexp->unread_init);
            for (const auto& now_read: regexp->read) {
                if (unread_init.find(now_read) != unread_init.end()) {
                    unread_init.erase(now_read);
                }
            }

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

    void alt_vars(Regexp* regexp) {
        union_sets(uninited_read, regexp->uninited_read);
        union_sets(unread_init, regexp->unread_init);
        union_sets(definitely_unread_init, regexp->definitely_unread_init);

//        intersect_sets(initialized, regexp->initialized);
        intersect_sets(read, regexp->read);
        union_sets(maybe_read, regexp->maybe_read);
        union_sets(maybe_initialized, regexp->maybe_initialized);
    }

    void star_kleene_vars(Regexp* regexp) {
        maybe_initialized = regexp->maybe_initialized;
        maybe_read = regexp->maybe_read;
        uninited_read = regexp->uninited_read;
        unread_init = regexp->unread_init;
        definitely_unread_init = regexp->definitely_unread_init;
    }

    void copy_vars(Regexp* regexp) {
        union_sets(uninited_read, regexp->uninited_read);
        union_sets(definitely_unread_init, regexp->definitely_unread_init);
        union_sets(unread_init, regexp->unread_init);
        if (regexp_type != alternationExpr)
            concat_maps(initialized, regexp->initialized);
        union_sets(read, regexp->read);
        union_sets(maybe_read, regexp->maybe_read);
        union_sets(maybe_initialized, regexp->maybe_initialized);
    }

    void change_vars(Regexp* regexp) {
        initialized = regexp->initialized;
        read = regexp->read;
        maybe_initialized = regexp->maybe_initialized;
        maybe_read = regexp->maybe_read;
        uninited_read = regexp->uninited_read;
        unread_init = regexp->unread_init;
        definitely_unread_init = regexp->definitely_unread_init;
    }

    Regexp() {
    };

    Regexp(RegexpType type) {
        regexp_type = type;
    }

    static Regexp* parse_regexp(string &s);
    string to_string();

    void do_concatenation();
    void do_collapse();
    void do_kleene(char c);
    void do_backreference(string name);
    void do_enumeration();

    /// Проверяет корректность расширенного регулярного выражения по правилу:
    /// чтение из переменной не может использоваться раньше инициализации переменной,
    /// при этом инициализировать переменную, в смысле записи выражения, можно ровно один раз.
    /// Допускаются выражения `({a*}:1|&1)*`
    bool is_backref_correct();
    void _is_backref_correct(set<string> &initialized_vars,
                             set<string> &read_before_init,
                             bool &double_initialized);

    Regexp* simplify_conc_alt();

    /// возможно неинициализированные чтения в вариантах альтернативы (под Клини)
    set<string> alt_read_without_init();
    /// количество инициализаций переменных в вариантах альтернативы (под Клини)
    map<string, int> alt_init();
    /// есть ли в альтернативе варианты с неиспользованной инициализацией
    set<string> alt_init_without_read();

    /// удаление лишних инициализий после преобразований
    Regexp *clear_initializations_and_read(set<string> vars, set<string> read_vars = {});
    Regexp* open_kleene_plus(set<string> vars);
    Regexp* open_kleene(set<string> vars);
    Regexp* open_kleene_with_read(set<string> vars);
    Regexp* kleene_star_to_plus();

    template<class Compare>
    list<string> make_var_queue(priority_queue<pair<string, int>, vector<pair<string, int>>, Compare> pq);
    Regexp* open_alt_under_kleene(bool min_init_order = false);
    Regexp* _open_alt_under_kleene(const string& var);

    Regexp* take_out_alt_under_backref();
    Regexp* distribute_to_right(int alt_pos);
    Regexp* distribute_to_left(int alt_pos);

    Regexp* conc_handle_init_without_read();
    Regexp* conc_handle_read_without_init();

    Regexp* bnf();
    Regexp* _bnf(bool under_kleene = false, bool under_alt = false);

    void bind_init_to_read(map<string, Regexp*> init);

    BinaryTree* to_binary_tree();

    Automata* compile();

    bool match(const string& input_str);
};

void match(string regexp_str);

void match_gt(string regexp_str);

void match_mfa(string regexp_str);

#endif //COURSEWORK_REGEX_H
