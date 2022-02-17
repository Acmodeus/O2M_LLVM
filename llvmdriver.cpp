#include "llvmdriver.h"

//���������� ������� LLVMDriver
LLVMDriver::~LLVMDriver()
{
    NamedValues.clear();
    GlobalValues.clear();
    Functions.clear();
    Structures.clear();
    SpecTypes.clear();
    BasicBlocks.clear();
}//~LLVMDriver

//-----------------------------------------------------------------------------
//�������� ���������� ��������� ������ � ��������� ����� �������. ������������ ��� ���������� � ��
AllocaInst *LLVMDriver::CreateEntryBlockAlloca(Function *TheFunction,
                                          std::string name,Type* type) {
  IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                 TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(type, 0,name);
}//CreateEntryBlockAlloca

//-----------------------------------------------------------------------------
//������� ���������� �����
Value *LLVMDriver::CastToType(Value* v, Type* destType) {
  //� ������ v=Nullptr, ������������ ���� Nullptr, ���� NullValue ���� destType
  if(!v){
    if(!destType) return v;
    return Constant::getNullValue(destType);
  }
  Type* valueType = v->getType();
  //��� ����������, �������� ���������� ���� �� ������������
  if (valueType->isPointerTy()||valueType->isArrayTy()) return v;
  if(v->getType() == destType) return v;
  //���������� � ���� Double
  if(destType==Type::getDoubleTy(TheContext)){
      return Builder.CreateSIToFP(v,destType,"castTmp");
  }
  //���������� � ������ ���� �������� 1 ��� ������������ ����� ��������� � �����
  if(destType==Type::getInt1Ty(TheContext)){
      if(valueType == Type::getDoubleTy(TheContext))
          return Builder.CreateFCmpONE(v,ConstantFP::get(valueType,0.0),"castTmp");
      else return Builder.CreateICmpNE(v,ConstantInt::get(valueType,0),"castTmp");
  }
  //���������� � ������ ���� �������� > 1 ���
  if(valueType == Type::getDoubleTy(TheContext))
      return Builder.CreateFPToSI(v,destType,"castTmp");
  return Builder.CreateIntCast(v,destType,true,"castTmp");
}//CastToType

//-----------------------------------------------------------------------------
//�������� ������� ��� ����������
Function *LLVMDriver::createFunction(Type *retType, std::string name)
{
    FunctionType *FT = FunctionType::get(retType,  false);
    return Function::Create(FT, Function::ExternalLinkage, name, TheModule.get());
}//createFunction

//-----------------------------------------------------------------------------
//�������� ������� � �����������
Function *LLVMDriver::createFunction(Type *retType, std::vector<Type *> typePars, StringVector namePars, std::string name)
{
    //��������� ���� �������
    FunctionType *FT = FunctionType::get(retType, typePars, false);
    //�������� �������
    Function *F = Function::Create(FT, Function::ExternalLinkage, name, TheModule.get());
    //���������� ���������� �������
    if (!namePars.empty()){
        StringVector::const_iterator i=namePars.begin();
        for (auto &Arg : F->args()){
            Arg.setName(*i);
            ++i;
        }//for
    }//if
    return F;
}//createFunction

//-----------------------------------------------------------------------------
//��������� ���� LLVM ��� ������� �����
Type *LLVMDriver::GetLLVMType(EName_id name_id)
{
    switch (name_id) {
    case id_CBooleanVar:
        return Type::getInt8Ty(TheContext);
    case id_CCharVar:
        return Type::getInt8Ty(TheContext);
    case id_CIntegerVar:
        return Type::getInt32Ty(TheContext);
    case id_CLongintVar:
        return Type::getInt32Ty(TheContext);
    case id_CLongrealVar:
        return Type::getDoubleTy(TheContext);
    case id_CRealVar:
        return Type::getDoubleTy(TheContext);
    case id_CSetVar:
        return Type::getInt32Ty(TheContext);
    case id_CShortintVar:
        return Type::getInt16Ty(TheContext);
    default:
        return nullptr;
    }//switch
}//GetLLVMType

//-----------------------------------------------------------------------------
//��������� ���� LLVM �� ������ ����������� ������ CBaseVar
Type *LLVMDriver::GetLLVMType(CBaseVar *v)
{
    switch (v->name_id) {
    default:
        //��� ������� ����� ���������� ��������������� �����
        return GetLLVMType(v->GetResultId());
    case id_CArrayVar:{
        //ARRAY
        CArrayType* AT = static_cast<CArrayVar*>(v)->ArrayType;
        return ArrayType::get(GetLLVMType(AT->Type),AT->size);
    }
    case id_CProcedureVar:{
        //PROCEDURE
        Type* retType;
        CProcedureVar* PT = static_cast<CProcedureVar*>(v);
        if (PT->FormalPars.Qualident) retType=GetLLVMType(PT->FormalPars.Qualident->TypeResultId);
        else retType=Type::getVoidTy(TheContext);
        return FunctionType::get(retType,WriteLLVM_pars(&PT->FormalPars),false)->getPointerTo();
    }
    case id_CPointerVar:{
        //POINTER
        CPointerVar* PT=static_cast<CPointerVar*>(v);
        Type* type=nullptr;
        if(PT->Type){
            type = GetLLVMType(PT->Type);
        }else{
                CBaseType* BT = PT->FindType();
                type = GetLLVMType(BT);
        }//if
        //� ������ ��������� �� ��������� ������������ ������ ���������
        //??????�������� ����� ��������??????
        if(type->isPointerTy())
            return type;
        return type->getPointerTo();
    }
    case id_CRecordVar:{
        //RECORD
        CRecordVar* RV = static_cast<CRecordVar*>(v);
        //���� ��� ��� ������ - ����������
        StructType* ST=TheModule->getTypeByName(RV->GetTypeName());
        if(ST){
           return ST;
        }//if
        //�������� ����
        ST=StructType::create(TheContext,RV->GetTypeName());
        std::vector<Type*> colTypes;
        CBaseVarVector::const_iterator ci;
        int i=0;
        //������������ ���� ������, � ���������� ����� Structure
        for (ci = RV->FieldStore.begin(); ci != RV->FieldStore.end(); ++ci,i++) {
            colTypes.push_back(GetLLVMType(static_cast<CBaseVar*>(*ci)));
            Structures[v->GetTypeName()][(*ci)->name] = i;
        }//for
        ST->setBody(colTypes);
        return ST;
    }
    case id_CCommonVar:{
        //COMMON
        /*StructType* ST=TheModule->getTypeByName(v->GetTypeName());
        if(ST){
           return ST;
        }*/
        //���� ����� ������������� ���������, �� ���������� ��, ���� ���, �� ��� ���������;
        //�������������� ��� ��� ��������� ��������� ��� ���������� ����,
        //���� ��� ���������� ���������� ���� ��������� �� ���������
        CCommonVar* CV=static_cast<CCommonVar*>(v);
        CBaseType* BT = CV->FindType();
        if(!BT) return TheModule->getTypeByName(v->GetTypeName());
        return GetLLVMType(BT);
    }
    }//switch
}//GetLLVMType

//-----------------------------------------------------------------------------
//��������� ���� LLVM �� ������ ����������� ������ CBaseType
Type *LLVMDriver::GetLLVMType(CBaseType *v)
{
    switch (v->name_id) {
    default:
        //��� ������� ����� ���������� ��������������� �����
        return GetLLVMType(v->GetResultId());
    case id_CArrayType:{
        //ARRAY
        CArrayType* AT = static_cast<CArrayType*>(v);
        return ArrayType::get(GetLLVMType(AT->Type),AT->size);
    }
    case id_CQualidentType:{
        //������� ���
        //���� ������� ��� � ���������� ������� ��������� � ��������� ���
        CQualidentType* QT = static_cast<CQualidentType*>(v);
        CBaseType* BT = static_cast<CBaseType*>(QT->parent_element->GetGlobalName(QT->Qualident->pref_ident, QT->Qualident->ident));
        return GetLLVMType(BT);
    }
    case id_CProcedureType:{
        //PROCEDURE
        Type* retType;
        CProcedureType* PT = static_cast<CProcedureType*>(v);
        if (PT->FormalPars.Qualident) retType=GetLLVMType(PT->FormalPars.Qualident->TypeResultId);
        else retType=Type::getVoidTy(TheContext);
        return FunctionType::get(retType,WriteLLVM_pars(&PT->FormalPars),false)->getPointerTo();
    }
    case id_CPointerType:{
        //POINTER
        CPointerType* PT=static_cast<CPointerType*>(v);
        Type* type=nullptr;
        if(PT->Type){
            type = GetLLVMType(PT->Type);
        }else{
                CBaseType* BT = PT->FindType();
                type = GetLLVMType(BT);
        }//if
        //� ������ ��������� �� ��������� ������������ ������ ���������
        //??????�������� ����� ��������??????
        if(type->isPointerTy())
            return type;
        return type->getPointerTo();
    }
    case id_CRecordType:{
        //RECORD
        //���� ��� ��� ������ - ����������
        StructType* ST=TheModule->getTypeByName(v->name);
        if(ST){
           return ST;
        }//if
        //�������� ����
        ST=StructType::create(TheContext,v->name);
        CRecordType* RT=static_cast<CRecordType*>(v);
        std::vector<Type*> colTypes;
        CBaseNameVector::const_iterator ci;
        int i=0;
        //������������ ���� ������, � ���������� ����� Structure
        for (ci = RT->FieldStore.begin(); ci != RT->FieldStore.end(); ++ci,++i) {
            if((*ci)->name_id==id_CProcedure) continue;
            colTypes.push_back(GetLLVMType(static_cast<CBaseVar*>(*ci)));
            Structures[v->name][(*ci)->name] = i;
        }//for
        ST->setBody(colTypes);
        return ST;
    }
    case id_CCommonType:{
        //COMMON
        //���� ��� ��� ������ - ����������
        StructType* ST=TheModule->getTypeByName(v->name);
        if(ST){
           return ST;
        }//if
        //�������� ����
        ST=StructType::create(TheContext,v->name);
        CCommonType* CT=static_cast<CCommonType*>(v);
        CCommonType::SpecStore_type::const_iterator ci;
        int i=0;
        //�������� ����� �������������
        for(ci = CT->SpecStore.begin(); ci != CT->SpecStore.end(); ++ci,++i){
            const CBaseName* BN = CT->GetParentModule();
            while (BN->name_id!=id_CModule) {
                BN=BN->GetParentModule();
            }//while
            CBaseType* BT = static_cast<CBaseType*>(BN->GetGlobalName((*ci)->QualName, (*ci)->Name));
            GetLLVMType(BT);
            SpecTypes[(*ci)->Name]=i;
        }//for
        //� �������� ���� ���� ��������� ��� ��������� �� ������ ���������
        //� ����� ���, ���������� �� ����� �������������
        std::vector<Type*> colTypes;
        colTypes.push_back(ST->getPointerTo());
        colTypes.push_back(Type::getInt32Ty(TheContext));
        ST->setBody(colTypes);
        return ST;
    }
    case id_CSpecType:{
        //�������������
        //���������� ������� ���
        CSpecType* CST=static_cast<CSpecType*>(v);
        CCommonType* CT = static_cast<CCommonType*>(CST->parent_element->GetGlobalName(CST->Qualident->pref_ident, CST->Qualident->ident));
        return GetLLVMType(CT);
    }
    }//switch
}//GetLLVMType

