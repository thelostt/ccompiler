#pragma once

#include "cci/syntax/diagnostics.hpp"
#include "cci/syntax/scanner.hpp"
#include "cci/syntax/source_map.hpp"
#include "cci/syntax/token.hpp"
#include "gtest/gtest.h"
#include <memory_resource>
#include <ostream>
#include <queue>
#include <type_traits>

namespace cci::test
{

struct CompilerFixture : ::testing::Test
{
protected:
    syntax::SourceMap source_map;
    diag::Handler diag_handler;
    std::queue<diag::Diagnostic> diags;
    std::pmr::monotonic_buffer_resource arena;

    CompilerFixture()
        : source_map()
        , diag_handler([this](const diag::Diagnostic &d) { diags.push(d); },
                       source_map)
        , arena()
    {}

    void TearDown() override
    {
        EXPECT_TRUE(diags.empty()) << "size(diags) = " << diags.size();
    }

    auto create_filemap(std::string name, std::string source)
        -> const syntax::FileMap &
    {
        return source_map.create_owned_filemap(std::move(name),
                                               std::move(source));
    }

    auto get_source_text(const syntax::Token &tok) const -> std::string_view
    {
        return source_map.span_to_snippet(tok.source_span);
    }

    auto get_lexeme_view(const syntax::Token &tok) -> std::string_view
    {
        char *lexeme_buffer = new (this->arena.allocate(
            tok.size(), alignof(char))) char[tok.size() + 1];
        const size_t lexeme_len = syntax::Scanner::get_spelling_to_buffer(
            tok, lexeme_buffer, this->source_map);
        lexeme_buffer[lexeme_len] = '\0';
        return {lexeme_buffer, lexeme_len};
    }

    auto get_lexeme(const syntax::Token &tok) const -> std::string
    {
        std::string lexeme;
        lexeme.resize(tok.size());
        const size_t len = syntax::Scanner::get_spelling_to_buffer(
            tok, lexeme.data(), source_map);
        lexeme.resize(len);
        return lexeme;
    }

    auto pop_diag() -> diag::Diagnostic
    {
        EXPECT_FALSE(diags.empty());
        auto d = diags.front();
        diags.pop();
        return d;
    }

    auto peek_diag() -> const diag::Diagnostic &
    {
        EXPECT_FALSE(diags.empty());
        return diags.front();
    }
};
} // namespace cci::test

namespace cci
{
inline void PrintTo(const syntax::TokenKind token_kind,
                    std::ostream *os) noexcept
{
    *os << to_string(token_kind);
}

namespace diag
{
    inline void PrintTo(const Diag msg, std::ostream *os) noexcept
    {
        *os << "Diag(" << static_cast<std::underlying_type_t<diag::Diag>>(msg)
            << ")";
    }
} // namespace diag
} // namespace cci
