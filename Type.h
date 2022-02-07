//=============================================================================
// Объявление классов типов (Type)
//=============================================================================

#ifndef O2M_Type_h
#define O2M_Type_h

#include "Common.h"
#include "Base.h"
#include "Expr.h"
#include "Project.h"
#include "LexAnl.h"

class CStatementSeq;
class CDfnModuleSeq;
class CRecordType;


//-----------------------------------------------------------------------------
//декларация объектов
class CDeclSeq
{
public:
	void WriteCPP_mod_init(CPP_files& f);	//инициализация импортируемых модулей
	int LoadDfnModule(const CProject *project, const char* module_name, const char* alias_name);
	CDeclSeq(const CBaseName* parent) : DfnModuleSeq(NULL),
		parent_element(parent) {};
	~CDeclSeq();
	//void debug_PutsDeclSeq();
	//void debug_ProcessImportedModules(const char tmp_name[], int (*run_analyser)(const char* fname, const char tmp_name[]));
	int Init(CLexBuf *lb);
	int ImportListInit(const CProject *project, CLexBuf *lb);
	int WriteCPP_type(CPP_files& f);
	int WriteCPP_proc(CPP_files& f);
	int WriteCPP_var(CPP_files& f);
	void WriteDFN(DFN_file& f);
	//поиск имени в таблице имен
	CBaseName* FindName(const char* search_name) const;
	//добавление указанного эл-та в таблицу имен
	void AddName(CBaseName* BN) const;
	//используемые модули (в виде dfn)
	CDfnModuleSeq* DfnModuleSeq;
    mutable CBaseNameVector BaseNameStore;
protected:
	int CheckCompleteRoot(CLexBuf *lb);
	//контейнер объектов, объявленных в послед-ти деклараций
    //mutable CBaseNameVector BaseNameStore;
	int ConstInit(CLexBuf *lb);
	int TypeInit(CLexBuf *lb);
	int VarInit(CLexBuf *lb);
	virtual int ProcInit(CLexBuf *lb);
public:
	const CBaseName* parent_element;
};//CDeclSeq


//-----------------------------------------------------------------------------
//декларация объектов в dfn модуле
class CDfnDeclSeq : public CDeclSeq
{
public:
	CDfnDeclSeq(const CBaseName* parent) : CDeclSeq(parent) {};
protected:
	int ProcInit(CLexBuf *lb);
};//CDfnDeclSeq


//-----------------------------------------------------------------------------
//приемник (Receiver) типа PROCEDURE
class CReceiver
{
public:
	CReceiver(const CBaseName* parent) : is_var(false),
		name(NULL),
		type_name(NULL),
		Recv(NULL) {};
	~CReceiver();
	//поиск имени в таблице имен, NULL - если имя не найдено
	CBaseName* FindName(const char* search_name) const;
	//инициализация
	int Init(CLexBuf *lb, const CBaseName* parent_element);
	//генерация кода приемника
	void WriteCPP(CPP_files &f) {fprintf(f.fc, "\t%s* %s = this;\n", type_name, name);};
	//запись кода приемника
	void WriteCPP_fp(CPP_files& f, const bool external, const bool single_param);
	//признак наличия кл. слова VAR (true означает связывание с записью, false - с ук. на запись)
	bool is_var;
	//название объекта-приемника (запись или ук. на запись)
	char* name;
	//название типа объекта-приемника
	char* type_name;
//private:
	//переменная-приемник (т.к. Receiver - это таблица имен с одной переменной)
	CBaseVar* Recv;
};//CReceiver


//-----------------------------------------------------------------------------
//класс для создания списка формальных параметров одного типа
class CFPSection
{
protected:
	bool is_var;
	const CBaseName *parent_element;
	CBaseType *BaseType;
	//тип списка загруженных имен
	typedef StringVector TmpIdents_type;
	//список загруженных имен
	TmpIdents_type tmp_idents;
public:
	CFPSection(const CBaseName* parent) : parent_element(parent), BaseType(NULL) {};
	~CFPSection();
	int AssignFPElems(CLexBuf *lb, CBaseVarVector &FPVector, const char* ReceiverName);
};//CFPSection