//-----------------------------------------------------------------------------
//����� (���� ��� � ������� ��������� ������ LLVM, ��������) �������
Function* LLVMDriver::GetFunction(CProcedure *p)
{
    //� ������ ������� ���������� ���������� ���������� ��������� �� CCommonProc
    CCommonProc* cp=nullptr;
    if ((id_CCommonProc == p->name_id || id_CDfnCommonProc == p->name_id ) && p->GetCommonParsCount()){
        cp=static_cast<CCommonProc*>(p);
    }//if
    //������������ ����� ������� � ������������ �� ���������� �++
    std::string firstName=p->parent_element->name;
    if (p->name_id==id_CHandlerProc&&static_cast<CHandlerProc*>(p)->QualName) firstName=static_cast<CHandlerProc*>(p)->QualName;
    std::string lastName=p->name;
    std::string funName;
    if(p->Receiver){
        std::string recieverName=p->Receiver->type_name;
        funName="_ZN"+std::to_string(firstName.size())+firstName+std::to_string(recieverName.size())+recieverName+std::to_string(lastName.size())+lastName+"E";
    }else funName="_ZN"+std::to_string(firstName.size())+firstName+std::to_string(lastName.size())+lastName+"E";
    if(!p->FormalPars->FPStore.empty()){
        for(CBaseVarVector::const_iterator i = p->FormalPars->FPStore.begin(); i != p->FormalPars->FPStore.end(); ++i) {
            if((*i)->is_var){
                funName+="R";
            }//if
            if((*i)->name_id==id_CRealVar){
                funName+="d";
            }else if((*i)->name_id==id_CLongintVar){
                funName+="l";
            }else if((*i)->name_id==id_CIntegerVar){
                funName+="i";
            }else if((*i)->name_id==id_CShortintVar){
                funName+="s";
            }else if((*i)->name_id==id_CCharVar){
                funName+="c";
            }else if((*i)->name_id==id_CBooleanVar){
                funName+="b";
            }else if((*i)->name_id==id_CCommonVar){
                funName+=static_cast<CCommonVar*>(*i)->GetCPPCompoundName();
            }else if((*i)->name_id==id_CArrayVar){
                CArrayVar* AV=static_cast<CArrayVar*>(*i);
                funName+="P";
                if(AV->ArrayType->Type->name_id==id_CCharType){
                    funName+="c";
                }//if
                funName+="i";
            }//if
        }//for
    }else if(!cp) funName+="v";

    //���� ������� � ����� ������ ��� �������, ���������� ��
    if(Functions[funName]) return Functions[funName];

    //�������� ��� ������������� �������� �������
    Type *FunRetType;
    if (!p->FormalPars->Qualident){
        FunRetType=Type::getVoidTy(TheContext);
    }else{
        FunRetType=GetLLVMType(p->FormalPars->Qualident->TypeResultId);
        if(!FunRetType) {
            FunRetType=GetLLVMType(static_cast<CBaseType*>(p->parent_element->GetGlobalName(p->FormalPars->Qualident->pref_ident, p->FormalPars->Qualident->ident)));
        }//if
    }//if

    //�������� ������ ����� ���������� ����������
    std::vector<Type *> pars=WriteLLVM_pars(p->FormalPars);
    //�������� ������ ���� ���������� ����������
    StringVector names;
    CBaseVarVector::const_iterator i;
    i = p->FormalPars->FPStore.begin();
    for (i = p->FormalPars->FPStore.begin(); i != p->FormalPars->FPStore.end(); ++i){
      names.push_back((*i)->name);
      //��� �������� ����������� ������� ��������� � �������� ���������� ����������
      if ((*i)->name_id==id_CArrayVar) {
          CArrayType* AT = static_cast<CArrayVar*>(*i)->ArrayType;
          int dimention = 0;
          while (AT->name_id == id_CArrayType) {
              std::string s="O2M_ARR_"+std::to_string(dimention)+"_"+(*i)->name;
              names.push_back(&s[0]);
              ++dimention;
              AT = static_cast<CArrayType*>(AT->Type);
          }//while
      }//if
    }//for
    //�������� �������� � �������� ����������� ���������
    if(p->Receiver){
        pars.push_back(GetLLVMType(static_cast<CBaseVar*>(p->Receiver->FindName(p->Receiver->name)))->getPointerTo());
        names.push_back(p->Receiver->name);
    }//if
    //��� ������� � ����������� ����������� ��������� ��
    if(cp){
        std::vector<Type *> compars=WriteLLVM_pars(cp->CommonPars);
        pars.insert(pars.end(),compars.begin(),compars.end());
        for (i = cp->CommonPars->FPStore.begin(); i != cp->CommonPars->FPStore.end(); ++i){
            names.push_back((*i)->name);
        }//for
    }//if

    //�������� ������� � ������� � ����� �������
    Function* F = createFunction(FunRetType,pars,names,funName);
    Functions[funName]=F;
    //� ������ ������� � ����������� ������� ������� ������ ������������������ �������
    if(cp) CreateCommonFun(cp,F);
    return F;
}//GetFunction

//-----------------------------------------------------------------------------
//�������� ������� ������ ������������������ �������
void LLVMDriver::CreateCommonFun(CCommonProc *p, Function *F)
{
    //���������� ������� ������� � ������ LLVM � ����� ��������� ����������
    BasicBlock* TempBB=Builder.GetInsertBlock();
    std::map<std::string, AllocaInst *> NamedValuesTemp=NamedValues;
    //������ ������ � ������� ������
    BasicBlock *BB = BasicBlock::Create(TheContext, "entry", F);
    Builder.SetInsertPoint(BB);
    // ������ ���������� ������� � ����� ��������� ����������
    NamedValues.clear();
    for (auto &Arg : F->args()) {
        // ��������� ������ ��� ��� ����������
        AllocaInst *Alloca = CreateEntryBlockAlloca(F,std::string(Arg.getName()),Arg.getType());
        // ���������� �������� ��������� � ���������� ������
        Builder.CreateStore(&Arg, Alloca);
        //���������� ��������� � ����� ��������� ����������
        NamedValues[std::string(Arg.getName())] = Alloca;
    }//for
    //��������� ������������ ����������
    CBaseVarVector::const_iterator i = p->CommonPars->FPStore.begin();
    std::string funName=F->getName().str();
    std::vector<Type *> pars=WriteLLVM_pars(p->FormalPars);
    std::vector<Value*> values;
    for(auto value : NamedValues){
        if(value.first == (*i)->name){
            break;
        }else
            values.push_back(Builder.CreateLoad(value.second));
    }//for
    //��������� ���������� ����������
    CreateCommonFun_loop(p,F,i,funName,values,pars);
    //�������� ���������������� ���� �� ���������������
    verifyFunction(*F);
    //�������������� ����������� ����������
    Builder.SetInsertPoint(TempBB);
    NamedValues=NamedValuesTemp;
}//CreateCommonFun

//-----------------------------------------------------------------------------
//���� ������ �������������
void LLVMDriver::CreateCommonFun_loop(CCommonProc *p, Function *F,CBaseVarVector::const_iterator i,std::string funName,std::vector<Value*> values,std::vector<Type *> pars){
    //��������� ����������� ���� � ������ �������������
    CCommonVar* CV=static_cast<CCommonVar*>(*i);
    const CBaseName* BN = p->GetParentModule();
    while (BN->name_id!=id_CModule) {
        BN=BN->GetParentModule();
    }//while
    CCommonType* CT = static_cast<CCommonType*>(BN->GetGlobalName(CV->GetTypeModuleName(),CV->GetTypeName()));
    CCommonType::SpecStore_type::const_iterator ci;
    //��������� ���������� ��� ������������� ���������� �� ��������� �������������
    std::string tempFunName=funName;
    std::vector<Value*> tempValues=values;
    std::vector<Type *> tempPars=pars;
    for(ci = CT->SpecStore.begin(); ci != CT->SpecStore.end(); ++ci){
        funName=tempFunName;
        values=tempValues;
        pars=tempPars;
        //������������ ����� ������� � ���������� ��������������
        if((*i)->is_var){
            funName+="R";
        }//if
        if ((*ci)->Tag) {
            funName+=(*ci)->Tag;
        }else if((*ci)->QualName) {
            funName+=std::string((*ci)->QualName)+"_"+(*ci)->Name;
        }else{
            funName+=std::string(BN->name)+"_"+(*ci)->Name;
        }//if
        //��������� ���� ���������� �������������� � ���������� ��� � ������ ���������� ����������
        Type* SpecType=TheModule->getTypeByName((*ci)->Name)->getPointerTo();
        pars.push_back(SpecType);
        //��������� ���� �������������
        Value* common=Builder.CreateLoad(NamedValues[(*i)->name]);
        Value* spec=Builder.CreateStructGEP(common,1);
        spec=Builder.CreateLoad(spec);
        //������������ ���������� if-then-else
        int sTag=SpecTypes[(*ci)->Name];
        Value* condV=Builder.CreateICmpEQ(spec,ConstantInt::get(Type::getInt32Ty(TheContext),sTag));
        BasicBlock *ThenBB = BasicBlock::Create(TheContext, "then", F);
        BasicBlock *ElseBB = BasicBlock::Create(TheContext, "else");
        Builder.CreateCondBr(condV, ThenBB, ElseBB);
        Builder.SetInsertPoint(ThenBB);
        //���������� ���������� ���������� � ������ ����������� ����������
        //��� ������ ������� ����� ���������� ���� �� ���������
        spec=Builder.CreateStructGEP(common,0);
        values.push_back(Builder.CreateLoad(spec));
        //��������� ��������� ���������� ���������� ��� �������
        if(i+1!=p->CommonPars->FPStore.end()){
            CreateCommonFun_loop(p,F,i+1,funName,values,pars);
            F->getBasicBlockList().push_back(ElseBB);
            Builder.SetInsertPoint(ElseBB);
            continue;
        }//if
        //�������� ���������� ������ ����������������� ������� � �������� �� ��������
        Function* SpecF=Functions[funName];
        if(!SpecF) SpecF=createFunction(F->getReturnType(),pars,*new StringVector,funName);
        Value* call=Builder.CreateCall(SpecF,values);
        Builder.CreateRet(call);
        //��������� ���������� else
        F->getBasicBlockList().push_back(ElseBB);
        Builder.SetInsertPoint(ElseBB);
    }//for
    //��������� ���������� 'ret <NullValue>'
    Value* retVal=Constant::getNullValue(F->getReturnType());
    Builder.CreateRet(retVal);
}//CreateCommonFun_loop

