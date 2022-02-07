#include "llvmdriver.h"

//-----------------------------------------------------------------------------
//Создание инструкции выделения памяти в начальном блоке функции. Используется для переменных и тд
AllocaInst *LLVMDriver::CreateEntryBlockAlloca(Function *TheFunction,
                                          std::string name,Type* type) {
  IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                 TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(type, 0,name);
}//CreateEntryBlockAlloca

//-----------------------------------------------------------------------------
//Функция приведения типов
Value *LLVMDriver::CastToType(Value* v, Type* destType) {
  //В случае v=Nullptr, возвращается либо Nullptr, либо NullValue типа destType
  if(!v){
    if(!destType) return v;
    return Constant::getNullValue(destType);
  }
  Type* valueType = v->getType();
  //для указателей, массивов приведение типа не производится
  if (valueType->isPointerTy()||valueType->isArrayTy()) return v;
  if(v->getType() == destType) return v;
  //приведение к типу Double
  if(destType==Type::getDoubleTy(TheContext)){
      return Builder.CreateSIToFP(v,destType,"castTmp");
  }
  //приведение к целому типу размером 1 бит производится через сравнение с нулем
  if(destType==Type::getInt1Ty(TheContext)){
      if(valueType == Type::getDoubleTy(TheContext))
          return Builder.CreateFCmpONE(v,ConstantFP::get(valueType,0.0),"castTmp");
      else return Builder.CreateICmpNE(v,ConstantInt::get(valueType,0),"castTmp");
  }
  //приведение к целому типу размером > 1 бит
  if(valueType == Type::getDoubleTy(TheContext))
      return Builder.CreateFPToSI(v,destType,"castTmp");
  return Builder.CreateIntCast(v,destType,true,"castTmp");
}//CastToType

//-----------------------------------------------------------------------------
//Создание функции без аргументов
Function *LLVMDriver::createFunction(Type *retType, std::string name)
{
    FunctionType *FT = FunctionType::get(retType,  false);
    return Function::Create(FT, Function::ExternalLinkage, name, TheModule.get());
}//createFunction

//-----------------------------------------------------------------------------
//Создание функции с аргументами
Function *LLVMDriver::createFunction(Type *retType, std::vector<Type *> typePars, StringVector namePars, std::string name)
{
    //получение типа функции
    FunctionType *FT = FunctionType::get(retType, typePars, false);
    //создание функции
    Function *F = Function::Create(FT, Function::ExternalLinkage, name, TheModule.get());
    //именование аргументов функции
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
//получения типа LLVM для простых типов
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
//получения типа LLVM на основе наследников класса CBaseVar
Type *LLVMDriver::GetLLVMType(CBaseVar *v)
{
    //отказ от карты типов вызван тем, что в каждом модуле приходится формировать ее заново
    //в процессе объявления типов и формальных параметров, в некоторых случаях это больше запутывает,
    //чем помогает. Также у модуля LLVM есть своя карта имменнованных типов
    /*if(!(v->name_id==id_CCommonVar)&&!Types.empty()&&Types[v->GetTypeName()]) {
        return Types[v->GetTypeName()];
    }*/
    switch (v->name_id) {
    default:
        //для простых типов вызывается соответствующий метод
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
        //В случае указателя на указатель возвращается первый указатель
        //??????возможно нужно изменить??????
        if(type->isPointerTy())
            return type;
        return type->getPointerTo();
    }
    case id_CRecordVar:{
        //RECORD
        CRecordVar* RV = static_cast<CRecordVar*>(v);
        //Если тип уже создан - возвращаем
        StructType* ST=TheModule->getTypeByName(RV->GetTypeName());
        if(ST){
           return ST;
        }//if
        //создание типа
        ST=StructType::create(TheContext,RV->GetTypeName());
        std::vector<Type*> colTypes;
        CBaseVarVector::const_iterator ci;
        int i=0;
        //формирование тела записи, и заполнение карты Structure
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
        //если знаем специализацию обобщения, то возвращаем ее, если нет, то тип обобщения;
        //предполагается что тип обобщения создастся при объявления типа,
        //либо при объявлении переменной типа указатель на обобщение
        CCommonVar* CV=static_cast<CCommonVar*>(v);
        CBaseType* BT = CV->FindType();
        if(!BT) return TheModule->getTypeByName(v->GetTypeName());
        return GetLLVMType(BT);
    }
    }//switch
}//GetLLVMType

//-----------------------------------------------------------------------------
//получения типа LLVM на основе наследников класса CBaseType
Type *LLVMDriver::GetLLVMType(CBaseType *v)
{
    switch (v->name_id) {
    default:
        //для простых типов вызывается соответствующий метод
        return GetLLVMType(v->GetResultId());
    case id_CArrayType:{
        //ARRAY
        CArrayType* AT = static_cast<CArrayType*>(v);
        return ArrayType::get(GetLLVMType(AT->Type),AT->size);
    }
    case id_CQualidentType:{
        //Именной тип
        //Ищем именной тип в глобальной области видимости и формируем его
        CQualidentType* QT = static_cast<CQualidentType*>(v);
        //структуру заполняем только для базового типа Record
   /*     if(!Structures.empty()&&!Structures[QT->Qualident->ident].empty()) Structures[v->name] = Structures[QT->Qualident->ident];
        if(!Types.empty()&&Types[QT->Qualident->ident])
            return Types[QT->Qualident->ident];*/
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
        //if(!Structures.empty()&&!Structures[PT->Qualident->ident].empty()) Structures[v->name] = Structures[PT->Qualident->ident];
        //В случае указателя на указатель возвращается первый указатель
        //??????возможно нужно изменить??????
        if(type->isPointerTy())
            return type;
        return type->getPointerTo();
    }
    case id_CRecordType:{
        //RECORD
        //Если тип уже создан - возвращаем
        StructType* ST=TheModule->getTypeByName(v->name);
        if(ST){
           return ST;
        }//if
        //создание типа
        ST=StructType::create(TheContext,v->name);
        CRecordType* RT=static_cast<CRecordType*>(v);
        std::vector<Type*> colTypes;
        CBaseNameVector::const_iterator ci;
        int i=0;
        //формирование тела записи, и заполнение карты Structure
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
        //Если тип уже создан - возвращаем
        StructType* ST=TheModule->getTypeByName(v->name);
        if(ST){
           return ST;
        }//if
        //создание типа
        ST=StructType::create(TheContext,v->name);
        CCommonType* CT=static_cast<CCommonType*>(v);
        CCommonType::SpecStore_type::const_iterator ci;
        int i=0;
        //создание типов специализаций
        for(ci = CT->SpecStore.begin(); ci != CT->SpecStore.end(); ++ci,++i){
            const CBaseName* BN = CT->GetParentModule();
            while (BN->name_id!=id_CModule) {
                BN=BN->GetParentModule();
            }//while
            CBaseType* BT = static_cast<CBaseType*>(BN->GetGlobalName((*ci)->QualName, (*ci)->Name));
            GetLLVMType(BT);
            SpecTypes[(*ci)->Name]=i;
        }//for
        //В качестве тела типа добавляем тип указатель на данную структуру
        //и целый тип, отвечающий за номер специализации
        std::vector<Type*> colTypes;
        colTypes.push_back(ST->getPointerTo());
        colTypes.push_back(Type::getInt32Ty(TheContext));
        ST->setBody(colTypes);
        return ST;
    }
    case id_CSpecType:{
        //Специализация
        //Возвращаем базовый тип
        CSpecType* CST=static_cast<CSpecType*>(v);
        CCommonType* CT = static_cast<CCommonType*>(CST->parent_element->GetGlobalName(CST->Qualident->pref_ident, CST->Qualident->ident));
        return GetLLVMType(CT);
    }
    }//switch
}//GetLLVMType

