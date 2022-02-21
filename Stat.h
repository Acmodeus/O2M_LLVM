//===============================================================
// Объявление классов инструкций (Statements)
//===============================================================

#ifndef O2M_Stat_h
#define O2M_Stat_h

#include "Common.h"
#include "Base.h"
#include "Expr.h"


//---------------------------------------------------------------
//последовательность операторов
class CStatementSeq : public CBase
{
public:
	CStatementSeq(const CBaseName* parent) : CBase(parent)
		{};
	~CStatementSeq();
	EHaveRet HaveRet() const;
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);

	CBaseVector StatStore;
private:
	int StatementInit(CLexBuf *lb);
};//CStatementSeq


//---------------------------------------------------------------
//контейнер Expr THEN StatementSeq ("ELSIF") для оператора IF
class CElsifPair
{
public:
	CElsifPair(const CBaseName* parent) : parent_element(parent),
		StatementSeq(NULL),
		Expr(NULL) {};
	~CElsifPair();
	EHaveRet HaveRet() const {return StatementSeq->HaveRet();};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	const CBaseName* parent_element;
	//THEN StatementSeq
	CStatementSeq *StatementSeq;
	CExpr* Expr;
};//CIfStatement


//---------------------------------------------------------------
//оператор IF
class CIfStatement : public CBase
{
public:
	CIfStatement(const CBaseName* parent) : CBase(parent),
		ElseStatementSeq(NULL) {};
	~CIfStatement();
	EHaveRet HaveRet() const;
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	CStatementSeq *ElseStatementSeq;
	typedef std::vector <CElsifPair*> ElsifPairList_type;
	ElsifPairList_type ElsifPairList;
};//CIfStatement


//для передачи указателей на объекты в параметрах процедуры CCaseLabels::Init
class CCaseLabelsSeq;
class CCaseStatement;


//---------------------------------------------------------------
//CASE метки ConstExpr [".." ConstExpr]
class CCaseLabels
{
public:
	bool ValueExists(const long Value, const bool IsRng, const long HighValue);	//проверка наличия указанного значения в текущей метке
	CCaseLabels(const CBaseName* parent) : ConstValue(0),
		IsRange(false),
		ConstHighValue(0),
		parent_element(parent) {};
	int Init(CLexBuf *lb, CCaseStatement* const CaseStatement, CCaseLabelsSeq* const CaseLabelsSeq);
	void WriteCPP(CPP_files& f, CExpr* Expr);
//private:
	long ConstValue;			//значение 1-го выражения
	bool IsRange;				//признак наличия второго выражения
	long ConstHighValue;		//значение 2-го выражения, действительно при IsRange
	const CBaseName* parent_element;
};//CCaseLabels


//---------------------------------------------------------------
//последовательность меток одного варианта Case
class CCaseLabelsSeq
{
public:
	bool ValueExists(const long Value, const bool IsRng, const long HighValue);
	CCaseLabelsSeq(const CBaseName* parent) : parent_element(parent),
		StatementSeq(parent) {};
	~CCaseLabelsSeq();
	//в случае отсутствия меток можно считать, что RETURN имеется
	EHaveRet HaveRet() const {return CaseLabelsList.empty() ? hr_Yes : StatementSeq.HaveRet();};
	int Init(CLexBuf *lb, CCaseStatement* const CaseStatement);
	void WriteCPP(CPP_files& f, CExpr* Expr);
//private:
	const CBaseName* parent_element;
	CStatementSeq StatementSeq;		//последовательность операторов
	typedef std::vector <CCaseLabels*> CaseLabelsList_type;	//тип списка меток
	CaseLabelsList_type CaseLabelsList;						//список меток
};//CCaseLabelsSeq


//---------------------------------------------------------------
//оператор CASE
class CCaseStatement : public CBase
{
public:
	CCaseStatement(const CBaseName* parent) : CBase(parent),
		Expr(NULL),
		ElseStatementSeq(NULL) {};
	~CCaseStatement();
	EName_id GetExprResultId() {return Expr->GetResultId();};
	EHaveRet HaveRet() const;
	int Init(CLexBuf *lb);
	bool ValueExists(const long Value, const bool IsRng, const long HighValue);
	void WriteCPP(CPP_files& f);
//private:
	CExpr *Expr;						//выражение
	CStatementSeq* ElseStatementSeq;	//последовательность операторов в блоке ELSE
	typedef std::vector <CCaseLabelsSeq*> CaseLabelsSeqList_type;
	CaseLabelsSeqList_type CaseLabelsSeqList;
};//CCaseStatement


//---------------------------------------------------------------
//оператор цикла WHILE
class CWhileStatement : public CBase
{
public:
	CWhileStatement(const CBaseName *parent) : CBase(parent),
		Expr(NULL),
		StatementSeq(NULL)
	{};
	~CWhileStatement();
	EHaveRet HaveRet() const {return StatementSeq->HaveRet();};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	CExpr *Expr;
	CStatementSeq *StatementSeq;
};//CWhileStatement


