#pragma once

#include <cassert>

#include <istream>
#include <unordered_map>

#include <fe/lexer.h>

#include "packrat/driver.h"
#include "packrat/tok.h"

namespace packrat {

class Lexer : public fe::Lexer<1, Lexer> {
public:
    Lexer(Driver&, std::istream&, const std::filesystem::path*);

    Tok lex(); ///< Get next Tok in stream.
    Driver& driver() { return driver_; }

private:
    void eat_comments();

    Driver& driver_;
    fe::SymMap<Tok::Tag> keywords_;
};

} // namespace packrat
