#pragma once

#include <deque>
#include <ostream>

#include <fe/cast.h>

#include "packrat/tok.h"

namespace packrat {

/// Base class for all @p Expr%essions.
class Node : public fe::RuntimeCast<Node> {
public:
    Node(Loc loc)
        : loc_(loc) {}
    virtual ~Node() {}

    Loc loc() const { return loc_; }
    void dump() const;

    /// Stream to @p o.
    virtual std::ostream& stream(std::ostream& o) const = 0;

private:
    Loc loc_;
};

template<class T> using AST  = fe::Arena::Ptr<const T>;
template<class T> using ASTs = std::deque<AST<T>>;
using Env                    = fe::SymMap<uint64_t>;

/*
 * Decl
 */

/// Base class for all @p Decl%arations.
class Decl : public Node {
public:
    Decl(Loc loc)
        : Node(loc) {}
};

class SyntaxCatDecl : public Decl {
public:
    SyntaxCatDecl(Loc loc, Sym sym)
        : Decl(loc)
        , sym_(sym) {}

    Sym sym() const { return sym_; }

    std::ostream& stream(std::ostream& o) const override;

private:
    Sym sym_;
};

/*
 * Expr
 */

/// Base class for all @p Expr%essions.
class Expr : public Node {
public:
    Expr(Loc loc)
        : Node(loc) {}
};

class LitExpr : public Expr {
public:
    LitExpr(Tok tok)
        : Expr(tok.loc())
        , u64_(tok.u64()) {}

    uint64_t u64() const { return u64_; }

    std::ostream& stream(std::ostream&) const override;

private:
    uint64_t u64_;
};

class SymExpr : public Expr {
public:
    SymExpr(Tok tok)
        : Expr(tok.loc())
        , sym_(tok.sym()) {}

    Sym sym() const { return sym_; }

    std::ostream& stream(std::ostream&) const override;

private:
    Sym sym_;
};

class PrefixExpr : public Expr {
public:
    PrefixExpr(Loc loc, Tok::Tag tag, AST<Expr>&& rhs)
        : Expr(loc)
        , tag_(tag)
        , rhs_(std::move(rhs)) {}

    Tok::Tag tag() const { return tag_; }
    const Expr* rhs() const { return rhs_.get(); }

    std::ostream& stream(std::ostream&) const override;

private:
    Tok::Tag tag_;
    AST<Expr> rhs_;
};

class PostfixExpr : public Expr {
public:
    PostfixExpr(Loc loc, AST<Expr>&& lhs, Tok::Tag tag)
        : Expr(loc)
        , lhs_(std::move(lhs))
        , tag_(tag) {}

    const Expr* lhs() const { return lhs_.get(); }
    Tok::Tag tag() const { return tag_; }

    std::ostream& stream(std::ostream&) const override;

private:
    AST<Expr> lhs_;
    Tok::Tag tag_;
};

class BinExpr : public Expr {
public:
    BinExpr(Loc loc, AST<Expr>&& lhs, Tok::Tag tag, AST<Expr>&& rhs)
        : Expr(loc)
        , lhs_(std::move(lhs))
        , tag_(tag)
        , rhs_(std::move(rhs)) {}

    const Expr* lhs() const { return lhs_.get(); }
    Tok::Tag tag() const { return tag_; }
    const Expr* rhs() const { return rhs_.get(); }

    std::ostream& stream(std::ostream&) const override;

private:
    AST<Expr> lhs_;
    Tok::Tag tag_;
    AST<Expr> rhs_;
};

class SeqExpr : public Expr {
public:
    SeqExpr(Loc loc, AST<Expr>&& lhs, AST<Expr>&& rhs)
        : Expr(loc)
        , lhs_(std::move(lhs))
        , rhs_(std::move(rhs)) {}

    const Expr* lhs() const { return lhs_.get(); }
    const Expr* rhs() const { return rhs_.get(); }

    std::ostream& stream(std::ostream&) const override;

private:
    AST<Expr> lhs_;
    AST<Expr> rhs_;
};

/// Just a dummy that does nothing and will only be constructed during parse errors.
class ErrExpr : public Expr {
public:
    ErrExpr(Loc loc)
        : Expr(loc) {}

    std::ostream& stream(std::ostream&) const override;
};

/*
 * Prog
 */

class Prog : public Node {
public:
    Prog(Loc loc, ASTs<Decl>&& decls)
        : Node(loc)
        , decls_(std::move(decls)) {}

    const ASTs<Decl>& decls() const { return decls_; }

    std::ostream& stream(std::ostream&) const override;

private:
    Sym sym_;
    ASTs<Decl> decls_;
};

} // namespace packrat