//-----------------------------------------------------------------------------
//список формальных параметров (FormalPars) типа PROCEDURE
class CFormalPars
{
public:
	CBaseName* FindName(const char* search_name) const;
	//поиск имени в списке формальных параметров по номеру
    CBaseName* GetNameByIndex(int index);
	CFormalPars() : Qualident(NULL), have_arrays(false) {}
	~CFormalPars();
	void Assign(CFormalPars& FP, const CBaseName* parent_element) const;
	//тип процедуры-функции
	CQualident* Qualident;
	//список формальных параметров
	CBaseVarVector FPStore;
	int Init(CLexBuf *lb, const CBaseName* parent_element);
	//генерация кода типа процедуры
	void WriteCPP_type(CPP_files& f, const bool to_h, const CBaseName* parent_element);
	//генерация кода формальных параметров
	void WriteCPP_pars(CPP_files& f, const bool to_h);
	//запись списка имен формальных параметров через ","
	void WriteCPP_names(CPP_files& f, const bool to_h);
	//генерация кода для инициализации массивов-значений
    void WriteCPP_begin(CPP_files& f);
	//генерация кода для деинициализации массивов-значений
	void WriteCPP_end(CPP_files& f, const bool ret_present);
	//запись объявления формальных параметров процедуры в dfn файл
	void WriteDFN(DFN_file& f);
	//запись типа процедуры в dfn файл
	void WriteDFN_type(DFN_file& f);
protected:
	int CheckCompleteRoot(CLexBuf *lb);
	//признак наличия массивов в качестве параметров-значений (требуют инициализации)
	bool have_arrays;
};//CFormalPars


//-----------------------------------------------------------------------------
//класс для создания списка формальных обобщенных параметров одного типа
class CCommonFPSection : public CFPSection
{
public:
	int AssignFPElems(CLexBuf *lb, CBaseVarVector &FPVector);
	CCommonFPSection(const CBaseName* parent) : CFPSection(parent) {};
};


//-----------------------------------------------------------------------------
//класс для создания списка формальных специализированных обобщенных параметров одного типа
class CSpecFPSection
{
	bool is_var;
	const CBaseName *parent_element;
	CBaseType *BaseType;
	//тип объекта для хранения информации об 1 загруженном параметре
	struct SSFPElem{
		char* ident;
		bool IsNeedDefaultSpec;
		char* QualTagName;
		char* TagName;
		CLexBufPos pos;
	};
	//тип списка загруженных специализированных параметров
	typedef std::vector<SSFPElem*> TSFPElemStore;
	//список загруженных специализированных параметров
	TSFPElemStore SFPElemStore;
public:
	CSpecFPSection(const CBaseName* parent) : parent_element(parent), BaseType(NULL) {};
	~CSpecFPSection();
	int AssignFPElems(CLexBuf *lb, CBaseVarVector &FPVector);
};//CSpecFPSection


//-----------------------------------------------------------------------------
//список обобщающих параметров
class CCommonPars : public CFormalPars
{
public:
	//инициализация обобщенных параметров обобщающей процедуры
	int Init(CLexBuf *lb, CBaseName* parent_element);
	//инициализация обобщенных параметров обработчика парам. спец-ции
	int InitSpec(CLexBuf *lb, CBaseName* parent_element);
};


