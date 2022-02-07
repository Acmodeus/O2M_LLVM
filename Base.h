//=============================================================================
// Объявление базовых и вспомогательных классов
//=============================================================================

#ifndef O2M_Base_h
#define O2M_Base_h

#include "Common.h"


//-----------------------------------------------------------------------------
//тип возможных состояний типов наличия RETURN (нет, не все послед-ти, есть)
enum EHaveRet {hr_No, hr_NotAll, hr_Yes};


//-----------------------------------------------------------------------------
//исключение для сообщения об внутренней ошибке
class error_Internal
{
public:
	error_Internal(char *message) : error_message(message) {};
	char *error_message;
};


//-----------------------------------------------------------------------------
//класс для передачи параметров в WriteCPP
class CPP_files
{
public:
	CPP_files() : f2ml(NULL), fc(NULL), fh(NULL), tab_level_c(0), ext_offs(0), f_name(NULL) {};
	//деструктор, закрываюший открытые файлы
	~CPP_files();
	//открытие файлов для записи с выдачей сообщения при возникновении ошибки
	int Init(const char* name);
	//вставка табуляций в файл описаний
	void tab_fc();
	//файл для создания обобщений
	TFileType *f2ml;
	//файл описаний
	TFileType *fc;
	//файл заголовков
	TFileType *fh;
	//количество табуляций для текущей строки файла описаний
	int tab_level_c;
private:
	//смещение описания расширения от начала полного имени файла
	int ext_offs;
	//для хранения полных имен файлов включая путь и расширение
	char* f_name;
};//CPP_files


//-----------------------------------------------------------------------------
//класс для передачи параметров в WriteDFN
class DFN_file
{
public:
	DFN_file() : f(NULL), tab_level(0), f_name(NULL) {};
	//деструктор, закрываюший открытый файл
	~DFN_file();
	//открытие файла для записи с выдачей сообщения при возникновении ошибки
	int Init(const char* name);
	void tab();
	//DFN файл
	TFileType *f;
	//количество табуляций для текущей строки DFN файла
	int tab_level;
private:
	//для хранения полного имени файла включая путь и расширение
	char* f_name;
};//DFN_file


//-----------------------------------------------------------------------------
//базовый класс для операторов
class CBase
{
public:
	CBase(const CBaseName* parent) : parent_element(parent) {};
	const CBaseName* parent_element;
	virtual ~CBase() {};
	//проверка наличия оператора RETURN (только для операторов)
	virtual EHaveRet HaveRet() const {return hr_No;};
	virtual int Init(CLexBuf *lb) = 0;
	virtual void WriteCPP(CPP_files& f) = 0;
};//CBase


//-----------------------------------------------------------------------------
//базовый класс для именованных объектов
class CBaseName
{
public:
	CBaseName(EName_id id, const CBaseName* parent) : external(false),
		name(NULL),
		name_id(id),
		parent_element(parent) {};
	virtual ~CBaseName() {delete[] name;};
	//добавление указанного эл-та в таблицу имен
	virtual void AddName(CBaseName*) const {throw error_Internal("CBaseName::AddName");};
	//копирование имени и атрибутов в указанный объект (BaseName)
	void Assign(CBaseName* BaseName) const;
	//поиск имени в списке имен объекта (вызывается только из классов, в которых переопределена)
	virtual CBaseName* FindName(const char*) const {throw error_Internal("CBaseName::FindName");};
	//получение по названию объекта из глобальной области видимости
	CBaseName* GetGlobalName(const char* global_name) const;
	//получение по уточненному названию объекта из глобальной области видимости
	CBaseName* GetGlobalName(const char* module_name, const char* global_name) const;
	//получение модуля (обычного или DFN) из эл-тов parent_element
	const CBaseName* GetParentModule() const;
	//получение id результата, если return id_CBaseName => выражение несовместимо
	virtual EName_id GetResultId() const {return id_CBaseName;};
	//инициализация объекта из потока лексем
	virtual int Init(CLexBuf *lb) = 0;
	//копирование в объект указанного имени
	void SetName(const char* NewName);
	//запись скомпилированного кода C++ (обычно объявления объекта)
	virtual void WriteCPP(CPP_files& f) = 0;
	//запись в .dfn файл (только для экспортируемых объектов)
	virtual void WriteDFN(DFN_file& f) {};
	//есть '*' (объект экспортируется)
	bool external;
	//имя объекта
	char *name;
	//множество допустимых типов именованных объектов
	EName_id name_id;
	//родительский эл-т (необходим для поиска в таблицах имен)
	const CBaseName* parent_element;
};//CBaseName


