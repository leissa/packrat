#pragma once

#include <fe/parser.h>

#include "packrat/ast.h"
#include "packrat/driver.h"
#include "packrat/lexer.h"

namespace packrat {

class Parser : public fe::Parser<Tok, Tok::Tag, 1, Parser> {
public:
    Parser(Driver&, std::istream&, const std::filesystem::path* = nullptr);

    Driver& driver() { return lexer_.driver(); }
    Lexer& lexer() { return lexer_; }

    AST<Prog> parse_prog();

private:
    template<class T, class... Args>
    auto ast(Args&&... args) {
        return driver().ast<T>(std::forward<Args&&>(args)...);
    }

    Sym parse_sym(std::string_view ctxt = {});

    AST<Decl> parse_category_decl();

    AST<Expr> parse_expr(std::string_view ctxt, Tok::Prec = Tok::Prec::Bottom);
    AST<Expr> parse_primary_or_prefix_expr(std::string_view ctxt);

    /// Issue an error message of the form:
    /// `expected <what>, got '<tok>' while parsing <ctxt>`
    void err(const std::string& what, const Tok& tok, std::string_view ctxt);

    /// Same above but uses Parser::ahead() as Tok%en.
    void err(const std::string& what, std::string_view ctxt) { err(what, ahead(), ctxt); }

    void syntax_err(Tok::Tag tag, std::string_view ctxt);

    Lexer lexer_;
    Sym error_;

    friend class fe::Parser<Tok, Tok::Tag, 1, Parser>;
};

} // namespace packrat