//-----------------------------------------------------------------------------
//модуль
class CModule : public CBaseName
{
public:
	CModule(const CBaseName* parent) : CBaseName(id_CModule, parent), DeclSeq(NULL), StatementSeq(NULL) {};
	~CModule();
	//поиск импортированного имени в таблице имен, NULL - если имя не найдено
	CBaseName* FindImportedName(const char* module_name, const char* search_name) const;
	void WriteDFN(DFN_file& f);
	//создание файла _O2M_main.cpp с функцией O2M_SYS_main_init
	bool WriteCPP_main();
	//декларации
	CDeclSeq* DeclSeq;
	//последовательность операторов
	CStatementSeq* StatementSeq;
	//процедура инициализации модуля (с передачей объекта project)
	int Init(const CProject *project, CLexBuf *lb);
	//заглушка для виртуального метода базового класса
	int Init(CLexBuf*) {throw error_Internal("CModule::Init(CLexBuf*)");};
	//генерация кода C++
	void WriteCPP(CPP_files& f);
	//поиск имени в таблице имен
	CBaseName* FindName(const char* search_name) const;
	//добавление указанного эл-та в таблицу имен
	void AddName(CBaseName* BN) const;
};//CModule


//-----------------------------------------------------------------------------
//модуль сгенерированный из .dfn
class CDfnModule : public CBaseName
{
public:
	CBaseName* FindImportedName(const char *module_name, const char *search_name) const;
	//добавление указанного эл-та в таблицу имен
	void AddName(CBaseName* BN) const;
	CDfnModule(const CBaseName* parent) : CBaseName(id_CDfnModule, parent),
		DfnDeclSeq(NULL), full_path(NULL), alias_name(NULL) {};
	~CDfnModule();
	void WriteCPP(CPP_files& f);
	//процедура инициализации модуля (с передачей объекта project)
	virtual int Init(const CProject *project, CLexBuf *lb);
	//заглушка для виртуального метода базового класса
	int Init(CLexBuf*) {throw error_Internal("CDfnModule::Init(CLexBuf*)");};
	//поиск имени в таблице имен
	CBaseName* FindName(const char* search_name) const;
	//dfn декларации
	CDfnDeclSeq* DfnDeclSeq;
	//путь к dfn модулю, если он во внешней директории (иначе NULL)
	char* full_path;
	//псевдоним модуля (AliasName := RealName) 
	char* alias_name;
};//CDfnModule


//-----------------------------------------------------------------------------
//модуль SYSTEM (его .dfn файл должен быть пустым)
class CDfnModuleSystem : public CDfnModule
{
public:
	CDfnModuleSystem(const CBaseName* parent);
	//процедура инициализации модуля (с передачей объекта project)
	int Init(const CProject *project, CLexBuf *lb);
};//CDfnModuleSystem


//-----------------------------------------------------------------------------
//импортированный модуль
class CImportModule : public CBaseName
{
public:
	void WriteDFN(DFN_file &f);
	char *real_name;				//настоящее имя модуля
	CImportModule(const CBaseName* parent) : CBaseName(id_CImportModule, parent), real_name(NULL) {};
	~CImportModule() {delete[] real_name;};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
};//CImportModule


//-----------------------------------------------------------------------------
//процедура
class CProcedure : public CBaseName
{
public:
	//добавление указанного эл-та в таблицу имен
	void AddName(CBaseName* BN) const;
	bool CompareProcNames(const CProcedure* P) const;
	CProcedure(const CBaseName* parent) : CBaseName(id_CProcedure, parent),
		Receiver(NULL), FormalPars(NULL), DeclSeq(NULL), StatementSeq(NULL), have_return(hr_No) {}
	~CProcedure();
	CBaseName* FindImportedName(const char *module_name, const char *search_name) const;
	//поиск имени в таблице имен
	CBaseName* FindName(const char* search_name) const;
	//количество обобщенных параметров процедуры (если есть)
	virtual int GetCommonParsCount() {return 0;};
	//количество формальных параметров процедуры
	int GetFormalParsCount() {return FormalPars->FPStore.size();};
	EName_id GetResultId() const;
	int Init(CLexBuf *lb);
	static bool IsProcId(const EName_id id);
	void WriteCPP(CPP_files& f);
	void WriteCPP_RECORD(CPP_files& f, const CRecordType* RT, const bool to_h);
	void WriteDFN(DFN_file& f);
	CReceiver* Receiver;
	//список формальных параметров
	CFormalPars *FormalPars;
	//декларации
	CDeclSeq *DeclSeq;
	//последовательность операторов
	CStatementSeq *StatementSeq;
protected:
	//признак наличия в теле процедуры(функции) оператора RETURN
	EHaveRet have_return;
};//CProcedure


