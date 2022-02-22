//===============================================================
// Объявление объектов - стандартных процедур
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
	CExpr Expr;			//целое выражение
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
	CExpr ExprX;			//целое выражение
	CExpr ExprN;			//целое выражение
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
	CExpr Expr;			//символьное выражение
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
	CExpr Expr;			//целое выражение
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
	CExpr Expr;			//вещественное выражение
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
	char * array_name;	//название измеряемого массива
	long dimension;		//проверяемая размерность (измерение) массива
	long array_size;	//найденная длина массива в заданном измерении
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
	//признак использования для типа SET, применим только при id_CIntegerVar == ResultId
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
	//признак использования для типа SET, применим только при id_CIntegerVar == ResultId
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
	CExpr Expr;			//целый тип
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
	CExpr Expr;			//символьное выражение
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
	CQualident Qualident;	//любой тип
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
	//для хранения признака в случае ук. на обобщение
	CQualident qual;
	//для получения названия переменной-указателя
	CDesignator Des;
	//тип списка выражений для хранения размерностей
	typedef CExprVector TExprStore;
	//список выражений (размерностей)
	TExprStore ExprStore;
};//CNewStdProc


//---------------------------------------------------------------
//COPY
class CCopyStdProc : public CBase
{
public:
	CExpr ExprSource;
	CExpr ExprDest;
//public:
	CCopyStdProc(const CBaseName* parent) : CBase(parent), ExprSource(parent), ExprDest(parent) {};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
};//CCopyStdProc


//---------------------------------------------------------------
//DEC
class CDecStdProc : public CBase
{
public:
	CExpr Expr1;		//целый тип, l-value (первый параметр)
	CExpr *Expr2;		//целый тип, r-value (второй параметр)
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
	CExpr Expr1;		//целый тип, l-value (первый параметр)
	CExpr Expr2;		//целый тип, r-value (второй параметр)
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
	CExpr Expr;		//выражение, для которого утверждается истинность (иначе прерывание)
	long AssertVal;	//возвращаемое в систему значение
};//CAssertStdProc


//---------------------------------------------------------------
//HALT
class CHaltStdProc : public CBase
{
public:
	CHaltStdProc(const CBaseName *parent) : CBase(parent), HaltVal(0) {};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
//private:
	long HaltVal;	//возвращаемое в систему значение
};//CHaltStdProc


//---------------------------------------------------------------
//INC
class CIncStdProc : public CBase
{
public:
	CExpr Expr1;		//целый тип, l-value (первый параметр)
	CExpr *Expr2;		//целый тип, r-value (второй параметр)
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
	CExpr Expr1;		//целый тип, l-value (первый параметр)
	CExpr Expr2;		//целый тип, r-value (второй параметр)
	CInclStdProc(const CBaseName *parent) : CBase(parent), Expr1(parent), Expr2(parent) {};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
};//CInclStdProc


//-----------------------------------------------------------------------------
//запись кода размерности массива в качестве параметра при вызове COPY
//предполагается, что BN является массивом символов или ук. на массив символов
void WriteCPP_COPY_Par(CPP_files &f, CBaseName* BN);


#endif	//O2M_StdProc_h
