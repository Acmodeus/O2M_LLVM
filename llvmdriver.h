#ifndef LLVMDRIVER_H
#define LLVMDRIVER_H

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "Type.h"
#include "Var.h"
#include "Stat.h"
#include <float.h>

using namespace llvm;
using namespace llvm::sys;

class LLVMDriver
{
public:
    LLVMContext TheContext;
    IRBuilder<> Builder;
    std::unique_ptr<Module> TheModule;
    //����� ���������� ��������, ���������� � ���������� �������
    std::map<std::string, AllocaInst *> NamedValues;
    //����� ���������� �������� � ����������
    std::map<std::string, GlobalVariable *> GlobalValues;
    //����� �������
    std::map<std::string, Function *> Functions;
    //����� ����� ����� �������� ��������� � ��� ���������� �������
    //��� ��������� � ���� � IR LLVM
    std::map<std::string, std::map<std::string,int>> Structures;
    //����� ������� �������������, ��� ������������ ������� ������
    std::map<std::string, int> SpecTypes;
    //����� ������, ������������ ��� ������ �� ������������ �����
    std::map<int, BasicBlock *> BasicBlocks;
    LLVMDriver(): Builder(TheContext){};
    ~LLVMDriver(){};
    //�������� ���������� ��������� ������ � ��������� ����� �������. ������������ ��� ���������� � ��
    AllocaInst *CreateEntryBlockAlloca(Function *TheFunction, std::string name,Type* type);
    //������� ���������� �����
    Value *CastToType(Value* v, Type* destType);
    //�������� ������� ��� ����������
    //????����� �������, ���������� �� ������ ��� ������????
    Function* createFunction(Type* retType,std::string name);
    //�������� ������� � �����������
    Function* createFunction(Type* retType,std::vector<Type*> typePars,StringVector namePars,std::string name);
    //������� ��������� ���� LLVM
    Type* GetLLVMType(EName_id name_id);
    Type* GetLLVMType(CBaseVar* v);
    Type* GetLLVMType(CBaseType* v);
    //����� (���� ���, ��������) �������
    Function* GetFunction(CProcedure* p);
    //�������� ������� ������ ������������������ �������
    void CreateCommonFun(CCommonProc* p,Function* F);
    void CreateCommonFun_loop(CCommonProc *p, Function *F,CBaseVarVector::const_iterator i,std::string funName,std::vector<Value*> values,std::vector<Type *> pars);
    //��������� ���� LLVM � ������ � ����
    int Init(CModule *m);
    //��������� ���� LLVM ������
    Value* WriteLLVM(CModule *m);
    Value* WriteLLVM_var(CDeclSeq* ds);
    Value* WriteLLVM_proc(CDeclSeq* ds);
    Value* WriteLLVM(CStatementSeq* ss);
    //��������� ���� LLVM ����������
    Value* WriteLLVM(CBaseVar* v);
    Value* WriteLLVM_ConstValue(CBaseVar* v);

    Value* WriteLLVM(CSetVar* v); //�� �����������
    Value* WriteLLVM(CPtrVar* v); //�� �����������
    //��������� ���� LLVM ���������
    Function* WriteLLVM(CProcedure* p);
    //��������� ������ ����� ���������� ����������
    std::vector<Type *> WriteLLVM_pars(CFormalPars *fp);
    void WriteLLVM_pars_array(CArrayVar* v,std::vector<Type*>& values);
    //��������� ���� LLVM ����������
    Value* WriteLLVM(CIfStatement* s);
    BasicBlock* WriteLLVM(CElsifPair* s,BasicBlock* bb);
    Value* WriteLLVM(CCaseStatement* s);
    Value* WriteLLVM(CCaseLabelsSeq* cls, CExpr* e,BasicBlock* exitBB);
    Value* WriteLLVM(CCaseLabels* cls, CExpr* e);
    Value* WriteLLVM(CWhileStatement* s);
    Value* WriteLLVM(CRepeatStatement* s);
    Value* WriteLLVM(CForStatement* s);
    Value* WriteLLVM(CLoopStatement* s);
    Value* WriteLLVM(CWithStatement* s){}; //�� �����������
    Value* WriteLLVM(CExitStatement* s);
    Value* WriteLLVM(CReturnStatement* s);
    Value* WriteLLVM(CAssignStatement* s); //�������� �����������
    Value* WriteLLVM_array(CAssignStatement* s);
    Value* WriteLLVM_COPY_Par(CBaseName* bn);
    Value* WriteLLVM(CCallStatement* s); //�������� �����������
    //��������� ������ ����������� ����������
    std::vector<Value *> WriteLLVM(CExprList* e,CFormalPars* fp);
    //��������� ���� LLVM ���������
    Value* WriteLLVM(CExpr* e); //�� ����������� rel_is,rel_IN
    Value* WriteLLVM(CSimpleExpr* e);
    Value* WriteLLVM(CTerm* t);
    Value* WriteLLVM(CTermPair* t);
    Value* WriteLLVM(CFactor* f);
    Value* WriteLLVM(CDesignator* d); //�������� �����������
    Value* WriteLLVM_index(CExprList* e, Value* array);
    Value* WriteLLVM_record_index(Value* record,char *ident,CDesignator *d);
    //��������� ���� LLVM ����������� ��������
    Value* WriteLLVM(CAbsStdProcFunc* d);
    Value* WriteLLVM(CAshStdProcFunc* d);
    Value* WriteLLVM(CCapStdProcFunc* d);
    Value* WriteLLVM(CChrStdProcFunc* d);
    Value* WriteLLVM(CEntierStdProcFunc* d);
    Value* WriteLLVM(CLenStdProcFunc* d);
    Value* WriteLLVM(CLongStdProcFunc* d);
    Value* WriteLLVM(CMaxStdProcFunc* d);
    Value* WriteLLVM(CMinStdProcFunc* d);
    Value* WriteLLVM(COddStdProcFunc* d);
    Value* WriteLLVM(COrdStdProcFunc* d);
    Value* WriteLLVM(CShortStdProcFunc* d);
    Value* WriteLLVM(CSizeStdProcFunc* d){}; //�� �����������
    Value* WriteLLVM(CDecStdProc* d);
    Value* WriteLLVM(CIncStdProc* d);
    Value* WriteLLVM(CNewStdProc* d);
};

#endif // LLVMDRIVER_H
