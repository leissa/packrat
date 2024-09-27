#pragma once

#include <cassert>

#include <fe/format.h>
#include <fe/loc.h>
#include <fe/sym.h>

namespace packrat {

using fe::Loc;
using fe::Pos;
using fe::Sym;

// clang-format off
#define RAT_KEY(m)              \
    m(K_category,   "category") \
    m(K_notation,   "notation") \
    m(K_macro,      "macro")    \

#define RAT_VAL(m)                        \
    m(V_int,        "<interger literal>") \
    m(V_sym,        "<identifier>")       \

#define RAT_TOK(m)                   \
    m(EoF,          "<end of file>") \
    /* delimiter */                  \
    m(D_paren_l,    "(")             \
    m(D_paren_r,    ")")             \
    /* further tokens */             \
    m(T_ass,        "=")             \
    m(T_semicolon,  ";")             \

#define CODE(t, str) + 1
constexpr auto Num_Keys = 0 RAT_KEY(CODE);
#undef CODE

#define RAT_OP(m)        \
    m(O_alt,  "/", Alt)  \
    m(O_and,  "&", Pre)  \
    m(O_not,  "!", Pre)  \
    m(O_star, "*", Post) \
    m(O_plus, "+", Post) \
    m(O_opt,  "?", Post)
// clang-format on

class Tok {
public:
    // clang-format off
    enum class Tag {
        Nil,
#define CODE(t, _) t,
        RAT_KEY(CODE)
        RAT_VAL(CODE)
        RAT_TOK(CODE)
#undef CODE
#define CODE(t, str, prec) t,
        RAT_OP (CODE)
#undef CODE
    };
    // clang-format on

    enum class Prec {
        Error,
        Bottom,
        Alt,
        Seq,
        Pre,
        Post,
    };

    Tok() {}
    Tok(Loc loc, Tag tag)
        : loc_(loc)
        , tag_(tag) {}
    Tok(Loc loc, Sym sym)
        : loc_(loc)
        , tag_(Tag::V_sym)
        , sym_(sym) {}
    Tok(Loc loc, uint64_t u64)
        : loc_(loc)
        , tag_(Tag::V_int)
        , u64_(u64) {}

    Loc loc() const { return loc_; }
    Tag tag() const { return tag_; }
    bool isa(Tag tag) const { return tag == tag_; }
    bool isa_key() const { return (int)tag() < Num_Keys; }
    explicit operator bool() const { return tag_ != Tag::Nil; }

    Sym sym() const {
        assert(isa(Tag::V_sym));
        return sym_;
    }
    uint64_t u64() const { return u64_; }

    static std::string_view str(Tok::Tag);

    friend std::ostream& operator<<(std::ostream&, Tag);
    friend std::ostream& operator<<(std::ostream&, Tok);

private:
    Loc loc_;
    Tag tag_ = Tag::Nil;
    union {
        Sym sym_;
        uint64_t u64_;
    };
};

} // namespace packrat

template<> struct std::formatter<packrat::Tok> : fe::ostream_formatter {};
