// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.

#include "GKPythonVisitor.h"

// Python

#if WITH_PYTHON
THIRD_PARTY_INCLUDES_START
PRAGMA_DISABLE_REGISTER_WARNINGS
extern "C"
{
#    include "ast.h"
#    include "structmember.h"
#    define Py_BUILD_CORE
#    include "internal/pegen_interface.h"
}
PRAGMA_ENABLE_REGISTER_WARNINGS
THIRD_PARTY_INCLUDES_END
#endif // WITH_PYTHON

#define CALL(FUNC, TYPE, ARG) \
    if (!FUNC((ARG), depth))  \
        return 0;

#define CALL_OPT(FUNC, TYPE, ARG)             \
    if ((ARG) != NULL && !FUNC((ARG), depth)) \
        return 0;

#define CALL_SEQ(FUNC, TYPE, ARG)                           \
    {                                                       \
        int       i;                                        \
        asdl_seq *seq = (ARG); /* avoid variable capture */ \
        for (i = 0; i < asdl_seq_LEN(seq); i++)             \
        {                                                   \
            TYPE elt = (TYPE)asdl_seq_GET(seq, i);          \
            if (elt != NULL && !FUNC(elt, depth))           \
                return 0;                                   \
        }                                                   \
    }

#define CALL_INT_SEQ(FUNC, TYPE, ARG)                           \
    {                                                           \
        int           i;                                        \
        asdl_int_seq *seq = (ARG); /* avoid variable capture */ \
        for (i = 0; i < asdl_seq_LEN(seq); i++)                 \
        {                                                       \
            TYPE elt = (TYPE)asdl_seq_GET(seq, i);              \
            if (!FUNC(elt, depth))                              \
                return 0;                                       \
        }                                                       \
    }

int visit_mod(PythonASTVisitor *visitor, mod_ty node_, int depth);
int visit_stmt(PythonASTVisitor *visitor, stmt_ty node_, int depth);
int visit_expr(PythonASTVisitor *visitor, expr_ty node_, int depth);
int visit_arguments(PythonASTVisitor *visitor, arguments_ty node_, int depth);
int visit_comprehension(PythonASTVisitor *visitor, comprehension_ty node_, int depth);
int visit_keyword(PythonASTVisitor *visitor, keyword_ty node_, int depth);
int visit_arg(PythonASTVisitor *visitor, arg_ty node_, int depth);
int visit_withitem(PythonASTVisitor *visitor, withitem_ty node_, int depth);
int visit_excepthandler(PythonASTVisitor *visitor, excepthandler_ty node_, int depth);
int visit_body(PythonASTVisitor *visitor, asdl_seq *stmts, int depth);

PythonASTVisitor::PythonASTVisitor() { Py_Initialize(); }

PythonASTVisitor::~PythonASTVisitor() { Py_Finalize(); }

void PythonASTVisitor::ParsePythoCode(const char *str)
{
    PyArena *arena;

    arena = PyArena_New();
    if (arena == NULL)
    {
        return;
    }

    PyCompilerFlags *flags = nullptr;

    mod_ty module = PyPegen_ASTFromString(str, "<string>", Py_file_input, flags, arena);

    visit(module, 0);

    PyArena_Free(arena);
}

int PythonASTVisitor::visit(mod_ty node_, int depth) { return visit_mod(this, node_, depth); }
int PythonASTVisitor::visit(expr_ty node_, int depth) { return visit_expr(this, node_, depth); }
int PythonASTVisitor::visit(stmt_ty node_, int depth) { return visit_stmt(this, node_, depth); }
int PythonASTVisitor::visit(excepthandler_ty node_, int depth) { return visit_excepthandler(this, node_, depth); }
int PythonASTVisitor::visit(withitem_ty node_, int depth) { return visit_withitem(this, node_, depth); }
int PythonASTVisitor::visit(arg_ty node_, int depth) { return visit_arg(this, node_, depth); }
int PythonASTVisitor::visit(arguments_ty node_, int depth) { return visit_arguments(this, node_, depth); }
int PythonASTVisitor::visit(comprehension_ty node_, int depth) { return visit_comprehension(this, node_, depth); }
int PythonASTVisitor::visit(keyword_ty node_, int depth) { return visit_keyword(this, node_, depth); }
int PythonASTVisitor::visit(asdl_seq *node_, int depth) { return visit_body(this, node_, depth); }

