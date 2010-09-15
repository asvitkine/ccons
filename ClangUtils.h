#ifndef CCONS_CLANG_UTILS_H
#define CCONS_CLANG_UTILS_H

//
// Header for ClangUtils.cpp which contains utility functionality that
// deals with the clang libraries and data structures.
//
// Part of ccons, the interactive console for the C programming language.
//
// Copyright (c) 2009 Alexei Svitkine. This file is distributed under the
// terms of MIT Open Source License. See file LICENSE for details.
//

#include <algorithm>

#include <llvm/ADT/OwningPtr.h>

namespace clang {
	class SourceLocation;
	class SourceManager;
	class Stmt;
	class Preprocessor;
	class LangOptions;
	class MacroInfo;
} // namespace clang

namespace ccons {

// Pair of start, end positions in the source.
typedef std::pair<unsigned, unsigned> SrcRange;

// Get the source range of the specified Stmt.
SrcRange getStmtRange(const clang::Stmt *S,
                      const clang::SourceManager& sm,
                      const clang::LangOptions& options);

// Get the source range of the specified Stmt, ensuring that a semicolon is
// included, if necessary - since the clang ranges do not guarantee this.
SrcRange getStmtRangeWithSemicolon(const clang::Stmt *S,
                                   const clang::SourceManager& sm,
                                   const clang::LangOptions& options);

// Get the source range of the macro definition excluding the #define.
SrcRange getMacroRange(const clang::MacroInfo *MI,
                       const clang::SourceManager& sm,
                       const clang::LangOptions& options);

SrcRange constructSrcRange(const clang::SourceManager& sm,
                           const clang::LangOptions& options,
                           const clang::SourceLocation& SLoc,
                           const clang::SourceLocation& ELoc);

} // namespace ccons

#endif // CCONS_CLANG_UTILS_H