//-----------------------------------------------------------------------------
//Поиск (если нет в области видисости модуля LLVM, создание) функции
Function* LLVMDriver::GetFunction(CProcedure *p)
{
    //В случае наличия обобщенных параметров используем указатель на CCommonProc
    CCommonProc* cp=nullptr;
    if ((id_CCommonProc == p->name_id || id_CDfnCommonProc == p->name_id ) && p->GetCommonParsCount()){
        cp=static_cast<CCommonProc*>(p);
    }//if
    //Формирование имени функции в соответствии со стандартом С++
    std::string firstName=p->parent_element->name;
    if (p->name_id==id_CHandlerProc&&static_cast<CHandlerProc*>(p)->QualName) firstName=static_cast<CHandlerProc*>(p)->QualName;
    std::string lastName=p->name;
    std::string funName;
    if(p->Receiver){
        std::string recieverName=p->Receiver->type_name;
        funName="_ZN"+std::to_string(firstName.size())+firstName+std::to_string(recieverName.size())+recieverName+std::to_string(lastName.size())+lastName+"E";
    }else funName="_ZN"+std::to_string(firstName.size())+firstName+std::to_string(lastName.size())+lastName+"E";
    if(!p->FormalPars->FPStore.empty()){
        CBaseVarVector::const_iterator i = p->FormalPars->FPStore.begin();
        for(i; i != p->FormalPars->FPStore.end(); ++i) {            
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

    //Если функция с таким именем уже создана, возвращаем ее
    if(Functions[funName]) return Functions[funName];

    //получаем тип возвращаемого значения функции
    Type *FunRetType;
    if (!p->FormalPars->Qualident){
        FunRetType=Type::getVoidTy(TheContext);
    }else{
        FunRetType=GetLLVMType(p->FormalPars->Qualident->TypeResultId);
        if(!FunRetType) {
            FunRetType=GetLLVMType(static_cast<CBaseType*>(p->parent_element->GetGlobalName(p->FormalPars->Qualident->pref_ident, p->FormalPars->Qualident->ident)));
        }//if
    }//if

    //получаем список типов формальных параметров
    std::vector<Type *> pars=WriteLLVM_pars(p->FormalPars);
    //получаем список имен формальных параметров
    StringVector names;
    CBaseVarVector::const_iterator i;
    i = p->FormalPars->FPStore.begin();
    for (i = p->FormalPars->FPStore.begin(); i != p->FormalPars->FPStore.end(); ++i){
      names.push_back((*i)->name);
      //для массивов добавляются размеры измерений в качестве формальных параметров
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
    //Рессивер передаем в качестве формального параметра
    if(p->Receiver){
        pars.push_back(GetLLVMType(static_cast<CBaseVar*>(p->Receiver->FindName(p->Receiver->name)))->getPointerTo());
        names.push_back(p->Receiver->name);
    }//if
    //для функции с обобщенными параметрами добавляем их
    if(cp){
        std::vector<Type *> compars=WriteLLVM_pars(cp->CommonPars);
        pars.insert(pars.end(),compars.begin(),compars.end());
        for (i = cp->CommonPars->FPStore.begin(); i != cp->CommonPars->FPStore.end(); ++i){
            names.push_back((*i)->name);
        }//for
    }//if

    //создание функции и заносим в карту функций
    Function* F = createFunction(FunRetType,pars,names,funName);
    Functions[funName]=F;
    //в случае функции с обобщениями создаем функцию выбора специализированных функций
    if(cp) CreateCommonFun(cp,F);
    return F;
}//GetFunction

//-----------------------------------------------------------------------------
//создание функции выбора специализированных функций
void LLVMDriver::CreateCommonFun(CCommonProc *p, Function *F)
{
    //сохранение текущей позиции в модуле LLVM и карты локальных переменных
    BasicBlock* TempBB=Builder.GetInsertBlock();
    std::map<std::string, AllocaInst *> NamedValuesTemp=NamedValues;
    //начало записи в функцию выбора
    BasicBlock *BB = BasicBlock::Create(TheContext, "entry", F);
    Builder.SetInsertPoint(BB);
    // Запись аргументов функции в карту локальных переменных
    NamedValues.clear();
    for (auto &Arg : F->args()) {
        // Выделение памяти под эту переменную
        AllocaInst *Alloca = CreateEntryBlockAlloca(F,std::string(Arg.getName()),Arg.getType());
        // Сохранение значения аргумента в выделенную память
        Builder.CreateStore(&Arg, Alloca);
        //добавление аргумента в карту локальных переменных
        NamedValues[std::string(Arg.getName())] = Alloca;
    }//for
    //обработка необобщенных параметров
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
    //обработка обобщенных параметров
    CreateCommonFun_loop(p,F,i,funName,values,pars);
    //Проверка сгенерированного кода на согласованность
    verifyFunction(*F);
    //восстановление сохраненных параметров
    Builder.SetInsertPoint(TempBB);
    NamedValues=NamedValuesTemp;
}//CreateCommonFun

//-----------------------------------------------------------------------------
//цикл обхода специализаций
void LLVMDriver::CreateCommonFun_loop(CCommonProc *p, Function *F,CBaseVarVector::const_iterator i,std::string funName,std::vector<Value*> values,std::vector<Type *> pars){
    //получение обобщенного типа и списка специализаций
    CCommonVar* CV=static_cast<CCommonVar*>(*i);
    const CBaseName* BN = p->GetParentModule();
    while (BN->name_id!=id_CModule) {
        BN=BN->GetParentModule();
    }//while
    CCommonType* CT = static_cast<CCommonType*>(BN->GetGlobalName(CV->GetTypeModuleName(),CV->GetTypeName()));
    CCommonType::SpecStore_type::const_iterator ci;
    //временные переменные для востановления параметров до обработки специализации
    std::string tempFunName=funName;
    std::vector<Value*> tempValues=values;
    std::vector<Type *> tempPars=pars;
    for(ci = CT->SpecStore.begin(); ci != CT->SpecStore.end(); ++ci){
        funName=tempFunName;
        values=tempValues;
        pars=tempPars;
        //формирование имени функции с конкретной специализацией
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
        //получение типа конкретной специализацией и добавление его в список формальных параметров
        Type* SpecType=TheModule->getTypeByName((*ci)->Name)->getPointerTo();
        pars.push_back(SpecType);
        //получение тэга специализации
        Value* common=Builder.CreateLoad(NamedValues[(*i)->name]);
        Value* spec=Builder.CreateStructGEP(common,1);
        spec=Builder.CreateLoad(spec);
        //spec=Builder.CreateBitCast(spec,SpecType);
        //формирование инструкции if-then-else
        int sTag=SpecTypes[(*ci)->Name];
        Value* condV=Builder.CreateICmpEQ(spec,ConstantInt::get(Type::getInt32Ty(TheContext),sTag));
        BasicBlock *ThenBB = BasicBlock::Create(TheContext, "then", F);
        BasicBlock *ElseBB = BasicBlock::Create(TheContext, "else");
        Builder.CreateCondBr(condV, ThenBB, ElseBB);
        Builder.SetInsertPoint(ThenBB);
        //добавление обобщенной переменной в список фактических параметров
        //при вызове функции явное приведение типа не требуется
        spec=Builder.CreateStructGEP(common,0);
        values.push_back(Builder.CreateLoad(spec));
        //обработка следующей обобщенной переменной при наличии
        if(i+1!=p->CommonPars->FPStore.end()){
            CreateCommonFun_loop(p,F,i+1,funName,values,pars);
            F->getBasicBlockList().push_back(ElseBB);
            Builder.SetInsertPoint(ElseBB);
            continue;
        }//if
        //создание инструкций вызова специалзированной функции и возврата ее значения
        Function* SpecF=Functions[funName];
        if(!SpecF) SpecF=createFunction(F->getReturnType(),pars,*new StringVector,funName);
        Value* call=Builder.CreateCall(SpecF,values);
        Builder.CreateRet(call);
        //генерация инструкции else
        F->getBasicBlockList().push_back(ElseBB);
        Builder.SetInsertPoint(ElseBB);
    }//for
    //генерации инструкции 'ret <NullValue>'
    Value* retVal=Constant::getNullValue(F->getReturnType());
    Builder.CreateRet(retVal);
}//CreateCommonFun_loop

//-----------------------------------------------------------------------------
//генерация кода LLVM и запись в файл
int LLVMDriver::Init(CModule* m)
{
      //генерация кода LLVM IR для модуля O2M
      WriteLLVM(m);
      //Инициализация целевых платформ
      InitializeAllTargetInfos();
      InitializeAllTargets();
      InitializeAllTargetMCs();
      InitializeAllAsmParsers();
      InitializeAllAsmPrinters();

      auto TargetTriple = sys::getDefaultTargetTriple();
      TheModule->setTargetTriple(TargetTriple);

       std::string Error;
       auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);

       //Печать ошибки и выход если не смогли найти целевую платформу
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
//генерация кода LLVM IR модуля O2M
Value* LLVMDriver::WriteLLVM(CModule *m)
{
    TheModule = std::make_unique<Module>(m->name, TheContext);

    //генерация кода деклараций типов
    WriteLLVM_type(m->DeclSeq);
    //генерация кода переменных
    WriteLLVM_var(m->DeclSeq);
    //генерация кода деклараций процедур
    WriteLLVM_proc(m->DeclSeq);

    //инициализация импортируемых модулей не используется
    //пока что модуль с операторами считается главным,
    //?????????доделать???????????
    //DeclSeq->WriteCPP_mod_init(f);

    //генерация кода для операторов (если есть)
    if (m->StatementSeq) {
        //создание главной функции модуля main (точка входа в программу)
        Function *F = createFunction(Type::getInt32Ty(TheContext),"main");
        // Создание нового базового блока для начала записи
        BasicBlock *BB = BasicBlock::Create(TheContext, "entry", F);
        Builder.SetInsertPoint(BB);
        //очистка карты локальных переменных
        NamedValues.clear();
        //собственно генерация кода операторов
        WriteLLVM(m->StatementSeq);
        //генерации инструкции 'ret 0'
        Value* retVal=Constant::getNullValue(Type::getInt32Ty(TheContext));
        Builder.CreateRet(retVal);
        //Проверка сгенерированного кода на согласованность
        verifyFunction(*F);
    }//if

    //печать текста модуля в поток ошибок для отладки
    TheModule->print(errs(),nullptr);
    fprintf(stderr, "\n");

    return nullptr;
}//WriteLLVM

//-----------------------------------------------------------------------------
//генерация кода деклараций типов
Value *LLVMDriver::WriteLLVM_type(CDeclSeq *ds)
{
    CBaseNameVector::const_iterator ci;

  /*  //запись кода импортируемых модулей
    for (ci = ds->BaseNameStore.begin(); ci != ds->BaseNameStore.end(); ++ci) {
        if (id_CImportModule == (*ci)->name_id) {
            WriteLLVM(static_cast<CImportModule*>(*ci));
        }
    }*/

    //запись кода типов
    for (ci = ds->BaseNameStore.begin(); ci != ds->BaseNameStore.end(); ++ci){
        if (CBaseType::IsTypeId((*ci)->name_id)) {
            WriteLLVM(static_cast<CBaseType*>(*ci));
        }//if
    }//for
    return nullptr;
}//WriteLLVM_type

//-----------------------------------------------------------------------------
//генерация кода переменных
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
//генерация кода деклараций процедур
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
//генерация кода операторов
Value *LLVMDriver::WriteLLVM(CStatementSeq *ss)
{
    //цикл выбора и генерации кода конкретного оператора
    CBaseVector::iterator i;
    for(i = ss->StatStore.begin(); i != ss->StatStore.end(); ++i) {
        if(typeid(**i) == typeid (CIfStatement)){
            WriteLLVM(static_cast<CIfStatement*>(*i));
        }else if(typeid(**i) == typeid (CCaseStatement)){
            WriteLLVM(static_cast<CCaseStatement*>(*i));
        }else if(typeid(**i) == typeid (CWhileStatement)){
            WriteLLVM(static_cast<CWhileStatement*>(*i));
        }else if(typeid(**i) == typeid (CRepeatStatement)){
            WriteLLVM(static_cast<CRepeatStatement*>(*i));
        }else if(typeid(**i) == typeid (CForStatement)){
            WriteLLVM(static_cast<CForStatement*>(*i));
        }else if(typeid(**i) == typeid (CLoopStatement)){
            WriteLLVM(static_cast<CLoopStatement*>(*i));
        }else if(typeid(**i) == typeid (CWithStatement)){
            WriteLLVM(static_cast<CWithStatement*>(*i));
        }else if(typeid(**i) == typeid (CExitStatement)){
            WriteLLVM(static_cast<CExitStatement*>(*i));
        }else if(typeid(**i) == typeid (CAssignStatement)){
            WriteLLVM(static_cast<CAssignStatement*>(*i));
        }else if(typeid(**i) == typeid (CReturnStatement)){
            WriteLLVM(static_cast<CReturnStatement*>(*i));
        }else if(typeid(**i) == typeid (CCallStatement)){
            WriteLLVM(static_cast<CCallStatement*>(*i));
        }else if(typeid(**i) == typeid (CDecStdProc)){
            WriteLLVM(static_cast<CDecStdProc*>(*i));
        }else if(typeid(**i) == typeid (CIncStdProc)){
            WriteLLVM(static_cast<CIncStdProc*>(*i));
        }else if(typeid(**i) == typeid (CNewStdProc)){
            WriteLLVM(static_cast<CNewStdProc*>(*i));
        }//if
    }//for
    return nullptr;
}//WriteLLVM
/*
Value *LLVMDriver::WriteLLVM(CImportModule *im)
{
    namespaces[im->name]=im->real_name;
    return nullptr;
}
*/
//-----------------------------------------------------------------------------
//генерация кода деклараций конкретного типа
//в основном нужно для обобщений
//???????возможно можно обойтись и без этой функции???????
Value *LLVMDriver::WriteLLVM(CBaseType *t)
{
    //Types[t->name]=
    GetLLVMType(t);
    return nullptr;
}//WriteLLVM

//-----------------------------------------------------------------------------
//генерация кода декларации конкретной переменной
Value *LLVMDriver::WriteLLVM(CBaseVar *v)
{
    //признак глобальности
    bool is_global = id_CModule == v->parent_element->name_id;    
    //Получение типа переменной
    Type* type=GetLLVMType(v);
    //в случае константы - инициализируем ее значением, иначе инициализируем нулем
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
        }//switch
    } else {
        InitVal = Constant::getNullValue(type);
    }//if
    if(is_global){
        //в случае глобальной переменной именуем ее в соответствии со стандартом C++
        //и добавляем в карту глобальных переменных
        std::string firstName=v->parent_element->name;
        std::string lastName=v->name;
        std::string name="_ZN"+std::to_string(firstName.size())+firstName+std::to_string(lastName.size())+lastName+"E";
        GlobalVariable *gVar= new GlobalVariable(*TheModule.get(),
        /*Type=*/type,
        /*isConstant=*/v->is_const,
        /*Linkage=*/GlobalValue::ExternalLinkage,
        /*Initializer=*/InitVal,
        /*Name=*/name);
        GlobalValues[v->name]=gVar;
        return gVar;
    }else{
        //для локальных переменных выделяем память и сохраняем в карте локальных переменных
        Function *TheFunction = Builder.GetInsertBlock()->getParent();
        AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, v->name,type);
        Builder.CreateStore(InitVal, Alloca);
        NamedValues[v->name] = Alloca;
        return Alloca;
    }//if
}//WriteLLVM