//-----------------------------------------------------------------------------
//обработчик параметрической специализации
class CHandlerProc : public CProcedure
{
	//преобразование обобщающих параметров в формальные
	int ConvertParams();
public:
	//количество обобщенных параметров процедуры
	int GetCommonParsCount() {return CommonPars->FPStore.size();};
	CHandlerProc(const CBaseName* parent);
	~CHandlerProc();
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
	//обработчик парам. спец-ции не заносится в DFN файл
	void WriteDFN(DFN_file& f) {throw error_Internal("CHandlerProc::WriteDFN");};
	//список обобщающих параметров
	CCommonPars *CommonPars;
//private:
	//имя модуля (NULL для текущего модуля), экспортирующего обобщенную процедуру, к которой привязан данный обработчик
	char* QualName;
	//числ. значение для создания уникального (в пределах модуля) имени обработчика
	int UID;
};//CHandlerProc


//-----------------------------------------------------------------------------
//опрережающее описание процедуры ForwardDecl
class CForwardDecl : public CProcedure
{
public:
	bool CheckSatisfied(CLexBuf *lb) const;
	CForwardDecl(const CBaseName* parent);
	int CompareDeclarations(CProcedure* P);
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
private:
	//позиция в потоке лексем на случай отсутствия процедуры для данного опережающего описания
	CLexBufPos FDeclPos;
	//признак наличия процедуры для данного опережающего описания
	bool Satisfied;
};//CForwardDecl


//-----------------------------------------------------------------------------
//обобщающая процедура
class CCommonProc : public CProcedure
{
protected:
	//запись кода одного параметра при вызове обработчика параметрической специализации
	void WriteCPP_HPar(CPP_files& f, CBaseVar* par);
	//проверка совпадения имен формальных и обобщающих параметров, конвертация записей в обобщенные переменные
	int CheckParams();
public:
	//количество обобщенных параметров процедуры
	int GetCommonParsCount() {return CommonPars->FPStore.size();};
	CCommonProc(const CBaseName* parent);
	~CCommonProc();
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
	CCommonPars *CommonPars;		//список обобщающих параметров
	//запись в .dfn файл
	void WriteDFN(DFN_file& f);
private:
	//признак наличия обработчика по умолчанию
	bool DefH;
};//CCommonProc


//-----------------------------------------------------------------------------
//процедура, объявленная в DFN
class CDfnProcedure : public CProcedure
{
public:
	CDfnProcedure(const CBaseName* parent);
	int Init(CLexBuf *lb);
};//CDfnProcedure


//-----------------------------------------------------------------------------
//обобщающая процедура, объявленная в DFN
class CDfnCommonProc : public CCommonProc
{
public:
	CDfnCommonProc(const CBaseName* parent);
	int Init(CLexBuf *lb);
};//CDfnCommonProc


//-----------------------------------------------------------------------------
//тип PTR (тип модуля SYSTEM)
class CPtrType : public CBaseType
{
public:
	CPtrType(const CBaseName* parent);
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	//тип PTR не инициализируется из потока лексем
	int Init(CLexBuf *lb) {return 0;};
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
};//CPtrType


//-----------------------------------------------------------------------------
//тип BOOEAN (основной тип)
class CBooleanType : public CBaseType
{
public:
	CBooleanType(const CBaseName* parent) : CBaseType(id_CBooleanType, parent) {};
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	//получение кода названия данного типа в C++
	static const char* GetCPPTypeName() {return "bool";};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
};//CBooleanType


