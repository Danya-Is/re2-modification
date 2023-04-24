#include <utility>

#include "regex.h"
#include "binary_tree.h"

using namespace std;

Regexp* Regexp::parse_regexp(string &s) {
    auto* regexp = new Regexp(rootExpr);

    while (!s.empty()) {
        char c = s[0];
        if (c == '*' || c == '+') {
            regexp->doKleene(c);
        } else if (c == '(') {
            auto* re = new Regexp(leftParenthesis);
            regexp->sub_regexps.push_back(re);
        } else if (c == '|') {
            regexp->doConcatenation();
        } else if (c == ')') {
            regexp->doCollapse();
        }
        else if (c == '['){
            auto* re = new Regexp(leftSquareBr);
            regexp->sub_regexps.push_back(re);
        }
        else if (c == ']') {
            regexp->doEnumeration();
        }
        else if (c == '-') {
            auto* re = new Regexp(dash);
            regexp->sub_regexps.push_back(re);
        }
        else if (c == '{') {
            auto* re = new Regexp(leftBrace);
            regexp->sub_regexps.push_back(re);
        }
        else if (c == '}') {
            s.erase(0, 1);
            string tmp = substr(s, 1);
            if (tmp != ":") {
                printf("Expected :");
            }
            s.erase(0, 1);
            string name  = substr(s, 1);
            regexp->doBackreference(name);
        }
        else if (c == '&') {
            s.erase(0, 1);
            string name  = substr(s, 1);
            auto* new_re = new Regexp(reference);
            new_re->variable = name;
            regexp->sub_regexps.push_back(new_re);
        }
        else if ((c >= 'a' and c <= 'z') or c == '.') {
            auto* re = new Regexp(literal);
            re->rune = c;
            regexp->sub_regexps.push_back(re);
        }
        else {
            printf("Unexpected literal %c", c);
        }
        s.erase(0, 1);
    }
    if (regexp->sub_regexps.size() == 1 && regexp->sub_regexps.front()->regexp_type == literal) {
        char c = regexp->sub_regexps.back()->rune;
        regexp->rune = c;
        regexp->regexp_type = literal;
    }
    else if (regexp->sub_regexps.size() > 1) {
        regexp->regexp_type = concatenationExpr;
    }
    if (regexp->regexp_type == rootExpr && regexp->sub_regexps.size() == 1) {
        auto* child = regexp->sub_regexps.back();
        regexp = child;
    }
    return regexp;
}

void Regexp::doConcatenation() {
    Regexp* stack_top = sub_regexps.back();
    auto* new_re = new Regexp();
    while (stack_top->regexp_type != leftParenthesis && stack_top->regexp_type != alternation) {
        new_re->sub_regexps.push_front(stack_top);
        sub_regexps.pop_back();
        stack_top = sub_regexps.back();
    }
    if (new_re->sub_regexps.size() == 1) {
        new_re = new_re->sub_regexps.back();
    }
    else {
        new_re->regexp_type = concatenationExpr;
    }
    sub_regexps.push_back(new_re);
    sub_regexps.push_back(new Regexp(alternation));
}


void Regexp::doCollapse() {
    Regexp* stack_top = sub_regexps.back();
    auto* new_re = new Regexp();
    new_re->regexp_type = concatenationExpr;
    auto* new_alt = new Regexp;
    new_alt->regexp_type = alternationExpr;
    while (stack_top->regexp_type != leftParenthesis) {
        sub_regexps.pop_back();
        if (stack_top->regexp_type != alternation) {
            new_re->sub_regexps.push_front(stack_top);
        } else {
            if (new_re->sub_regexps.size() == 1) {
                new_re = new_re->sub_regexps.back();
            }
            new_alt->sub_regexps.push_front(&(*new_re));
            new_re = new Regexp();
            new_re->regexp_type = concatenationExpr;
        }
        stack_top = sub_regexps.back();
    }
    sub_regexps.pop_back();
    if (new_alt->sub_regexps.empty()) {
        if (new_re->sub_regexps.size() == 1) {
            new_re = new_re->sub_regexps.back();
        }
        sub_regexps.push_back(new_re);
    }
    else {
        if (new_re->sub_regexps.size() == 1) {
            new_re = new_re->sub_regexps.back();
        }
        new_alt->sub_regexps.push_front(&(*new_re));
        sub_regexps.push_back(new_alt);
    }

}

void Regexp::doKleene(char c) {
    auto* stack_top = sub_regexps.back();
    sub_regexps.pop_back();
    Regexp* re;
    if (c == '*')
        re = new Regexp(kleeneStar);
    else
        re = new Regexp(kleenePlus);
    re->sub_regexp = stack_top;
    sub_regexps.push_back(re);
}

void Regexp::doBackreference(string name) {
    Regexp* stack_top = sub_regexps.back();
    auto* new_re = new Regexp();
    new_re->regexp_type = backreferenceExpr;
    new_re->variable = std::move(name);
    while (stack_top->regexp_type != leftBrace) {
        sub_regexps.pop_back();
        new_re->sub_regexps.push_front(stack_top);
        stack_top = sub_regexps.back();
    }
    sub_regexps.pop_back();

    new_re->sub_regexps.push_front(new Regexp(leftParenthesis));
    new_re->doCollapse();
    if (new_re->sub_regexps.size() == 1) {
        new_re->sub_regexp = new_re->sub_regexps.back();
        new_re->sub_regexps.clear();
    } else {
        printf("PARSER: Backreference can not have more than one subregex");
    }

    sub_regexps.push_back(new_re);
}

void Regexp::doEnumeration() {
    Regexp* stack_top = sub_regexps.back();
    auto* new_re = new Regexp(alternationExpr);
    while (stack_top->regexp_type != leftSquareBr) {
        if (stack_top->regexp_type == literal) {
            new_re->sub_regexps.push_front(stack_top);
        } else if (stack_top->regexp_type == dash) {
            Regexp* last_literal = new_re->sub_regexps.front();
            new_re->sub_regexps.pop_front();
            sub_regexps.pop_back();
            Regexp* first_literal = sub_regexps.back();
            char c = first_literal->rune;
            while (c <= last_literal->rune) {
                auto* new_sub = new Regexp(literal);
                new_sub->rune = c;
                new_re->sub_regexps.push_front(new_sub);
                c = char(int(c) + 1);
            }
        } else {
            printf("PARSER: Expected dash or literal inside enumeration");
        }

        sub_regexps.pop_back();
        stack_top = sub_regexps.back();
    }
    sub_regexps.pop_back();
    if (new_re->sub_regexps.size() == 1) {
        new_re = new_re->sub_regexps.back();
    }
    sub_regexps.push_back(new_re);
}