//-----------------------------------------------------------------------------
//генерация кода констатных значений
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

Function *LLVMDriver::WriteLLVM(CProcedure *p)
{
    if(p->name_id==id_CCommonProc) return nullptr;
    Function* F = GetFunction(p);
    BasicBlock *BB = BasicBlock::Create(TheContext, "entry", F);
    Builder.SetInsertPoint(BB);

    NamedValues.clear();
    for (auto &Arg : F->args()) {
        // Create an alloca for this variable.
        AllocaInst *Alloca = CreateEntryBlockAlloca(F,std::string(Arg.getName()),Arg.getType());

        // Store the initial value into the alloca.
        Builder.CreateStore(&Arg, Alloca);

        // Add arguments to variable symbol table.
        NamedValues[std::string(Arg.getName())] = Alloca;
      }

    //генерация кода деклараций типов
    WriteLLVM_type(p->DeclSeq);
    //генерация кода переменных
    WriteLLVM_var(p->DeclSeq);

    //генерация кода для операторов
    if (p->StatementSeq) {
        WriteLLVM(p->StatementSeq);
    }
    Builder.CreateRet(Constant::getNullValue(Builder.getCurrentFunctionReturnType()));
  //  F->print(errs(),nullptr);
  //  fprintf(stderr, "\n");
    verifyFunction(*F);
    return F;
}