//-----------------------------------------------------------------------------
//тип CHAR (основной тип)
class CCharType : public CBaseType
{
public:
	CCharType(const CBaseName* parent) : CBaseType(id_CCharType, parent) {};
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	//получение кода названия данного типа в C++
	static const char* GetCPPTypeName() {return "char";};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
};//CCharType


//-----------------------------------------------------------------------------
//тип SHORTINT (основной тип)
class CShortintType : public CBaseType
{
public:
	CShortintType(const CBaseName* parent) : CBaseType(id_CShortintType, parent) {};
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	//получение кода названия данного типа в C++
	static const char* GetCPPTypeName() {return "short";};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
};//CShortintType


//-----------------------------------------------------------------------------
//тип INTEGER (основной тип)
class CIntegerType : public CBaseType
{
public:
	CIntegerType(const CBaseName* parent) : CBaseType(id_CIntegerType, parent) {};
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	//получение кода названия данного типа в C++
	static const char* GetCPPTypeName() {return "int";};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
};//CIntegerType


//-----------------------------------------------------------------------------
//тип LONGINT (основной тип)
class CLongintType : public CBaseType
{
public:
	CLongintType(const CBaseName* parent) : CBaseType(id_CLongintType, parent) {};
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	//получение кода названия данного типа в C++
	static const char* GetCPPTypeName() {return "long";};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
};//CLongintType


//-----------------------------------------------------------------------------
//тип REAL (основной тип)
class CRealType : public CBaseType
{
public:
	CRealType(const CBaseName* parent) : CBaseType(id_CRealType, parent) {};
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	//получение кода названия данного типа в C++
	static const char* GetCPPTypeName() {return "double";};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
};//CRealType


//-----------------------------------------------------------------------------
//тип LONGREAL (основной тип)
class CLongrealType : public CBaseType
{
public:
	CLongrealType(const CBaseName* parent) : CBaseType(id_CLongrealType, parent) {};
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	//получение кода названия данного типа в C++
	static const char* GetCPPTypeName() {return "long double";};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
};//CLongrealType


//-----------------------------------------------------------------------------
//тип SET (основной тип)
class CSetType : public CBaseType
{
public:
	CSetType(const CBaseName* parent) : CBaseType(id_CSetType, parent) {};
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	//получение кода названия данного типа в C++
	static const char* GetCPPTypeName() {return "int";};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
};//CSetType


//-----------------------------------------------------------------------------
//тип Qualident
class CQualidentType : public CBaseType
{
public:
	CQualidentType(const CBaseName* parent) : CBaseType(id_CQualidentType, parent), Qualident(NULL) {};
	~CQualidentType() {delete Qualident;};
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	//получение ук. на действительный тип (не QualidentType), имя которого содержит QualidentType (с проверкой экспорта типа)
	int GetNamedType(CBaseType* &NamedType, const bool check_external) const;
	EName_id GetResultId() const;
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
	//имя типа
	CQualident* Qualident;
};//CQualidentType


//-----------------------------------------------------------------------------
//тип ARRAY (одномерный массив)
class CArrayType : public CBaseType
{
public:
	CArrayType(const CBaseName* parent) : CBaseType(id_CArrayType, parent), size(0), Type(NULL) {};
	~CArrayType() {delete Type;};
	int CheckComplete(CLexBuf *lb) const;
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	//получение ук. на тип эл-тов массива (последний тип в цепочке типов, не являющийся массивом)
	CBaseType* FindLastType() const;
	//для перекрытия метода CBaseName::FindName, который не должен вызываться
	CBaseName* FindName(const char*) const {return NULL;};
	//вычисление максимального количества размерностей массива
	long GetDimCount();
	EName_id GetResultId(int dimension) const;
	//получение типа для указанной размерности массива, отсчет размерностей с 0
	//при указании недопустимой размерности возврат NULL
	CBaseType* GetType(const int dimension);
	int Init(CLexBuf *lb);
	//запись кода объявления CArrayType
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
	//размер массива, если 0 == size, массив является открытым
	long size;
	//тип эл-тов массива: ARRAY OF Type
	CBaseType *Type;
};//CArrayType


