#include "packrat/tok.h"

#include <fe/assert.h>

using namespace std::literals;

namespace packrat {

std::string_view Tok::str(Tok::Tag tag) {
    switch (tag) {
#define CODE(t, str) \
    case Tok::Tag::t: return str##sv;
        RAT_KEY(CODE)
        RAT_VAL(CODE)
        RAT_TOK(CODE)
#undef CODE
#define CODE(t, str, prec) \
    case Tok::Tag::t: return str##sv;
        RAT_OP(CODE)
#undef CODE
        default: fe::unreachable();
    }
}

std::ostream& operator<<(std::ostream& o, Tok::Tag tag) { return o << Tok::str(tag); }

std::ostream& operator<<(std::ostream& o, Tok tok) {
    if (tok.isa(Tok::Tag::V_sym)) return o << *tok.sym();
    if (tok.isa(Tok::Tag::V_int)) return o << tok.u64();
    return o << Tok::str(tok.tag());
}

} // namespace packrat
