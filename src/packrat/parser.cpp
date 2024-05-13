#include "packrat/parser.h"

#include <iostream>

// clang-format off
#define C_EXPR           \
              D_paren_l: \
    case Tag::V_sym:     \
    case Tag::O_and:     \
    case Tag::O_not
// clang-format on

/* case Tag::O_alt:     \ */

using namespace std::literals;

namespace packrat {

using Tag  = Tok::Tag;
using Prec = Tok::Prec;

Parser::Parser(Driver& driver, std::istream& istream, const std::filesystem::path* path)
    : lexer_(driver, istream, path)
    , error_(driver.sym("<error>"s)) {
    init(path);
}

void Parser::err(const std::string& what, const Tok& tok, std::string_view ctxt) {
    driver().err(tok.loc(), "expected {}, got '{}' while parsing {}", what, tok, ctxt);
}

void Parser::syntax_err(Tag tag, std::string_view ctxt) {
    std::string msg("'");
    msg.append(Tok::str(tag)).append("'");
    err(msg, ctxt);
}

Sym Parser::parse_sym(std::string_view ctxt) {
    if (ahead().isa(Tag::V_sym)) return lex().sym();
    err("identifier", ctxt);
    return error_;
}

/*
 * Expr
 */

AST<Expr> Parser::parse_expr(std::string_view ctxt, Tok::Prec curr_prec) {
    auto track = tracker();
    auto lhs   = parse_primary_or_prefix_expr(ctxt);

    while (true) {
        switch (ahead().tag()) {
            case Tag::O_alt: {
                if (curr_prec >= Prec::Alt) return lhs;
                auto op  = lex().tag();
                auto rhs = parse_expr("right-hand side of binary expression", Prec::Alt);
                lhs      = ast<BinExpr>(track, std::move(lhs), op, std::move(rhs));
                continue;
            }
            case Tag::O_star:
            case Tag::O_plus:
            case Tag::O_opt: {
                if (curr_prec >= Prec::Post) return lhs;
                auto op = lex().tag();
                lhs     = ast<PostfixExpr>(track, std::move(lhs), op);
                continue;
            }
            case Tag::C_EXPR: {
                auto track = tracker();
                auto rhs   = parse_expr({}, Prec::Seq);
                lhs        = ast<SeqExpr>(track, std::move(lhs), std::move(rhs));
                continue;
            }
            default: return lhs;
        }
    }

    return lhs;
}

AST<Expr> Parser::parse_primary_or_prefix_expr(std::string_view ctxt) {
    switch (ahead().tag()) {
        case Tag::V_sym: return ast<SymExpr>(lex());
        case Tag::V_int: return ast<LitExpr>(lex());
        case Tag::D_paren_l: {
            lex();
            auto expr = parse_expr("parenthesized expression");
            expect(Tag::D_paren_r, "parenthesized expression");
            return expr;
        }
        case Tag::O_and:
        case Tag::O_not: {
            auto track = tracker();
            auto op    = lex().tag();
            return ast<PrefixExpr>(track, op, parse_expr("operand of unary expression", Prec::Pre));
        }
        default: break;
    }

    if (!ctxt.empty()) {
        err("primary or unary expression", ctxt);
        return ast<ErrExpr>(prev_);
    }

    fe::unreachable();
}

/*
 * Stmt
 */

AST<Stmt> Parser::parse_let_stmt() {
    auto track = tracker();
    eat(Tag::K_let);
    auto sym = parse_sym("name of a let-statement");
    expect(Tag::T_ass, "let-statement");
    auto init = parse_expr("initialization expression of a let-statement");
    expect(Tag::T_semicolon, "return-statement");
    return ast<LetStmt>(track, sym, std::move(init));
}

AST<Stmt> Parser::parse_print_stmt() {
    auto track = tracker();
    eat(Tag::K_print);
    auto expr = parse_expr("print-statement");
    expect(Tag::T_semicolon, "print-statement");
    return ast<PrintStmt>(track, std::move(expr));
}

/*
 * Prog
 */

AST<Prog> Parser::parse_prog() {
    auto track = tracker();
    ASTs<Stmt> stmts;
    while (true) {
        // clang-format off
        switch (ahead().tag()) {
            case Tag::K_let:   stmts.emplace_back(parse_let_stmt());   break;
            case Tag::K_print: stmts.emplace_back(parse_print_stmt()); break;
            case Tag::EoF:     return ast<Prog>(track, std::move(stmts));
            default:
                auto tok = lex();
                driver().err(tok.loc(), "expected statement, got '{}' while parsing program", tok);
        }
        // clang-format on
    }
}

} // namespace packrat
