//===============================================================
// ���������� �������� - ����������� ��������
//===============================================================

#ifndef O2M_StdProc_h
#define O2M_StdProc_h

#include "Common.h"
#include "Base.h"
#include "Expr.h"
#include "Type.h"


//---------------------------------------------------------------
//ABS
class CAbsStdProcFunc : public CStdProcFunc
{
public:
	int CreateConst(CBaseVar *&BaseConst);
	CAbsStdProcFunc(const CBaseName *parent) : CStdProcFunc(parent), Expr(parent) {};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	CExpr Expr;			//����� ���������
};


//---------------------------------------------------------------
//ASH
class CAshStdProcFunc : public CStdProcFunc
{
public:
	int CreateConst(CBaseVar *&BaseConst);
	CAshStdProcFunc(const CBaseName *parent) : CStdProcFunc(parent), ExprX(parent), ExprN(parent) {};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	CExpr ExprX;			//����� ���������
	CExpr ExprN;			//����� ���������
};


//---------------------------------------------------------------
//CAP
class CCapStdProcFunc : public CStdProcFunc
{
public:
	int CreateConst(CBaseVar *&BaseConst);
	CCapStdProcFunc(const CBaseName *parent) : CStdProcFunc(parent), Expr(parent, ek_Char, false) {};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	CExpr Expr;			//���������� ���������
};


//---------------------------------------------------------------
//CHR
class CChrStdProcFunc : public CStdProcFunc
{
public:
	int CreateConst(CBaseVar *&BaseConst);
	CChrStdProcFunc(const CBaseName *parent) : CStdProcFunc(parent), Expr(parent) {};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	CExpr Expr;			//����� ���������
};



//---------------------------------------------------------------
//ENTIER
class CEntierStdProcFunc : public CStdProcFunc
{
public:
	int CreateConst(CBaseVar *&BaseConst);
	CEntierStdProcFunc(const CBaseName *parent) : CStdProcFunc(parent), Expr(parent) {};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	CExpr Expr;			//������������ ���������
};


//---------------------------------------------------------------
//LEN
class CLenStdProcFunc : public CStdProcFunc
{
public:
	int CreateConst(CBaseVar *&BaseConst);
	CLenStdProcFunc(const CBaseName *parent);
	~CLenStdProcFunc();
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	char * array_name;	//�������� ����������� �������
	long dimension;		//����������� ����������� (���������) �������
	long array_size;	//��������� ����� ������� � �������� ���������
};//CLenStdProc


//---------------------------------------------------------------
//LONG
class CLongStdProcFunc : public CStdProcFunc
{
public:
	int CreateConst(CBaseVar *&BaseConst);
	CLongStdProcFunc(const CBaseName *parent) : CStdProcFunc(parent), Expr(parent) {};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	CExpr Expr;			//SHORTINT, INTEGER, REAL
};


//---------------------------------------------------------------
//MAX
class CMaxStdProcFunc : public CStdProcFunc
{
public:
	CMaxStdProcFunc(const CBaseName *parent) : CStdProcFunc(parent),
		AppliedToSET(false) {};
	int CreateConst(CBaseVar *&BaseConst);
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	//������� ������������� ��� ���� SET, �������� ������ ��� id_CIntegerVar == ResultId
	bool AppliedToSET;
};


//---------------------------------------------------------------
//MIN
class CMinStdProcFunc : public CStdProcFunc
{
public:
	CMinStdProcFunc(const CBaseName *parent) : CStdProcFunc(parent),
		AppliedToSET(false) {};
	int CreateConst(CBaseVar *&BaseConst);
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	//������� ������������� ��� ���� SET, �������� ������ ��� id_CIntegerVar == ResultId
	bool AppliedToSET;
};


//---------------------------------------------------------------
//ODD
class COddStdProcFunc : public CStdProcFunc
{
public:
	int CreateConst(CBaseVar *&BaseConst);
	COddStdProcFunc(const CBaseName *parent) : CStdProcFunc(parent), Expr(parent) {};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	CExpr Expr;			//����� ���
};


//---------------------------------------------------------------
//ORD
class COrdStdProcFunc : public CStdProcFunc
{
public:
	int CreateConst(CBaseVar* &BaseConst);
	COrdStdProcFunc(const CBaseName *parent) : CStdProcFunc(parent), Expr(parent, ek_Char, false) {};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	CExpr Expr;			//���������� ���������
};