std::vector<Type *> LLVMDriver::WriteLLVM_pars(CFormalPars *fp)
{
    //проверка наличия формальных параметров
        std::vector<Type*> values;
        if (fp->FPStore.empty()) return values;
        //запись первого формального параметра
        CBaseVarVector::const_iterator i = fp->FPStore.begin();
        //запись следующих формальных параметров
        for(i; i != fp->FPStore.end(); ++i) {
            if((*i)->name_id == id_CArrayVar){
                WriteLLVM_fp(static_cast<CArrayVar*>(*i),values);
                continue;
            }
            if((*i)->is_var)
                values.push_back(GetLLVMType((*i))->getPointerTo());
            else values.push_back(GetLLVMType((*i)));
        }
        return values;
}

Value *LLVMDriver::WriteLLVM_fp(CArrayVar *v, std::vector<Type *> &values)
{
    CArrayType* AT = static_cast<CArrayType*>(v->ArrayType->FindLastType());
    CBaseVar* BV;
    static_cast<CBaseType*>(AT)->CreateVar(BV, v->parent_element);
    values.push_back(GetLLVMType((BV)->GetResultId())->getPointerTo());
    delete BV;
    AT=v->ArrayType;
    while (AT->name_id == id_CArrayType) {
        values.push_back(Type::getInt32Ty(TheContext));
        AT = static_cast<CArrayType*>(AT->Type);
    }
}

Value *LLVMDriver::WriteLLVM(CIfStatement *s)
{
    CIfStatement::ElsifPairList_type::iterator i = s->ElsifPairList.begin();
    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    BasicBlock *ElseBB;
    BasicBlock *MergeBB = BasicBlock::Create(TheContext, "ifcont");
    for(i; i != s->ElsifPairList.end(); ++i) {
        ElseBB = WriteLLVM((*i),MergeBB);
        TheFunction->getBasicBlockList().push_back(ElseBB);
        Builder.SetInsertPoint(ElseBB);
    }
    if (s->ElseStatementSeq) {
        WriteLLVM(s->ElseStatementSeq);
    }
    Builder.CreateBr(MergeBB);
    TheFunction->getBasicBlockList().push_back(MergeBB);
    Builder.SetInsertPoint(MergeBB);
    return nullptr;
}

BasicBlock *LLVMDriver::WriteLLVM(CElsifPair *s, BasicBlock *bb)
{
    Value* expr = WriteLLVM(s->Expr);
    while(expr->getType()->isPointerTy()){
        expr=Builder.CreateLoad(expr,expr->getName());
    }
    if(expr->getType()!=Type::getInt1Ty(TheContext))
        expr=CastToType(expr,Type::getInt1Ty(TheContext));
    Function *TheFunction = Builder.GetInsertBlock()->getParent();

    // Create blocks for the then and else cases.  Insert the 'then' block at the
    // end of the function.
    BasicBlock *ThenBB = BasicBlock::Create(TheContext, "then", TheFunction);
    BasicBlock *ElseBB = BasicBlock::Create(TheContext, "else");
    Builder.CreateCondBr(expr, ThenBB, ElseBB);
    //TheFunction->getBasicBlockList().push_back(ThenBB);
    Builder.SetInsertPoint(ThenBB);
    WriteLLVM(s->StatementSeq);
    Builder.CreateBr(bb);
    return ElseBB;
}

Value *LLVMDriver::WriteLLVM(CWhileStatement *s)
{
    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    BasicBlock *WhileBB = BasicBlock::Create(TheContext, "while",TheFunction);
    BasicBlock *BodyBB = BasicBlock::Create(TheContext, "body");
    BasicBlock *EndBB = BasicBlock::Create(TheContext, "endwhile");
    Builder.CreateBr(WhileBB);
    Builder.SetInsertPoint(WhileBB);
    Value* expr = WriteLLVM(s->Expr);
    if(expr->getType()!=Type::getInt1Ty(TheContext))
        expr = CastToType(expr,Type::getInt1Ty(TheContext));

    Builder.CreateCondBr(expr, BodyBB, EndBB);
    TheFunction->getBasicBlockList().push_back(BodyBB);
    Builder.SetInsertPoint(BodyBB);

    WriteLLVM(s->StatementSeq);

    Builder.CreateBr(WhileBB);
    TheFunction->getBasicBlockList().push_back(EndBB);
    Builder.SetInsertPoint(EndBB);
    return nullptr;
}

Value *LLVMDriver::WriteLLVM(CRepeatStatement *s)
{
    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    BasicBlock *RepeatBB = BasicBlock::Create(TheContext, "repeat",TheFunction);
    BasicBlock *EndBB = BasicBlock::Create(TheContext, "endrepeat");
    Builder.CreateBr(RepeatBB);
    Builder.SetInsertPoint(RepeatBB);

    WriteLLVM(s->StatementSeq);
    Value* expr = WriteLLVM(s->Expr);
    //????????????
    while(expr->getType()->isPointerTy()){
        expr=Builder.CreateLoad(expr,expr->getName());
    }
    //????????????
    if(expr->getType()!=Type::getInt1Ty(TheContext))
        expr = CastToType(expr,Type::getInt1Ty(TheContext));

    Builder.CreateCondBr(expr, EndBB, RepeatBB);
    TheFunction->getBasicBlockList().push_back(EndBB);
    Builder.SetInsertPoint(EndBB);
    return nullptr;
}

