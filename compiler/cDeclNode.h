#pragma once

#include <string>

#include "cStmtNode.h"

class cSymbol;

class cDeclNode : public cStmtNode
{
    public:
        cDeclNode() : cStmtNode()
    {}

        std::string TypeId();
        std::string Name();

        virtual cDeclNode *GetBaseType() { return this; }

        virtual bool IsType()       { return false; }
        virtual bool IsFunc()       { return false; }
        virtual bool IsChar()       { return false; }
        virtual bool IsInt()        { return false; }
        virtual bool IsString()     { return false; }
        virtual bool IsStruct()     { return false; }
        virtual bool IsArray()      { return false; }
        virtual bool IsPointer()    { return false; }
        virtual bool IsGlobal()     { return false; }
        virtual bool IsConst()      { return false; }
        virtual int  GetValue()     { return 0; }
        virtual int Size()          { return mSize; }
        virtual int GetOffset()     { return mOffset; }
        virtual int ComputeOffsets(int base) { return base; }
        virtual cDeclNode* GetPointsTo() { return NULL; }
        virtual int GetPtrLevel() { return 0; }
        virtual bool CompatibleWith(cDeclNode *other)
        {
            cDeclNode *left = GetBaseType();
            cDeclNode *right = other->GetBaseType();

            // acceptable assignments:
            // foo abc = foo xyz
            // foo * abc = foo[5] xyz
            // foo * abc = foo * xyz
            // foo * abc = 123456

            if(IsArray())
            {
                if(right->IsPointer() && right->GetPointsTo() == GetBaseType()) return true;
            }

            if (left == right)
                return true;
            if (left->IsPointer() && right->IsArray())
                return true;
            if(right->IsPointer() && right->IsArray())
                return true;
            if(other->IsArray() && right->IsPointer())
                return true;
            if(left->IsPointer() && right->IsPointer())
                return true;
            if(left->IsPointer() && right->IsInt())
                return true;
            if (left->IsInt() && right->IsInt() && left->Size() >= right->Size())
                return true;

            std::cout << this->toString() << " is left, right is " << other->toString() << "\n";

            return false;
        }
    protected:
        cSymbol *mId;
        int mOffset;
        int mSize;
};