int PythonASTVisitor::mod_module(mod_ty node_, int depth)
{
    CALL(visit, asdl_seq, node_->v.Module.body);
    return 1;
}
int PythonASTVisitor::mod_interactive(mod_ty node_, int depth)
{
    CALL_SEQ(visit, stmt_ty, node_->v.Interactive.body);
    return 1;
}
int PythonASTVisitor::mod_expression(mod_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->v.Expression.body);
    return 1;
}

int PythonASTVisitor::boolop(expr_ty node_, int depth)
{
    CALL_SEQ(visit, expr_ty, node_->v.BoolOp.values);
    return 1;
}
int PythonASTVisitor::binop(expr_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->v.BinOp.left);
    CALL(visit, expr_ty, node_->v.BinOp.right);
    return 1;
}
int PythonASTVisitor::unaryop(expr_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->v.UnaryOp.operand);
    return 1;
}
int PythonASTVisitor::lambda(expr_ty node_, int depth)
{
    CALL(visit, arguments_ty, node_->v.Lambda.args);
    CALL(visit, expr_ty, node_->v.Lambda.body);
    return 1;
}
int PythonASTVisitor::ifexp(expr_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->v.IfExp.test);
    CALL(visit, expr_ty, node_->v.IfExp.body);
    CALL(visit, expr_ty, node_->v.IfExp.orelse);
    return 1;
}
int PythonASTVisitor::dict(expr_ty node_, int depth)
{
    CALL_SEQ(visit, expr_ty, node_->v.Dict.keys);
    CALL_SEQ(visit, expr_ty, node_->v.Dict.values);
    return 1;
}
int PythonASTVisitor::set(expr_ty node_, int depth)
{
    CALL_SEQ(visit, expr_ty, node_->v.Set.elts);
    return 1;
}
int PythonASTVisitor::listcomp(expr_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->v.ListComp.elt);
    CALL_SEQ(visit, comprehension_ty, node_->v.ListComp.generators);
    return 1;
}
int PythonASTVisitor::setcomp(expr_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->v.SetComp.elt);
    CALL_SEQ(visit, comprehension_ty, node_->v.SetComp.generators);
    return 1;
}
int PythonASTVisitor::dictcomp(expr_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->v.DictComp.key);
    CALL(visit, expr_ty, node_->v.DictComp.value);
    CALL_SEQ(visit, comprehension_ty, node_->v.DictComp.generators);
    return 1;
}
int PythonASTVisitor::generatorexp(expr_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->v.GeneratorExp.elt);
    CALL_SEQ(visit, comprehension_ty, node_->v.GeneratorExp.generators);
    return 1;
}
int PythonASTVisitor::await(expr_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->v.Await.value);
    return 1;
}
int PythonASTVisitor::yield(expr_ty node_, int depth)
{
    CALL_OPT(visit, expr_ty, node_->v.Yield.value);
    return 1;
}
int PythonASTVisitor::yieldfrom(expr_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->v.YieldFrom.value);
    return 1;
}
int PythonASTVisitor::compare(expr_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->v.Compare.left);
    CALL_SEQ(visit, expr_ty, node_->v.Compare.comparators);
    return 1;
}
int PythonASTVisitor::call(expr_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->v.Call.func);
    CALL_SEQ(visit, expr_ty, node_->v.Call.args);
    CALL_SEQ(visit, keyword_ty, node_->v.Call.keywords);
    return 1;
}
int PythonASTVisitor::formatted(expr_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->v.FormattedValue.value);
    CALL_OPT(visit, expr_ty, node_->v.FormattedValue.format_spec);
    return 1;
}
int PythonASTVisitor::joinedstr(expr_ty node_, int depth)
{
    CALL_SEQ(visit, expr_ty, node_->v.JoinedStr.values);
    return 1;
}
int PythonASTVisitor::attribute(expr_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->v.Attribute.value);
    return 1;
}
int PythonASTVisitor::subscript(expr_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->v.Subscript.value);
    CALL(visit, expr_ty, node_->v.Subscript.slice);
    CALL(visit, expr_ty, node_);
    return 1;
}
int PythonASTVisitor::starred(expr_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->v.Starred.value);
    return 1;
}
int PythonASTVisitor::slice(expr_ty node_, int depth)
{
    CALL_OPT(visit, expr_ty, node_->v.Slice.lower);
    CALL_OPT(visit, expr_ty, node_->v.Slice.upper);
    CALL_OPT(visit, expr_ty, node_->v.Slice.step);
    return 1;
}
int PythonASTVisitor::list(expr_ty node_, int depth)
{
    CALL_SEQ(visit, expr_ty, node_->v.List.elts);
    return 1;
}
int PythonASTVisitor::tuple(expr_ty node_, int depth)
{
    CALL_SEQ(visit, expr_ty, node_->v.Tuple.elts);
    return 1;
}
int PythonASTVisitor::name(expr_ty node_, int depth) { return 1; }