Value *LLVMDriver::WriteLLVM(CForStatement *s)
{
    Value* vToExpr=WriteLLVM(s->ToExpr);
    Value* vForExpr=WriteLLVM(s->ForExpr);
    Value* CondV;
    //---------------вынести в отдельный метод
    Value* store=NamedValues[s->var_name];
    if(!store) store=GlobalValues[s->var_name];
    while(store->getType()->getContainedType(0)->isPointerTy()){
        store=Builder.CreateLoad(store,store->getName());
    }
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
    //---------------------------------------
    Builder.CreateStore(vForExpr, store);
    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    BasicBlock* ForBB = BasicBlock::Create(TheContext, "for", TheFunction);
    BasicBlock *ForBodyBB = BasicBlock::Create(TheContext, "forbody");
    BasicBlock *ExitForBB = BasicBlock::Create(TheContext, "exitfor");
    Builder.CreateBr(ForBB);
    Builder.SetInsertPoint(ForBB);
    Value* vStore=Builder.CreateLoad(store,store->getName());
    if(s->step > 0){
        CondV=Builder.CreateICmpSLE(vStore,vToExpr,"forcond");
    }else{
        CondV=Builder.CreateICmpSGE(vStore,vToExpr,"forcond");
    }
    Builder.CreateCondBr(CondV,ForBodyBB,ExitForBB);
    TheFunction->getBasicBlockList().push_back(ForBodyBB);
    Builder.SetInsertPoint(ForBodyBB);

    WriteLLVM(s->StatementSeq);

    vStore=Builder.CreateLoad(store,store->getName());
    Value* vStep = Builder.CreateAdd(vStore,ConstantInt::get(vStore->getType(),s->step),"step");
    Builder.CreateStore(vStep, store);
    Builder.CreateBr(ForBB);
    TheFunction->getBasicBlockList().push_back(ExitForBB);
    Builder.SetInsertPoint(ExitForBB);
    return nullptr;
}

Value *LLVMDriver::WriteLLVM(CLoopStatement *s)
{
    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    BasicBlock *LoopBB = BasicBlock::Create(TheContext, "loop", TheFunction);
    BasicBlock *ExitBB = BasicBlock::Create(TheContext, "exitLoop");
    BasicBlocks[s->WithLoopLink.LoopUID]=ExitBB;
    Builder.CreateBr(LoopBB);
    Builder.SetInsertPoint(LoopBB);

    WriteLLVM(s->StatementSeq);

    Builder.CreateBr(LoopBB);

    TheFunction->getBasicBlockList().push_back(ExitBB);
    Builder.SetInsertPoint(ExitBB);
    return nullptr;
}

Value *LLVMDriver::WriteLLVM(CCaseStatement *s)
{
    BasicBlock *ExitBB = BasicBlock::Create(TheContext, "exitCase");
    //цикл перебора последовательностей меток
    CCaseStatement::CaseLabelsSeqList_type::const_iterator i;
    for (i = s->CaseLabelsSeqList.begin(); i != s->CaseLabelsSeqList.end(); ++i)
    {
        WriteLLVM((*i), s->Expr,ExitBB);
    }//for

    //проверка наличия условия ELSE
    if (s->ElseStatementSeq) {
        WriteLLVM(s->ElseStatementSeq);
    }
    Builder.CreateBr(ExitBB);
    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    TheFunction->getBasicBlockList().push_back(ExitBB);
    Builder.SetInsertPoint(ExitBB);
    return nullptr;
}

Value *LLVMDriver::WriteLLVM(CCaseLabelsSeq *cls, CExpr *e, BasicBlock *exitBB)
{
    CCaseLabelsSeq::CaseLabelsList_type::const_iterator i;

    //проверка первой метки
    if (cls->CaseLabelsList.empty()) throw error_Internal("CCaseLabelsSeq::WriteCPP");
    i = cls->CaseLabelsList.begin();
    Value* expr = WriteLLVM((*i), e);
    //цикл перебора оставшихся меток
    for (++i; i != cls->CaseLabelsList.end(); ++i)
    {
        //fprintf(f.fc, ")||(");
        Value* expr2 = WriteLLVM((*i), e);
        expr=Builder.CreateOr(expr,expr2,"cond");
    }

    BasicBlock *caseBB = BasicBlock::Create(TheContext, "case");
    BasicBlock *elseBB = BasicBlock::Create(TheContext, "else");
    Builder.CreateCondBr(expr,caseBB,elseBB);
    //генерация кода послед-ти операторов
    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    TheFunction->getBasicBlockList().push_back(caseBB);
    Builder.SetInsertPoint(caseBB);
    WriteLLVM(&cls->StatementSeq);
    Builder.CreateBr(exitBB);

    TheFunction->getBasicBlockList().push_back(elseBB);
    Builder.SetInsertPoint(elseBB);
    return nullptr;
}

Value *LLVMDriver::WriteLLVM(CCaseLabels *cls, CExpr *e)
{
    Value* expr = WriteLLVM(e);
    //проверка наличия диапазона
    if (cls->IsRange) {
        Value* uge = Builder.CreateICmpUGE(expr,ConstantInt::get(expr->getType(),cls->ConstValue));
        Value* ule = Builder.CreateICmpULE(expr,ConstantInt::get(expr->getType(),cls->ConstHighValue));
        return Builder.CreateAnd(uge,ule);
    } else
        return Builder.CreateICmpEQ(expr,ConstantInt::get(expr->getType(),cls->ConstValue));
}

Value *LLVMDriver::WriteLLVM(CReturnStatement *s)
{
    Type* funRetType=Builder.getCurrentFunctionReturnType();
    Value* retVal;
    if (s->Expr) {
        retVal=WriteLLVM(s->Expr);
    } else retVal=Constant::getNullValue(funRetType);
    retVal=CastToType(retVal,funRetType);
    Builder.CreateRet(retVal);

    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    BasicBlock *unreachBB = BasicBlock::Create(TheContext, "unreach");
    TheFunction->getBasicBlockList().push_back(unreachBB);
    Builder.SetInsertPoint(unreachBB);
    return nullptr;
}

Value *LLVMDriver::WriteLLVM(CAssignStatement *s)
{
    ////////////////////////////////////////////////////
    //генерация кода присвоения текстовой строки массиву
    if (s->str_to_array) {
        return WriteLLVM_array(s);
    }

    if(s->Designator->ResultId==id_CProcedureVar){
        Value* store=WriteLLVM(s->Designator);
        while (store->getType()->getContainedType(0)->getContainedType(0)->isPointerTy()) {
            store=Builder.CreateLoad(store,store->getName());
        }
        Value* expr=WriteLLVM(s->Expr);
        while (expr->getType()->getContainedType(0)->isPointerTy()) {
            expr=Builder.CreateLoad(expr,expr->getName());
        }
        return Builder.CreateStore(expr, store);
    }
    if(s->Designator->ResultId==id_CPointerVar){
        Value* store=WriteLLVM(s->Designator);
        while (store->getType()->getContainedType(0)->getContainedType(0)->isPointerTy()) {
            store=Builder.CreateLoad(store,store->getName());
        }
        Value* expr=WriteLLVM(s->Expr);
        if(!expr) expr=Constant::getNullValue(store->getType()->getContainedType(0));
        CBaseName* BN=s->Expr->FindLastName();
        if(BN && SpecTypes[BN->name]){
            SpecTypes[s->Designator->Qualident->ident]=SpecTypes[BN->name];
        }
        return Builder.CreateStore(expr, store);
    }
    //////////////////////////////////////
    //генерация кода обычного присваивания
    Value* store=WriteLLVM(s->Designator);
    while (store->getType()->getContainedType(0)->isPointerTy()) {
        store=Builder.CreateLoad(store,store->getName());
    }
    Value* expr=CastToType(WriteLLVM(s->Expr),store->getType()->getContainedType(0));
    while (expr->getType()->isPointerTy()) {
        expr=Builder.CreateLoad(expr,expr->getName());
    }
    return Builder.CreateStore(expr, store);
}

