//=============================================================================
// ���������� ������� ���������� (Var)
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


//������, �� ������� ������� ������ � ������ �����
class CCaseLabels;
class CGuard;


//-----------------------------------------------------------------------------
//���������� ���� ARRAY
class CArrayVar : public CBaseVar
{
public:
	CArrayVar(const CBaseName* parent) : CBaseVar(id_CArrayVar, parent),
		ArrayType(NULL), ConstString(NULL), ConstStringIsChar(false) {};
	~CArrayVar();
	int CheckComplete(CLexBuf *lb);
	CBaseVar* CreateConst(const CBaseName* parent) const;
	CBaseVar* CreateVar(const CBaseName* parent) const;
	//������������� ������� ������� ��������
	int SetConstValue(const char* st);
	void WriteCPP(CPP_files& f);
	void WriteCPP_ConstValue(CPP_files& f);
	void WriteCPP_fp(CPP_files &f, const bool to_h);
	//������ ���������� ����-������� � ������ (� ������� �� ��������� ���������� ����� ������ ������������ WriteCPP_fp)
	void WriteCPP_rec(CPP_files &f, const bool to_h);
	void WriteDFN(DFN_file& f);
	//��� �������� ���� ������ ����������
	CArrayType* ArrayType;
	//��� �������� ������ �������� (ARRAY OF CHAR)
	char* ConstString;
	//������� ����, ��� ������ ������������� 1 �������
	bool ConstStringIsChar;
protected:
	typedef CBaseVarVector VarList_type;
};//CArrayVar


//-----------------------------------------------------------------------------
//���������� ���� BOOLEAN
class CBooleanVar : public CBaseVar
{
	//���������� ��� ������� ��������
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
	//�������� ��������� (� ������ ���������)
	bool ConstValue;
};//CBooleanVar


//-----------------------------------------------------------------------------
//���������� ���� CHAR
class CCharVar : public CBaseVar
{
	//���������� ��� ������� ��������
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
	//�������� ��������� (� ������ ���������)
	char ConstValue;
};//CCharVar


//-----------------------------------------------------------------------------
//���������� ���� CommonVar
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
	//����� ���� ������������� (���� ����)
	CBaseType* FindType() const;
	const char* GetCPPCompoundName() const {return CPPCompoundName;};
	void GetTagName(const char* &QualName, const char* &Name, const char* &TagName);
	bool IsPureCommon();
	//��������� �������� �������� � �������� ���� ��������
	int SetTagName(const char* QualName, const char* Name);
	void WriteCPP(CPP_files& f);
	void WriteCPP_fp(CPP_files &f, const bool to_h);
	void WriteDFN(DFN_file& f);
//private:
	//�������� C++ ��������� (����� union), ���������������� ������������� ����������
	char* CPPCompoundName;
	//�������� ������, ��������������� ��� ��������
	char* QualSpecName;
	//�������� ���� ��������
	char* SpecName;
	//�������� �������� (����� �������������)
	char* Tag;
};//CCommonVar


//-----------------------------------------------------------------------------
//���������� ���� INTEGER
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
	//�������� ��������� (� ������ ���������)
	int ConstValue;
};//CIntegerVar


//-----------------------------------------------------------------------------
//���������� ���� LONGINT
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
	//�������� ��������� (� ������ ���������)
	long ConstValue;
};//CLongintVar


//-----------------------------------------------------------------------------
//���������� ���� LONGREAL
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
	//�������� ��������� (� ������ ���������)
	long double ConstValue;
};//CLongrealVar


//-----------------------------------------------------------------------------
//���������� ���� POINTER
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
	//����� ���� (�� QualidentType), �� ���. ��������� ���������
	CBaseType* FindType() const;
	//������� true, ���� ��������� ��������� �� ARRAY
	bool IsArrayPointer() const {return IsArray;};
	//������� true, ���� ��������� ��������� �� RECORD ��� SpecType
	bool IsRecordPointer() const {return IsRecord;};
	//�������������� ��������� �������� ��. �� ������ - ���������� � ��������, ��������������
	//����������� �������������� ��������� (CReceiver, CGuard)
	void SetIsRecord() {IsRecord = true; IsArray = false;};
	//��������� ����
	void WriteCPP(CPP_files& f);
	void WriteCPP_fp(CPP_files &f, const bool to_h);
	void WriteCPP_ConstValue(CPP_files& f);
	void WriteDFN(DFN_file& f);
//private:
	//������ ���� ���. ����������, ���������� ����������� � ������ ��������� �������
	void WriteCPP_array(CPP_files& f);
	//������� ����, ��� ����������� ��� - ������
	bool IsArray;
	//������� ����, ��� ����������� ��� - ������ ��� �������������
	bool IsRecord;
	//������� ���������� ���� QualidentType (�.�. type_name ����� ���� ��������������� � CPointerType::CreateVar)
	bool qualident_type;
	//������������� ���, �� ���. ��������� ���������� (CRecordType, CArrayType, ...) ��� NULL (��� ������������ ���� ��� ��. �� ����������� ���)
	CBaseType* Type;
	//������� �������� ���� ���������� � ������ ������ (��������������� � CPointerType::CreateVar)
	CLexBufPos TypePos;
};//CPointerVar


