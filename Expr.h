//=============================================================================
// Объявление деклараций для построения выражений
//=============================================================================

#ifndef O2M_Expr_h
#define O2M_Expr_h

#include "Common.h"
#include "Base.h"

#include <math.h>


class CTermPair;
class CSimpleExprPair;
class CExpr;
class CFactor;
class CDesignator;
class CFormalPars;
class CCommonPars;

typedef std::vector <CExpr*> CExprVector;


//-----------------------------------------------------------------------------
//типы отношений в выражении
enum ERelation {
	rel_NULL,	//отношение отсутствует
	rel_EQ,		//равно =
	rel_NE,		//не равно #
	rel_LT,		//меньше <
	rel_LE,		//меньше или равно <=
	rel_GT,		//больше >
	rel_GE,		//больше или равно >=
	rel_IN,		//принадлежет множеству IN
	rel_IS		//проверка типа IS
};


//-----------------------------------------------------------------------------
//типы мат. операций умножения
enum EMulOp {
	mop_NULL,
	mop_M,
	mop_D,
	mop_DIV,
	mop_MOD,
	mop_AND
};


//-----------------------------------------------------------------------------
//типы мат. операций сложения
enum EAddOp {
	aop_NULL,
	aop_ADD,
	aop_SUB,
	aop_OR
};


//-----------------------------------------------------------------------------
//тип фактора
enum EFactorKind {
	fk_Negation,
	fk_Expr,
	fk_ConstVar,
	fk_StdProcFunc,
	fk_Designator
};


//-----------------------------------------------------------------------------
//тип выражения
enum EExprKind {
	ek_Normal,			//обычное выражение
	ek_ProcedureVar,	//выражение рассматривается как возвращающее процедурный тип (а не вызывающее процедуру)
	ek_Char				//константное строковое выражение размером 1 символ рассматривается
						//как возвращающее одиночный символ, по умолчанию возвращается строка
};


//-----------------------------------------------------------------------------
//ExprList
class CExprList : public CBase
{
	//в случае вызова обобщенной процедуры вида {x}.ProcName() необходимо иметь возможность
	//инициализировать PFP после создания объекта и вызова Init
	friend class CFactor;
public:
	//конструктор, PFormalPars - ук. на формальные параметры или NULL
	CExprList(const CBaseName *parent, CFormalPars* PFormalPars) : CBase(parent),
		PFP(PFormalPars) {ExprVector = new CExprVector;};
	~CExprList();
	//вычисление количества выражений в списке
	int GetExprCount() {return ExprVector->size();};
	int Init(CLexBuf *lb);
	//перемещение всех элементов в указанный список выражений
	void MoveAllExpr(CExprList *ExprList);
	void WriteCPP(CPP_files& f);
	//запись кода, если CExprList содержит список параметров при вызове параметрической процедуры
	void WriteCPP_common_proc(CPP_files& f, CFormalPars* FP);
	void WriteCPP_index(CPP_files& f, bool IsOpenArray, const char* ArrayName/*CBaseName* BN*/);
	//запись кода, если CExprList содержит список параметров при вызове процедуры
	void WriteCPP_proc(CPP_files& f, CFormalPars* FP);
//private:
	//запись кода одного фактического параметра при вызове процедуры, для открытых массивов записываются доп. параметры
	//FactExpr - выражение фактического параметра , FormalPar - формальный параметр
	void WriteCPP_proc_OneParam(CPP_files& f, CExpr* Expr, CBaseName* FormalPar);
	//контейнер выражений
	CExprVector *ExprVector;
	//ук. на формальные параметры в случае, если ExprList содержит набор фактических параметров
	//если !PFP, ExprList не является набором фактических параметров
	CFormalPars* PFP;
};//CExprList