Value *LLVMDriver::WriteLLVM_array(CAssignStatement *s)
{
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
    }
    Value* string1 = WriteLLVM(s->Expr);
    CBaseName* BN = s->Expr->FindLastName();
    Value* size1 = WriteLLVM_COPY_Par(BN);
    //запись значения приемника
    Value* string2 = WriteLLVM(s->Designator);
    //получение приемника (переменной-массива или указателя)
    BN = s->Designator->FindLastName();
    Value* size2 = WriteLLVM_COPY_Par(BN);
    //********???????????
    while(string2->getType()->getContainedType(0)->isArrayTy()){
        std::vector<Value*> values;
        values.push_back(Constant::getNullValue(Type::getInt32Ty(TheContext)));
        values.push_back(Constant::getNullValue(Type::getInt32Ty(TheContext)));
        string2=Builder.CreateGEP(string2,values);
    }
    //???????????
    std::vector<Value *> values={string1,size1,string2,size2};
    return Builder.CreateCall(F,values);
}

Value *LLVMDriver::WriteLLVM_COPY_Par(CBaseName *bn)
{
    if (id_CPointerVar == bn->name_id) bn = static_cast<CPointerVar*>(bn)->FindType();
    //получение размера для переменной или типа
    int arr_size;
    if (id_CArrayVar == bn->name_id)
        arr_size = static_cast<CArrayVar*>(bn)->ArrayType->size;
    else
        arr_size = static_cast<CArrayType*>(bn)->size;
    return ConstantInt::get(Type::getInt32Ty(TheContext),arr_size);
}

Value *LLVMDriver::WriteLLVM(CExitStatement *s)
{
    Builder.CreateBr(BasicBlocks[s->UID]);

    Function *TheFunction = Builder.GetInsertBlock()->getParent();
    BasicBlock *unreachBB = BasicBlock::Create(TheContext, "unreach");
    TheFunction->getBasicBlockList().push_back(unreachBB);
    Builder.SetInsertPoint(unreachBB);
    return nullptr;
}


Value *LLVMDriver::WriteLLVM(CCallStatement *s)
{
    //получение ук. на CProcedure или CProcedureVar
    CBaseName *BN = s->Designator->FindLastName();
    CFormalPars* FP;
    if (CProcedure::IsProcId(BN->name_id)){
        CProcedure* BV = static_cast<CProcedure*>(BN);
        FP=BV->FormalPars;
        Function* F;
        /*if(id_CCommonProc == BN->name_id || id_CDfnCommonProc == BN->name_id || id_CHandlerProc == BN->name_id){
            F=GetFunction(static_cast<CCommonProc*>(BV),s->CommonList);
        }else */
            F=GetFunction(BV);
        std::vector<Value *> ArgsV;
        if (s->ExprList) {
            ArgsV = WriteLLVM(s->ExprList,FP);
        }
        if (s->CommonList) {
            std::vector<Value *> ArgsVC = WriteLLVM(s->CommonList,static_cast<CCommonProc*>(BV)->CommonPars);
            ArgsV.insert(ArgsV.end(),ArgsVC.begin(),ArgsVC.end());
        }
        if(BV->Receiver){
            ArgsV.push_back(WriteLLVM(s->Designator));
        }
        return Builder.CreateCall(F,ArgsV,"calltmp");
    }
    else
        if (id_CProcedureVar == BN->name_id){
            FP = &static_cast<CProcedureVar*>(BN)->FormalPars;
            Value* d = Builder.CreateLoad(WriteLLVM(s->Designator),BN->name);
            FunctionType* ft= static_cast<FunctionType*>(d->getType()->getContainedType(0));
            std::vector<Value *> ArgsV;
            if (s->ExprList) {
                ArgsV = WriteLLVM(s->ExprList,FP);
            }
            return Builder.CreateCall(ft,d,ArgsV,"calltmp");;
        }
        else{
            return nullptr;
        }
}

std::vector<Value *> LLVMDriver::WriteLLVM(CExprList *e,CFormalPars* fp)
{
    std::vector<Value *> ArgsV;
    CExprVector::const_iterator i = e->ExprVector->begin();
    CBaseVarVector::const_iterator ci = fp->FPStore.begin();
    for(i; i != e->ExprVector->end(); ++i, ++ci) {
        Value* value;
        if((*ci)->is_var){
            value = WriteLLVM((*i)->SimpleExpr1->Term->Factor->Designator);
        }else value=WriteLLVM((*i));
        while (value->getType()->getNumContainedTypes()>0&&value->getType()->getContainedType(0)->isPointerTy()
               &&((*ci)->name_id!=id_CPointerVar or value->getType()->getContainedType(0)->getContainedType(0)->isPointerTy())) {
            value=Builder.CreateLoad(value,value->getName());
        }
        while(value->getType()->getNumContainedTypes()>0&&value->getType()->getContainedType(0)->isArrayTy()){
            std::vector<Value*> values;
            values.push_back(Constant::getNullValue(Type::getInt32Ty(TheContext)));
            values.push_back(Constant::getNullValue(Type::getInt32Ty(TheContext)));
            value=Builder.CreateGEP(value,values);
        }
        ArgsV.push_back(CastToType(value,GetLLVMType((*ci)->name_id)));
        if(id_CArrayVar == (*ci)->name_id){
            CArrayVar* AV=static_cast<CArrayVar*>((*i)->FindLastName());
            CArrayType* AT=static_cast<CArrayVar*>(AV)->ArrayType;
            long size=AT->size;
            if(size>0)
                ArgsV.push_back(ConstantInt::get(Type::getInt32Ty(TheContext),size));
            else{
                int dimention = 0;
                while (AT->name_id == id_CArrayType) {
                    std::string s="O2M_ARR_"+std::to_string(dimention)+"_"+AV->name;
                    ArgsV.push_back(Builder.CreateLoad(NamedValues[s],s));
                    ++dimention;
                    AT = static_cast<CArrayType*>(AT->Type);
                }
            }
        }
    }
    return ArgsV;
}

Value *LLVMDriver::WriteLLVM(CExpr *e)
{
    Value* simpleExpr1 = CastToType(WriteLLVM(e->SimpleExpr1),GetLLVMType(e->SimpleExpr1->GetResultId()));
    Value* simpleExpr2 =nullptr;
    if (e->SimpleExpr2) {
        simpleExpr2 = CastToType(WriteLLVM(e->SimpleExpr2),GetLLVMType(e->SimpleExpr2->GetResultId()));
        if(!simpleExpr2)
            simpleExpr2 = ConstantPointerNull::get(simpleExpr1->getType()->getContainedType(0)->getPointerTo());
        if(IsId1IncloseId2(e->SimpleExpr1->GetResultId(),e->SimpleExpr2->GetResultId()))
            simpleExpr2 = CastToType(simpleExpr2,simpleExpr1->getType());
        else simpleExpr1 = CastToType(simpleExpr1,simpleExpr2->getType());
    }

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
}

