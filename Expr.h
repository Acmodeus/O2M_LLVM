//=============================================================================
// ���������� ���������� ��� ���������� ���������
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
//���� ��������� � ���������
enum ERelation {
	rel_NULL,	//��������� �����������
	rel_EQ,		//����� =
	rel_NE,		//�� ����� #
	rel_LT,		//������ <
	rel_LE,		//������ ��� ����� <=
	rel_GT,		//������ >
	rel_GE,		//������ ��� ����� >=
	rel_IN,		//����������� ��������� IN
	rel_IS		//�������� ���� IS
};


//-----------------------------------------------------------------------------
//���� ���. �������� ���������
enum EMulOp {
	mop_NULL,
	mop_M,
	mop_D,
	mop_DIV,
	mop_MOD,
	mop_AND
};


//-----------------------------------------------------------------------------
//���� ���. �������� ��������
enum EAddOp {
	aop_NULL,
	aop_ADD,
	aop_SUB,
	aop_OR
};


//-----------------------------------------------------------------------------
//��� �������
enum EFactorKind {
	fk_Negation,
	fk_Expr,
	fk_ConstVar,
	fk_StdProcFunc,
	fk_Designator
};


//-----------------------------------------------------------------------------
//��� ���������
enum EExprKind {
	ek_Normal,			//������� ���������
	ek_ProcedureVar,	//��������� ��������������� ��� ������������ ����������� ��� (� �� ���������� ���������)
	ek_Char				//����������� ��������� ��������� �������� 1 ������ ���������������
						//��� ������������ ��������� ������, �� ��������� ������������ ������
};


//-----------------------------------------------------------------------------
//ExprList
class CExprList : public CBase
{
	//� ������ ������ ���������� ��������� ���� {x}.ProcName() ���������� ����� �����������
	//���������������� PFP ����� �������� ������� � ������ Init
	friend class CFactor;
public:
	//�����������, PFormalPars - ��. �� ���������� ��������� ��� NULL
	CExprList(const CBaseName *parent, CFormalPars* PFormalPars) : CBase(parent),
		PFP(PFormalPars) {ExprVector = new CExprVector;};
	~CExprList();
	//���������� ���������� ��������� � ������
	int GetExprCount() {return ExprVector->size();};
	int Init(CLexBuf *lb);
	//����������� ���� ��������� � ��������� ������ ���������
	void MoveAllExpr(CExprList *ExprList);
	void WriteCPP(CPP_files& f);
	//������ ����, ���� CExprList �������� ������ ���������� ��� ������ ��������������� ���������
	void WriteCPP_common_proc(CPP_files& f, CFormalPars* FP);
	void WriteCPP_index(CPP_files& f, bool IsOpenArray, const char* ArrayName/*CBaseName* BN*/);
	//������ ����, ���� CExprList �������� ������ ���������� ��� ������ ���������
	void WriteCPP_proc(CPP_files& f, CFormalPars* FP);
//private:
	//������ ���� ������ ������������ ��������� ��� ������ ���������, ��� �������� �������� ������������ ���. ���������
	//FactExpr - ��������� ������������ ��������� , FormalPar - ���������� ��������
	void WriteCPP_proc_OneParam(CPP_files& f, CExpr* Expr, CBaseName* FormalPar);
	//��������� ���������
	CExprVector *ExprVector;
	//��. �� ���������� ��������� � ������, ���� ExprList �������� ����� ����������� ����������
	//���� !PFP, ExprList �� �������� ������� ����������� ����������
	CFormalPars* PFP;
};//CExprList