int PythonASTVisitor::functiondef(stmt_ty node_, int depth)
{
    CALL(visit, arguments_ty, node_->v.FunctionDef.args);
    CALL(visit, asdl_seq, node_->v.FunctionDef.body);
    CALL_SEQ(visit, expr_ty, node_->v.FunctionDef.decorator_list);

    // if (!(state->ff_features & CO_FUTURE_ANNOTATIONS))
    {
        CALL_OPT(visit, expr_ty, node_->v.FunctionDef.returns);
    }
    return 1;
}
int PythonASTVisitor::asyncfunctiondef(stmt_ty node_, int depth)
{
    CALL(visit, arguments_ty, node_->v.AsyncFunctionDef.args);
    CALL(visit, asdl_seq, node_->v.AsyncFunctionDef.body);
    CALL_SEQ(visit, expr_ty, node_->v.AsyncFunctionDef.decorator_list);

    // if (!(state->ff_features & CO_FUTURE_ANNOTATIONS))
    {
        CALL_OPT(visit, expr_ty, node_->v.AsyncFunctionDef.returns);
    }
    return 1;
}
int PythonASTVisitor::classdef(stmt_ty node_, int depth)
{
    CALL_SEQ(visit, expr_ty, node_->v.ClassDef.bases);
    CALL_SEQ(visit, keyword_ty, node_->v.ClassDef.keywords);
    CALL(visit, asdl_seq, node_->v.ClassDef.body);
    CALL_SEQ(visit, expr_ty, node_->v.ClassDef.decorator_list);
    return 1;
}
int PythonASTVisitor::returnstmt(stmt_ty node_, int depth)
{
    CALL_OPT(visit, expr_ty, node_->v.Return.value);
    return 1;
}
int PythonASTVisitor::deletestmt(stmt_ty node_, int depth)
{
    CALL_SEQ(visit, expr_ty, node_->v.Delete.targets);
    return 1;
}
int PythonASTVisitor::assign(stmt_ty node_, int depth)
{
    CALL_SEQ(visit, expr_ty, node_->v.Assign.targets);
    CALL(visit, expr_ty, node_->v.Assign.value);
    return 1;
}
int PythonASTVisitor::augassign(stmt_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->v.AugAssign.target);
    CALL(visit, expr_ty, node_->v.AugAssign.value);
    return 1;
}
int PythonASTVisitor::annassign(stmt_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->v.AnnAssign.target);
    // if (!(state->ff_features & CO_FUTURE_ANNOTATIONS))
    {
        CALL(visit, expr_ty, node_->v.AnnAssign.annotation);
    }
    CALL_OPT(visit, expr_ty, node_->v.AnnAssign.value);
    return 1;
}
int PythonASTVisitor::forstmt(stmt_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->v.For.target);
    CALL(visit, expr_ty, node_->v.For.iter);
    CALL_SEQ(visit, stmt_ty, node_->v.For.body);
    CALL_SEQ(visit, stmt_ty, node_->v.For.orelse);
    return 1;
}
int PythonASTVisitor::asyncfor(stmt_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->v.AsyncFor.target);
    CALL(visit, expr_ty, node_->v.AsyncFor.iter);
    CALL_SEQ(visit, stmt_ty, node_->v.AsyncFor.body);
    CALL_SEQ(visit, stmt_ty, node_->v.AsyncFor.orelse);
    return 1;
}
int PythonASTVisitor::whilestmt(stmt_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->v.While.test);
    CALL_SEQ(visit, stmt_ty, node_->v.While.body);
    CALL_SEQ(visit, stmt_ty, node_->v.While.orelse);
    return 1;
}
int PythonASTVisitor::ifstmt(stmt_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->v.If.test);
    CALL_SEQ(visit, stmt_ty, node_->v.If.body);
    CALL_SEQ(visit, stmt_ty, node_->v.If.orelse);
    return 1;
}
int PythonASTVisitor::with(stmt_ty node_, int depth)
{
    CALL_SEQ(visit, withitem_ty, node_->v.With.items);
    CALL_SEQ(visit, stmt_ty, node_->v.With.body);
    return 1;
}
int PythonASTVisitor::asyncwith(stmt_ty node_, int depth)
{
    CALL_SEQ(visit, withitem_ty, node_->v.AsyncWith.items);
    CALL_SEQ(visit, stmt_ty, node_->v.AsyncWith.body);
    return 1;
}
int PythonASTVisitor::raise(stmt_ty node_, int depth)
{
    CALL_OPT(visit, expr_ty, node_->v.Raise.exc);
    CALL_OPT(visit, expr_ty, node_->v.Raise.cause);
    return 1;
}
int PythonASTVisitor::trystmt(stmt_ty node_, int depth)
{
    CALL_SEQ(visit, stmt_ty, node_->v.Try.body);
    CALL_SEQ(visit, excepthandler_ty, node_->v.Try.handlers);
    CALL_SEQ(visit, stmt_ty, node_->v.Try.orelse);
    CALL_SEQ(visit, stmt_ty, node_->v.Try.finalbody);
    return 1;
}
int PythonASTVisitor::assertstmt(stmt_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->v.Assert.test);
    CALL_OPT(visit, expr_ty, node_->v.Assert.msg);
    return 1;
}
int PythonASTVisitor::exprstmt(stmt_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->v.Expr.value);
    return 1;
}

