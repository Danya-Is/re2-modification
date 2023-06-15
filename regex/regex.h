#ifndef COURSEWORK_REGEX_H
#define COURSEWORK_REGEX_H

#include <string>
#include <queue>
#include <fstream>

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

    /// регулярка не обращается в текущей версии алгоритма
    bool is_bad_bnf;

    /// итерация над rw, которая уже была раскрыта, и соответственно должна далее игнорироваться, чтобы не зациклиться
    bool is_slided;

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
    /// для чтений перед инициализацией под итерацией `(&1{(a|b)}:1)*`
    set<string> definitely_uninit_read;

    /// переменные строящие rw блок на текущем уровне, a(&1|{}:1)b не считается
    set<string> rw_vars;

    bool is_equal(Regexp* other);

    Regexp* last_init(const string& var) {
        if (initialized.find(var) == initialized.end())
            return nullptr;
        else
            return initialized[var].back();
    }

    Regexp* prefix_last_init(const string& var, list<Regexp*>::iterator it) {
        while (it != sub_regexps.end()) {
            auto new_last_init = (*it)->last_init(var);
            if (new_last_init)
                return new_last_init;
            it--;
        }
        return nullptr;
    }

    /// ситуации под инициализацией в духе ({&j}:i {}:j &i)*
    bool is_cross_references();
    map<string, list<string>> get_inner_reads();

    void _push_sub_regexp(Regexp *sub_r, bool front = false);
    void push_concat(Regexp* sub_r, bool front = false);
    void push_alt(Regexp* sub_r, bool front = false);
    void push_sub_regexp(Regexp* sub_r, bool front = false);

    void concat_vars(Regexp* regexp);
    void alt_vars(Regexp* regexp);
    void star_kleene_vars(Regexp* regexp);
    void copy_vars(Regexp* regexp);
    void change_vars(Regexp* regexp);

    // чтобы каждой инициализации при раскрытии по преобразованиям соответствовал уникальный указатель
    Regexp* copy(Regexp* regexp);

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

    Regexp* open_kleene_plus(set<string> vars, bool need_for_cleen = true);
    Regexp* open_kleene(set<string> vars, bool need_for_cleen = true);
    Regexp* open_kleene_with_read(set<string> vars, Regexp* parent, list<Regexp*>::iterator prefix_index);

    template<class Compare>
    list<string> make_var_queue(priority_queue<pair<string, int>, vector<pair<string, int>>, Compare> pq);
    Regexp* open_alt_under_kleene(bool min_init_order = false);
    Regexp* _open_alt_under_kleene(const string& var);

    Regexp* denesting(Regexp* a_alt, Regexp* b_alt);
    Regexp* rw_in_conc_under_kleene(Regexp* parent, list<Regexp*>::iterator prefix_index);
    Regexp* rw_in_alt_under_kleene();
    Regexp* handle_rw_under_kleene(Regexp* parent, list<Regexp*>::iterator prefix_index);

    Regexp* take_out_alt_under_backref();
    Regexp* distribute_to_right(int alt_pos);
    Regexp* distribute_to_left(int alt_pos);
    Regexp* distribute_completely(int alt_pos);

    Regexp* conc_handle_init_without_read();
    Regexp* conc_handle_read_without_init();

    Regexp* bnf();
    /// parent важен только если это конкатенация (rw блоки)
    Regexp* _bnf(Regexp* parent, bool under_kleene = false, list<Regexp*>::iterator cur_position = list<Regexp*>().begin());

    Regexp* replace_read_write(set<Regexp*>& initialized_in_reverse);
    Regexp* _reverse();
    Regexp* reverse();

    void bind_init_to_read(map<string, Regexp *>& init);

    BinaryTree* to_binary_tree();

    Automata * compile(bool &is_mfa);

    bool match(const string& input_str);
};

void run_examples();

void match(string regexp_str);

void match_gt(string regexp_str);

void match_mfa(string regexp_str);

#endif //COURSEWORK_REGEX_H