//-----------------------------------------------------------------------------
//��������� ���� LLVM � ������ � ����
int LLVMDriver::Init(CModule* m)
{
      //��������� ���� LLVM IR ��� ������ O2M
      WriteLLVM(m);
      //������������� ������� ��������
      InitializeAllTargetInfos();
      InitializeAllTargets();
      InitializeAllTargetMCs();
      InitializeAllAsmParsers();
      InitializeAllAsmPrinters();

      auto TargetTriple = sys::getDefaultTargetTriple();
      TheModule->setTargetTriple(TargetTriple);

       std::string Error;
       auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);

       //������ ������ � ����� ���� �� ������ ����� ������� ���������
       if (!Target) {
         errs() << Error;
         return 1;
       }//if

       auto CPU = "generic";
       auto Features = "";

       TargetOptions opt;
       auto RM = Optional<Reloc::Model>();
       auto TheTargetMachine =
            Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

       TheModule->setDataLayout(TheTargetMachine->createDataLayout());

        std::string f_name = std::string(m->name)+".o";
        std::error_code EC;
        raw_fd_ostream dest(f_name, EC, sys::fs::OF_None);

        if (EC) {
          errs() << "Could not open file: " << EC.message();
          return 1;
        }//if

        legacy::PassManager pass;
        auto FileType = CGFT_ObjectFile;

        if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
          errs() << "TheTargetMachine can't emit a file of this type";
          return 1;
        }//if

        pass.run(*TheModule);
        dest.flush();

        outs() << "Wrote " << f_name << "\n";

        return 0;
}//Init