int PythonASTVisitor::excepthandler(excepthandler_ty node_, int depth)
{
    CALL_OPT(visit, expr_ty, node_->v.ExceptHandler.type);
    CALL_SEQ(visit, stmt_ty, node_->v.ExceptHandler.body);
    return 1;
}
int PythonASTVisitor::with_item(withitem_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->context_expr);
    CALL_OPT(visit, expr_ty, node_->optional_vars);
    return 1;
}
int PythonASTVisitor::_arg(arg_ty node_, int depth)
{
    // if (!(state->ff_features & CO_FUTURE_ANNOTATIONS))
    {
        CALL_OPT(visit, expr_ty, node_->annotation);
    }
    return 1;
}
int PythonASTVisitor::_arguments(arguments_ty node_, int depth)
{
    CALL_SEQ(visit, arg_ty, node_->posonlyargs);
    CALL_SEQ(visit, arg_ty, node_->args);
    CALL_OPT(visit, arg_ty, node_->vararg);
    CALL_SEQ(visit, arg_ty, node_->kwonlyargs);
    CALL_SEQ(visit, expr_ty, node_->kw_defaults);
    CALL_OPT(visit, arg_ty, node_->kwarg);
    CALL_SEQ(visit, expr_ty, node_->defaults);
    return 1;
}
int PythonASTVisitor::_comprehension(comprehension_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->target);
    CALL(visit, expr_ty, node_->iter);
    CALL_SEQ(visit, expr_ty, node_->ifs);
    return 1;
}
int PythonASTVisitor::_keyword(keyword_ty node_, int depth)
{
    CALL(visit, expr_ty, node_->value);
    return 1;
}
int PythonASTVisitor::body(asdl_seq *stmts, int depth)
{
    CALL_SEQ(visit, stmt_ty, stmts);
    return 1;
}