//-----------------------------------------------------------------------------
//тип RECORD
class CRecordType : public CBaseType
{
public:
	//добавление указанного эл-та в таблицу имен
	void AddName(CBaseName* BN) const;
	int AddProcReference(CProcedure* P) const;
	int CheckComplete(CLexBuf *lb) const;
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	CRecordType(const CBaseName* parent) : CBaseType(id_CRecordType, parent),
		Qualident(NULL),
		in_checking_complete(false),
		RuntimeId(NULL) {};
	~CRecordType();
	//поиск имени в списке полей
	CBaseName* FindName(const char* search_name) const;
	//получение ид. типа времени исполнения
	const char* GetRuntimeId() {return RuntimeId;};
	int Init(CLexBuf *lb);
	//инициализация строкового ид. типа времени исполнения
	int InitRuntimeId();
	CRecordType* IsExtension(const char* module_name, const char* type_name);
	//генерация кода
	void WriteCPP(CPP_files& f, const bool to_h);
	//запись в DFN объявления типа
	void WriteDFN(DFN_file& f);
	//описание приемника
	CQualident* Qualident;
//private:
	//список полей записи и ссылок (не требующих очистки) на связанные с данным типом процедуры
	mutable CBaseNameVector FieldStore;
	//для проверки объявления рекурсивного указателя (ук. на объект того же типа, в кот. он объявлен)
	mutable bool in_checking_complete;
	//строковый ид. типа времени исполнения
	char* RuntimeId;
};//CRecordType


//-----------------------------------------------------------------------------
//тип параметрическое обобщение
class CCommonType : public CBaseType
{
public:
	//описание эл-та обобщения
	struct SSpec {
		char* Tag;		//действителен только при TagType != tt_Type
		char* QualName;	//имя модуля в случае импорта эл-та обобщения
		char* Name;		//имя эл-та обобщения
		bool IsExtended;//специализация добавлена путем расширения обобщения
	};
	const SSpec* FindSpec(const char* QualName, const char* Name, const char *Tag);
	const SSpec* FindSpecByName(const char *QualName, const char* Name) const;
	const SSpec* FindSpecByTag(const char* Tag) const;
	const SSpec* GetDefaultSpec() {return DefaultSpec;};
	CCommonType(const CBaseName* parent) : CBaseType(id_CCommonType, parent),
		IsLocal(false),
		TagType(tt_Default),
		DefaultSpec(NULL) {};
	~CCommonType();
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	//уничтожение эл-та обобщения
	static void DelSpec(SSpec* sp);
	int Init(CLexBuf *lb);
	int InitExtension(CLexBuf *lb, CDeclSeq* DS);
	//создание нового эл-та обобщения
	static SSpec* NewSpec(const char* newTag, const char* newQual, const char* newName, const bool newIsExtended);
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
	//признак локальности обобщения (объявлено с признаком LOCAL)
	bool IsLocal;
	//список эл-тов обобщения
	typedef std::vector<SSpec*> SpecStore_type;
	SpecStore_type SpecStore;
	//тип параметров обобщения
	enum {tt_Default, tt_Type} TagType;
private:
	//добавление эл-та обобщения в список
	void AddSpec(const char* newTag, const char* newQual, const char* newName, const bool newIsExtended);
	//проверка допустимости типа обобщения
	int CheckSpecType(const char* pref_ident, const char* ident, const CBaseName* parent) const;
	//очистка списка признаков
	void ClearTmpTagStore();
	//занесение в .2ml файл информации об одном эл-те обобщения
	static void WriteCPP_SpecInfo(TFileType* f2ml, const bool IsDef, const CBaseName* parent_element, const SSpec* sp);
	//тип списка признаков
	typedef StringVector TmpTagStore_type;
	//специализация по умолчанию
	SSpec* DefaultSpec;
	//список признаков
	TmpTagStore_type TmpTagStore;
};


