#include <iostream>
#include <fstream>
#include <string>

#include "../interp/machine_def.h"

#include "lex.h"
#include "cCodeGen.h"
#include "cGenAddr.h"

using std::string;

const int cCodeGen::STACKL_WORD_SIZE = WORD_SIZE;

cCodeGen::cCodeGen(string filename) : cVisitor()
{
    m_Output.open(filename);
    if (!m_Output.is_open())
    {
        fatal_error("Unable to open output file");
    }

    m_Filename = filename;
    m_GenAddr = new cGenAddr(this);

    // Leave room for ISR address (if any)
    EmitInst(".data",  "interrupt");
    EmitInst(".data",  "systrap");

    EmitInst("JUMP", "startup__");
    EmitInst("POP");            // need to throw away the return value
    EmitInst("HALT");
}

cCodeGen::~cCodeGen()
{
    m_Output.close();

    string makebin("slasm ");
    makebin += m_Filename;

    int result = system(makebin.c_str());

    if (result < 0) fatal_error("Error creating binary output");
}

void cCodeGen::VisitAllNodes(cAstNode *node) { node->Visit(this); }

void cCodeGen::Visit(cAddressExpr *node)
{
    cVarRef *var = node->GetVar();
    if (var == NULL)
        fatal_error("address of without underlying cVarRef");

    var->Visit(m_GenAddr);
}

void cCodeGen::Visit(cArrayRef *node)
{
    node->Visit(m_GenAddr);
    if (node->GetType()->ElementSize() == 1)
        EmitInst("PUSHCVARIND");
    else
        EmitInst("PUSHVARIND");
}

void cCodeGen::Visit(cArrayType *node)
{ }     // Don't gen code for array size

void cCodeGen::Visit(cAsmNode *node)
{
    if (node->GetParams() != NULL) node->GetParams()->Visit(this);
    if (node->UsesTwoArgs()) 
        EmitInst(node->GetOp1String(), node->GetOp2());
    else
        EmitInst(node->GetOp1String());
}

void cCodeGen::Visit(cAssignExpr *node)
{
    node->GetExpr()->Visit(this);

    // Need to dup the result in case the assign is treated as an expr
    EmitInst("DUP");
    node->GetVar()->Visit(m_GenAddr);
    if (node->GetVar()->IsArrayRef() && 
        node->GetVar()->GetType()->ElementSize() == 1)
    {
        EmitInst("POPCVARIND");
    }
    else if (node->GetVar()->GetType()->Size() == 1)
        EmitInst("POPCVARIND");
    else
        EmitInst("POPVARIND");
}

//void cCodeGen::Visit(cAstNode *node)            { VisitAllChildren(node); }
//void cCodeGen::Visit(cBaseDeclNode *node)       { VisitAllChildren(node); }
void cCodeGen::Visit(cBinaryExpr *node)
{
    node->GetLeft()->Visit(this);
    node->GetRight()->Visit(this);
    EmitInst(node->OpAsString());
}
//void cCodeGen::Visit(cDecl *node)               { VisitAllChildren(node); }
//void cCodeGen::Visit(cDeclsList *node)          { VisitAllChildren(node); }
//void cCodeGen::Visit(cExpr *node)               { VisitAllChildren(node); }

void cCodeGen::Visit(cExprStmt *node)
{
    node->GetExpr()->Visit(this);

    // remove the result from the stack
    EmitInst("POP");
}

void cCodeGen::Visit(cForStmt *node)
{
    std::string start_loop = GenerateLabel();
    std::string end_loop = GenerateLabel();

    node->GetInit()->Visit(this);
    EmitInst("POP");            // need to handle VOID
    EmitLabel(start_loop);
    node->GetExpr()->Visit(this);
    EmitInst("JUMPE", end_loop);
    node->GetStmt()->Visit(this);
    node->GetUpdate()->Visit(this);
    EmitInst("POP");            // need to handle VOID
    EmitInst("JUMP", start_loop);
    EmitLabel(end_loop);
}

void cCodeGen::Visit(cFuncCall *node)
{
    if (node->GetParams() != NULL) node->GetParams()->Visit(this);

    EmitInst("CALL", node->GetFuncName());

    // Need to pop the args off the stack without affecting the return val
    if (node->GetParams() != NULL)
    {
        for (int ii=0; ii<node->GetParams()->Size()/WORD_SIZE; ii++)
        {
            EmitInst("SWAP");
            EmitInst("POP");
        }
    }
}