//-----------------------------------------------------------------------------
//базовый класс стандартных процедур-функций
class CStdProcFunc : public CBase
{
public:
	CStdProcFunc(const CBaseName *parent) : CBase(parent), ResultId(id_CBaseName) {};
	//получение вычисленного id результата выражения
	EName_id GetResultId() {return ResultId;};
	//создание константы
	virtual int CreateConst(CBaseVar* &BaseConst) {return s_m_Error;};
protected:
	EName_id ResultId;	//для хранения вычисленного id результата (или id_CBaseName)
};


//-----------------------------------------------------------------------------
//базовый класс для типов
class CBaseType : public CBaseName
{
public:
	CBaseType(EName_id id, const CBaseName* parent) : CBaseName(id, parent) {};
	//создание копии данного типа
	virtual int CreateType(CBaseType* &BaseType) const = 0;
	//создание переменной данного типа
	virtual int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const = 0;
	//получение кода названия данного типа в C++
	static const char* GetCPPTypeName(const EName_id ResultId);
	//получение псевдонима DFN модуля, экспортирующего данный тип (если есть)
	const char* GetModuleAlias() const;
	const char* GetModuleName() const;
	//получение id результата (преобразование id типа в id соответствующей переменной)
	EName_id GetResultId() const;
	bool IsSame(const char *module_name, const char *type_name) const;
	static bool IsTypeId(const EName_id id);
	virtual void WriteCPP(CPP_files &f, const bool to_h) = 0;
private:
	//сокрытие WriteCPP, т.к. в наследниках вызов данного метода должен отсутствовать
	void WriteCPP(CPP_files &f) {};
};//CBaseType


//-----------------------------------------------------------------------------
//базовый класс для переменных
class CBaseVar : public CBaseName
{
public:
	//копирование атрибутов текущего объекта в указанную переменную
	void Assign(CBaseVar *BaseVar) const;
	CBaseVar(EName_id id, const CBaseName* parent) : CBaseName(id, parent),
		is_const(false),
		is_guarded(false),
		is_read_only(false),
		is_var(false),
		type_module_name(NULL),
		type_name(NULL) {};
	virtual ~CBaseVar();
	//проверка завершенности всех типов, входящих в состав переменной
	//все классы переменных, которые могут включать тип (типы) указатель, должны переопределять данный метод
	virtual int CheckComplete(CLexBuf *lb) {return 0;};
	//создание константы, подразумевается, что is_const == true
	//все классы переменных, имеющих константную форму, должны переопределять данный метод
	virtual CBaseVar* CreateConst(const CBaseName* parent) const {throw error_Internal("CBaseVar::CreateConst");};
	//проверка идентификаторов объектов
	static bool IsRealId(const EName_id id);
	static bool IsIntId(const EName_id id);
	static bool IsDigitId(const EName_id id);
	static bool IsVarId(const EName_id id);
	//получение целочисленного значения переменной
	virtual long GetIntValue() const {throw error_Internal("CBaseVar::GetIntValue");};
	//код переменной в параметрах процедуры
	virtual void WriteCPP_fp(CPP_files &f, const bool to_h) = 0;
	//выдача псевдонима модуля, экспортирующего тип переменной (работает и для неименованных типов)
	const char* GetTypeModuleAlias();
	const char* GetTypeModuleName();
	//выдача имени типа переменной (именованный тип или тип CPP)
	const char* GetTypeName();
	void SetTypeName(const char *TypeModuleName, const char* TypeName);
	//создание копии переменной (без копирования данных в случае констант)
	virtual CBaseVar* CreateVar(const CBaseName* parent) const = 0;
	//запись кода при использовании константного значения, подразумевается, что is_const == true
	//все классы переменных, имеющих константную форму, должны переопределять данный метод
	virtual void WriteCPP_ConstValue(CPP_files& f) {throw error_Internal("CBaseVar::WriteCPP_ConstValue");};
	//если переменная объявлена как константа
	bool is_const;
	//если переменная под охраной (внутри WITH), при генерации кода необходимо приведение типа
	bool is_guarded;
	//есть '-' (объект экспортируется только для чтения)
	bool is_read_only;
	//если формальный параметр объявлен как VAR
	bool is_var;
	//получение id результата (в данном случае id переменной)
	EName_id GetResultId() const {return name_id;};
protected:
	//запись кода переменной именованного типа в параметры процедуры
	void WriteCPP_fp_named_type(CPP_files &f, const bool to_h);
	void WriteDFN_named_type(DFN_file &f);
	//название модуля, содержащего именованный тип переменной, или NULL
	char* type_module_name;
	//название типа переменной в случае именованного типа, или NULL
	char* type_name;
private:
	//переменные не инициализируются (обычно они создаются через Type->CreateVar)
	int Init(CLexBuf *lb) {throw error_Internal("CBaseVar::Init");};
};//CBaseVar


