//=============================================================================
// Объявление классов переменных (Var)
//=============================================================================

#ifndef O2M_Var_h
#define O2M_Var_h

#include "Common.h"
#include "Base.h"
#include "Expr.h"
#include "Project.h"
#include "LexAnl.h"
#include "Type.h"
#include "StdProc.h"


//классы, на которые имеются ссылки в данном файле
class CCaseLabels;
class CGuard;


//-----------------------------------------------------------------------------
//переменная типа ARRAY
class CArrayVar : public CBaseVar
{
public:
	CArrayVar(const CBaseName* parent) : CBaseVar(id_CArrayVar, parent),
		ArrayType(NULL), ConstString(NULL), ConstStringIsChar(false) {};
	~CArrayVar();
	int CheckComplete(CLexBuf *lb);
	CBaseVar* CreateConst(const CBaseName* parent) const;
	CBaseVar* CreateVar(const CBaseName* parent) const;
	//инициализация массива строкой символов
	int SetConstValue(const char* st);
	void WriteCPP(CPP_files& f);
	void WriteCPP_ConstValue(CPP_files& f);
	void WriteCPP_fp(CPP_files &f, const bool to_h);
	//запись объявления поля-массива в записи (в отличие от остальных переменных здесь нельзя использовать WriteCPP_fp)
	void WriteCPP_rec(CPP_files &f, const bool to_h);
	void WriteDFN(DFN_file& f);
	//для хранения типа данной переменной
	CArrayType* ArrayType;
	//для хранения строки символов (ARRAY OF CHAR)
	char* ConstString;
	//признак того, что массив соответствует 1 символу
	bool ConstStringIsChar;
protected:
	typedef CBaseVarVector VarList_type;
};//CArrayVar


//-----------------------------------------------------------------------------
//переменная типа BOOLEAN
class CBooleanVar : public CBaseVar
{
	//необходимо для расчета констант
	friend class CMaxStdProcFunc;
	friend class CMinStdProcFunc;
	friend class COddStdProcFunc;
	friend class CExpr;
	friend class CFactor;
	friend class CTermPair;
	friend class CSimpleExprPair;
public:
	CBooleanVar(const CBaseName* parent) : CBaseVar(id_CBooleanVar, parent) {};
	CBaseVar* CreateConst(const CBaseName* parent) const;
	CBaseVar* CreateVar(const CBaseName* parent) const;
	void SetConstValue(const bool& Value) {
		is_const = true;
		ConstValue = Value;
	};
	void WriteCPP(CPP_files& f);
	void WriteCPP_ConstValue(CPP_files& f);
	void WriteCPP_fp(CPP_files &f, const bool to_h);
	void WriteDFN(DFN_file& f);
//private:
	//значение константы (в случае константы)
	bool ConstValue;
};//CBooleanVar


//-----------------------------------------------------------------------------
//переменная типа CHAR
class CCharVar : public CBaseVar
{
	//необходимо для расчета констант
	friend class CExprList;
	friend class CCaseLabels;
	friend class CCapStdProcFunc;
	friend class CChrStdProcFunc;
	friend class CMaxStdProcFunc;
	friend class CMinStdProcFunc;
	friend class COrdStdProcFunc;
public:
	CCharVar(const CBaseName* parent) : CBaseVar(id_CCharVar, parent), ConstValue('\0') {};
	CBaseVar* CreateConst(const CBaseName* parent) const;
	CBaseVar* CreateVar(const CBaseName* parent) const;
	void SetConstValue(const char ch);
	void WriteCPP(CPP_files& f);
	void WriteCPP_ConstValue(CPP_files& f);
	void WriteCPP_fp(CPP_files &f, const bool to_h);
	void WriteDFN(DFN_file& f);
//private:
	//значение константы (в случае константы)
	char ConstValue;
};//CCharVar