#undef CALL
#undef CALL_OPT
#undef CALL_SEQ
#undef CALL_INT_SEQ

//
// Dispatch
//

#define CALL(FUNC, TYPE, ARG)         \
    if (!FUNC(visitor, (ARG), depth)) \
        return 0;

#define CALL_OPT(FUNC, TYPE, ARG)                      \
    if ((ARG) != NULL && !FUNC(visitor, (ARG), depth)) \
        return 0;

#define CALL_SEQ(FUNC, TYPE, ARG)                           \
    {                                                       \
        int       i;                                        \
        asdl_seq *seq = (ARG); /* avoid variable capture */ \
        for (i = 0; i < asdl_seq_LEN(seq); i++)             \
        {                                                   \
            TYPE elt = (TYPE)asdl_seq_GET(seq, i);          \
            if (elt != NULL && !FUNC(visitor, elt, depth))  \
                return 0;                                   \
        }                                                   \
    }

#define CALL_INT_SEQ(FUNC, TYPE, ARG)                           \
    {                                                           \
        int           i;                                        \
        asdl_int_seq *seq = (ARG); /* avoid variable capture */ \
        for (i = 0; i < asdl_seq_LEN(seq); i++)                 \
        {                                                       \
            TYPE elt = (TYPE)asdl_seq_GET(seq, i);              \
            if (!FUNC(visitor, elt, depth))                     \
                return 0;                                       \
        }                                                       \
    }

int visit_body(PythonASTVisitor *visitor, asdl_seq *stmts, int depth) { return visitor->body(stmts, depth); }

int visit_keyword(PythonASTVisitor *visitor, keyword_ty node_, int depth) { return visitor->_keyword(node_, depth); }

int visit_comprehension(PythonASTVisitor *visitor, comprehension_ty node_, int depth)
{

    return visitor->_comprehension(node_, depth);
}

int visit_arguments(PythonASTVisitor *visitor, arguments_ty node_, int depth)
{
    return visitor->_arguments(node_, depth);
}

int visit_arg(PythonASTVisitor *visitor, arg_ty node_, int depth) { return visitor->_arg(node_, depth); }
int visit_excepthandler(PythonASTVisitor *visitor, excepthandler_ty node_, int depth)
{
    switch (node_->kind)
    {
    case ExceptHandler_kind:
        return visitor->excepthandler(node_, depth);
    }
    return 1;
}

int visit_withitem(PythonASTVisitor *visitor, withitem_ty node_, int depth) { return visitor->with_item(node_, depth); }

