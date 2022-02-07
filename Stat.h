//===============================================================
// ���������� ������� ���������� (Statements)
//===============================================================

#ifndef O2M_Stat_h
#define O2M_Stat_h

#include "Common.h"
#include "Base.h"
#include "Expr.h"


//---------------------------------------------------------------
//������������������ ����������
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
//��������� Expr THEN StatementSeq ("ELSIF") ��� ��������� IF
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
//�������� IF
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


//��� �������� ���������� �� ������� � ���������� ��������� CCaseLabels::Init
class CCaseLabelsSeq;
class CCaseStatement;


//---------------------------------------------------------------
//CASE ����� ConstExpr [".." ConstExpr]
class CCaseLabels
{
public:
	bool ValueExists(const long Value, const bool IsRng, const long HighValue);	//�������� ������� ���������� �������� � ������� �����
	CCaseLabels(const CBaseName* parent) : ConstValue(0),
		IsRange(false),
		ConstHighValue(0),
		parent_element(parent) {};
	int Init(CLexBuf *lb, CCaseStatement* const CaseStatement, CCaseLabelsSeq* const CaseLabelsSeq);
	void WriteCPP(CPP_files& f, CExpr* Expr);
//private:
	long ConstValue;			//�������� 1-�� ���������
	bool IsRange;				//������� ������� ������� ���������
	long ConstHighValue;		//�������� 2-�� ���������, ������������� ��� IsRange
	const CBaseName* parent_element;
};//CCaseLabels


//---------------------------------------------------------------
//������������������ ����� ������ �������� Case
class CCaseLabelsSeq
{
public:
	bool ValueExists(const long Value, const bool IsRng, const long HighValue);
	CCaseLabelsSeq(const CBaseName* parent) : parent_element(parent),
		StatementSeq(parent) {};
	~CCaseLabelsSeq();
	//� ������ ���������� ����� ����� �������, ��� RETURN �������
	EHaveRet HaveRet() const {return CaseLabelsList.empty() ? hr_Yes : StatementSeq.HaveRet();};
	int Init(CLexBuf *lb, CCaseStatement* const CaseStatement);
	void WriteCPP(CPP_files& f, CExpr* Expr);
//private:
	const CBaseName* parent_element;
	CStatementSeq StatementSeq;		//������������������ ����������
	typedef std::vector <CCaseLabels*> CaseLabelsList_type;	//��� ������ �����
	CaseLabelsList_type CaseLabelsList;						//������ �����
};//CCaseLabelsSeq


//---------------------------------------------------------------
//�������� CASE
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
	CExpr *Expr;						//���������
	CStatementSeq* ElseStatementSeq;	//������������������ ���������� � ����� ELSE
	typedef std::vector <CCaseLabelsSeq*> CaseLabelsSeqList_type;
	CaseLabelsSeqList_type CaseLabelsSeqList;
};//CCaseStatement


//---------------------------------------------------------------
//�������� ����� WHILE
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
//�������� ����� REPEAT
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
//�������� ����� FOR
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
	char* var_name;					//�������� ���������� �����
	CStatementSeq *StatementSeq;	//������������������ ����������
	CExpr *ForExpr;					//��������� For
	CExpr *ToExpr;					//��������� To
	long step;						//��� �����
};//CForStatement


//---------------------------------------------------------------
//�������� ����� LOOP
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
//������ Guard ��� ��������� WITH
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
	//�������� �������� ������������� (����� �������������)
	CQualident spec_name;
	//��� ����������� ����������
	CQualident VarName;
	//��������� ��� ������������� ���� ���������� (������)
	CQualident TypeName;
	//id ����-������ (������������ � Init, ����� �� ��������� ��������)
	EName_id type_id;
};//CGuard


//---------------------------------------------------------------
//��������� Guard DO StatementSeq ("|") ��� ��������� WITH
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
	//��� ����������� ������� ���� ��� StatementSeq
	CWithLoopLink WithLoopLink;
};//CGuardPair


//---------------------------------------------------------------
//�������� WITH
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
	int UID;		//�������� UID ��������� LOOP, � �������� ��������� ������ EXIT
};//CExitStatement


//---------------------------------------------------------------
//RETURN
class CReturnStatement : public CBase
{
public:
	CReturnStatement(const CBaseName *parent) : CBase(parent),
		Expr(NULL)//����� �� ����
		{};
	~CReturnStatement();
	EHaveRet HaveRet() const {return hr_Yes;};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	CExpr *Expr;
};//CReturnStatement


//---------------------------------------------------------------
//�������� ������������
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
	//��� ��������� ���� � ������ ��. �� ���������� ����������
	const char* cv_compound_name;
	CDesignator *Designator;
	CExpr *Expr;
	//������� ���������� ��������� ������ �������
	bool str_to_array;
};//CAssignStatement


#endif	//O2M_Stat_h