Value *LLVMDriver::WriteLLVM(CSimpleExpr *e)
{
    Value* term = CastToType(WriteLLVM(e->Term),GetLLVMType(e->Term->GetResultId()));
    if (e->negative) {
        if(term->getType()==Type::getDoubleTy(TheContext))
            term=Builder.CreateFNeg(term);
        else term=Builder.CreateNeg(term);
    }
    if (e->SimpleExprPairStore) {
        CBaseVector::const_iterator ci;
        for (ci = e->SimpleExprPairStore->begin(); ci != e->SimpleExprPairStore->end(); ++ci){
            CSimpleExprPair* sep = static_cast<CSimpleExprPair*>(*ci);
            Value* simplePair=CastToType(WriteLLVM(sep),GetLLVMType(sep->GetResultId()));
            if(IsId1IncloseId2(e->Term->GetResultId(),sep->GetResultId()))
                simplePair = CastToType(simplePair,term->getType());
            else term = CastToType(term,simplePair->getType());

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
                //проверить
                term=Builder.CreateOr(term, simplePair, "ortmp");
                break;
            }
        }
    }
    return term;
}

Value *LLVMDriver::WriteLLVM(CSimpleExprPair *e)
{
    return WriteLLVM(e->Term);
}

Value *LLVMDriver::WriteLLVM(CTerm *t)
{
    Value* factor = CastToType(WriteLLVM(t->Factor),GetLLVMType(t->Factor->GetResultId()));
    if (t->TermPairStore) {
        CBaseVector::const_iterator ci;
        for (ci = t->TermPairStore->begin(); ci != t->TermPairStore->end(); ++ci) {
            CTermPair* tp=static_cast<CTermPair*>(*ci);
            Value* termPair = CastToType(WriteLLVM(tp),GetLLVMType(tp->GetResultId()));
            if(IsId1IncloseId2(t->Factor->GetResultId(),tp->GetResultId()))
                termPair = CastToType(termPair,factor->getType());
            else factor = CastToType(factor,termPair->getType());
            switch (tp->MulOp) {
            case mop_DIV:
                //проверить
                factor=Builder.CreateSDiv(factor, termPair, "quotmp");
                break;
            case mop_MOD:
                //проверить
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
            }//switch
        }//for
    }
    return factor;
}

Value *LLVMDriver::WriteLLVM(CTermPair *t)
{
    return WriteLLVM(t->Factor);
}

Value *LLVMDriver::WriteLLVM(CFactor *f)
{
    switch (f->FactorKind) {
    case fk_Negation:{
        Value* factor=WriteLLVM(f->Factor);
        return Builder.CreateICmpEQ(factor,Constant::getNullValue(factor->getType()),"neg");
    }
    case fk_Expr:
        return WriteLLVM(f->Expr);
    case fk_ConstVar:
        return WriteLLVM_ConstValue(f->ConstVar);
    case fk_StdProcFunc:
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
        //проверка наличия вызова процедуры функции (если нет - запись кода обозначения)
        if (f->Call)
            return WriteLLVM(f->Call);
        else{
            Value* designator = WriteLLVM(f->Designator);
            while (f->GetResultId()!=id_CArrayVar && f->GetResultId()!=id_CProcedureVar
                   && f->GetResultId()!=id_CProcedure && designator->getType()->isPointerTy()
                   && !designator->getType()->getContainedType(0)->isStructTy()) {
                designator=Builder.CreateLoad(designator,designator->getName());
            }
            return designator;
        }
    }//switch
}

Value *LLVMDriver::WriteLLVM(CDesignator *d)
{
    //перебор и генерация кода эл-тов обозначения
    Value* designator=NamedValues[d->Qualident->ident];
    if(!designator) designator=GlobalValues[d->Qualident->ident];
    CDesignator::SDesElemStore::const_iterator ci;
    if(!designator) {
        CBaseName* BN = d->FindLastName();
        if(BN->name_id==id_CProcedure)
            designator=GetFunction(static_cast<CProcedure*>(BN));
    }
    if(!designator){
        const CBaseName* BN = d->parent_element;
        while(BN->name_id!=id_CModule){
            BN=BN->parent_element;
        }
        CBaseVar* BV=static_cast<CBaseVar*>(BN->GetGlobalName(d->Qualident->pref_ident,d->Qualident->ident));
        std::string firstName=BV->parent_element->name;
        std::string lastName=BV->name;
        std::string name="_ZN"+std::to_string(firstName.size())+firstName+std::to_string(lastName.size())+lastName+"E";
        GlobalVariable *gVar= new GlobalVariable(*TheModule.get(),
        /*Type=*/GetLLVMType(BV),
        /*isConstant=*/false,
        /*Linkage=*/GlobalValue::ExternalLinkage,
        /*Initializer=*/nullptr,
        /*Name=*/name);
        GlobalValues[lastName]=gVar;
        designator=gVar;
    }
    for (ci = d->DesElemStore.begin(); ci != d->DesElemStore.end(); ci++) {
        switch ((*ci)->DesKind) {
        case CDesignator::EDesKind::dk_Array:       
            designator=WriteLLVM_index((*ci)->ExprList, false,designator);
            break;
        case CDesignator::EDesKind::dk_OpenArray:
            designator=WriteLLVM_index((*ci)->ExprList, true,designator);
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
}

Value *LLVMDriver::WriteLLVM_index(CExprList *e, bool IsOpenArray, Value* array)
{
    std::vector<Value*> values;
    if(!IsOpenArray /*&& array->getType()->isArrayTy()*/){
        values.push_back(Constant::getNullValue(Type::getInt32Ty(TheContext)));
    }
    CExprVector::const_iterator ci = e->ExprVector->begin();
    for (ci; ci != e->ExprVector->end(); ++ci) {
        Value* value=WriteLLVM((*ci));
        value=CastToType(value,Type::getInt32Ty(TheContext));
        values.push_back(value);
    }//for
    if(IsOpenArray /*or !array->getType()->isArrayTy()*/){
        array=Builder.CreateLoad(array,array->getName());
        return Builder.CreateInBoundsGEP(array,values,"gep");
    }
    return Builder.CreateGEP(array,values,"gep");
}

Value *LLVMDriver::WriteLLVM_record_index(Value *record, char *ident,CDesignator *d)
{
    CBaseVar* BN = static_cast<CBaseVar*>(d->parent_element->GetGlobalName(d->Qualident->pref_ident, d->Qualident->ident));
    while(record->getType()->getContainedType(0)->isPointerTy()){
        record=Builder.CreateLoad(record,record->getName());
    }
    const char* parentIdent;
    if(BN->name_id==id_CRecordVar)
     parentIdent= static_cast<CRecordVar*>(BN)->GetTypeName();
    else if(BN->name_id==id_CCommonVar) parentIdent=static_cast<CCommonVar*>(BN)->SpecName;
    else{
        CPointerVar* PV = static_cast<CPointerVar*>(BN);
        CBaseType* BT = PV->FindType();
        if(BT->name_id==id_CRecordType){
            parentIdent=BT->name;
        }
        else{
            CSpecType* ST = static_cast<CSpecType*>(BT);
            parentIdent = ST->GetSpecName();
            if(!TheModule->getTypeByName(parentIdent)){
                CCommonType* CT = static_cast<CCommonType*>(ST->parent_element->GetGlobalName(ST->Qualident->pref_ident, ST->Qualident->ident));
                const CCommonType::SSpec* SS = CT->FindSpec(ST->GetQualSpecName(),ST->GetSpecName(),ST->GetSpecName());
                parentIdent = SS->Name;
            }
            record=Builder.CreateStructGEP(record,0,"gep");
            while(record->getType()->getContainedType(0)->isPointerTy()){
                record=Builder.CreateLoad(record,record->getName());
            }
            Type* type=TheModule->getTypeByName(parentIdent)->getPointerTo();
            record=Builder.CreateBitCast(record,type,"bitcast");
        }
    }
    if(Structures[parentIdent].find(ident) == Structures[parentIdent].end()) return record;
    return Builder.CreateStructGEP(record,Structures[parentIdent][ident],"gep");
}

Value *LLVMDriver::WriteLLVM(CAbsStdProcFunc *d)
{
    std::string name="abs";
    Type* type=GetLLVMType(id_CIntegerVar);
    if (id_CLongintVar == d->GetResultId()) {
        name="labs";
        type=GetLLVMType(id_CLongintVar);
    }else
        if (CBaseVar::IsRealId(d->GetResultId())) {
            name="fabs";
            type=GetLLVMType(id_CRealVar);
        }
    Function *F=Functions[name];
    if(!F){
    std::vector<Type *> pars(1, type);
    StringVector emptyVector;
    F = createFunction(type,pars,emptyVector,name);
    Functions[name]=F;
    }
    Value* expr=WriteLLVM(&d->Expr);
    return Builder.CreateCall(F, expr, "calltmp");
}

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
    }
    Value* exp=WriteLLVM(&d->Expr);
    exp=CastToType(exp,Type::getInt32Ty(TheContext));
    return Builder.CreateCall(F,exp,"cap");
}