//-----------------------------------------------------------------------------
//IdentDef - временный объект для инициализации именованного объекта
//	(переменной, типа, и т.д.)
class CIdentDef : public CBaseName
{
public:
	int AssignVar(CBaseVar* BaseVar) const;
	CIdentDef(const CBaseName* parent, bool ReadOnlyEnabled) : CBaseName(id_CIdentDef, parent),
		is_common(false),
		is_read_only(false),
		QualTagName(NULL),
		read_only_enabled(ReadOnlyEnabled),
		TagName(NULL) {};
	~CIdentDef() {
		delete[] QualTagName;
		delete[] TagName;
	};
	int Init(CLexBuf *lb);
private:
	//не используется
	void WriteCPP(CPP_files& f) {};
	//признак наличия угловых скобок => объект относится к параметризованным обобщениям
	bool is_common;
	//признак наличия '-' - переменная только для чтения (для остальных объектов не используется)
	bool is_read_only;
	//уточнение названия признака
	char* QualTagName;
	//признак допустимости признака is_read_only
	bool read_only_enabled;
	//название признака
	char* TagName;
};//CIdentDef


//-----------------------------------------------------------------------------
//IdentList - временный объект для инициализации полей записей 
//	и формальных параметров процедур и опережающих описаний
class CIdentList
{
	CBaseType* BaseType;		//тип всех идент-ров в списке
	CBaseNameVector BNVector;	//список загруженных ид-ов
	const CBaseName* parent_element;
	bool is_local;				//признак создания локального IdentList (запрещен экспорт идентификаторов)
	//загрузка списка идентификаторов без создания новых объектов
	int LoadIdentList(CLexBuf *lb);
public:
	CIdentList(const CBaseName* parent, const bool is_local_list) : BaseType(NULL),
		parent_element(parent), is_local(is_local_list) {};
	~CIdentList();
	//инициализация указанного списка объектами-переменными
	int AssignVars(CLexBuf *lb);
};//CIdentList


//-----------------------------------------------------------------------------
//Qualident - [название модуля] "." название переменной
class CQualident
{
public:
	void Assign(CQualident* Qualident) const;
	void Clear();
	CQualident() : ident(NULL), pref_ident(NULL), TypeResultId(id_CBaseName) {};
	~CQualident();
	//создание копии CQualident
	void CreateCopy(CQualident *&qual) const;
	int Init(CLexBuf *lb, const CBaseName* parent_element);
	int InitTypeName(CLexBuf *lb, const CBaseName* parent_element);
	//запись кода для CQualident, содержащего имя типа
	void WriteCPP_type(CPP_files& f, const bool to_h, const CBaseName* parent_element);
	void WriteDFN(DFN_file& f);
	//основной идент
	char *ident;
	//первый идент
	char *pref_ident;
	//ResultId для типа (если Qualident является именем типа), или id_CBaseName
	EName_id TypeResultId;
};//CQualident


//-----------------------------------------------------------------------------
//класс для связи с операторами LOOP и WITH через систему parent_element
class CWithLoopLink : public CBaseName
{
public:
	CBaseName* FindImportedName(const char *module_name, const char *search_name) const;
	CWithLoopLink(const CBaseName *parent, bool UnderLoop) : CBaseName(id_CWithLoopLink, parent),
		IsUnderLoop(UnderLoop),
		Var(NULL),
		VarModuleName(NULL) {};
	~CWithLoopLink() {
		delete VarModuleName;
		delete Var;
	};
	//не требуется при связи с LOOP
	int Init(CLexBuf *lb) {return 0;};
	//методы для работы с таблицей имен (содержит только 1 переменную)
	void AddName(const char* ModuleName, CBaseName* GuardVar) const;
	CBaseName* FindName(const char* search_name) const;
	//получение значения IsUnderLoop (признак принадлежности к LOOP)
	bool UnderLoop() const {return IsUnderLoop;};
	//для передачи UID для LOOP через parent_element
	int LoopUID;
private:
	//признак CWithLoopLink для оператора LOOP
	bool IsUnderLoop;
	//переменная, для которой применяется охрана типа (только для WITH)
	mutable CBaseName* Var;
	//имя модуля в случае использования импортированной переменной
	mutable char* VarModuleName;
	//сокрытие метода генерации кода
	void WriteCPP(CPP_files& f) {};
};


#endif	//O2M_Base_h