//---------------------------------------------------------------
//SHORT
class CShortStdProcFunc : public CStdProcFunc
{
public:
	int CreateConst(CBaseVar *&BaseConst);
	CShortStdProcFunc(const CBaseName *parent) : CStdProcFunc(parent), Expr(parent) {};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	CExpr Expr;			//LONGINT, INTEGER, LONGREAL
};


//---------------------------------------------------------------
//SIZE
class CSizeStdProcFunc : public CStdProcFunc
{
public:
	int CreateConst(CBaseVar *&BaseConst);
	CSizeStdProcFunc(const CBaseName *parent) : CStdProcFunc(parent) {};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	CQualident Qualident;	//����� ���
};


//---------------------------------------------------------------
//NEW
class CNewStdProc : public CBase
{
public:
	CNewStdProc(const CBaseName *parent) : CBase(parent),
		Des(parent, false)
		{};
	~CNewStdProc();
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	//��� �������� �������� � ������ ��. �� ���������
	CQualident qual;
	//��� ��������� �������� ����������-���������
	CDesignator Des;
	//��� ������ ��������� ��� �������� ������������
	typedef CExprVector TExprStore;
	//������ ��������� (������������)
	TExprStore ExprStore;
};//CNewStdProc


//---------------------------------------------------------------
//COPY
class CCopyStdProc : public CBase
{
	CExpr ExprSource;
	CExpr ExprDest;
public:
	CCopyStdProc(const CBaseName* parent) : CBase(parent), ExprSource(parent), ExprDest(parent) {};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
};//CCopyStdProc


//---------------------------------------------------------------
//DEC
class CDecStdProc : public CBase
{
public:
	CExpr Expr1;		//����� ���, l-value (������ ��������)
	CExpr *Expr2;		//����� ���, r-value (������ ��������)
	CDecStdProc(const CBaseName* parent) : CBase(parent), Expr1(parent), Expr2(NULL) {};
	~CDecStdProc() {delete Expr2;};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
};//CDecStdProc


//---------------------------------------------------------------
//EXCL
class CExclStdProc : public CBase
{
public:
	CExpr Expr1;		//����� ���, l-value (������ ��������)
	CExpr Expr2;		//����� ���, r-value (������ ��������)
	CExclStdProc(const CBaseName *parent) : CBase(parent), Expr1(parent), Expr2(parent) {};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
};//CExclStdProc


//---------------------------------------------------------------
//ASSERT
class CAssertStdProc : public CBase
{
public:
	CAssertStdProc(const CBaseName* parent) : CBase(parent), Expr(parent), AssertVal(0) {};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	CExpr Expr;		//���������, ��� �������� ������������ ���������� (����� ����������)
	long AssertVal;	//������������ � ������� ��������
};//CAssertStdProc


//---------------------------------------------------------------
//HALT
class CHaltStdProc : public CBase
{
public:
	CHaltStdProc(const CBaseName *parent) : CBase(parent), HaltVal(0) {};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
private:
	long HaltVal;	//������������ � ������� ��������
};//CHaltStdProc


//---------------------------------------------------------------
//INC
class CIncStdProc : public CBase
{
public:
	CExpr Expr1;		//����� ���, l-value (������ ��������)
	CExpr *Expr2;		//����� ���, r-value (������ ��������)
	CIncStdProc(const CBaseName *parent) : CBase(parent), Expr1(parent), Expr2(NULL) {};;
	~CIncStdProc() {delete Expr2;};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
};//CIncStdProc


//---------------------------------------------------------------
//INCL
class CInclStdProc : public CBase
{
public:
	CExpr Expr1;		//����� ���, l-value (������ ��������)
	CExpr Expr2;		//����� ���, r-value (������ ��������)
	CInclStdProc(const CBaseName *parent) : CBase(parent), Expr1(parent), Expr2(parent) {};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
};//CInclStdProc


//-----------------------------------------------------------------------------
//������ ���� ����������� ������� � �������� ��������� ��� ������ COPY
//��������������, ��� BN �������� �������� �������� ��� ��. �� ������ ��������
void WriteCPP_COPY_Par(CPP_files &f, CBaseName* BN);


#endif	//O2M_StdProc_h
