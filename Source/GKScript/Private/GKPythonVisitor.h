
#pragma once

#if WITH_PYTHON
THIRD_PARTY_INCLUDES_START
PRAGMA_DISABLE_REGISTER_WARNINGS
extern "C" {
#    include "Python.h"
    // Second
#    include "Python-ast.h"

}
PRAGMA_ENABLE_REGISTER_WARNINGS
THIRD_PARTY_INCLUDES_END

DECLARE_STATS_GROUP(TEXT("Python"), STATGROUP_Python, STATCAT_Advanced);
#endif // WITH_PYTHON


// Python AST visitor
// Default Implementation traverse everything
class PythonASTVisitor
{
    public:
    PythonASTVisitor();

    virtual ~PythonASTVisitor();

    void ParsePythoCode(const char *str);

    // Module
    virtual int mod_module(mod_ty node_, int depth);
    virtual int mod_interactive(mod_ty node_, int depth);
    virtual int mod_expression(mod_ty node_, int depth);

    virtual int visit(mod_ty node_, int depth);
    virtual int visit(expr_ty node_, int depth);
    virtual int visit(stmt_ty node_, int depth);
    virtual int visit(excepthandler_ty node_, int depth);
    virtual int visit(withitem_ty node_, int depth);
    virtual int visit(arg_ty node_, int depth);
    virtual int visit(arguments_ty node_, int depth);
    virtual int visit(comprehension_ty node_, int depth);
    virtual int visit(keyword_ty node_, int depth);
    virtual int visit(asdl_seq* node_, int depth);
    
    // Expression
    virtual int boolop(expr_ty node_, int depth);
    virtual int binop(expr_ty node_, int depth);
    virtual int unaryop(expr_ty node_, int depth);
    virtual int lambda(expr_ty node_, int depth);
    virtual int ifexp(expr_ty node_, int depth);
    virtual int dict(expr_ty node_, int depth);
    virtual int set(expr_ty node_, int depth);
    virtual int listcomp(expr_ty node_, int depth);
    virtual int setcomp(expr_ty node_, int depth);
    virtual int dictcomp(expr_ty node_, int depth);
    virtual int generatorexp(expr_ty node_, int depth);
    virtual int await(expr_ty node_, int depth);
    virtual int yield(expr_ty node_, int depth);
    virtual int yieldfrom(expr_ty node_, int depth);
    virtual int compare(expr_ty node_, int depth);
    virtual int call(expr_ty node_, int depth);
    virtual int formatted(expr_ty node_, int depth);
    virtual int joinedstr(expr_ty node_, int depth);
    virtual int attribute(expr_ty node_, int depth);
    virtual int subscript(expr_ty node_, int depth);
    virtual int starred(expr_ty node_, int depth);
    virtual int slice(expr_ty node_, int depth);
    virtual int list(expr_ty node_, int depth);
    virtual int tuple(expr_ty node_, int depth);
    virtual int name(expr_ty node_, int depth);

    // Statement
    virtual int functiondef(stmt_ty node_, int depth);
    virtual int asyncfunctiondef(stmt_ty node_, int depth);
    virtual int classdef(stmt_ty node_, int depth);
    virtual int returnstmt(stmt_ty node_, int depth);
    virtual int deletestmt(stmt_ty node_, int depth);
    virtual int assign(stmt_ty node_, int depth);
    virtual int augassign(stmt_ty node_, int depth);
    virtual int annassign(stmt_ty node_, int depth);
    virtual int forstmt(stmt_ty node_, int depth);
    virtual int asyncfor(stmt_ty node_, int depth);
    virtual int whilestmt(stmt_ty node_, int depth);
    virtual int ifstmt(stmt_ty node_, int depth);
    virtual int with(stmt_ty node_, int depth);
    virtual int asyncwith(stmt_ty node_, int depth);
    virtual int raise(stmt_ty node_, int depth);
    virtual int trystmt(stmt_ty node_, int depth);
    virtual int assertstmt(stmt_ty node_, int depth);
    virtual int exprstmt(stmt_ty node_, int depth);

    // Others
    virtual int excepthandler(excepthandler_ty node_, int depth);
    virtual int with_item(withitem_ty node_, int depth);
    virtual int _arg(arg_ty node_, int depth);
    virtual int _arguments(arguments_ty node_, int depth);
    virtual int _comprehension(comprehension_ty node_, int depth);
    virtual int _keyword(keyword_ty node_, int depth);
    virtual int body(asdl_seq* node_, int depth);
};
