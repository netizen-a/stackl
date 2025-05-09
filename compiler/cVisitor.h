#pragma once

class cAddressExpr;
class cArrayRef;
class cArrayType;
class cAsmNode;
class cAssignExpr;
class cAstNode;
class cBaseDeclNode;
class cBinaryExpr;
class cCastExpr;
class cDecl;
class cDeclsList;
class cDoWhileStmt;
class cExpr;
class cExprStmt;
class cForStmt;
class cFuncCall;
class cFuncDecl;
class cGotoStmt;
class cIfStmt;
class cIntExpr;
class cLabeledStmt;
class cNopStmt;
class cParams;
class cPlainVarRef;
class cPointerDeref;
class cPointerType;
class cPostfixExpr;
class cPragma;
class cPrefixExpr;
class cReturnStmt;
class cShortCircuitExpr;
class cSizeofExpr;
class cStmt;
class cStmtsList;
class cStringLit;
class cStructDeref;
class cStructRef;
class cStructType;
class cSymbol;
class cTypeDecl;
class cTypedef;
class cUnaryExpr;
class cVarDecl;
class cVarRef;
class cWhileStmt;
class cTernaryExpr;

class cVisitor
{
    public:
        cVisitor() {}

        virtual void VisitAllNodes(cAstNode *node) = 0;

        virtual void Visit(cAddressExpr *node);
        virtual void Visit(cArrayRef *node);
        virtual void Visit(cArrayType *node);
        virtual void Visit(cAsmNode *node);
        virtual void Visit(cAssignExpr *node);
        virtual void Visit(cAstNode *node);
        virtual void Visit(cBaseDeclNode *node);
        virtual void Visit(cBinaryExpr *node);
        virtual void Visit(cCastExpr *node);
        virtual void Visit(cDecl *node);
        virtual void Visit(cDeclsList *node);
        virtual void Visit(cDoWhileStmt *node);
        virtual void Visit(cExpr *node);
        virtual void Visit(cExprStmt *node);
        virtual void Visit(cForStmt *node);
        virtual void Visit(cFuncCall *node);
        virtual void Visit(cFuncDecl *node);
        virtual void Visit(cGotoStmt *node);
        virtual void Visit(cIfStmt *node);
        virtual void Visit(cIntExpr *node);
        virtual void Visit(cLabeledStmt *node);
        virtual void Visit(cNopStmt *node);
        virtual void Visit(cParams *node);
        virtual void Visit(cPlainVarRef *node);
        virtual void Visit(cPointerDeref *node);
        virtual void Visit(cPointerType *node);
        virtual void Visit(cPostfixExpr *node);
        virtual void Visit(cPragma *node);
        virtual void Visit(cPrefixExpr *node);
        virtual void Visit(cReturnStmt *node);
        virtual void Visit(cShortCircuitExpr *node);
        virtual void Visit(cSizeofExpr *node);
        virtual void Visit(cStmt *node);
        virtual void Visit(cStmtsList *node);
        virtual void Visit(cStringLit *node);
        virtual void Visit(cStructDeref *node);
        virtual void Visit(cStructRef *node);
        virtual void Visit(cStructType *node);
        virtual void Visit(cSymbol *node);
        virtual void Visit(cTypeDecl *node);
        virtual void Visit(cTypedef *node);
        virtual void Visit(cUnaryExpr *node);
        virtual void Visit(cVarDecl *node);
        virtual void Visit(cVarRef *node);
        virtual void Visit(cWhileStmt *node);
        virtual void Visit(cTernaryExpr *node);
    protected:
        virtual void VisitAllChildren(cAstNode *node);
};