//-----------------------------------------------------------------------------
//переменная типа CommonVar
class CCommonVar : public CBaseVar
{
public:
	CCommonVar(const CBaseName* parent) : CBaseVar(id_CCommonVar, parent),
		CPPCompoundName(NULL),
		QualSpecName(NULL),
		SpecName(NULL),
		Tag(NULL)
	{};
	~CCommonVar() {
		delete[] CPPCompoundName;
		delete[] QualSpecName;
		delete[] SpecName;
		delete[] Tag;
	};
	CBaseVar* CreateVar(const CBaseName* parent) const;
	CBaseName* FindName(const char *search_name) const;
	//поиск типа специализации (если есть)
	CBaseType* FindType() const;
	const char* GetCPPCompoundName() const {return CPPCompoundName;};
	void GetTagName(const char* &QualName, const char* &Name, const char* &TagName);
	bool IsPureCommon();
	//установка названия признака и названия типа признака
	int SetTagName(const char* QualName, const char* Name);
	void WriteCPP(CPP_files& f);
	void WriteCPP_fp(CPP_files &f, const bool to_h);
	void WriteDFN(DFN_file& f);
//private:
	//название C++ параметра (члена union), соответствующего специализации переменной
	char* CPPCompoundName;
	//название модуля, экспортирующего тип признака
	char* QualSpecName;
	//название типа признака
	char* SpecName;
	//название признака (может отсутствовать)
	char* Tag;
};//CCommonVar


//-----------------------------------------------------------------------------
//переменная типа INTEGER
class CIntegerVar : public CBaseVar
{
public:
	CIntegerVar(const CBaseName* parent) : CBaseVar(id_CIntegerVar, parent) {};
	CBaseVar* CreateConst(const CBaseName *parent) const;
	CBaseVar* CreateVar(const CBaseName* parent) const;
	long GetIntValue() const {return ConstValue;};
	void SetConstValue(const char* st);
	void WriteCPP(CPP_files& f);
	void WriteCPP_ConstValue(CPP_files& f);
	void WriteCPP_fp(CPP_files &f, const bool to_h);
	void WriteDFN(DFN_file& f);
	//значение константы (в случае константы)
	int ConstValue;
};//CIntegerVar


//-----------------------------------------------------------------------------
//переменная типа LONGINT
class CLongintVar : public CBaseVar
{
public:
	CLongintVar(const CBaseName* parent) : CBaseVar(id_CLongintVar, parent) {};
	CBaseVar* CreateConst(const CBaseName *parent) const;
	CBaseVar* CreateVar(const CBaseName* parent) const;
	long GetIntValue() const {return ConstValue;};
	void WriteCPP(CPP_files& f);
	void WriteCPP_ConstValue(CPP_files& f);
	void WriteCPP_fp(CPP_files &f, const bool to_h);
	void WriteDFN(DFN_file& f);
	//значение константы (в случае константы)
	long ConstValue;
};//CLongintVar


//-----------------------------------------------------------------------------
//переменная типа LONGREAL
class CLongrealVar : public CBaseVar
{
public:
	CLongrealVar(const CBaseName* parent) : CBaseVar(id_CLongrealVar, parent) {};
	CBaseVar* CreateConst(const CBaseName *parent) const;
	CBaseVar* CreateVar(const CBaseName* parent) const;
	void SetConstValue(const char* st);
	void WriteCPP(CPP_files& f);
	void WriteCPP_ConstValue(CPP_files& f);
	void WriteCPP_fp(CPP_files &f, const bool to_h);
	void WriteDFN(DFN_file& f);
	//значение константы (в случае константы)
	long double ConstValue;
};//CLongrealVar