int visit_stmt(PythonASTVisitor *visitor, stmt_ty node_, int depth)
{
    switch (node_->kind)
    {
    case FunctionDef_kind:
        return visitor->functiondef(node_, depth + 1);
    case AsyncFunctionDef_kind:
        return visitor->asyncfunctiondef(node_, depth + 1);
    case ClassDef_kind:
        return visitor->classdef(node_, depth + 1);
    case Return_kind:
        return visitor->returnstmt(node_, depth + 1);
    case Delete_kind:
        return visitor->deletestmt(node_, depth + 1);
    case Assign_kind:
        return visitor->assign(node_, depth + 1);
    case AugAssign_kind:
        return visitor->augassign(node_, depth + 1);
    case AnnAssign_kind:
        return visitor->annassign(node_, depth + 1);
    case For_kind:
        return visitor->forstmt(node_, depth + 1);
    case AsyncFor_kind:
        return visitor->asyncfor(node_, depth + 1);
    case While_kind:
        return visitor->whilestmt(node_, depth + 1);
    case If_kind:
        return visitor->ifstmt(node_, depth + 1);
    case With_kind:
        return visitor->with(node_, depth + 1);
    case AsyncWith_kind:
        return visitor->asyncwith(node_, depth + 1);
    case Raise_kind:
        return visitor->raise(node_, depth + 1);
    case Try_kind:
        return visitor->trystmt(node_, depth + 1);
    case Assert_kind:
        return visitor->assertstmt(node_, depth + 1);
    case Expr_kind:
        return visitor->exprstmt(node_, depth + 1);
    }
    return 1;
}

int visit_mod(PythonASTVisitor *visitor, mod_ty node_, int depth)
{
    switch (node_->kind)
    {
    case Module_kind:
        return visitor->mod_module(node_, depth + 1);
    case Interactive_kind:
        return visitor->mod_interactive(node_, depth + 1);
    case Expression_kind:
        return visitor->mod_expression(node_, depth + 1);

        break;
    default:
        break;
    }
    return 1;
}

int visit_expr(PythonASTVisitor *visitor, expr_ty node_, int depth)
{
    switch (node_->kind)
    {
    case BoolOp_kind:
        return visitor->boolop(node_, depth + 1);

    case BinOp_kind:
        return visitor->binop(node_, depth + 1);

    case UnaryOp_kind:
        return visitor->unaryop(node_, depth + 1);

    case Lambda_kind:
        return visitor->lambda(node_, depth + 1);

    case IfExp_kind:
        return visitor->ifexp(node_, depth + 1);

    case Dict_kind:
        return visitor->dict(node_, depth + 1);

    case Set_kind:
        return visitor->set(node_, depth + 1);

    case ListComp_kind:
        return visitor->list(node_, depth + 1);

    case SetComp_kind:
        return visitor->setcomp(node_, depth + 1);

    case DictComp_kind:
        return visitor->dictcomp(node_, depth + 1);

    case GeneratorExp_kind:
        return visitor->generatorexp(node_, depth + 1);

    case Await_kind:
        return visitor->await(node_, depth + 1);

    case Yield_kind:
        return visitor->yield(node_, depth + 1);

    case YieldFrom_kind:
        return visitor->yieldfrom(node_, depth + 1);

    case Compare_kind:
        return visitor->compare(node_, depth + 1);

    case Call_kind:
        return visitor->call(node_, depth + 1);

    case FormattedValue_kind:
        return visitor->formatted(node_, depth + 1);

    case JoinedStr_kind:
        return visitor->joinedstr(node_, depth + 1);

    case Attribute_kind:
        return visitor->attribute(node_, depth + 1);

    case Subscript_kind:
        return visitor->subscript(node_, depth + 1);

    case Starred_kind:
        return visitor->starred(node_, depth + 1);

    case Slice_kind:
        return visitor->slice(node_, depth + 1);

    case List_kind:
        return visitor->list(node_, depth + 1);

    case Tuple_kind:
        return visitor->tuple(node_, depth + 1);

    case Name_kind:
        return visitor->name(node_, depth + 1);
    }
    return 1;
}

#undef CALL
#undef CALL_OPT
#undef CALL_SEQ
#undef CALL_INT_SEQ
