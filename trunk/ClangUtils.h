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
	class SourceManager;
	class Stmt;
	class Preprocessor;
	class LangOptions;
	class MacroInfo;
} // namespace clang

namespace ccons {

typedef std::pair<unsigned, unsigned> SrcRange;

SrcRange getStmtRange(const clang::Stmt *S,
                      const clang::SourceManager& sm,
                      const clang::LangOptions& options);
SrcRange getStmtRangeWithSemicolon(const clang::Stmt *S,
                                   const clang::SourceManager& sm,
                                   const clang::LangOptions& options);
SrcRange getMacroRange(const clang::MacroInfo *MI,
                       const clang::SourceManager& sm,
                       const clang::LangOptions& options);
} // namespace ccons

#endif // CCONS_CLANG_UTILS_H
