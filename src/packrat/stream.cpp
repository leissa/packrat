#include <iostream>

#include "packrat/ast.h"

namespace packrat {

// stream

void Node::dump() const { stream(std::cout); }

/*
 * Decl
 */

std::ostream& SyntaxCatDecl::stream(std::ostream& o) const { return o << "category " << sym(); }

/*
 * Expr
 */

// clang-format off
std::ostream& ErrExpr::stream(std::ostream& o) const { return o << "<error expression>"; }
std::ostream& LitExpr::stream(std::ostream& o) const { return o << u64(); }
std::ostream& SymExpr::stream(std::ostream& o) const { return o << sym(); }
// clang-format on

std::ostream& PrefixExpr::stream(std::ostream& o) const {
    return rhs()->stream(o << tag() << '(') << ')';
}

std::ostream& PostfixExpr::stream(std::ostream& o) const {
    return lhs()->stream(o << '(') << ')' << tag();
}

std::ostream& BinExpr::stream(std::ostream& o) const {
    o << '(';
    lhs()->stream(o);
    o << ' ' << tag() << ' ';
    rhs()->stream(o);
    return o << ')';
}

std::ostream& SeqExpr::stream(std::ostream& o) const {
    o << '(';
    lhs()->stream(o);
    o << ' ';
    rhs()->stream(o);
    return o << ')';
}

/*
 * Prog
 */

std::ostream& Prog::stream(std::ostream& o) const {
    for (auto&& decl : decls()) decl->stream(o) << std::endl;
    return o;
}

} // namespace packrat
