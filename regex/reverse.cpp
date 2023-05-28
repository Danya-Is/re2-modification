#include "regex.h"

void Regexp::bind_init_to_read(map<string, Regexp *> init) {
    if (regexp_type == epsilon || regexp_type == literal){}
    else if (regexp_type == reference) {
        if (init.find(variable) != init.end()){
            reference_to = init[variable];
            init[variable]->is_read = true;
        }

    }
    else if (regexp_type == concatenationExpr || regexp_type == alternationExpr) {
        for (auto &sub_r : sub_regexps) {
            sub_r->bind_init_to_read(init);
            if (regexp_type == concatenationExpr && !sub_r->initialized.empty()) {
                for (auto el: sub_r->initialized) {
                    // интересна последняя инициализация перед возможным чтением
                    init[el.first] = el.second.back();
                }
            }
        }
    }
    else if (regexp_type == kleeneStar || regexp_type == kleenePlus || regexp_type == backreferenceExpr) {
        sub_regexp->bind_init_to_read(init);
    }
}