//-----------------------------------------------------------------------------
//Designator
class CDesignator : public CBase
{
public:
	void WriteCPP_Guardless(CPP_files& f);
	//конструктор, InFactPars - признак вхождения Designator в фактические параметры вызываемой процедуры
	CDesignator(const CBaseName *parent, bool InFactPars) : CBase(parent),
		Qualident(NULL),
		req_proc_var(false),
		gen_Guard_code(true),
		in_fact_pars(InFactPars),
		is_proc_name(false),
		is_read_only(false),
		present_last_up(false),
		ResultId(id_CBaseName) {};
	~CDesignator();
	//проверка наличия полей (переменных) только для чтения в цепочке имен импортированного объекта
	bool IsReadOnly() const {return is_read_only;};
	//поиск последнего имени в цепочке эл-тов обозначения
	//это может быть переменная, процедура, тип (в случае массива)
	CBaseName* FindLastName();
	//получение вычисленного id результата
	EName_id GetResultId() const {return ResultId;};
	//инициализация обозначения
	int Init(CLexBuf *lb);
	//признак того, что Designator содержит название процедуры
	bool IsProcName() const {return is_proc_name;};
	//[уточненное.] название Designatora
	CQualident *Qualident;
	//признак необходимости интерпретации CProcedure и CProcedureVar не как вызова функции,
	//в других случаях данный признак игнорируется
	bool req_proc_var;
	//запись кода CDesignator
	void WriteCPP(CPP_files& f);
//private:
	//тип типа эл-та обозначения (зависит от предыдущего эл-та обозначения)
	enum EDesKind {
		dk_Pointer,		//эл-т следует после указателя или записи - VAR-параметра
		dk_Record,		//эл-т следует после записи
		dk_Common,		//эл-т следует после обобщенной переменной
		dk_Array,		//эл-т является описанием индексов обычного массива (следует после массива)
		dk_OpenArray,	//эл-т является описанием индексов открытого массива
		dk_Guard		//эл-т является охраной типа
	};
	//эл-т обозначения
	struct SDesElem {
		//тип эл-та обозначения, определяется _предыдущим_ эл-том обозначения
		EDesKind DesKind;
		//список выражений (для массива)
		CExprList* ExprList;
		//идент. (для записи, ук. или скаляра)
		char* ident;
		//Qualident (для охраны типа)
		CQualident* Qualident;
		//имя параметра обощенной переменной
		char* CPPCompoundName;
	};
	//создание нового эл-та обозначения и инициализация его типа
	static SDesElem* CreateSDesElem(const EDesKind DKind);
	//тип контейнера эл-тов обозначения
	typedef std::vector<SDesElem*> SDesElemStore;
	//контейнер эл-тов обозначения
	SDesElemStore DesElemStore;
	//признак необходимости генерации кода охраны типа
	bool gen_Guard_code;
	//признак нахождения в списке фактических параметров вызываемой процедуры (необходимо для генерации кода массивов)
	bool in_fact_pars;
	//признак названия процедуры (не связанной с типом)
	bool is_proc_name;
	//признак read-only - в случае импортированного объекта =true если хоть один
	//эл-т обозначения в цепочке имеет признак read-only, иначе =false
	bool is_read_only;
	//признак наличия последнего '^' (требуется разыменование)
	bool present_last_up;
	//для хранения вычисленного id результата (или id_CBaseName)
	EName_id ResultId;
};//CDesignator


//---------------------------------------------------------------
//оператор вызова процедуры
class CCallStatement : public CBase
{
public:
	//при Des == NULL Designator создается в Init, проверяются {a}.Proc(b) и Proc{a}(b),
	//при Des != NULL проверяется только Proc{a}(b), IsDesOwner - признак собственности переданного Designator'a
	CCallStatement(const CBaseName *parent, CDesignator* Des, bool IsDesOwner, bool IsProcCall) : CBase(parent),
		CommonList(NULL),
		Designator(Des),
		ExprList(NULL),
		is_des_owner(IsDesOwner),
		is_proc_call(IsProcCall) {};
	~CCallStatement() {
		delete ExprList;
		//Designator удаляется только тогда, когда является собственным объектом
		if (is_des_owner) delete Designator;
		delete CommonList;
	};
	//выдача ук. на Designator и снятие признака собственности, после вызова данного метода
	//за уничтожение Designator'a отвечает вызвавший метод объект
	CDesignator* GiveDesAway() {
		is_des_owner = false;
		return Designator;
	};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	//проверка наличия и считывание обобщенных параметров при вызове процедуры (функции)
	int InitProcCommonPars(CLexBuf *lb);
	//считывание обычных и обобщенных фактических параметров для вызовов процедур (функций)
	int InitProcPars(CLexBuf *lb, CCommonPars* CP, CFormalPars* FP);
	//список обобщенных фактических параметров процедуры
	CExprList *CommonList;
	//обозначение, содержащее имя вызываемой процедуры
	CDesignator *Designator;
	//список фактических параметров процедуры
	CExprList* ExprList;
	//признак необходимости уничтожения Designator в деструкторе
	bool is_des_owner;
	//равно true если данный CallStatement описывает вызов процедуры (не функции)
	bool is_proc_call;
};//CCallStatement