//-----------------------------------------------------------------------------
//��������� ���� LLVM IR ������ O2M
Value* LLVMDriver::WriteLLVM(CModule *m)
{
    TheModule = std::make_unique<Module>(m->name, TheContext);

    //��������� ���� ����������
    WriteLLVM_var(m->DeclSeq);
    //��������� ���� ���������� ��������
    WriteLLVM_proc(m->DeclSeq);

    //������������� ������������� ������� �� ������������
    //���� ��� ������ � ����������� ��������� �������,
    //?????????��������???????????
    //DeclSeq->WriteCPP_mod_init(f);

    //��������� ���� ��� ���������� (���� ����)
    if (m->StatementSeq) {
        //�������� ������� ������� ������ main (����� ����� � ���������)
        Function *F = createFunction(Type::getInt32Ty(TheContext),"main");
        // �������� ������ �������� ����� ��� ������ ������
        BasicBlock *BB = BasicBlock::Create(TheContext, "entry", F);
        Builder.SetInsertPoint(BB);
        //������� ����� ��������� ����������
        NamedValues.clear();
        //���������� ��������� ���� ����������
        WriteLLVM(m->StatementSeq);
        //��������� ���������� 'ret 0'
        Value* retVal=Constant::getNullValue(Type::getInt32Ty(TheContext));
        Builder.CreateRet(retVal);
        //�������� ���������������� ���� ������� �� ���������������
        verifyFunction(*F);
    }//if

    //������ ������ ������ � ����� ������ ��� �������
    //TheModule->print(errs(),nullptr);
    //fprintf(stderr, "\n");

    return nullptr;
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� ����������
Value *LLVMDriver::WriteLLVM_var(CDeclSeq *ds){
    CBaseNameVector::const_iterator ci;
    for(ci = ds->BaseNameStore.begin(); ci != ds->BaseNameStore.end(); ++ci){
        if (CBaseVar::IsVarId((*ci)->name_id)) {
            WriteLLVM(static_cast<CBaseVar*>(*ci));
        }//if
    }//for
    return nullptr;
}//WriteLLVM_var

//-----------------------------------------------------------------------------
//��������� ���� ���������� ��������
Value *LLVMDriver::WriteLLVM_proc(CDeclSeq *ds){
    CBaseNameVector::const_iterator ci;
    for (ci = ds->BaseNameStore.begin(); ci != ds->BaseNameStore.end(); ++ci){
        if (CProcedure::IsProcId((*ci)->name_id)) {
            WriteLLVM(static_cast<CProcedure*>(*ci));
        }//if
    }//for
    return nullptr;
}//WriteLLVM_proc

//-----------------------------------------------------------------------------
//��������� ���� ����������
Value *LLVMDriver::WriteLLVM(CStatementSeq *ss)
{
    //���� ������ � ��������� ���� ����������� ���������
    for(CBaseVector::iterator i = ss->StatStore.begin(); i != ss->StatStore.end(); ++i) {
        CBase* base = *i;
        if(typeid(*base) == typeid (CIfStatement)){
            WriteLLVM(static_cast<CIfStatement*>(base));
        }else if(typeid(*base) == typeid (CCaseStatement)){
            WriteLLVM(static_cast<CCaseStatement*>(base));
        }else if(typeid(*base) == typeid (CWhileStatement)){
            WriteLLVM(static_cast<CWhileStatement*>(base));
        }else if(typeid(*base) == typeid (CRepeatStatement)){
            WriteLLVM(static_cast<CRepeatStatement*>(base));
        }else if(typeid(*base) == typeid (CForStatement)){
            WriteLLVM(static_cast<CForStatement*>(base));
        }else if(typeid(*base) == typeid (CLoopStatement)){
            WriteLLVM(static_cast<CLoopStatement*>(base));
        }else if(typeid(*base) == typeid (CWithStatement)){
            WriteLLVM(static_cast<CWithStatement*>(base));
        }else if(typeid(*base) == typeid (CExitStatement)){
            WriteLLVM(static_cast<CExitStatement*>(base));
        }else if(typeid(*base) == typeid (CAssignStatement)){
            WriteLLVM(static_cast<CAssignStatement*>(base));
        }else if(typeid(*base) == typeid (CReturnStatement)){
            WriteLLVM(static_cast<CReturnStatement*>(base));
        }else if(typeid(*base) == typeid (CCallStatement)){
            WriteLLVM(static_cast<CCallStatement*>(base));
        }else if(typeid(*base) == typeid (CDecStdProc)){
            WriteLLVM(static_cast<CDecStdProc*>(base));
        }else if(typeid(*base) == typeid (CIncStdProc)){
            WriteLLVM(static_cast<CIncStdProc*>(base));
        }else if(typeid(*base) == typeid (CNewStdProc)){
            WriteLLVM(static_cast<CNewStdProc*>(base));
        }//if
    }//for
    return nullptr;
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� ���������� ���������� ����������
Value *LLVMDriver::WriteLLVM(CBaseVar *v)
{
    //������� ������������
    bool is_global = id_CModule == v->parent_element->name_id;    
    //��������� ���� ����������
    Type* type=GetLLVMType(v);
    //� ������ ��������� - �������������� �� ���������, ����� �������������� �����
    Constant *InitVal=nullptr;
    if (v->is_const) {
        switch (v->name_id) {
            case id_CBooleanVar:
                InitVal = ConstantInt::get(type,static_cast<CBooleanVar*>(v)->ConstValue);
                break;
            case id_CCharVar:
                InitVal = ConstantInt::get(type,static_cast<CCharVar*>(v)->ConstValue);
                break;
            case id_CIntegerVar:
                InitVal = ConstantInt::get(type,static_cast<CIntegerVar*>(v)->ConstValue);
                break;
            case id_CLongintVar:
                InitVal = ConstantInt::get(type,static_cast<CLongintVar*>(v)->ConstValue);
                break;
            case id_CLongrealVar:
                InitVal = ConstantFP::get(type,static_cast<CLongrealVar*>(v)->ConstValue);
                break;
            case id_CRealVar:
                InitVal = ConstantFP::get(type,static_cast<CRealVar*>(v)->ConstValue);
                break;
            case id_CShortintVar:
                InitVal = ConstantInt::get(type,static_cast<CShortintVar*>(v)->ConstValue);
                break;
        default:
            break;
        }//switch
    } else {
        InitVal = Constant::getNullValue(type);
    }//if
    if(is_global){
        //� ������ ���������� ���������� ������� �� � ������������ �� ���������� C++
        //� ��������� � ����� ���������� ����������
        std::string firstName=v->parent_element->name;
        std::string lastName=v->name;
        std::string name="_ZN"+std::to_string(firstName.size())+firstName+std::to_string(lastName.size())+lastName+"E";
        GlobalVariable *gVar= new GlobalVariable(*TheModule.get(),type, v->is_const,GlobalValue::ExternalLinkage,InitVal,name);
        GlobalValues[v->name]=gVar;
        return gVar;
    }else{
        //��� ��������� ���������� �������� ������ � ��������� � ����� ��������� ����������
        Function *TheFunction = Builder.GetInsertBlock()->getParent();
        AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, v->name,type);
        Builder.CreateStore(InitVal, Alloca);
        NamedValues[v->name] = Alloca;
        return Alloca;
    }//if
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� ���������� ��������
Value *LLVMDriver::WriteLLVM_ConstValue(CBaseVar *v)
{
    switch (v->name_id) {
        case id_CBooleanVar:
            return ConstantInt::get(GetLLVMType(v->name_id),static_cast<CBooleanVar*>(v)->ConstValue);
        case id_CCharVar:
            return ConstantInt::get(GetLLVMType(v->name_id),static_cast<CCharVar*>(v)->ConstValue);
        case id_CIntegerVar:
            return ConstantInt::get(GetLLVMType(v->name_id),static_cast<CIntegerVar*>(v)->ConstValue);
        case id_CLongintVar:
            return ConstantInt::get(GetLLVMType(v->name_id),static_cast<CLongintVar*>(v)->ConstValue);
        case id_CLongrealVar:
            return ConstantFP::get(GetLLVMType(v->name_id),static_cast<CLongrealVar*>(v)->ConstValue);
        case id_CRealVar:
            return ConstantFP::get(GetLLVMType(v->name_id),static_cast<CRealVar*>(v)->ConstValue);
        case id_CShortintVar:
            return ConstantInt::get(GetLLVMType(v->name_id),static_cast<CShortintVar*>(v)->ConstValue);
        case id_CArrayVar:
            return Builder.CreateGlobalStringPtr(StringRef(static_cast<CArrayVar*>(v)->ConstString));
        default: return nullptr;
    }//switch
}//WriteLLVM_ConstValue

//-----------------------------------------------------------------------------
//��������� ���� LLVM ���������
Function *LLVMDriver::WriteLLVM(CProcedure *p)
{
    //��� ���������� ������� ��� ����� �� ������������
    //��������� ���������� � ������� CreateCommonFun
    if(p->name_id==id_CCommonProc) return nullptr;
    //�������� ��������� �� ������� LLVM
    Function* F = GetFunction(p);
    // �������� ������ �������� ����� ��� ������ ������
    BasicBlock *BB = BasicBlock::Create(TheContext, "entry", F);
    Builder.SetInsertPoint(BB);
    //������� ����� ��������� ����������
    NamedValues.clear();
    //���������� ���������� ������� � ����� ��������� ����������
    for (auto &Arg : F->args()) {
        // ��������� ������ ��� �������� ���������
        AllocaInst *Alloca = CreateEntryBlockAlloca(F,std::string(Arg.getName()),Arg.getType());
        // ���������� �������� ��������� � ������.
        Builder.CreateStore(&Arg, Alloca);
        //���������� ��������� �� ������ � ����� ��������� ����������
        NamedValues[std::string(Arg.getName())] = Alloca;
    }//for
    //��������� ���� ����������
    WriteLLVM_var(p->DeclSeq);
    //��������� ���� ��� ����������
    if (p->StatementSeq) {
        WriteLLVM(p->StatementSeq);
    }//if
    //��������� ���� �������� �������� ��������
    //����� ��� ��� ����� ����� RETURN ����� ���� ��� ����� ����
    //��� LLVM ����������� ����� ������� � ����� �������
    Builder.CreateRet(Constant::getNullValue(Builder.getCurrentFunctionReturnType()));
    //�������� ���������������� ���� ������� �� ���������������
    verifyFunction(*F);
    //������� ��������� �� �������
    return F;
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ������ ����� ���������� ����������
std::vector<Type *> LLVMDriver::WriteLLVM_pars(CFormalPars *fp)
{
    std::vector<Type*> values;
    //�������� ������� ���������� ����������
    if (fp->FPStore.empty()) return values;
    //������������ ������ ����� ���������� ����������
    for(CBaseVarVector::const_iterator i = fp->FPStore.begin(); i != fp->FPStore.end(); ++i) {
        //������ ������� � ������ ����� ���������� ����������
        if((*i)->name_id == id_CArrayVar){
            WriteLLVM_pars_array(static_cast<CArrayVar*>(*i),values);
            continue;
        }//if
        //���� �������� �������� ����������, ���������� ��������� �� ���
        //����� ��� ���
        if((*i)->is_var)
            values.push_back(GetLLVMType((*i))->getPointerTo());
        else values.push_back(GetLLVMType((*i)));
    }//for
    return values;
}//WriteLLVM_pars

//-----------------------------------------------------------------------------
//������ ������� � ������ ����� ���������� ����������
void LLVMDriver::WriteLLVM_pars_array(CArrayVar *v, std::vector<Type *> &values)
{
    //���������� ��������� �� ������� ��� � ������ ����� ���������� ����������
    CArrayType* AT = static_cast<CArrayType*>(v->ArrayType->FindLastType());
    CBaseVar* BV;
    static_cast<CBaseType*>(AT)->CreateVar(BV, v->parent_element);
    values.push_back(GetLLVMType((BV)->GetResultId())->getPointerTo());
    delete BV;
    //������������� ���������� � ������ ����� ����� ��� ��� ������� ������� �������
    AT=v->ArrayType;
    while (AT->name_id == id_CArrayType) {
        values.push_back(Type::getInt32Ty(TheContext));
        AT = static_cast<CArrayType*>(AT->Type);
    }//while
}//WriteLLVM_pars_array

//-----------------------------------------------------------------------------
//��������� ���� LLVM ��� ��������� IF
Value *LLVMDriver::WriteLLVM(CIfStatement *s)
{
    CIfStatement::ElsifPairList_type::iterator i = s->ElsifPairList.begin();
    //��������� �� ������� �������
    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    //�lse ����
    BasicBlock *ElseBB;
    //���� ����� ���� �������
    BasicBlock *MergeBB = BasicBlock::Create(TheContext, "ifcont");
    //��������� ���� �������
    for(; i != s->ElsifPairList.end(); ++i) {
        ElseBB = WriteLLVM((*i),MergeBB);
        TheFunction->getBasicBlockList().push_back(ElseBB);
        Builder.SetInsertPoint(ElseBB);
    }//for
    //��������� ���� ���������� ������� ELSE
    if (s->ElseStatementSeq) {
        WriteLLVM(s->ElseStatementSeq);
    }//if
    //������� � ����� ����� ���� �������
    Builder.CreateBr(MergeBB);
    TheFunction->getBasicBlockList().push_back(MergeBB);
    Builder.SetInsertPoint(MergeBB);
    return nullptr;
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� �������, ���������� ��������� �� ��������� ���� �������
BasicBlock *LLVMDriver::WriteLLVM(CElsifPair *s, BasicBlock *bb)
{
    //��������� ���� ������� ����� � ����
    Value* expr = WriteLLVM(s->Expr);
    while(expr->getType()->isPointerTy()){
        expr=Builder.CreateLoad(expr,expr->getName());
    }//while
    if(expr->getType()!=Type::getInt1Ty(TheContext))
        expr=CastToType(expr,Type::getInt1Ty(TheContext));
    //��������� �� ������� �������
    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    // ������� ����� ��� ����� then � else.  ��������� ���� then � ����� �������
    BasicBlock *ThenBB = BasicBlock::Create(TheContext, "then", TheFunction);
    BasicBlock *ElseBB = BasicBlock::Create(TheContext, "else");
    //������� ������� �������� � �����
    Builder.CreateCondBr(expr, ThenBB, ElseBB);
    Builder.SetInsertPoint(ThenBB);
    //��������� ���� ����� then
    WriteLLVM(s->StatementSeq);
    Builder.CreateBr(bb);
    //������� ��������� �� ���� ELSE, ELSEIF
    return ElseBB;
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ��� ��������� WHILE
Value *LLVMDriver::WriteLLVM(CWhileStatement *s)
{
    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    //������� ����� while, body, endwhile. ��������� � ����� while
    BasicBlock *WhileBB = BasicBlock::Create(TheContext, "while",TheFunction);
    BasicBlock *BodyBB = BasicBlock::Create(TheContext, "body");
    BasicBlock *EndBB = BasicBlock::Create(TheContext, "endwhile");
    Builder.CreateBr(WhileBB);
    Builder.SetInsertPoint(WhileBB);
    //��������� ������� �������� � ����� body ��� endwhile
    Value* expr = WriteLLVM(s->Expr);
    while(expr->getType()->isPointerTy()){
        expr=Builder.CreateLoad(expr,expr->getName());
    }//while
    if(expr->getType()!=Type::getInt1Ty(TheContext))
        expr = CastToType(expr,Type::getInt1Ty(TheContext));
    Builder.CreateCondBr(expr, BodyBB, EndBB);
    TheFunction->getBasicBlockList().push_back(BodyBB);
    Builder.SetInsertPoint(BodyBB);
    //��������� ���� ���� ��������� WHILE
    WriteLLVM(s->StatementSeq);
    Builder.CreateBr(WhileBB);
    //������� � ����� endwhile
    TheFunction->getBasicBlockList().push_back(EndBB);
    Builder.SetInsertPoint(EndBB);
    return nullptr;
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ��� ��������� Repeat
Value *LLVMDriver::WriteLLVM(CRepeatStatement *s)
{
    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    //������� �����
    BasicBlock *RepeatBB = BasicBlock::Create(TheContext, "repeat",TheFunction);
    BasicBlock *EndBB = BasicBlock::Create(TheContext, "endrepeat");
    Builder.CreateBr(RepeatBB);
    Builder.SetInsertPoint(RepeatBB);
    //��������� ���� ���� �����
    WriteLLVM(s->StatementSeq);
    //��������� ���� ������� ������� �����
    Value* expr = WriteLLVM(s->Expr);
    while(expr->getType()->isPointerTy()){
        expr=Builder.CreateLoad(expr,expr->getName());
    }//while
    if(expr->getType()!=Type::getInt1Ty(TheContext))
        expr = CastToType(expr,Type::getInt1Ty(TheContext));
    Builder.CreateCondBr(expr, EndBB, RepeatBB);
    TheFunction->getBasicBlockList().push_back(EndBB);
    Builder.SetInsertPoint(EndBB);
    return nullptr;
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ��� ��������� FOR
Value *LLVMDriver::WriteLLVM(CForStatement *s)
{
    //��������� ��������� For � To
    Value* vToExpr=WriteLLVM(s->ToExpr);
    Value* vForExpr=WriteLLVM(s->ForExpr);
    Value* CondV;
    //��������� ������������ ����������
    Value* store=NamedValues[s->var_name];
    if(!store) store=GlobalValues[s->var_name];
    while(store->getType()->getContainedType(0)->isPointerTy()){
        store=Builder.CreateLoad(store,store->getName());
    }
    //���������� ����� ��������� � ���� ����������
    if(store->getType()->getTypeID()==Type::PointerTyID)   {
        if(store->getType()->getContainedType(0)!=vForExpr->getType())
            vForExpr=CastToType(vForExpr,store->getType()->getContainedType(0));
    }else if(store->getType()!=vForExpr->getType())
        vForExpr=CastToType(vForExpr,store->getType());
    if(store->getType()->getTypeID()==Type::PointerTyID)   {
        if(store->getType()->getContainedType(0)!=vToExpr->getType())
            vToExpr=CastToType(vToExpr,store->getType()->getContainedType(0));
    }else if(store->getType()!=vToExpr->getType())
        vToExpr=CastToType(vToExpr,store->getType());
    //��������� ��������� For � ����������
    Builder.CreateStore(vForExpr, store);
    //�������� ��������� �� �������, ������� �����
    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    BasicBlock* ForBB = BasicBlock::Create(TheContext, "for", TheFunction);
    BasicBlock *ForBodyBB = BasicBlock::Create(TheContext, "forbody");
    BasicBlock *ExitForBB = BasicBlock::Create(TheContext, "exitfor");
    Builder.CreateBr(ForBB);
    Builder.SetInsertPoint(ForBB);
    //��������� ������� ����� � ���� �����
    Value* vStore=Builder.CreateLoad(store,store->getName());
    if(s->step > 0){
        CondV=Builder.CreateICmpSLE(vStore,vToExpr,"forcond");
    }else{
        CondV=Builder.CreateICmpSGE(vStore,vToExpr,"forcond");
    }
    Builder.CreateCondBr(CondV,ForBodyBB,ExitForBB);
    TheFunction->getBasicBlockList().push_back(ForBodyBB);
    Builder.SetInsertPoint(ForBodyBB);
    //��������� ���� ���� �����
    WriteLLVM(s->StatementSeq);
    //���������� ���� � ����������
    vStore=Builder.CreateLoad(store,store->getName());
    Value* vStep = Builder.CreateAdd(vStore,ConstantInt::get(vStore->getType(),s->step),"step");
    Builder.CreateStore(vStep, store);

    Builder.CreateBr(ForBB);
    TheFunction->getBasicBlockList().push_back(ExitForBB);
    Builder.SetInsertPoint(ExitForBB);
    return nullptr;
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ��� ��������� LOOP
Value *LLVMDriver::WriteLLVM(CLoopStatement *s)
{
    //�������� ��������� �� �������, ������� �����
    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    BasicBlock *LoopBB = BasicBlock::Create(TheContext, "loop", TheFunction);
    BasicBlock *ExitBB = BasicBlock::Create(TheContext, "exitLoop");
    //��������� ���� ������ �� ����� � ����� ������
    BasicBlocks[s->WithLoopLink.LoopUID]=ExitBB;
    Builder.CreateBr(LoopBB);
    Builder.SetInsertPoint(LoopBB);
    //��������� ���� ���� �����
    WriteLLVM(s->StatementSeq);
    Builder.CreateBr(LoopBB);
    //������� ����� ������ �� �����
    TheFunction->getBasicBlockList().push_back(ExitBB);
    Builder.SetInsertPoint(ExitBB);
    return nullptr;
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ��� ��������� CASE
Value *LLVMDriver::WriteLLVM(CCaseStatement *s)
{
    //���� ������, ���� ��� ���� �����
    BasicBlock *ExitBB = BasicBlock::Create(TheContext, "exitCase");
    //���� �������� ������������������� �����
    CCaseStatement::CaseLabelsSeqList_type::const_iterator i;
    for (i = s->CaseLabelsSeqList.begin(); i != s->CaseLabelsSeqList.end(); ++i)
    {
        WriteLLVM((*i), s->Expr,ExitBB);
    }//for
    //�������� ������� ������� ELSE
    if (s->ElseStatementSeq) {
        WriteLLVM(s->ElseStatementSeq);
    }
    Builder.CreateBr(ExitBB);
    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    TheFunction->getBasicBlockList().push_back(ExitBB);
    Builder.SetInsertPoint(ExitBB);
    return nullptr;
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM �������� ��������� CASE
Value *LLVMDriver::WriteLLVM(CCaseLabelsSeq *cls, CExpr *e, BasicBlock *exitBB)
{
    CCaseLabelsSeq::CaseLabelsList_type::const_iterator i = cls->CaseLabelsList.begin();
    //��������� ������ �����
    Value* expr = WriteLLVM((*i), e);
    //���� �������� ���������� �����
    for (++i; i != cls->CaseLabelsList.end(); ++i)
    {
        Value* expr2 = WriteLLVM((*i), e);
        expr=Builder.CreateOr(expr,expr2,"cond");
    }
    //�������� ������, ��������� ������� ����� � ���� case
    BasicBlock *caseBB = BasicBlock::Create(TheContext, "case");
    BasicBlock *elseBB = BasicBlock::Create(TheContext, "else");
    Builder.CreateCondBr(expr,caseBB,elseBB);
    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    TheFunction->getBasicBlockList().push_back(caseBB);
    Builder.SetInsertPoint(caseBB);
    //��������� ���� ������-�� ����������
    WriteLLVM(&cls->StatementSeq);
    Builder.CreateBr(exitBB);
    //������� ����� ��������� �����
    TheFunction->getBasicBlockList().push_back(elseBB);
    Builder.SetInsertPoint(elseBB);
    return nullptr;
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ����� ��������� CASE
Value *LLVMDriver::WriteLLVM(CCaseLabels *cls, CExpr *e)
{
    Value* expr = WriteLLVM(e);
    //�������� ������� ���������
    if (cls->IsRange) {
        Value* uge = Builder.CreateICmpUGE(expr,ConstantInt::get(expr->getType(),cls->ConstValue));
        Value* ule = Builder.CreateICmpULE(expr,ConstantInt::get(expr->getType(),cls->ConstHighValue));
        return Builder.CreateAnd(uge,ule);
    } else
        return Builder.CreateICmpEQ(expr,ConstantInt::get(expr->getType(),cls->ConstValue));
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ��������� Return
Value *LLVMDriver::WriteLLVM(CReturnStatement *s)
{
    //�������� ��� ������������� ��������
    Type* funRetType=Builder.getCurrentFunctionReturnType();
    //��������� ������������� ��������
    Value* retVal;
    if (s->Expr) {
        retVal=WriteLLVM(s->Expr);
    } else retVal=Constant::getNullValue(funRetType);
    retVal=CastToType(retVal,funRetType);
    Builder.CreateRet(retVal);
    //������� � ������������� �����, ��-�� ������������ LLVM
    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    BasicBlock *unreachBB = BasicBlock::Create(TheContext, "unreach");
    TheFunction->getBasicBlockList().push_back(unreachBB);
    Builder.SetInsertPoint(unreachBB);
    return nullptr;
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ��������� ������������
Value *LLVMDriver::WriteLLVM(CAssignStatement *s)
{
    ////////////////////////////////////////////////////
    //��������� ���� ���������� ��������� ������ �������
    if (s->str_to_array) {
        return WriteLLVM_array(s);
    }//if

    ////////////////////////////////////////////////////
    //��������� ���� ������������ ���������
    if(s->Designator->ResultId==id_CProcedureVar){
        Value* store=WriteLLVM(s->Designator);
        while (store->getType()->getContainedType(0)->getContainedType(0)->isPointerTy()) {
            store=Builder.CreateLoad(store,store->getName());
        }//while
        Value* expr=WriteLLVM(s->Expr);
        while (expr->getType()->getContainedType(0)->isPointerTy()) {
            expr=Builder.CreateLoad(expr,expr->getName());
        }//while
        return Builder.CreateStore(expr, store);
    }//if

    ////////////////////////////////////////////////////
    //��������� ���� ������������ ���������
    if(s->Designator->ResultId==id_CPointerVar){
        Value* store=WriteLLVM(s->Designator);
        while (store->getType()->getContainedType(0)->getContainedType(0)->isPointerTy()) {
            store=Builder.CreateLoad(store,store->getName());
        }//while
        Value* expr=WriteLLVM(s->Expr);
        if(!expr) expr=Constant::getNullValue(store->getType()->getContainedType(0));
        if (!SpecTypes.empty()){
            CBaseName* BN=s->Expr->FindLastName();
            if(BN && !SpecTypes.empty() && SpecTypes[BN->name]){
                SpecTypes[s->Designator->Qualident->ident]=SpecTypes[BN->name];
            }//if
        }//if
        return Builder.CreateStore(expr, store);
    }//if

    //////////////////////////////////////
    //��������� ���� �������� ������������
    Value* store=WriteLLVM(s->Designator);
    while (store->getType()->getContainedType(0)->isPointerTy()) {
        store=Builder.CreateLoad(store,store->getName());
    }//while
    Value* expr=CastToType(WriteLLVM(s->Expr),store->getType()->getContainedType(0));
    while (expr->getType()->isPointerTy()) {
        expr=Builder.CreateLoad(expr,expr->getName());
    }//while
    return Builder.CreateStore(expr, store);
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM � ������ ���������� ��������� ������ �������
Value *LLVMDriver::WriteLLVM_array(CAssignStatement *s)
{
    //������������ ������� COPY �� ���������������� ����� _O2M_sys.cpp
    std::string name="_Z4COPYPKciPci";
    Function *F=Functions[name];
    if(!F){
        Type *FunRetType=Type::getVoidTy(TheContext);
        std::vector<Type *> pars={Type::getInt8PtrTy(TheContext),
                                  Type::getInt32Ty(TheContext),
                                  Type::getInt8PtrTy(TheContext),
                                  Type::getInt32Ty(TheContext)};
        StringVector emptyVector;
        F=createFunction(FunRetType,pars,emptyVector,name);
        Functions[name]=F;
    }//if
    //��������� �������� ���������
    Value* string1 = WriteLLVM(s->Expr);
    CBaseName* BN = s->Expr->FindLastName();
    Value* size1 = WriteLLVM_COPY_Par(BN);
    //��������� �������� ���������
    Value* string2 = WriteLLVM(s->Designator);
    BN = s->Designator->FindLastName();
    Value* size2 = WriteLLVM_COPY_Par(BN);
    //���� ��������� �� ������, ������� ���� ������
    while(string2->getType()->getContainedType(0)->isArrayTy()){
        std::vector<Value*> values;
        values.push_back(Constant::getNullValue(Type::getInt32Ty(TheContext)));
        values.push_back(Constant::getNullValue(Type::getInt32Ty(TheContext)));
        string2=Builder.CreateGEP(string2,values);
    }//while
    //����� ������� COPY
    std::vector<Value *> values={string1,size1,string2,size2};
    return Builder.CreateCall(F,values);
}//WriteLLVM_array

//-----------------------------------------------------------------------------
//��������� ����������� ������� � �������� ��������� ��� ������ COPY
//��������������, ��� BN �������� �������� �������� ��� ��. �� ������ ��������
Value *LLVMDriver::WriteLLVM_COPY_Par(CBaseName *bn)
{
    if (id_CPointerVar == bn->name_id) bn = static_cast<CPointerVar*>(bn)->FindType();
    //��������� ������� ��� ���������� ��� ����
    int arr_size;
    if (id_CArrayVar == bn->name_id)
        arr_size = static_cast<CArrayVar*>(bn)->ArrayType->size;
    else
        arr_size = static_cast<CArrayType*>(bn)->size;
    return ConstantInt::get(Type::getInt32Ty(TheContext),arr_size);
}//WriteLLVM_COPY_Par

//-----------------------------------------------------------------------------
//��������� ���� LLVM ��������� EXIT
Value *LLVMDriver::WriteLLVM(CExitStatement *s)
{
    //��������� ������ �� ����� LOOP
    Builder.CreateBr(BasicBlocks[s->UID]);
    //�������� ������������� ����� � ������� � ����
    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    BasicBlock *unreachBB = BasicBlock::Create(TheContext, "unreach");
    TheFunction->getBasicBlockList().push_back(unreachBB);
    Builder.SetInsertPoint(unreachBB);
    return nullptr;
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ��������� ������ ���������
Value *LLVMDriver::WriteLLVM(CCallStatement *s)
{
    //��������� ��. �� CProcedure ��� CProcedureVar
    CBaseName *BN = s->Designator->FindLastName();
    CFormalPars* FP;
    if (CProcedure::IsProcId(BN->name_id)){
        CProcedure* BV = static_cast<CProcedure*>(BN);
        FP=BV->FormalPars;
        //��������� ���������� �� �������
        Function* F=GetFunction(BV);
        //������������ ������ ����������, ���� ����
        std::vector<Value *> ArgsV;
        if (s->ExprList) {
            ArgsV = WriteLLVM(s->ExprList,FP);
        }//if
        //������������ ������ ���������� ����������, ���� ����
        if (s->CommonList) {
            std::vector<Value *> ArgsVC = WriteLLVM(s->CommonList,static_cast<CCommonProc*>(BV)->CommonPars);
            ArgsV.insert(ArgsV.end(),ArgsVC.begin(),ArgsVC.end());
        }//if
        //���� ���� ��������, ��������� ��� � ���������
        if(BV->Receiver){
            ArgsV.push_back(WriteLLVM(s->Designator));
        }//if
        //����� �������
        return Builder.CreateCall(F,ArgsV,"calltmp");
    } else if (id_CProcedureVar == BN->name_id){
        //������ ���������� ���� PROCEDURE
        FP = &static_cast<CProcedureVar*>(BN)->FormalPars;
        //�������� ��������� �� ���������� ���� PRCEDURE
        Value* d = Builder.CreateLoad(WriteLLVM(s->Designator),BN->name);
        //��������� ���� �������
        FunctionType* ft= static_cast<FunctionType*>(d->getType()->getContainedType(0));
        //������������ ������ ����������, ���� ����
        std::vector<Value *> ArgsV;
        if (s->ExprList) {
            ArgsV = WriteLLVM(s->ExprList,FP);
        }//if
        ////!�������� ��������� CommonList � Receiver!
        //����� �������
        return Builder.CreateCall(ft,d,ArgsV,"calltmp");;
    } else{
        ////!�������� ��������� CProcedureType!
        return nullptr;
    }//if
}//WriteLLVM

//-----------------------------------------------------------------------------
//������������ ������ ����������� ���������� �������
std::vector<Value *> LLVMDriver::WriteLLVM(CExprList *e,CFormalPars* fp)
{
    std::vector<Value *> ArgsV;
    CBaseVarVector::const_iterator ci = fp->FPStore.begin();
    for(CExprVector::const_iterator i = e->ExprVector->begin(); i != e->ExprVector->end(); ++i, ++ci) {
        Value* value;
        //���� �������� ����������, ����� ���������� ��������� �� ����
        if((*ci)->is_var){
            value = WriteLLVM((*i)->SimpleExpr1->Term->Factor->Designator);
        }else value=WriteLLVM((*i));
        //���� ��� �� ����������-��������� - ����� ������� �� ������� ���������
        while (value->getType()->getNumContainedTypes()>0&&value->getType()->getContainedType(0)->isPointerTy()
               &&((*ci)->name_id!=id_CPointerVar or value->getType()->getContainedType(0)->getContainedType(0)->isPointerTy())) {
            value=Builder.CreateLoad(value,value->getName());
        }//while
        //���� ��������� �� ������, �������� ��� ������
        while(value->getType()->getNumContainedTypes()>0&&value->getType()->getContainedType(0)->isArrayTy()){
            std::vector<Value*> values;
            values.push_back(Constant::getNullValue(Type::getInt32Ty(TheContext)));
            values.push_back(Constant::getNullValue(Type::getInt32Ty(TheContext)));
            value=Builder.CreateGEP(value,values);
        }//while
        ArgsV.push_back(CastToType(value,GetLLVMType((*ci)->name_id)));
        //��� ������� ����� ����� �������� � ������ ������� �������
        if(id_CArrayVar == (*ci)->name_id){
            CArrayVar* AV=static_cast<CArrayVar*>((*i)->FindLastName());
            CArrayType* AT=static_cast<CArrayVar*>(AV)->ArrayType;
            int dimention = 0;
            while (AT->name_id == id_CArrayType) {
                if(AT->size>0)
                    ArgsV.push_back(ConstantInt::get(Type::getInt32Ty(TheContext),AT->size));
                else{
                    std::string s="O2M_ARR_"+std::to_string(dimention)+"_"+AV->name;
                    ArgsV.push_back(Builder.CreateLoad(NamedValues[s],s));
                }//if
                ++dimention;
                AT = static_cast<CArrayType*>(AT->Type);
            }//while
        }//if
    }//for
    return ArgsV;
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ���������
Value *LLVMDriver::WriteLLVM(CExpr *e)
{
    //��������� 1-�� �������� ���������
    Value* simpleExpr1 = CastToType(WriteLLVM(e->SimpleExpr1),GetLLVMType(e->SimpleExpr1->GetResultId()));
    //��������� 2-�� �������� ��������� (���� ����)
    Value* simpleExpr2 =nullptr;
    if (e->SimpleExpr2) {
        simpleExpr2 = CastToType(WriteLLVM(e->SimpleExpr2),GetLLVMType(e->SimpleExpr2->GetResultId()));
        /* �����������, ��������������
        ���� expr2 - ������� ���������, �������� ��� � ���� expr1*/
        if(!simpleExpr2)
            simpleExpr2 = ConstantPointerNull::get(simpleExpr1->getType()->getContainedType(0)->getPointerTo());
        //���������� �����
        if(IsId1IncloseId2(e->SimpleExpr1->GetResultId(),e->SimpleExpr2->GetResultId()))
            simpleExpr2 = CastToType(simpleExpr2,simpleExpr1->getType());
        else simpleExpr1 = CastToType(simpleExpr1,simpleExpr2->getType());
    }//if
    //���������� �������� (���� ����)
    switch (e->Relation) {
    case rel_EQ:
        if(simpleExpr1->getType()==Type::getDoubleTy(TheContext))
            return Builder.CreateFCmpOEQ(simpleExpr1, simpleExpr2, "eqtmp");
        return Builder.CreateICmpEQ(simpleExpr1, simpleExpr2, "eqtmp");
    case rel_NE:
        if(simpleExpr1->getType()==Type::getDoubleTy(TheContext))
            return Builder.CreateFCmpONE(simpleExpr1, simpleExpr2, "netmp");
        return Builder.CreateICmpNE(simpleExpr1, simpleExpr2, "netmp");
    case rel_LT:
        if(simpleExpr1->getType()==Type::getDoubleTy(TheContext))
            return Builder.CreateFCmpOLT(simpleExpr1, simpleExpr2, "lttmp");
        return Builder.CreateICmpSLT(simpleExpr1, simpleExpr2, "lttmp");
    case rel_LE:
        if(simpleExpr1->getType()==Type::getDoubleTy(TheContext))
            return Builder.CreateFCmpOLE(simpleExpr1, simpleExpr2, "letmp");
        return Builder.CreateICmpSLE(simpleExpr1, simpleExpr2, "letmp");
    case rel_GT:
        if(simpleExpr1->getType()==Type::getDoubleTy(TheContext))
            return Builder.CreateFCmpOGT(simpleExpr1, simpleExpr2, "gttmp");
        return Builder.CreateICmpSGT(simpleExpr1, simpleExpr2, "gttmp");
    case rel_GE:
        if(simpleExpr1->getType()==Type::getDoubleTy(TheContext))
            return Builder.CreateFCmpOGE(simpleExpr1, simpleExpr2, "getmp");
        return Builder.CreateICmpSGE(simpleExpr1, simpleExpr2, "getmp");
    }//switch
    return  simpleExpr1;
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM �������� ���������
Value *LLVMDriver::WriteLLVM(CSimpleExpr *e)
{
    //��������� ������� Term
    Value* term = CastToType(WriteLLVM(e->Term),GetLLVMType(e->Term->GetResultId()));
    //��������� ��������� (���� ������������)
    if (e->negative) {
        if(term->getType()==Type::getDoubleTy(TheContext))
            term=Builder.CreateFNeg(term);
        else term=Builder.CreateNeg(term);
    }//if
    //��������� ��������� Term'�� (���� ����)
    if (e->SimpleExprPairStore) {
        CBaseVector::const_iterator ci;
        for (ci = e->SimpleExprPairStore->begin(); ci != e->SimpleExprPairStore->end(); ++ci){
            CSimpleExprPair* sep = static_cast<CSimpleExprPair*>(*ci);
            Value* simplePair=CastToType(WriteLLVM(sep->Term),GetLLVMType(sep->GetResultId()));
            //���������� �����
            if(IsId1IncloseId2(e->Term->GetResultId(),sep->GetResultId()))
                simplePair = CastToType(simplePair,term->getType());
            else term = CastToType(term,simplePair->getType());
            //���������� ��������
            switch (sep->AddOp) {
            case aop_ADD:
                if(term->getType()==Type::getDoubleTy(TheContext))
                    term=Builder.CreateFAdd(term, simplePair, "addtmp");
                else
                    term=Builder.CreateAdd(term, simplePair, "addtmp");
                break;
            case aop_SUB:
                if(term->getType()==Type::getDoubleTy(TheContext))
                    term=Builder.CreateFSub(term, simplePair, "subtmp");
                else
                  term=Builder.CreateSub(term, simplePair, "subtmp");
                break;
            case aop_OR:
                term=Builder.CreateOr(term, simplePair, "ortmp");
                break;
            default:
                break;
            }//switch
        }//for
    }//if
    return term;
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ��� CTerm
Value *LLVMDriver::WriteLLVM(CTerm *t)
{
    //��������� ������� ���������
    Value* factor = CastToType(WriteLLVM(t->Factor),GetLLVMType(t->Factor->GetResultId()));
    if (t->TermPairStore) {
        //������ ���������� ���������� (���� ����)
        CBaseVector::const_iterator ci;
        for (ci = t->TermPairStore->begin(); ci != t->TermPairStore->end(); ++ci) {
            CTermPair* tp=static_cast<CTermPair*>(*ci);
            //��������� ������� �������� ��� ������� �������
            Value* termPair = CastToType(WriteLLVM(tp->Factor),GetLLVMType(tp->GetResultId()));
            //���������� �����
            if(IsId1IncloseId2(t->Factor->GetResultId(),tp->GetResultId()))
                termPair = CastToType(termPair,factor->getType());
            else factor = CastToType(factor,termPair->getType());
            //���������� ��������
            switch (tp->MulOp) {
            case mop_DIV:
                factor=Builder.CreateSDiv(factor, termPair, "quotmp");
                break;
            case mop_MOD:
                factor=Builder.CreateSRem(factor, termPair, "remtmp");
                break;
            case mop_M:
                if(factor->getType()==Type::getDoubleTy(TheContext))
                    factor=Builder.CreateFMul(factor, termPair, "multmp");
                else factor=Builder.CreateMul(factor, termPair, "multmp");
                break;
            case mop_D:
                if(factor->getType()!=Type::getDoubleTy(TheContext)){
                    termPair = CastToType(termPair,Type::getDoubleTy(TheContext));
                    factor = CastToType(factor,Type::getDoubleTy(TheContext));
                }
                factor=Builder.CreateFDiv(factor, termPair, "divtmp");
                break;
            case mop_AND:
                factor=Builder.CreateAnd(factor, termPair, "andtmp");
                break;
            default:
                break;
            }//switch
        }//for
    }//if
    return factor;
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ��� CFactor
Value *LLVMDriver::WriteLLVM(CFactor *f)
{
    switch (f->FactorKind) {
    case fk_Negation:{
        Value* factor=WriteLLVM(f->Factor);
        if (factor->getType()==Type::getDoubleTy(TheContext))
            return Builder.CreateFCmpOEQ(factor,Constant::getNullValue(factor->getType()),"neg");
        return Builder.CreateICmpEQ(factor,Constant::getNullValue(factor->getType()),"neg");
    } case fk_Expr:
        return WriteLLVM(f->Expr);
    case fk_ConstVar:
        return WriteLLVM_ConstValue(f->ConstVar);
    case fk_StdProcFunc:
        //����� � ��������� ���� ���������� ����������� ���������
        if(typeid(*f->StdProcFunc) == typeid (CAbsStdProcFunc) )
            return WriteLLVM(static_cast<CAbsStdProcFunc*>(f->StdProcFunc));
        if(typeid(*f->StdProcFunc) == typeid (CAshStdProcFunc) )
            return WriteLLVM(static_cast<CAshStdProcFunc*>(f->StdProcFunc));
        if(typeid(*f->StdProcFunc) == typeid (CCapStdProcFunc) )
            return WriteLLVM(static_cast<CCapStdProcFunc*>(f->StdProcFunc));
        if(typeid(*f->StdProcFunc) == typeid (CChrStdProcFunc) )
            return WriteLLVM(static_cast<CChrStdProcFunc*>(f->StdProcFunc));
        if(typeid(*f->StdProcFunc) == typeid (CEntierStdProcFunc) )
            return WriteLLVM(static_cast<CEntierStdProcFunc*>(f->StdProcFunc));
        if(typeid(*f->StdProcFunc) == typeid (CLenStdProcFunc) )
            return WriteLLVM(static_cast<CLenStdProcFunc*>(f->StdProcFunc));
        if(typeid(*f->StdProcFunc) == typeid (CLongStdProcFunc) )
            return WriteLLVM(static_cast<CLongStdProcFunc*>(f->StdProcFunc));
        if(typeid(*f->StdProcFunc) == typeid (CMaxStdProcFunc) )
            return WriteLLVM(static_cast<CMaxStdProcFunc*>(f->StdProcFunc));
        if(typeid(*f->StdProcFunc) == typeid (CMinStdProcFunc) )
            return WriteLLVM(static_cast<CMinStdProcFunc*>(f->StdProcFunc));
        if(typeid(*f->StdProcFunc) == typeid (COddStdProcFunc) )
            return WriteLLVM(static_cast<COddStdProcFunc*>(f->StdProcFunc));
        if(typeid(*f->StdProcFunc) == typeid (COrdStdProcFunc) )
            return WriteLLVM(static_cast<COrdStdProcFunc*>(f->StdProcFunc));
        if(typeid(*f->StdProcFunc) == typeid (CShortStdProcFunc) )
            return WriteLLVM(static_cast<CShortStdProcFunc*>(f->StdProcFunc));
        if(typeid(*f->StdProcFunc) == typeid (CSizeStdProcFunc) )
            return WriteLLVM(static_cast<CSizeStdProcFunc*>(f->StdProcFunc));
    case fk_Designator:
        //�������� ������� ������ ��������� ������� (���� ��� - ������ ���� �����������)
        if (f->Call)
            return WriteLLVM(f->Call);
        else{
            Value* designator = WriteLLVM(f->Designator);
            //��� ������� ����������, ���������� �� ��������, � �� ��������� �� ���
            //(����� ��������� �� ������� ������� ��� � CastToType)
            while (f->GetResultId()!=id_CArrayVar && f->GetResultId()!=id_CProcedureVar
                   && f->GetResultId()!=id_CProcedure && designator->getType()->isPointerTy()
                   && !designator->getType()->getContainedType(0)->isStructTy()) {
                designator=Builder.CreateLoad(designator,designator->getName());
            }
            return designator;
        }
    }//switch
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ��� CDesignator
Value *LLVMDriver::WriteLLVM(CDesignator *d)
{
    //������� ���� ����������� � ��������� ����������, ����� � ����������
    Value* designator=NamedValues[d->Qualident->ident];
    if(!designator) designator=GlobalValues[d->Qualident->ident];
    //���� �� �����, �������� ��� ���������
    if(!designator) {
        CBaseName* BN = d->FindLastName();
        if(BN->name_id==id_CProcedure)
            designator=GetFunction(static_cast<CProcedure*>(BN));
    }//if
    //���� ���, ������� ���������� ����������
    if(!designator){
        const CBaseName* BN = d->parent_element;
        while(BN->name_id!=id_CModule){
            BN=BN->parent_element;
        }//while
        CBaseVar* BV=static_cast<CBaseVar*>(BN->GetGlobalName(d->Qualident->pref_ident,d->Qualident->ident));
        std::string firstName=BV->parent_element->name;
        std::string lastName=BV->name;
        std::string name="_ZN"+std::to_string(firstName.size())+firstName+std::to_string(lastName.size())+lastName+"E";
        GlobalVariable *gVar= new GlobalVariable(*TheModule.get(),GetLLVMType(BV),false,GlobalValue::ExternalLinkage,nullptr,name);
        GlobalValues[lastName]=gVar;
        designator=gVar;
    }//if
    //��������� �������� �� ��-��� ����������� (��������)
    for (CDesignator::SDesElemStore::const_iterator ci = d->DesElemStore.begin(); ci != d->DesElemStore.end(); ci++) {
        switch ((*ci)->DesKind) {
        case CDesignator::EDesKind::dk_Array:       
        case CDesignator::EDesKind::dk_OpenArray:
            designator=WriteLLVM_index((*ci)->ExprList, designator);
            break;
        case CDesignator::EDesKind::dk_Pointer:
        case CDesignator::EDesKind::dk_Record:
            designator=WriteLLVM_record_index(designator,(*ci)->ident,d);
            break;        
        default:
            break;
        }//switch
    }//for
    return designator;
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� �������� ������� �� ��������
Value *LLVMDriver::WriteLLVM_index(CExprList *e, Value* array)
{
    std::vector<Value*> values;
    //�������� ������ - ��� ������ ���� ��������� �� ��� ������
    bool isCloseArray = array->getType()->isPointerTy() && array->getType()->getContainedType(0)->isArrayTy();
    //��� ��������� ������������ �������������� �������� �������
    if(isCloseArray){
        values.push_back(Constant::getNullValue(Type::getInt32Ty(TheContext)));
    }//if
    //������ �������� �������
    for (CExprVector::const_iterator ci = e->ExprVector->begin(); ci != e->ExprVector->end(); ++ci) {
        Value* value=WriteLLVM((*ci));
        value=CastToType(value,Type::getInt32Ty(TheContext));
        values.push_back(value);
    }//for
    //������ �������� � ��������� � ��������� ������� �������������� ������� ���������
    if(!isCloseArray){
        array=Builder.CreateLoad(array,array->getName());
        return Builder.CreateInBoundsGEP(array,values,"gep");
    }//if
    return Builder.CreateGEP(array,values,"gep");
}//WriteLLVM_index

//-----------------------------------------------------------------------------
//��������� �������� �������� ������ �� ��������
Value *LLVMDriver::WriteLLVM_record_index(Value *record, char *ident,CDesignator *d)
{
    CBaseVar* BN = static_cast<CBaseVar*>(d->parent_element->GetGlobalName(d->Qualident->pref_ident, d->Qualident->ident));
    while(record->getType()->getContainedType(0)->isPointerTy()){
        record=Builder.CreateLoad(record,record->getName());
    }//while
    const char* parentIdent;
    if(BN->name_id==id_CRecordVar)
        parentIdent= static_cast<CRecordVar*>(BN)->GetTypeName();
    else if(BN->name_id==id_CCommonVar) parentIdent=static_cast<CCommonVar*>(BN)->SpecName;
    else{
        CPointerVar* PV = static_cast<CPointerVar*>(BN);
        CBaseType* BT = PV->FindType();
        if(BT->name_id==id_CRecordType){
            parentIdent=BT->name;
        }else{
            CSpecType* ST = static_cast<CSpecType*>(BT);
            parentIdent = ST->GetSpecName();
            if(!TheModule->getTypeByName(parentIdent)){
                CCommonType* CT = static_cast<CCommonType*>(ST->parent_element->GetGlobalName(ST->Qualident->pref_ident, ST->Qualident->ident));
                const CCommonType::SSpec* SS = CT->FindSpec(ST->GetQualSpecName(),ST->GetSpecName(),ST->GetSpecName());
                parentIdent = SS->Name;
            }//if
            record=Builder.CreateStructGEP(record,0,"gep");
            while(record->getType()->getContainedType(0)->isPointerTy()){
                record=Builder.CreateLoad(record,record->getName());
            }//while
            Type* type=TheModule->getTypeByName(parentIdent)->getPointerTo();
            record=Builder.CreateBitCast(record,type,"bitcast");
        }//if
    }//if
    if(Structures[parentIdent].find(ident) == Structures[parentIdent].end()) return record;
    return Builder.CreateStructGEP(record,Structures[parentIdent][ident],"gep");
}//WriteLLVM_record_index

//-----------------------------------------------------------------------------
//��������� ���� LLVM ���������-������� ABS
Value *LLVMDriver::WriteLLVM(CAbsStdProcFunc *d)
{
    //����� ����������� ������� � (� ����������� �� ���� ���������)
    std::string name;
    Type* type;
    if (id_CLongintVar == d->GetResultId()) {
        name="labs";
        type=GetLLVMType(id_CLongintVar);
    }else if (CBaseVar::IsRealId(d->GetResultId())) {
        name="fabs";
        type=GetLLVMType(id_CRealVar);
    }else{
        name="abs";
        type=GetLLVMType(id_CIntegerVar);
    }//if
    Function *F=Functions[name];
    if(!F){
        std::vector<Type *> pars(1, type);
        StringVector emptyVector;
        F = createFunction(type,pars,emptyVector,name);
        Functions[name]=F;
    }//if
    Value* expr=WriteLLVM(&d->Expr);
    return Builder.CreateCall(F, expr, "calltmp");
}

//-----------------------------------------------------------------------------
//��������� ���� LLVM ���������-������� ASH
Value *LLVMDriver::WriteLLVM(CAshStdProcFunc *d)
{
    Value* exprX = WriteLLVM(&d->ExprX);
    Value* exprN = WriteLLVM(&d->ExprN);
    if(IsId1IncloseId2((&d->ExprX)->GetResultId(),(&d->ExprN)->GetResultId()))
        exprN = CastToType(exprN,exprX->getType());
    else exprX = CastToType(exprX,exprN->getType());
    return Builder.CreateShl(exprX,exprN,"ASH");
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ���������-������� CAP
Value *LLVMDriver::WriteLLVM(CCapStdProcFunc *d)
{
    std::string name="toupper";
    Function *F=Functions[name];
    if(!F){
        Type* reType=Type::getInt32Ty(TheContext);
        std::vector<Type *> pars(1, Type::getInt32Ty(TheContext));
        StringVector emptyVector;
        F = createFunction(reType,pars,emptyVector,name);
        Functions[name]=F;
    }//if
    Value* exp=WriteLLVM(&d->Expr);
    exp=CastToType(exp,Type::getInt32Ty(TheContext));
    return Builder.CreateCall(F,exp,"cap");
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ���������-������� CHR
Value *LLVMDriver::WriteLLVM(CChrStdProcFunc *d)
{
    return WriteLLVM(&d->Expr);
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ���������-������� ENTIER
Value *LLVMDriver::WriteLLVM(CEntierStdProcFunc *d)
{
    return CastToType(WriteLLVM(&d->Expr),Type::getInt32Ty(TheContext));
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ��������� LEN
Value *LLVMDriver::WriteLLVM(CLenStdProcFunc *d)
{
    //�������� ���� ������� (�������� / �������)
    if (!d->array_size){
        std::string name="O2M_ARR_"+std::to_string(d->dimension)+"_"+d->array_name;
        return Builder.CreateLoad(NamedValues[name],name);
    }
    return ConstantInt::get(Type::getInt32Ty(TheContext),d->array_size);
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ���������-������� LONG
Value *LLVMDriver::WriteLLVM(CLongStdProcFunc *d)
{
    return WriteLLVM(&d->Expr);
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ���������-������� MAX
Value *LLVMDriver::WriteLLVM(CMaxStdProcFunc *d){
    switch (d->GetResultId()) {
    case id_CBooleanVar:
        return ConstantInt::get(GetLLVMType(d->GetResultId()),1);
    case id_CCharVar:
        return ConstantInt::get(GetLLVMType(d->GetResultId()),255);
    case id_CShortintVar:
        return ConstantInt::get(GetLLVMType(d->GetResultId()),SHRT_MAX);
    case id_CIntegerVar:
        //����� ����� SET � INTEGER
        if (d->AppliedToSET)
            return ConstantInt::get(GetLLVMType(d->GetResultId()),SET_MAX);
        else
            return ConstantInt::get(GetLLVMType(d->GetResultId()),INT_MAX);
    case id_CLongintVar:
        return ConstantInt::get(GetLLVMType(d->GetResultId()),LONG_MAX);
    case id_CRealVar:
    case id_CLongrealVar:
        //double �� ������� ��������� � long double
        return ConstantFP::get(GetLLVMType(d->GetResultId()),DBL_MAX);
    default:
        return nullptr;
    }//switch
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ���������-������� MIN
Value *LLVMDriver::WriteLLVM(CMinStdProcFunc *d){
    switch (d->GetResultId()) {
    case id_CBooleanVar:
    case id_CCharVar:
        return Constant::getNullValue(GetLLVMType(d->GetResultId()));
    case id_CShortintVar:
        return ConstantInt::get(GetLLVMType(d->GetResultId()),SHRT_MIN);
    case id_CIntegerVar:
        //����� ����� SET � INTEGER
        if (d->AppliedToSET)
            return Constant::getNullValue(GetLLVMType(d->GetResultId()));
        else
            return ConstantInt::get(GetLLVMType(d->GetResultId()),INT_MIN);
    case id_CLongintVar:
        return ConstantInt::get(GetLLVMType(d->GetResultId()),LONG_MIN);
    case id_CRealVar:
    case id_CLongrealVar:
        //double �� ������� ��������� � long double
        return ConstantFP::get(GetLLVMType(d->GetResultId()),DBL_MIN);
    default:
        return nullptr;
    }//switch
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ���������-������� ODD
Value *LLVMDriver::WriteLLVM(COddStdProcFunc *d)
{
    Value* expr = WriteLLVM(&d->Expr);
    Value* rem = Builder.CreateSRem(expr,ConstantInt::get(expr->getType(),2));
    return Builder.CreateICmpEQ(rem,ConstantInt::get(rem->getType(),1));
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ���������-������� ORD
Value *LLVMDriver::WriteLLVM(COrdStdProcFunc *d)
{
    //����� ����������� ������� �� ���������������� ����� _O2M_sys.cpp
    //(��� ������� ����� ������� ������������� ��� �������)
    std::string name="_Z3ORDc";
    Function *F=Functions[name];
    if(!F){
        Type *FunRetType=Type::getInt32Ty(TheContext);
        std::vector<Type *> pars={Type::getInt8Ty(TheContext)};
        StringVector emptyVector;
        F=createFunction(FunRetType,pars,emptyVector,name);
        Functions[name]=F;
    }//if
    Value* exp=WriteLLVM(&d->Expr);
    exp=CastToType(exp,Type::getInt8Ty(TheContext));
    return Builder.CreateCall(F,exp,"ORD");
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ���������-������� SHORT
Value *LLVMDriver::WriteLLVM(CShortStdProcFunc *d)
{
    return WriteLLVM(&d->Expr);
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ���������-������� SIZE
Value *LLVMDriver::WriteLLVM(CSizeStdProcFunc *d)
{
    return llvm::ConstantInt::get(llvm::Type::getInt64Ty(TheContext), TheModule->getDataLayout().getTypeAllocSize(GetLLVMType(d->Qualident.TypeResultId)));
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ��������� DEC
Value *LLVMDriver::WriteLLVM(CDecStdProc *d)
{
    //��������� ���������� (������� ���������)
    Value* store=WriteLLVM(d->Expr1.SimpleExpr1->Term->Factor->Designator);
    while(store->getType()->getContainedType(0)->isPointerTy()){
        store=Builder.CreateLoad(store,store->getName());
    }//while
    Value* expr=Builder.CreateLoad(store,store->getName());
    Value* dec=nullptr;
    //��������� ������� ��������� ��� �������� ��������� ���������
    if (d->Expr2) dec=CastToType(WriteLLVM(d->Expr2),expr->getType());
    else dec=ConstantInt::get(expr->getType(),1);
    //���������� �������� � ����������
    expr=Builder.CreateSub(expr,dec,"dec");
    return Builder.CreateStore(expr,store);
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ��������� INC
Value *LLVMDriver::WriteLLVM(CIncStdProc *d)
{
    //��������� ���������� (������� ���������)
    Value* store=WriteLLVM(d->Expr1.SimpleExpr1->Term->Factor->Designator);
    while(store->getType()->getContainedType(0)->isPointerTy()){
        store=Builder.CreateLoad(store,store->getName());
    }//while
    Value* expr=Builder.CreateLoad(store,store->getName());
    Value* inc=nullptr;
    //��������� ������� ��������� ��� �������� ��������� ���������
    if (d->Expr2) inc=CastToType(WriteLLVM(d->Expr2),expr->getType());
    else inc=ConstantInt::get(expr->getType(),1);
    //���������� �������� � ����������
    expr=Builder.CreateAdd(expr,inc,"inc");
    return Builder.CreateStore(expr,store);
}//WriteLLVM

//-----------------------------------------------------------------------------
//��������� ���� LLVM ��������� NEW
Value *LLVMDriver::WriteLLVM(CNewStdProc *d)
{
    //��������� ���������� �� �����������
    CPointerVar* pVar = static_cast<CPointerVar*>(d->Des.FindLastName());
    //��������� ���� �� ����������
    CBaseType* BT = pVar->FindType();
    //��������� ����������
    Value* des = WriteLLVM(&(d->Des));
    Type* type = des->getType();
    //��������� ��������� �� ������� ���
    while(type->isPointerTy()&&type->getContainedType(0)->isPointerTy()&&type->getContainedType(0)->getContainedType(0)->isPointerTy()){
        des = Builder.CreateLoad(des,des->getName());
        type = des->getType();
    }//while
    type = type->getContainedType(0);
    //�������� ����������� ������� new
    std::string name="_Znwy";
    Function *F=Functions[name];
    if(!F){
        Type* reType=Type::getInt8PtrTy(TheContext);
        std::vector<Type *> pars(1, Type::getInt64Ty(TheContext));
        StringVector emptyVector;
        F = createFunction(reType,pars,emptyVector,name);
        Functions[name]=F;
    }
    //����� ������� new
    Value* call = Builder.CreateCall(F, ConstantInt::get(Type::getInt64Ty(TheContext),32), "new");
    //���������� ���������� � �������� ����
    call=Builder.CreateBitCast(call,type,"bitcast");
    //��� ������������� ����� ����� �������� ������ ��� ���� �������������
    if (id_CSpecType == BT->name_id){
        //��������� ������ ��� ��� �������������
        Value* val=Builder.CreateStructGEP(call,0,"val");
        type=val->getType()->getContainedType(0);
        Value* call2 = Builder.CreateCall(F, ConstantInt::get(Type::getInt64Ty(TheContext),32), "new");
        call2=Builder.CreateBitCast(call2,type,"bitcast2");
        Builder.CreateStore(call2,val);
        //�� ������ ��������� ��������� ������������� �������������
        Value* spec=Builder.CreateStructGEP(call,1,"val");
        CSpecType* ST=static_cast<CSpecType*>(BT);
        std::string specname=std::string(ST->GetSpecName());
        if(!TheModule->getTypeByName(specname)){
            CCommonType* CT = static_cast<CCommonType*>(ST->parent_element->GetGlobalName(ST->Qualident->pref_ident, ST->Qualident->ident));
            const CCommonType::SSpec* SS = CT->FindSpec(ST->GetQualSpecName(),ST->GetSpecName(),ST->GetSpecName());
            specname = SS->Name;
        }//if
        Builder.CreateStore(ConstantInt::get(Type::getInt32Ty(TheContext),SpecTypes[specname]),spec);
    }//if
    return Builder.CreateStore(call,des);
}//WriteLLVM