//-----------------------------------------------------------------------------
//переменная типа POINTER
class CPointerVar : public CBaseVar
{
	friend class CPointerType;
	friend class CQualidentType;
	friend class CGuard;
public:
	int CheckComplete(CLexBuf *lb);
	CBaseName* FindName(const char* search_name) const;
	CPointerVar(const CBaseName* parent) : CBaseVar(id_CPointerVar, parent),
		IsArray(false),
		IsRecord(false),
		qualident_type(false),
		Type(NULL) {};
	CPointerVar(const CBaseName* parent, CBaseType* PointedType) : CBaseVar(id_CPointerVar, parent),
		IsArray(false),
		IsRecord(false),
		qualident_type(false),
		Type(PointedType) {};
	~CPointerVar() {delete Type;};
	CBaseVar* CreateConst(const CBaseName *parent) const;
	CBaseVar* CreateVar(const CBaseName* parent) const;
	//поиск типа (не QualidentType), на кот. указывает указатель
	CBaseType* FindType() const;
	//возврат true, если указатель указывает на ARRAY
	bool IsArrayPointer() const {return IsArray;};
	//возврат true, если указатель указывает на RECORD или SpecType
	bool IsRecordPointer() const {return IsRecord;};
	//принудительная установка признака ук. на запись - вызывается в объектах, самостоятельно
	//проверяющих принадлежность указателя (CReceiver, CGuard)
	void SetIsRecord() {IsRecord = true; IsArray = false;};
	//генерация кода
	void WriteCPP(CPP_files& f);
	void WriteCPP_fp(CPP_files &f, const bool to_h);
	void WriteCPP_ConstValue(CPP_files& f);
	void WriteDFN(DFN_file& f);
//private:
	//запись кода доп. переменных, содержащих размерности в случае открытого массива
	void WriteCPP_array(CPP_files& f);
	//признак того, что указываемый тип - массив
	bool IsArray;
	//признак того, что указываемый тип - запись или специализация
	bool IsRecord;
	//признак переменной типа QualidentType (т.к. type_name может быть инициализирован в CPointerType::CreateVar)
	bool qualident_type;
	//неименованный тип, на кот. указывает переменная (CRecordType, CArrayType, ...) или NULL (для именованного типа или ук. на именованный тип)
	CBaseType* Type;
	//позиция описания типа переменной в буфере лексем (устанавливается в CPointerType::CreateVar)
	CLexBufPos TypePos;
};//CPointerVar


//-----------------------------------------------------------------------------
//переменная типа PROCEDURE
class CProcedureVar : public CBaseVar
{
public:
	CProcedureVar(const CBaseName* parent) : CBaseVar(id_CProcedureVar, parent) {};
	CBaseVar* CreateVar(const CBaseName* parent) const;
	EName_id GetResultId() const;
	void WriteCPP(CPP_files& f);
	void WriteCPP_fp(CPP_files &f, const bool to_h);
	void WriteDFN(DFN_file& f);
	//список формальных параметров
	CFormalPars FormalPars;
};//CProcedureVar


//-----------------------------------------------------------------------------
//переменная типа PTR
class CPtrVar : public CBaseVar
{
public:
	CPtrVar(const CBaseName* parent) : CBaseVar(id_CPtrVar, parent) {};
	CBaseVar* CreateVar(const CBaseName* parent) const;
	void WriteCPP(CPP_files& f);
	void WriteCPP_fp(CPP_files &f, const bool to_h);
	void WriteDFN(DFN_file& f);
};//CBooleanVar


//-----------------------------------------------------------------------------
//переменная типа REAL
class CRealVar : public CBaseVar
{
public:
	CRealVar(const CBaseName* parent) : CBaseVar(id_CRealVar, parent) {};
	CBaseVar* CreateConst(const CBaseName *parent) const;
	CBaseVar* CreateVar(const CBaseName* parent) const;
	void SetConstValue(const char* st);
	void WriteCPP(CPP_files& f);
	void WriteCPP_ConstValue(CPP_files& f);
	void WriteCPP_fp(CPP_files &f, const bool to_h);
	void WriteDFN(DFN_file& f);
	//значение константы (в случае константы)
	double ConstValue;
};//CRealVar