//-----------------------------------------------------------------------------
//Designator
class CDesignator : public CBase
{
public:
	void WriteCPP_Guardless(CPP_files& f);
	//�����������, InFactPars - ������� ��������� Designator � ����������� ��������� ���������� ���������
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
	//�������� ������� ����� (����������) ������ ��� ������ � ������� ���� ���������������� �������
	bool IsReadOnly() const {return is_read_only;};
	//����� ���������� ����� � ������� ��-��� �����������
	//��� ����� ���� ����������, ���������, ��� (� ������ �������)
	CBaseName* FindLastName();
	//��������� ������������ id ����������
	EName_id GetResultId() const {return ResultId;};
	//������������� �����������
	int Init(CLexBuf *lb);
	//������� ����, ��� Designator �������� �������� ���������
	bool IsProcName() const {return is_proc_name;};
	//[����������.] �������� Designatora
	CQualident *Qualident;
	//������� ������������� ������������� CProcedure � CProcedureVar �� ��� ������ �������,
	//� ������ ������� ������ ������� ������������
	bool req_proc_var;
	//������ ���� CDesignator
	void WriteCPP(CPP_files& f);
//private:
	//��� ���� ��-�� ����������� (������� �� ����������� ��-�� �����������)
	enum EDesKind {
		dk_Pointer,		//��-� ������� ����� ��������� ��� ������ - VAR-���������
		dk_Record,		//��-� ������� ����� ������
		dk_Common,		//��-� ������� ����� ���������� ����������
		dk_Array,		//��-� �������� ��������� �������� �������� ������� (������� ����� �������)
		dk_OpenArray,	//��-� �������� ��������� �������� ��������� �������
		dk_Guard		//��-� �������� ������� ����
	};
	//��-� �����������
	struct SDesElem {
		//��� ��-�� �����������, ������������ _����������_ ��-��� �����������
		EDesKind DesKind;
		//������ ��������� (��� �������)
		CExprList* ExprList;
		//�����. (��� ������, ��. ��� �������)
		char* ident;
		//Qualident (��� ������ ����)
		CQualident* Qualident;
		//��� ��������� ��������� ����������
		char* CPPCompoundName;
	};
	//�������� ������ ��-�� ����������� � ������������� ��� ����
	static SDesElem* CreateSDesElem(const EDesKind DKind);
	//��� ���������� ��-��� �����������
	typedef std::vector<SDesElem*> SDesElemStore;
	//��������� ��-��� �����������
	SDesElemStore DesElemStore;
	//������� ������������� ��������� ���� ������ ����
	bool gen_Guard_code;
	//������� ���������� � ������ ����������� ���������� ���������� ��������� (���������� ��� ��������� ���� ��������)
	bool in_fact_pars;
	//������� �������� ��������� (�� ��������� � �����)
	bool is_proc_name;
	//������� read-only - � ������ ���������������� ������� =true ���� ���� ����
	//��-� ����������� � ������� ����� ������� read-only, ����� =false
	bool is_read_only;
	//������� ������� ���������� '^' (��������� �������������)
	bool present_last_up;
	//��� �������� ������������ id ���������� (��� id_CBaseName)
	EName_id ResultId;
};//CDesignator


//---------------------------------------------------------------
//�������� ������ ���������
class CCallStatement : public CBase
{
public:
	//��� Des == NULL Designator ��������� � Init, ����������� {a}.Proc(b) � Proc{a}(b),
	//��� Des != NULL ����������� ������ Proc{a}(b), IsDesOwner - ������� ������������� ����������� Designator'a
	CCallStatement(const CBaseName *parent, CDesignator* Des, bool IsDesOwner, bool IsProcCall) : CBase(parent),
		CommonList(NULL),
		Designator(Des),
		ExprList(NULL),
		is_des_owner(IsDesOwner),
		is_proc_call(IsProcCall) {};
	~CCallStatement() {
		delete ExprList;
		//Designator ��������� ������ �����, ����� �������� ����������� ��������
		if (is_des_owner) delete Designator;
		delete CommonList;
	};
	//������ ��. �� Designator � ������ �������� �������������, ����� ������ ������� ������
	//�� ����������� Designator'a �������� ��������� ����� ������
	CDesignator* GiveDesAway() {
		is_des_owner = false;
		return Designator;
	};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	//�������� ������� � ���������� ���������� ���������� ��� ������ ��������� (�������)
	int InitProcCommonPars(CLexBuf *lb);
	//���������� ������� � ���������� ����������� ���������� ��� ������� �������� (�������)
	int InitProcPars(CLexBuf *lb, CCommonPars* CP, CFormalPars* FP);
	//������ ���������� ����������� ���������� ���������
	CExprList *CommonList;
	//�����������, ���������� ��� ���������� ���������
	CDesignator *Designator;
	//������ ����������� ���������� ���������
	CExprList* ExprList;
	//������� ������������� ����������� Designator � �����������
	bool is_des_owner;
	//����� true ���� ������ CallStatement ��������� ����� ��������� (�� �������)
	bool is_proc_call;
};//CCallStatement


//-----------------------------------------------------------------------------
//Factor
class CFactor : public CBase
{
public:
	//�����������, InFactPars - ������� ��������� � ����������� ��������� ���������� ���������
	CFactor(const CBaseName *parent, EExprKind ReqKind, bool InFactPars) : CBase(parent),
		Call(NULL),
		ExprKind(ReqKind),
		in_fact_pars(InFactPars),
		ResultId(id_CBaseName),
		Designator(NULL),
		designator_brackets(false),
		Expr(NULL),
		Factor(NULL),
		FactorKind(fk_Expr),	//���� ��� ������� �� ��������
		StdProcFunc(NULL),
		ConstVar(NULL) {};
	~CFactor();
	int CreateConst(CBaseVar *&BaseConst);
	CBaseName* FindLastName();
	//��������� ������������ id ����������
	EName_id GetResultId() {return ResultId;};
	int Init(CLexBuf *lb);
	bool IsReadOnly() const;
	void WriteCPP(CPP_files& f);