//-----------------------------------------------------------------------------
//Factor
class CFactor : public CBase
{
public:
	//конструктор, InFactPars - признак вхождения в фактические параметры вызываемой процедуры
	CFactor(const CBaseName *parent, EExprKind ReqKind, bool InFactPars) : CBase(parent),
		Call(NULL),
		ExprKind(ReqKind),
		in_fact_pars(InFactPars),
		ResultId(id_CBaseName),
		Designator(NULL),
		designator_brackets(false),
		Expr(NULL),
		Factor(NULL),
		FactorKind(fk_Expr),	//пока вид фактора не известен
		StdProcFunc(NULL),
		ConstVar(NULL) {};
	~CFactor();
	int CreateConst(CBaseVar *&BaseConst);
	CBaseName* FindLastName();
	//получение вычисленного id результата
	EName_id GetResultId() {return ResultId;};
	int Init(CLexBuf *lb);
	bool IsReadOnly() const;
	void WriteCPP(CPP_files& f);

	CCallStatement* Call;
private:
	int CalcResultId();
	//признак требуемой интерпретации выражения
	EExprKind ExprKind;
	//признак нахождения в списке фактических параметров вызываемой процедуры
	bool in_fact_pars;
	//для хранения вычисленного id результата (или id_CBaseName)
	EName_id ResultId;
public:
	CDesignator *Designator;
	bool designator_brackets;
	CExpr *Expr;
	CFactor *Factor;
	EFactorKind FactorKind;
	//для хранения стандартной процедуры-функции
	CStdProcFunc *StdProcFunc;
	//для хранения текущего значения Factora
	CBaseVar *ConstVar;
};//CFactor


//-----------------------------------------------------------------------------
//TermPair
class CTermPair : public CBase
{
public:
	int ApplyOperation(CBaseVar *&BaseConst) const;
	CTermPair(const CBaseName *parent) : CBase(parent),
		Factor(NULL),
		ResultId(id_CBaseName) {};
	~CTermPair() {delete Factor;};
	EMulOp MulOp;
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
	//получение вычисленного id результата
	EName_id GetResultId() {return ResultId;};
//private:
	//x DIV y = ENTIER(x / y)
	long DIV(const long x, const long y) const {return long(floor(double(x) / y));};
	//x MOD y = x - (x DIV y) * y
	long MOD(const long x, const long y) const {return x - long(floor(double(x) / y)) * y;};
	//множитель
	CFactor *Factor;
	//для хранения вычисленного id результата (или id_CBaseName)
	EName_id ResultId;
};//CTermPair


//-----------------------------------------------------------------------------
//Term
class CTerm : public CBase
{
	/**/
	//для доступа к Factor - в будущем может не понадобиться
	friend class CExpr;
public:
	int CreateConst(CBaseVar *&BaseConst);
	//конструктор, InFactPars - признак вхождения в фактические параметры вызываемой процедуры
	CTerm(const CBaseName *parent, EExprKind ReqKind, bool InFactPars) : CBase(parent),
		ExprKind(ReqKind),
		in_fact_pars(InFactPars),
		ResultId(id_CBaseName),
		Factor(NULL), TermPairStore(NULL) {};
	~CTerm();
	CBaseName* FindLastName();
	//получение вычисленного id результата
	EName_id GetResultId() {return ResultId;};
	int Init(CLexBuf *lb);
	bool IsReadOnly() const;
	void WriteCPP(CPP_files& f);
//private:
	//признак требуемой интерпретации выражения
	EExprKind ExprKind;
	//признак нахождения в списке фактических параметров вызываемой процедуры
	bool in_fact_pars;
	//для хранения вычисленного id результата (или id_CBaseName)
	EName_id ResultId;
	CFactor *Factor;
	CBaseVector* TermPairStore;
};//CTerm