//-----------------------------------------------------------------------------
//���������� ���� PROCEDURE
class CProcedureVar : public CBaseVar
{
public:
	CProcedureVar(const CBaseName* parent) : CBaseVar(id_CProcedureVar, parent) {};
	CBaseVar* CreateVar(const CBaseName* parent) const;
	EName_id GetResultId() const;
	void WriteCPP(CPP_files& f);
	void WriteCPP_fp(CPP_files &f, const bool to_h);
	void WriteDFN(DFN_file& f);
	//������ ���������� ����������
	CFormalPars FormalPars;
};//CProcedureVar


//-----------------------------------------------------------------------------
//���������� ���� PTR
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
//���������� ���� REAL
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
	//�������� ��������� (� ������ ���������)
	double ConstValue;
};//CRealVar


//-----------------------------------------------------------------------------
//���������� ���� RECORD
class CRecordVar : public CBaseVar
{
public:
	//���������� ���������� ��-�� � ������ �����
	void AddName(CBaseName* BN) const;
	int CheckComplete(CLexBuf *lb);
	CBaseVar* CreateVar(const CBaseName* parent) const;
	CRecordVar(const CBaseName* parent) : CBaseVar(id_CRecordVar, parent), Qualident(NULL) {};
	~CRecordVar();
	//����� ����� � ������ �����
	CBaseName* FindName(const char* search_name) const;
	//��������� ���� ��� ���������� ����������
	void WriteCPP(CPP_files& f);
	void WriteCPP_fp(CPP_files &f, const bool to_h);
	//������ ���� ��� ������������� CRecordVar ��� �������� ���� ���������
	void WriteCPP_pointer(CPP_files& f);
	void WriteDFN(DFN_file& f);
	//�������� �������� ����
	CQualident* Qualident;
//private:
	//��� ���������� ��� �������� ������ ����� ������
	typedef CBaseVarVector TFieldStore;
	//��������� ��� �������� ������ ����� ������
	mutable TFieldStore FieldStore;
};//CRecordVar


//-----------------------------------------------------------------------------
//���������� ���� SET
class CSetVar : public CBaseVar
{
	//���������� ��� ������� ��������
	friend class CExpr;
	friend class CSimpleExpr;
	friend class CTermPair;
	friend class CSimpleExprPair;
public:
	CBaseVar* CreateConst(const CBaseName *parent) const;
	CBaseVar* CreateVar(const CBaseName* parent) const;
	CSetVar(const CBaseName* parent) : CBaseVar(id_CSetVar, parent), ConstValue(0) {};
	~CSetVar();
	//��������� ������������ ���������, ���� ���� (����� �� ����), ��� ������������� ������
	//������ �������������� ����������� ���� ��������� ��������
	int SetInit(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
	void WriteCPP_ConstValue(CPP_files& f);
	void WriteCPP_fp(CPP_files &f, const bool to_h);
	void WriteDFN(DFN_file& f);
private:
	//������������ ����������� ��������, ������������� ������ ��� is_const == true
	//��� ConstValue ������ ��������������� ����, �������� �������� ���������� CSetType::GetCPPTypeName,
	//��� �� ��������� �� ���� �����, ������� ��������� � SET (��������, � CSetVarElem)
	int ConstValue;
	//������ ��-��� ���������, ����� ��������� ���������, ������������ ������ ��� is_const == false
	CBaseVector SetElems;
};//CSetVar


//-----------------------------------------------------------------------------
//��-� ��������� SET
class CSetVarElem : public CBase
{
public:
	CSetVarElem(const CBaseName *parent) : CBase(parent),
		HighBoundValue(0), HighExpr(NULL), IsRange(false), LowBoundValue(0), LowExpr(NULL), SetValue(0) {};
	~CSetVarElem();
	//��������� ������������ �������� SET (������������� ������ ��� IsConst() == true)
	int GetConstValue() const {return SetValue;};
	int Init(CLexBuf *lb);
	//�������� ������������ ������� �������
	bool IsConst() const {return !(HighExpr || LowExpr);};
	void WriteCPP(CPP_files& f);
private:
	//������������ �������� ������� ������� ���������, ������������� ��� !HighExpr
	int HighBoundValue;
	//��������� ��� ������� ������� ���������, ������������� ��� IsRange
	CExpr *HighExpr;
	//������� ��������� (LowBoundValue..HighBoundValue)
	bool IsRange;
	//������������ �������� ������ ������� ���������, ������������� ��� !LowExpr
	int LowBoundValue;
	//��������� ��� ������ ������� ���������
	CExpr *LowExpr;
	//������������ ����������� �������� ���������, ������������� ������ ��� IsConst() == true
	int SetValue;
};//CSetVarElem


//-----------------------------------------------------------------------------
//���������� ���� SHORTINT
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
	//�������� ��������� (� ������ ���������)
	short ConstValue;
};//CShortintVar


#endif	//O2M_Var_h
