#include "packrat/lexer.h"

#include <ranges>

#include <fe/loc.cpp.h>

#include "fe/utf8.h"

using namespace std::literals;

namespace packrat {

using Tag      = Tok::Tag;
namespace utf8 = fe::utf8;

Lexer::Lexer(Driver& driver, std::istream& istream, const std::filesystem::path* path)
    : fe::Lexer<1, Lexer>(istream, path)
    , driver_(driver) {
    if (!istream_) throw std::runtime_error("stream is bad");
#define CODE(t, str) keywords_[driver_.sym(str)] = Tag::t;
    RAT_KEY(CODE)
#undef CODE
}

Tok Lexer::lex() {
    while (true) {
        start();

        if (accept(utf8::EoF)) return {loc_, Tag::EoF};
        if (accept(utf8::isspace)) continue;
        if (accept('(')) return {loc_, Tag::D_paren_l};
        if (accept(')')) return {loc_, Tag::D_paren_r};
        if (accept('=')) return {loc_, Tag::T_ass};
        if (accept(';')) return {loc_, Tag::T_semicolon};
        if (accept('*')) return {loc_, Tag::O_star};
        if (accept('+')) return {loc_, Tag::O_plus};
        if (accept('?')) return {loc_, Tag::O_opt};
        if (accept('&')) return {loc_, Tag::O_and};
        if (accept('!')) return {loc_, Tag::O_not};
        if (accept('/')) {
            if (accept('*')) {
                eat_comments();
                continue;
            }
            if (accept('/')) {
                while (ahead() != utf8::EoF && ahead() != '\n') next();
                continue;
            }

            return {loc_, Tag::O_alt};
        }

        // integer value
        if (accept(utf8::isdigit)) {
            while (accept(utf8::isdigit)) {}
            return {loc_, std::strtoull(str_.c_str(), nullptr, 10)};
        }

        // lex identifier or keyword
        if (accept<Append::Lower>([](char32_t c) { return c == '_' || utf8::isalpha(c); })) {
            while (accept<Append::Lower>([](char32_t c) { return c == '_' || utf8::isalpha(c) || utf8::isdigit(c); })) {
            }
            auto sym = driver_.sym(str_);
            if (auto i = keywords_.find(sym); i != keywords_.end()) return {loc_, i->second}; // keyword
            return {loc_, Tag::V_sym, sym};                                                   // identifier
        }

        if (accept(utf8::Null)) {
            driver().err(loc_, "invalid UTF-8 character");
            continue;
        }

        while (ahead() != utf8::EoF && utf8::isspace(ahead())) {}
        auto sym = driver_.sym(str_);
        return {loc_, Tag::V_any, sym};
    }
}

void Lexer::eat_comments() {
    while (true) {
        while (ahead() != utf8::EoF && ahead() != '*') next();
        if (ahead() == utf8::EoF) {
            driver_.err(loc_, "non-terminated multiline comment");
            return;
        }
        next();
        if (accept('/')) break;
    }
}

} // namespace packrat