//-----------------------------------------------------------------------------
//SimpleExprPair
class CSimpleExprPair : public CBase
{
public:
	int ApplyOperation(CBaseVar *&BaseConst) const;
	CSimpleExprPair(const CBaseName *parent) : CBase(parent),
		AddOp(aop_NULL),
		ResultId(id_CBaseName),
		Term(NULL) {};
	~CSimpleExprPair() {delete Term;};
	EAddOp AddOp;
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
	//получение вычисленного id результата
	EName_id GetResultId() {return ResultId;};
//private:
	EName_id ResultId;				//для хранения вычисленного id результата (или id_CBaseName)
	CTerm *Term;
};//CSimpleExprPair


//-----------------------------------------------------------------------------
//простое выражение SimpleExpr
class CSimpleExpr : public CBase
{
public:
	int CreateConst(CBaseVar* &BaseConst);
	//конструктор, InFactPars - признак вхождения в фактические параметры вызываемой процедуры
	CSimpleExpr(const CBaseName *parent, EExprKind ReqKind, bool InFactPars) : CBase(parent),
		ExprKind(ReqKind),
		in_fact_pars(InFactPars),
		ResultId(id_CBaseName),
		SimpleExprPairStore(NULL),
		negative(false),
		Term(NULL),
		unary(false) {};
	~CSimpleExpr();
	CBaseName* FindLastName();
	//получение вычисленного id результата
	EName_id GetResultId() {return ResultId;};
	int Init(CLexBuf *lb);
	bool IsReadOnly() const;
	void WriteCPP(CPP_files& f);
//private:
	//признак требуемой интерпретации выражения
	EExprKind ExprKind;
	//признак нахождения в списке фактических параметров вызываемой процедуры
	bool in_fact_pars;
	//для хранения вычисленного id результата (или id_CBaseName)
	EName_id ResultId;
	CBaseVector* SimpleExprPairStore;
public:
	//признак наличия унарного '-'
	bool negative;
	CTerm *Term;
	//признак наличия унарной операции ('+' или '-')
	bool unary;
};//CSimpleExpr


//-----------------------------------------------------------------------------
//выражение Expr
class CExpr : public CBase
{
public:
	CBaseName* FindLValue();
	bool IsReadOnly() const;
	//конструктор обычного выражения
	CExpr(const CBaseName *parent) : CBase(parent),
		ExprKind(ek_Normal),
		in_fact_pars(false),
		IS_qual(NULL),
		ResultId(id_CBaseName),
		Relation(rel_NULL), SimpleExpr1(NULL), SimpleExpr2(NULL) {};
	//конструктор для выражения, на которое накладываются требования,
	//InFactPars - признак вхождения в фактические параметры вызываемой процедуры
	CExpr(const CBaseName *parent, EExprKind ReqKind, bool InFactPars) : CBase(parent),
		ExprKind(ReqKind),
		in_fact_pars(InFactPars),
		IS_qual(NULL),
		ResultId(id_CBaseName),
		Relation(rel_NULL), SimpleExpr1(NULL), SimpleExpr2(NULL) {};
	~CExpr();
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
	int CreateConst(CBaseVar* &BaseConst);
	//поиск последней переменной (l-value) в цепочке выражений (NULL если не найдено)
	CBaseName *FindLastName();
	//получение вычисленного id результата выражения
	EName_id GetResultId() const {return ResultId;};
//private:
	bool ApplyOperation(const CBaseVar* const c1, const CBaseVar* const c2) const;
	//признак требуемой интерпретации выражения
	EExprKind ExprKind;
	//признак нахождения в списке фактических параметров вызываемой процедуры
	bool in_fact_pars;
	//уточн. идент., следующий после IS (имя типа или специализация)
	CQualident *IS_qual;
	//вычисление ResultId с проверкой ошибок
	int CalcResultId();
	//для хранения вычисленного id результата (или id_CBaseName)
	EName_id ResultId;
	ERelation Relation;
	CSimpleExpr *SimpleExpr1;
	CSimpleExpr *SimpleExpr2;
};//CExpr


#endif	//O2M_Expr_h