//-----------------------------------------------------------------------------
//переменная типа RECORD
class CRecordVar : public CBaseVar
{
public:
	//добавление указанного эл-та в список полей
	void AddName(CBaseName* BN) const;
	int CheckComplete(CLexBuf *lb);
	CBaseVar* CreateVar(const CBaseName* parent) const;
	CRecordVar(const CBaseName* parent) : CBaseVar(id_CRecordVar, parent), Qualident(NULL) {};
	~CRecordVar();
	//поиск имени в списке полей
	CBaseName* FindName(const char* search_name) const;
	//генерация кода при объявлении переменной
	void WriteCPP(CPP_files& f);
	void WriteCPP_fp(CPP_files &f, const bool to_h);
	//запись кода при использования CRecordVar для описания типа указателя
	void WriteCPP_pointer(CPP_files& f);
	void WriteDFN(DFN_file& f);
	//название базового типа
	CQualident* Qualident;
//private:
	//тип контейнера для хранения списка полей записи
	typedef CBaseVarVector TFieldStore;
	//контейнер для хранения списка полей записи
	mutable TFieldStore FieldStore;
};//CRecordVar


//-----------------------------------------------------------------------------
//переменная типа SET
class CSetVar : public CBaseVar
{
	//необходимо для расчета констант
	friend class CExpr;
	friend class CSimpleExpr;
	friend class CTermPair;
	friend class CSimpleExprPair;
public:
	CBaseVar* CreateConst(const CBaseName *parent) const;
	CBaseVar* CreateVar(const CBaseName* parent) const;
	CSetVar(const CBaseName* parent) : CBaseVar(id_CSetVar, parent), ConstValue(0) {};
	~CSetVar();
	//обработка конструктора множества, если есть (может не быть), при возникновении ошибок
	//должно обеспечиваться уничтожение всех временных объектов
	int SetInit(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
	void WriteCPP_ConstValue(CPP_files& f);
	void WriteCPP_fp(CPP_files &f, const bool to_h);
	void WriteDFN(DFN_file& f);
private:
	//рассчитанное константное значение, действительно только при is_const == true
	//тип ConstValue должен соответствовать типу, название которого возвращает CSetType::GetCPPTypeName,
	//это же относится ко всем типам, имеющим отношение к SET (например, в CSetVarElem)
	int ConstValue;
	//список эл-тов множества, может содержать выражения, действителен только при is_const == false
	CBaseVector SetElems;
};//CSetVar


//-----------------------------------------------------------------------------
//эл-т множества SET
class CSetVarElem : public CBase
{
public:
	CSetVarElem(const CBaseName *parent) : CBase(parent),
		HighBoundValue(0), HighExpr(NULL), IsRange(false), LowBoundValue(0), LowExpr(NULL), SetValue(0) {};
	~CSetVarElem();
	//получение константного значения SET (действительно только при IsConst() == true)
	int GetConstValue() const {return SetValue;};
	int Init(CLexBuf *lb);
	//проверка контантности данного объекта
	bool IsConst() const {return !(HighExpr || LowExpr);};
	void WriteCPP(CPP_files& f);
private:
	//рассчитанное значение верхней границы диапазона, действительно при !HighExpr
	int HighBoundValue;
	//выражение для верхней границы диапазона, действительно при IsRange
	CExpr *HighExpr;
	//признак диапазона (LowBoundValue..HighBoundValue)
	bool IsRange;
	//рассчитанное значение нижней границы диапазона, действительно при !LowExpr
	int LowBoundValue;
	//выражение для нижней границы диапазона
	CExpr *LowExpr;
	//рассчитанное константное значение множества, действительно только при IsConst() == true
	int SetValue;
};//CSetVarElem


//-----------------------------------------------------------------------------
//переменная типа SHORTINT
class CShortintVar : public CBaseVar
{
public:
	CBaseVar* CreateConst(const CBaseName *parent) const;
	CBaseVar* CreateVar(const CBaseName* parent) const;
	CShortintVar(const CBaseName* parent) : CBaseVar(id_CShortintVar, parent) {};
	long GetIntValue() const {return ConstValue;};
	void WriteCPP(CPP_files& f);
	void WriteCPP_ConstValue(CPP_files& f);
	void WriteCPP_fp(CPP_files &f, const bool to_h);
	void WriteDFN(DFN_file& f);
	//значение константы (в случае константы)
	short ConstValue;
};//CShortintVar


#endif	//O2M_Var_h