//-----------------------------------------------------------------------------
//расширение параметрического обобщения
//используется только для занесения в .2ml файл информации о расширении обобщения
class CCommonExtensionType : public CBaseType
{
	int CreateType(CBaseType* &BaseType) const {return 0;};
	int Init(CLexBuf *lb) {return 0;};
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const {return 0;};
public:
	//добавление эл-та обобщения в список
	void AddSpec(const char* newTag, const char* newQual, const char* newName);
	~CCommonExtensionType();
	CCommonExtensionType(const CBaseName* parent) : CBaseType(id_CCommonExtensionType, parent),
		TypeModuleName(NULL),
		TypeName(NULL)
	{
		name = str_new_copy("");
	};
	void WriteCPP(CPP_files& f, const bool to_h);
	//список эл-тов обобщения
	CCommonType::SpecStore_type SpecStore;
	char* TypeModuleName;
	char* TypeName;
};


//-----------------------------------------------------------------------------
//обобщенный тип, специализированный конкретным признаком
class CSpecType : public CBaseType
{
public:
	const char* GetQualSpecName() const;
	const char* GetSpecName() const {return TagName;};
	//данная процедура не должна использоваться для SpecType
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	CSpecType(const CBaseName* parent) : CBaseType(id_CSpecType, parent),
		Qualident(NULL),
		QualTagName(NULL),
		TagName(NULL)
	{};
	~CSpecType() {
		delete Qualident;
		delete[] QualTagName;
		delete[] TagName;
	};
	CBaseName* FindName(const char* search_name) const;
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
	//имя обобщенного типа
	CQualident* Qualident;
private:
	//уточнение названия признака
	char* QualTagName;
	//название признака
	char* TagName;
};


//-----------------------------------------------------------------------------
//тип POINTER
class CPointerType : public CBaseType
{
public:
	int CheckComplete(CLexBuf *lb) const;
	CPointerType(const CBaseName* parent) : CBaseType(id_CPointerType, parent),
		forward(false),
		Qualident(NULL),
		Type(NULL) {};
	~CPointerType();
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	CBaseType* FindType() const;
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
//private:
	//признак использования еще не объявленного имени
	bool forward;
	//не NULL в случае ук. на именованный тип
	CQualident* Qualident;
	//не NULL в случае ук. на  неименованный тип RECORD
	CBaseType *Type;
	//позиция описания именованного типа в буфере лексем (при отсутствии описания типа в локальном блоке)
	CLexBufPos TypePos;
};//CPointerType


//-----------------------------------------------------------------------------
//тип PROCEDURE
class CProcedureType : public CBaseType
{
public:
	CProcedureType(const CBaseName* parent) : CBaseType(id_CProcedureType, parent) {};
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	EName_id GetResultId() const;
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
	//список формальных параметров
	CFormalPars FormalPars;
};//CProcedureType


//-----------------------------------------------------------------------------
//последовательнось загруженных dfn модулей
class CDfnModuleSeq
{
public:
	CDfnModuleSeq(const CBaseName* parent);
	~CDfnModuleSeq();
	void EnumDfnModules(CProject &project);
	//добавление эл-та в список модулей
	void AddModule(CDfnModule* DM);
	//поиск имени в таблице имен
	CDfnModule* FindName(const char* search_name) const;
	const CBaseName* parent_element;
	void WriteCPP(CPP_files& f);
	typedef std::vector <CDfnModule*> DfnModuleList_type;
	DfnModuleList_type DfnModuleList;
protected:
};//CDfnModuleSeq


#endif	//O2M_Type_h