Value *LLVMDriver::WriteLLVM(CChrStdProcFunc *d)
{
    return WriteLLVM(&d->Expr);
}

Value *LLVMDriver::WriteLLVM(CEntierStdProcFunc *d)
{
    return CastToType(WriteLLVM(&d->Expr),Type::getInt32Ty(TheContext));
}

Value *LLVMDriver::WriteLLVM(CLenStdProcFunc *d)
{
    if (!d->array_size){
        std::string name="O2M_ARR_"+std::to_string(d->dimension)+"_"+d->array_name;
        return Builder.CreateLoad(NamedValues[name],name);
    }
    return ConstantInt::get(Type::getInt32Ty(TheContext),d->array_size);
}

Value *LLVMDriver::WriteLLVM(CLongStdProcFunc *d)
{
    return WriteLLVM(&d->Expr);
}

Value *LLVMDriver::WriteLLVM(COddStdProcFunc *d)
{
    Value* expr = WriteLLVM(&d->Expr);
    Value* rem = Builder.CreateSRem(expr,ConstantInt::get(expr->getType(),2));
    return Builder.CreateICmpEQ(rem,ConstantInt::get(rem->getType(),1));
}

Value *LLVMDriver::WriteLLVM(COrdStdProcFunc *d)
{
    std::string name="_Z3ORDc";
    Function *F=Functions[name];
    if(!F){
        Type *FunRetType=Type::getInt32Ty(TheContext);
        std::vector<Type *> pars={Type::getInt8Ty(TheContext)};
        StringVector emptyVector;
        F=createFunction(FunRetType,pars,emptyVector,name);
        Functions[name]=F;
    }
    Value* exp=WriteLLVM(&d->Expr);
    exp=CastToType(exp,Type::getInt8Ty(TheContext));
    return Builder.CreateCall(F,exp,"ORD");
}

Value *LLVMDriver::WriteLLVM(CShortStdProcFunc *d)
{
    return WriteLLVM(&d->Expr);
}

Value *LLVMDriver::WriteLLVM(CDecStdProc *d)
{
    Value* store=WriteLLVM(d->Expr1.SimpleExpr1->Term->Factor->Designator);
    while(store->getType()->getContainedType(0)->isPointerTy()){
        store=Builder.CreateLoad(store,store->getName());
    }
    Value* expr=Builder.CreateLoad(store,store->getName());
    Value* dec=nullptr;
    if (d->Expr2) dec=WriteLLVM(d->Expr2);
    else dec=ConstantInt::get(expr->getType(),1);
    if(expr->getType()!=dec->getType())
        dec=CastToType(dec,expr->getType());
    expr=Builder.CreateSub(expr,dec,"dec");
    return Builder.CreateStore(expr,store);
}

Value *LLVMDriver::WriteLLVM(CIncStdProc *d)
{
    Value* store=WriteLLVM(d->Expr1.SimpleExpr1->Term->Factor->Designator);
    while(store->getType()->getContainedType(0)->isPointerTy()){
        store=Builder.CreateLoad(store,store->getName());
    }
    Value* expr=Builder.CreateLoad(store,store->getName());
    Value* inc=nullptr;
    if (d->Expr2) inc=WriteLLVM(d->Expr2);
    else inc=ConstantInt::get(expr->getType(),1);
    if(expr->getType()!=inc->getType())
        inc=CastToType(inc,expr->getType());
    expr=Builder.CreateAdd(expr,inc,"inc");
    return Builder.CreateStore(expr,store);
}

Value *LLVMDriver::WriteLLVM(CNewStdProc *d)
{
    CPointerVar* pVar = static_cast<CPointerVar*>(d->Des.FindLastName());
    CBaseType* BT = pVar->FindType();
    Value* des = WriteLLVM(&(d->Des));
    Type* type = des->getType();
    while(type->isPointerTy()&&type->getContainedType(0)->isPointerTy()&&type->getContainedType(0)->getContainedType(0)->isPointerTy()){
        des = Builder.CreateLoad(des,des->getName());
        type = des->getType();
    }
    type = type->getContainedType(0);
    std::string name="_Znwy";
    Function *F=Functions[name];
    if(!F){
        Type* reType=Type::getInt8PtrTy(TheContext);
        std::vector<Type *> pars(1, Type::getInt64Ty(TheContext));
        StringVector emptyVector;
        F = createFunction(reType,pars,emptyVector,name);
        Functions[name]=F;
    }
    Value* call = Builder.CreateCall(F, ConstantInt::get(Type::getInt64Ty(TheContext),32), "new");
    call=Builder.CreateBitCast(call,type,"bitcast");
    if (id_CSpecType == BT->name_id){
        Value* val=Builder.CreateStructGEP(call,0,"val");
        type=val->getType()->getContainedType(0);
        Value* call2 = Builder.CreateCall(F, ConstantInt::get(Type::getInt64Ty(TheContext),32), "new");
        call2=Builder.CreateBitCast(call2,type,"bitcast2");
        Builder.CreateStore(call2,val);
        Value* spec=Builder.CreateStructGEP(call,1,"val");
        CSpecType* ST=static_cast<CSpecType*>(BT);
        std::string specname=std::string(ST->GetSpecName());
        if(!TheModule->getTypeByName(specname)){
            CCommonType* CT = static_cast<CCommonType*>(ST->parent_element->GetGlobalName(ST->Qualident->pref_ident, ST->Qualident->ident));
            const CCommonType::SSpec* SS = CT->FindSpec(ST->GetQualSpecName(),ST->GetSpecName(),ST->GetSpecName());
            specname = SS->Name;
        }
        Builder.CreateStore(ConstantInt::get(Type::getInt32Ty(TheContext),SpecTypes[specname]),spec);
    }
    return Builder.CreateStore(call,des);
}