void cCodeGen::Visit(cFuncDecl *node)
{
    if (node->IsDefinition())
    {
        EmitComment(node->GetName()->Name() + "\n");
        EmitLabel(node->GetName()->Name());
        int adj_size = (node->DeclsSize() / WORD_SIZE * WORD_SIZE) + WORD_SIZE;
        if (node->DeclsSize() != 0)
        {
            EmitInst("ADJSP", adj_size);
        }

        node->GetStmts()->Visit(this);

        // Force return statement
        cReturnStmt *ret = new cReturnStmt(new cIntExpr(0));
        ret->Visit(this);
    }
}

void cCodeGen::Visit(cIfStmt *node)
{
    std::string if_label = GenerateLabel();
    node->GetCond()->Visit(this);
    EmitInst("JUMPE", if_label);
    if (node->GetIfStmt() != NULL) node->GetIfStmt()->Visit(this);

    if (node->GetElseStmt() != NULL)
    {
        std::string else_label = GenerateLabel();
        EmitInst("JUMP", else_label);
        EmitLabel(if_label);
        node->GetElseStmt()->Visit(this);
        EmitLabel(else_label);
    }
    else
    {
        EmitLabel(if_label);
    }
}

void cCodeGen::Visit(cIntExpr *node)
{
    EmitInst("PUSH", node->ConstValue());
}

//void cCodeGen::Visit(cNopStmt *node)

void cCodeGen::Visit(cParams *node)
{
    // Visit the children in the reverse order
    cAstNode::iterator it = node->LastChild(); 
    do
    {
        it--;
        (*it)->Visit(this);
    } while (it != node->FirstChild());
}

void cCodeGen::Visit(cPlainVarRef *node)
{
    cVarDecl *var = node->GetDecl();

    if (var->GetType()->IsArray())
    {
        node->Visit(m_GenAddr);
    } else {
        if (var->IsGlobal())
        {
            node->Visit(m_GenAddr);
            
            if (var->GetType()->Size() == 1)    // ElementSize?
                EmitInst("PUSHCVARIND");
            else
                EmitInst("PUSHVARIND");
        } else {
            if (var->GetType()->Size() == 1)
                EmitInst("PUSHCVAR", var->GetOffset());
            else
                EmitInst("PUSHVAR", var->GetOffset());
        }
    }
}

void cCodeGen::Visit(cPointerDeref *node)
{
    node->GetBase()->Visit(this);
    if (node->GetType()->Size() == 1)     // EmementSize?
    {
        EmitInst("PUSHCVARIND");
    } else {
        EmitInst("PUSHVARIND");
    }
}

//void cCodeGen::Visit(cPointerType *node)

void cCodeGen::Visit(cPostfixExpr *node)
{
    cVarRef *var = node->GetExpr();

    cBinaryExpr *performOp = 
        new cBinaryExpr(var, node->GetOp(), new cIntExpr(1));

    performOp->Visit(this);
    EmitInst("DUP");
    var->Visit(m_GenAddr);
    if (var->GetType()->Size() == 1)
        EmitInst("POPCVARIND");
    else
        EmitInst("POPVARIND");
}

void cCodeGen::Visit(cPrefixExpr *node)
{
    cVarRef *var = node->GetExpr();

    node->GetExpr()->Visit(this);
    cBinaryExpr *performOp = new 
        cBinaryExpr(node->GetExpr(), node->GetOp(), new cIntExpr(1));

    performOp->Visit(this);
    var->Visit(m_GenAddr);
    if (var->GetType()->Size() == 1)
        EmitInst("POPCVARIND");
    else
        EmitInst("POPVARIND");
}

void cCodeGen::Visit(cReturnStmt *node)
{
    if (node->GetExpr() != NULL) 
    {
        node->GetExpr()->Visit(this);
        EmitInst("RETURNV");
    } else {
        EmitInst("RETURN");
    }
}

void cCodeGen::Visit(cShortCircuitExpr *node)
{
    if(node->ShortOnTrue())
    {
        // get a label to jump to
        std::string skipExpression = GenerateLabel();
        std::string checkExpression = GenerateLabel();

        // at a minimum, we want to output the left expression
        node->GetLeft()->Visit(this);
        EmitInst("DUP");
        EmitInst("JUMPE", checkExpression);
        EmitInst("JUMP", skipExpression);

        EmitLabel(checkExpression);
        node->GetRight()->Visit(this);
        EmitInst(node->OpAsString());
        EmitLabel(skipExpression);
    }
    else
    {
        // get a label to jump to
        std::string jmp = GenerateLabel();

        // at a minimum, we want to output the left expression
        node->GetLeft()->Visit(this);

        // if the result of the left expression left a 0
        // on the stack, skip the right expression
        EmitInst("DUP");
        EmitInst("JUMPE", jmp);

        // generate the code for the right expression
        node->GetRight()->Visit(this);

        // generate the code for the operator
        EmitInst(node->OpAsString());

        // jump to the end if the left expression was false
        EmitLabel(jmp);
    }
}