	CCallStatement* Call;
private:
	int CalcResultId();
	//������� ��������� ������������� ���������
	EExprKind ExprKind;
	//������� ���������� � ������ ����������� ���������� ���������� ���������
	bool in_fact_pars;
	//��� �������� ������������ id ���������� (��� id_CBaseName)
	EName_id ResultId;
public:
	CDesignator *Designator;
	bool designator_brackets;
	CExpr *Expr;
	CFactor *Factor;
	EFactorKind FactorKind;
	//��� �������� ����������� ���������-�������
	CStdProcFunc *StdProcFunc;
	//��� �������� �������� �������� Factora
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
	//��������� ������������ id ����������
	EName_id GetResultId() {return ResultId;};
//private:
	//x DIV y = ENTIER(x / y)
	long DIV(const long x, const long y) const {return long(floor(double(x) / y));};
	//x MOD y = x - (x DIV y) * y
	long MOD(const long x, const long y) const {return x - long(floor(double(x) / y)) * y;};
	//���������
	CFactor *Factor;
	//��� �������� ������������ id ���������� (��� id_CBaseName)
	EName_id ResultId;
};//CTermPair


//-----------------------------------------------------------------------------
//Term
class CTerm : public CBase
{
	/**/
	//��� ������� � Factor - � ������� ����� �� ������������
	friend class CExpr;
public:
	int CreateConst(CBaseVar *&BaseConst);
	//�����������, InFactPars - ������� ��������� � ����������� ��������� ���������� ���������
	CTerm(const CBaseName *parent, EExprKind ReqKind, bool InFactPars) : CBase(parent),
		ExprKind(ReqKind),
		in_fact_pars(InFactPars),
		ResultId(id_CBaseName),
		Factor(NULL), TermPairStore(NULL) {};
	~CTerm();
	CBaseName* FindLastName();
	//��������� ������������ id ����������
	EName_id GetResultId() {return ResultId;};
	int Init(CLexBuf *lb);
	bool IsReadOnly() const;
	void WriteCPP(CPP_files& f);
//private:
	//������� ��������� ������������� ���������
	EExprKind ExprKind;
	//������� ���������� � ������ ����������� ���������� ���������� ���������
	bool in_fact_pars;
	//��� �������� ������������ id ���������� (��� id_CBaseName)
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
	//��������� ������������ id ����������
	EName_id GetResultId() {return ResultId;};
//private:
	EName_id ResultId;				//��� �������� ������������ id ���������� (��� id_CBaseName)
	CTerm *Term;
};//CSimpleExprPair


//-----------------------------------------------------------------------------
//������� ��������� SimpleExpr
class CSimpleExpr : public CBase
{
public:
	int CreateConst(CBaseVar* &BaseConst);
	//�����������, InFactPars - ������� ��������� � ����������� ��������� ���������� ���������
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
	//��������� ������������ id ����������
	EName_id GetResultId() {return ResultId;};
	int Init(CLexBuf *lb);
	bool IsReadOnly() const;
	void WriteCPP(CPP_files& f);
//private:
	//������� ��������� ������������� ���������
	EExprKind ExprKind;
	//������� ���������� � ������ ����������� ���������� ���������� ���������
	bool in_fact_pars;
	//��� �������� ������������ id ���������� (��� id_CBaseName)
	EName_id ResultId;
	CBaseVector* SimpleExprPairStore;
public:
	//������� ������� �������� '-'
	bool negative;
	CTerm *Term;
	//������� ������� ������� �������� ('+' ��� '-')
	bool unary;
};//CSimpleExpr


//-----------------------------------------------------------------------------
//��������� Expr
class CExpr : public CBase
{
public:
	CBaseName* FindLValue();
	bool IsReadOnly() const;
	//����������� �������� ���������
	CExpr(const CBaseName *parent) : CBase(parent),
		ExprKind(ek_Normal),
		in_fact_pars(false),
		IS_qual(NULL),
		ResultId(id_CBaseName),
		Relation(rel_NULL), SimpleExpr1(NULL), SimpleExpr2(NULL) {};
	//����������� ��� ���������, �� ������� ������������� ����������,
	//InFactPars - ������� ��������� � ����������� ��������� ���������� ���������
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
	//����� ��������� ���������� (l-value) � ������� ��������� (NULL ���� �� �������)
	CBaseName *FindLastName();
	//��������� ������������ id ���������� ���������
	EName_id GetResultId() const {return ResultId;};
//private:
	bool ApplyOperation(const CBaseVar* const c1, const CBaseVar* const c2) const;
	//������� ��������� ������������� ���������
	EExprKind ExprKind;
	//������� ���������� � ������ ����������� ���������� ���������� ���������
	bool in_fact_pars;
	//�����. �����., ��������� ����� IS (��� ���� ��� �������������)
	CQualident *IS_qual;
	//���������� ResultId � ��������� ������
	int CalcResultId();
	//��� �������� ������������ id ���������� (��� id_CBaseName)
	EName_id ResultId;
	ERelation Relation;
	CSimpleExpr *SimpleExpr1;
	CSimpleExpr *SimpleExpr2;
};//CExpr


#endif	//O2M_Expr_h