//---------------------------------------------------------------
//оператор цикла REPEAT
class CRepeatStatement : public CBase
{
public:
	CRepeatStatement(const CBaseName *parent) : CBase(parent),
		Expr(NULL),
		StatementSeq(NULL)
	{};
	~CRepeatStatement();
	EHaveRet HaveRet() const {return StatementSeq->HaveRet();};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	CExpr *Expr;
	CStatementSeq *StatementSeq;
};//CRepeatStatement


//---------------------------------------------------------------
//оператор цикла FOR
class CForStatement : public CBase
{
public:
	CForStatement(const CBaseName *parent) : CBase(parent),
		var_name(NULL),
		StatementSeq(NULL),
		ForExpr(NULL),
		ToExpr(NULL) {};
	~CForStatement();
	EHaveRet HaveRet() const {return StatementSeq->HaveRet();};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	char* var_name;					//название переменной цикла
	CStatementSeq *StatementSeq;	//последовательность операторов
	CExpr *ForExpr;					//выражение For
	CExpr *ToExpr;					//выражение To
	long step;						//шаг цикла
};//CForStatement


//---------------------------------------------------------------
//оператор цикла LOOP
class CLoopStatement : public CBase
{
public:
	CLoopStatement(const CBaseName *parent) : CBase(parent),
		StatementSeq(NULL), WithLoopLink(parent, true) {};
	~CLoopStatement();
	EHaveRet HaveRet() const {return StatementSeq->HaveRet();};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	static int CurrentUID;
	CStatementSeq *StatementSeq;
	CWithLoopLink WithLoopLink;
};//CLoopStatement


//---------------------------------------------------------------
//объект Guard для оператора WITH
class CGuard
{
public:
	const char* GetVarModuleName();
	CGuard(const CBaseName* parent) : parent_element(parent),
		type_id(id_CBaseName) {};
	CBaseVar* CreateGuardVar();
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
private:
	const CBaseName* parent_element;
	//название признака специализации (может отсутствовать)
	CQualident spec_name;
	//имя тестируемой переменной
	CQualident VarName;
	//требуемое имя динамического типа переменной (охрана)
	CQualident TypeName;
	//id типа-охраны (запоминается в Init, чтобы не извлекать повторно)
	EName_id type_id;
};//CGuard


//---------------------------------------------------------------
//контейнер Guard DO StatementSeq ("|") для оператора WITH
class CGuardPair : public CBase
{
public:
	CGuardPair(const CBaseName* parent) : CBase(parent),
		Guard(NULL), StatementSeq(NULL), WithLoopLink(parent, false) {};
	~CGuardPair();
	EHaveRet HaveRet() const {return StatementSeq->HaveRet();};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
private:
	CGuard *Guard;
	CStatementSeq *StatementSeq;	//DO StatementSeq
	//для организации таблицы имен для StatementSeq
	CWithLoopLink WithLoopLink;
};//CGuardPair


//---------------------------------------------------------------
//оператор WITH
class CWithStatement : public CBase
{
public:
	CWithStatement(const CBaseName* parent) : CBase(parent),
		GuardPairStore(NULL), ElseStatementSeq(NULL) {};
	~CWithStatement();
	EHaveRet HaveRet() const;
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
private:
	CBaseVector* GuardPairStore;
	CStatementSeq *ElseStatementSeq;
};//CWithStatement


//---------------------------------------------------------------
//EXIT
class CExitStatement : public CBase
{
public:
	CExitStatement(const CBaseName *parent) : CBase(parent) {};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	int UID;		//содержит UID оператора LOOP, к которому относится данный EXIT
};//CExitStatement


//---------------------------------------------------------------
//RETURN
class CReturnStatement : public CBase
{
public:
	CReturnStatement(const CBaseName *parent) : CBase(parent),
		Expr(NULL)//может не быть
		{};
	~CReturnStatement();
	EHaveRet HaveRet() const {return hr_Yes;};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	CExpr *Expr;
};//CReturnStatement


//---------------------------------------------------------------
//оператор присваивания
class CAssignStatement : public CBase
{
public:
	CAssignStatement(const CBaseName *parent, CDesignator* Des) : CBase(parent),
		cv_compound_name(NULL),
		Designator(Des),
		Expr(NULL),
		str_to_array(false) {}
	~CAssignStatement() {
		delete Expr;
		delete Designator;
	}
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	void WriteCPP_array(CPP_files& f);
	//для генерации кода в случае ук. на обобщенную переменную
	const char* cv_compound_name;
	CDesignator *Designator;
	CExpr *Expr;
	//признак присвоения текстовой строки массиву
	bool str_to_array;
};//CAssignStatement


#endif	//O2M_Stat_h