void cCodeGen::Visit(cSizeofExpr *node)
{
    EmitInst("PUSH", node->ConstValue());
}

//void cCodeGen::Visit(cStmt *node)               { VisitAllChildren(node); }
//void cCodeGen::Visit(cStmtsList *node)

void cCodeGen::Visit(cStringLit *node)
{
    string label = GenerateLabel();
    EmitInst("PUSH", label);
    EmitStringLit(node->GetString(), label);
}

void cCodeGen::Visit(cStructDeref *node)
{
    node->Visit(m_GenAddr);

    if (node->GetField()->GetDecl()->GetType()->IsArray())
    {
        // do nothing: we want the addr
    }
    else
    {
        if (node->GetField()->GetDecl()->GetType()->Size() == 1)
            EmitInst("PUSHCVARIND");
        else
            EmitInst("PUSHVARIND");
    }
}

void cCodeGen::Visit(cStructRef *node)
{
    node->Visit(m_GenAddr);

    if (node->GetField()->GetDecl()->GetType()->IsArray())
    {
        // do nothing: we want the addr
    }
    else
    {
        if (node->GetField()->GetDecl()->GetType()->Size() == 1)
            EmitInst("PUSHCVARIND");
        else
            EmitInst("PUSHVARIND");
    }
}

//void cCodeGen::Visit(cStructType *node)
//void cCodeGen::Visit(cSymbol *node)             { VisitAllChildren(node); }
//void cCodeGen::Visit(cTypeDecl *node)           { VisitAllChildren(node); }

void cCodeGen::Visit(cUnaryExpr *node)
{
    node->GetExpr()->Visit(this);

    switch (node->GetOp())
    {
        case '-':
            EmitInst("NEG");
            break;
        case '~':
            EmitInst("COMP");
            break;
        case '!':
            EmitInst("NOT");
            break;
        default:
            fatal_error("Unrecognized unary operator");
            break;
    }
}

void cCodeGen::Visit(cVarDecl *node)
{
    // NOTE: we explicitly avoit visiting children

    // If global, emit label and allocate space
    if (node->IsGlobal())
    {
        EmitInst(".dataseg");
        EmitLabel(node->GetName()->Name());
        if (node->IsConst())
        {
            EmitInst(".data", node->GetInit()->ConstValue());
        }
        else
        {
            EmitInst(".block", 
                    (node->GetType()->Size() + WORD_SIZE - 1)/WORD_SIZE);
        }
        EmitInst(".codeseg");
    }
}

//void cCodeGen::Visit(cVarRef *node)             { VisitAllChildren(node); }

void cCodeGen::Visit(cWhileStmt *node)
{
    std::string start_loop = GenerateLabel();
    std::string end_loop = GenerateLabel();

    EmitLabel(start_loop);
    node->GetCond()->Visit(this);
    EmitInst("JUMPE", end_loop);
    node->GetStmt()->Visit(this);
    EmitInst("JUMP", start_loop);
    EmitLabel(end_loop);
}

//*****************************************
// Emit an instruction
void cCodeGen::EmitInst(string inst, string label)
{
    m_Output << inst << " $" << label << "\n";
}

//*****************************************
// Emit an instruction
void cCodeGen::EmitInst(string inst, int param)
{
    m_Output << inst << " " << param << "\n";
}

//*****************************************
// Emit an instruction
void cCodeGen::EmitInst(string inst)
{
    m_Output << inst << "\n";
}

//*****************************************
// Generate a unique label for GOTO statements
string cCodeGen::GenerateLabel()
{
    m_Next_Label++;
    string label("LABEL_");
    label += std::to_string(m_Next_Label);
    return label;
}
//*****************************************
void cCodeGen::EmitStringLit(string str, string label)
{
    m_Output << ".dataseg\n";
    EmitLabel(label);
    m_Output << ".string \"" << str << "\"\n";
    m_Output << ".codeseg\n";
}
//*****************************************
void cCodeGen::EmitComment(string str)
{
    m_Output << "; " << m_Location << " " << str << "\n";
}
//*****************************************
void cCodeGen::EmitLabel(string label)
{
    m_Output << label << ":\n";
}