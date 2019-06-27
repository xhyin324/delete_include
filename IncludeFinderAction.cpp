#include "IncludeFinderAction.hpp"
#include "path_utils.hpp"

#include <clang/Frontend/CompilerInstance.h>

#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/PPCallbacks.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <llvm/Support/CommandLine.h>
#include "llvm/ADT/StringRef.h"
#include <clang/Basic/Diagnostic.h>
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceLocation.h"

#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include "clang/Rewrite/Core/Rewriter.h"

using namespace clang::tooling;
using namespace llvm;
using namespace clang;
using namespace std;

namespace
{

class CallbacksProxy : public clang::PPCallbacks
{
public:
    inline CallbacksProxy(clang::PPCallbacks &master);

public:
    virtual inline void InclusionDirective(clang::SourceLocation hashLoc,
                                           const clang::Token &includeTok,
                                           clang::StringRef fileName,
                                           bool isAngled,
                                           clang::CharSourceRange filenameRange,
                                           const clang::FileEntry *file,
                                           clang::StringRef searchPath,
                                           clang::StringRef relativePath,
                                           const clang::Module *imported,
                                           clang::SrcMgr::CharacteristicKind filetype);

private:
    clang::PPCallbacks &master;
};

inline
CallbacksProxy::CallbacksProxy(clang::PPCallbacks &master) : master(master){}

inline void
CallbacksProxy::InclusionDirective(clang::SourceLocation hashLoc,
                                   const clang::Token &includeTok,
                                   clang::StringRef fileName,
                                   bool isAngled,
                                   clang::CharSourceRange filenameRange,
                                   const clang::FileEntry *file,
                                   clang::StringRef searchPath,
                                   clang::StringRef relativePath,
                                   const clang::Module *imported,
                                   clang::SrcMgr::CharacteristicKind filetype){
    master.InclusionDirective(hashLoc, includeTok, fileName, isAngled, filenameRange, file, searchPath, relativePath, imported, filetype);
}

class IncludeFinder : private clang::PPCallbacks
{
public:
    explicit inline IncludeFinder(const clang::CompilerInstance &compiler);

public:
    inline clang::PPCallbacks * createPreprocessorCallbacks();
    virtual inline void InclusionDirective(clang::SourceLocation hashLoc,
                                           const clang::Token &includeTok,
                                           clang::StringRef fileName,
                                           bool isAngled,
                                           clang::CharSourceRange filenameRange,
                                           const clang::FileEntry *file,
                                           clang::StringRef searchPath,
                                           clang::StringRef relativePath,
                                           const clang::Module *imported,
                                           clang::SrcMgr::CharacteristicKind filetype);

private:
    const clang::CompilerInstance &compiler;
    std::string name;
    LangOptions lopt;
    Rewriter rewriter_;
};

inline
IncludeFinder::IncludeFinder(const clang::CompilerInstance &compiler) : compiler(compiler){
    const clang::FileID mainFile = compiler.getSourceManager().getMainFileID();
    name = compiler.getSourceManager().getFileEntryForID(mainFile)->getName();
   // outs() << "/"+ name + "/";
}

inline clang::PPCallbacks *IncludeFinder::createPreprocessorCallbacks()
{
    return new CallbacksProxy(*this);
}
         
inline void IncludeFinder::InclusionDirective(clang::SourceLocation hashLoc,
                                  const clang::Token &includeTok,
                                  clang::StringRef fileName,
                                  bool isAngled,
                                  clang::CharSourceRange filenameRange,
                                  const clang::FileEntry *file,
                                  clang::StringRef searchPath,
                                  clang::StringRef relativePath,
                                  const clang::Module *imported, 
                                  clang::SrcMgr::CharacteristicKind filetype){ //#include
    
    clang::SourceManager &sm = compiler.getSourceManager();
    if (sm.isInMainFile(hashLoc)) {   
        SourceLocation include_end = clang::Lexer::getLocForEndOfToken(filenameRange.getEnd(), 0, sm, lopt);
        SourceRange range(hashLoc, include_end);
       // const std::string vardecl_stmt = Lexer::getSourceText(CharSourceRange(range, true), sm, lopt).str();
       // outs() << vardecl_stmt;
       // cout<<endl;
        clang::Rewriter::RewriteOptions RO;
        RO.IncludeInsertsAtBeginOfRange = true;
        RO.IncludeInsertsAtEndOfRange = true;
        RO.RemoveLineIfEmpty = false;
        CharSourceRange CSR1 = CharSourceRange(range, true);
        rewriter_.RemoveText(CSR1, RO);
    }
 }

}

void IncludeFinderAction::ExecuteAction(){
    
    IncludeFinder includeFinder(getCompilerInstance());
    getCompilerInstance().getPreprocessor().addPPCallbacks(std::unique_ptr<clang::PPCallbacks> (includeFinder.createPreprocessorCallbacks()));
    clang::PreprocessOnlyAction::ExecuteAction();
}

static llvm::cl::OptionCategory toolCategory("self-inc-first options");
static llvm::cl::extrahelp commonHelp(CommonOptionsParser::HelpMessage);

int main(int argc, const char *argv[])
{
    CommonOptionsParser optionsParser(argc, argv, toolCategory);
    ClangTool tool(optionsParser.getCompilations(), optionsParser.getSourcePathList());

    class : public clang::DiagnosticConsumer
    {
    public:
        virtual bool
        IncludeInDiagnosticCounts() const
        {
            return false;
        }
    } diagConsumer;
    tool.setDiagnosticConsumer(&diagConsumer);

    return tool.run(newFrontendActionFactory<IncludeFinderAction>().get());
}
