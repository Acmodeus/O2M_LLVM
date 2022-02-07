//=============================================================================
// ���������� ������� ����� (Type)
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
//���������� ��������
class CDeclSeq
{
public:
	void WriteCPP_mod_init(CPP_files& f);	//������������� ������������� �������
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
	//����� ����� � ������� ����
	CBaseName* FindName(const char* search_name) const;
	//���������� ���������� ��-�� � ������� ����
	void AddName(CBaseName* BN) const;
	//������������ ������ (� ���� dfn)
	CDfnModuleSeq* DfnModuleSeq;
    mutable CBaseNameVector BaseNameStore;
protected:
	int CheckCompleteRoot(CLexBuf *lb);
	//��������� ��������, ����������� � ������-�� ����������
    //mutable CBaseNameVector BaseNameStore;
	int ConstInit(CLexBuf *lb);
	int TypeInit(CLexBuf *lb);
	int VarInit(CLexBuf *lb);
	virtual int ProcInit(CLexBuf *lb);
public:
	const CBaseName* parent_element;
};//CDeclSeq


//-----------------------------------------------------------------------------
//���������� �������� � dfn ������
class CDfnDeclSeq : public CDeclSeq
{
public:
	CDfnDeclSeq(const CBaseName* parent) : CDeclSeq(parent) {};
protected:
	int ProcInit(CLexBuf *lb);
};//CDfnDeclSeq


//-----------------------------------------------------------------------------
//�������� (Receiver) ���� PROCEDURE
class CReceiver
{
public:
	CReceiver(const CBaseName* parent) : is_var(false),
		name(NULL),
		type_name(NULL),
		Recv(NULL) {};
	~CReceiver();
	//����� ����� � ������� ����, NULL - ���� ��� �� �������
	CBaseName* FindName(const char* search_name) const;
	//�������������
	int Init(CLexBuf *lb, const CBaseName* parent_element);
	//��������� ���� ���������
	void WriteCPP(CPP_files &f) {fprintf(f.fc, "\t%s* %s = this;\n", type_name, name);};
	//������ ���� ���������
	void WriteCPP_fp(CPP_files& f, const bool external, const bool single_param);
	//������� ������� ��. ����� VAR (true �������� ���������� � �������, false - � ��. �� ������)
	bool is_var;
	//�������� �������-��������� (������ ��� ��. �� ������)
	char* name;
	//�������� ���� �������-���������
	char* type_name;
//private:
	//����������-�������� (�.�. Receiver - ��� ������� ���� � ����� ����������)
	CBaseVar* Recv;
};//CReceiver


//-----------------------------------------------------------------------------
//����� ��� �������� ������ ���������� ���������� ������ ����
class CFPSection
{
protected:
	bool is_var;
	const CBaseName *parent_element;
	CBaseType *BaseType;
	//��� ������ ����������� ����
	typedef StringVector TmpIdents_type;
	//������ ����������� ����
	TmpIdents_type tmp_idents;
public:
	CFPSection(const CBaseName* parent) : parent_element(parent), BaseType(NULL) {};
	~CFPSection();
	int AssignFPElems(CLexBuf *lb, CBaseVarVector &FPVector, const char* ReceiverName);
};//CFPSection


//-----------------------------------------------------------------------------
//������ ���������� ���������� (FormalPars) ���� PROCEDURE
class CFormalPars
{
public:
	CBaseName* FindName(const char* search_name) const;
	//����� ����� � ������ ���������� ���������� �� ������
    CBaseName* GetNameByIndex(int index);
	CFormalPars() : Qualident(NULL), have_arrays(false) {}
	~CFormalPars();
	void Assign(CFormalPars& FP, const CBaseName* parent_element) const;
	//��� ���������-�������
	CQualident* Qualident;
	//������ ���������� ����������
	CBaseVarVector FPStore;
	int Init(CLexBuf *lb, const CBaseName* parent_element);
	//��������� ���� ���� ���������
	void WriteCPP_type(CPP_files& f, const bool to_h, const CBaseName* parent_element);
	//��������� ���� ���������� ����������
	void WriteCPP_pars(CPP_files& f, const bool to_h);
	//������ ������ ���� ���������� ���������� ����� ","
	void WriteCPP_names(CPP_files& f, const bool to_h);
	//��������� ���� ��� ������������� ��������-��������
    void WriteCPP_begin(CPP_files& f);
	//��������� ���� ��� ��������������� ��������-��������
	void WriteCPP_end(CPP_files& f, const bool ret_present);
	//������ ���������� ���������� ���������� ��������� � dfn ����
	void WriteDFN(DFN_file& f);
	//������ ���� ��������� � dfn ����
	void WriteDFN_type(DFN_file& f);
protected:
	int CheckCompleteRoot(CLexBuf *lb);
	//������� ������� �������� � �������� ����������-�������� (������� �������������)
	bool have_arrays;
};//CFormalPars


//-----------------------------------------------------------------------------
//����� ��� �������� ������ ���������� ���������� ���������� ������ ����
class CCommonFPSection : public CFPSection
{
public:
	int AssignFPElems(CLexBuf *lb, CBaseVarVector &FPVector);
	CCommonFPSection(const CBaseName* parent) : CFPSection(parent) {};
};


//-----------------------------------------------------------------------------
//����� ��� �������� ������ ���������� ������������������ ���������� ���������� ������ ����
class CSpecFPSection
{
	bool is_var;
	const CBaseName *parent_element;
	CBaseType *BaseType;
	//��� ������� ��� �������� ���������� �� 1 ����������� ���������
	struct SSFPElem{
		char* ident;
		bool IsNeedDefaultSpec;
		char* QualTagName;
		char* TagName;
		CLexBufPos pos;
	};
	//��� ������ ����������� ������������������ ����������
	typedef std::vector<SSFPElem*> TSFPElemStore;
	//������ ����������� ������������������ ����������
	TSFPElemStore SFPElemStore;
public:
	CSpecFPSection(const CBaseName* parent) : parent_element(parent), BaseType(NULL) {};
	~CSpecFPSection();
	int AssignFPElems(CLexBuf *lb, CBaseVarVector &FPVector);
};//CSpecFPSection


//-----------------------------------------------------------------------------
//������ ���������� ����������
class CCommonPars : public CFormalPars
{
public:
	//������������� ���������� ���������� ���������� ���������
	int Init(CLexBuf *lb, CBaseName* parent_element);
	//������������� ���������� ���������� ����������� �����. ����-���
	int InitSpec(CLexBuf *lb, CBaseName* parent_element);
};


//-----------------------------------------------------------------------------
//������
class CModule : public CBaseName
{
public:
	CModule(const CBaseName* parent) : CBaseName(id_CModule, parent), DeclSeq(NULL), StatementSeq(NULL) {};
	~CModule();
	//����� ���������������� ����� � ������� ����, NULL - ���� ��� �� �������
	CBaseName* FindImportedName(const char* module_name, const char* search_name) const;
	void WriteDFN(DFN_file& f);
	//�������� ����� _O2M_main.cpp � �������� O2M_SYS_main_init
	bool WriteCPP_main();
	//����������
	CDeclSeq* DeclSeq;
	//������������������ ����������
	CStatementSeq* StatementSeq;
	//��������� ������������� ������ (� ��������� ������� project)
	int Init(const CProject *project, CLexBuf *lb);
	//�������� ��� ������������ ������ �������� ������
	int Init(CLexBuf*) {throw error_Internal("CModule::Init(CLexBuf*)");};
	//��������� ���� C++
	void WriteCPP(CPP_files& f);
	//����� ����� � ������� ����
	CBaseName* FindName(const char* search_name) const;
	//���������� ���������� ��-�� � ������� ����
	void AddName(CBaseName* BN) const;
};//CModule


//-----------------------------------------------------------------------------
//������ ��������������� �� .dfn
class CDfnModule : public CBaseName
{
public:
	CBaseName* FindImportedName(const char *module_name, const char *search_name) const;
	//���������� ���������� ��-�� � ������� ����
	void AddName(CBaseName* BN) const;
	CDfnModule(const CBaseName* parent) : CBaseName(id_CDfnModule, parent),
		DfnDeclSeq(NULL), full_path(NULL), alias_name(NULL) {};
	~CDfnModule();
	void WriteCPP(CPP_files& f);
	//��������� ������������� ������ (� ��������� ������� project)
	virtual int Init(const CProject *project, CLexBuf *lb);
	//�������� ��� ������������ ������ �������� ������
	int Init(CLexBuf*) {throw error_Internal("CDfnModule::Init(CLexBuf*)");};
	//����� ����� � ������� ����
	CBaseName* FindName(const char* search_name) const;
	//dfn ����������
	CDfnDeclSeq* DfnDeclSeq;
	//���� � dfn ������, ���� �� �� ������� ���������� (����� NULL)
	char* full_path;
	//��������� ������ (AliasName := RealName) 
	char* alias_name;
};//CDfnModule


//-----------------------------------------------------------------------------
//������ SYSTEM (��� .dfn ���� ������ ���� ������)
class CDfnModuleSystem : public CDfnModule
{
public:
	CDfnModuleSystem(const CBaseName* parent);
	//��������� ������������� ������ (� ��������� ������� project)
	int Init(const CProject *project, CLexBuf *lb);
};//CDfnModuleSystem


//-----------------------------------------------------------------------------
//��������������� ������
class CImportModule : public CBaseName
{
public:
	void WriteDFN(DFN_file &f);
	char *real_name;				//��������� ��� ������
	CImportModule(const CBaseName* parent) : CBaseName(id_CImportModule, parent), real_name(NULL) {};
	~CImportModule() {delete[] real_name;};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
};//CImportModule


//-----------------------------------------------------------------------------
//���������
class CProcedure : public CBaseName
{
public:
	//���������� ���������� ��-�� � ������� ����
	void AddName(CBaseName* BN) const;
	bool CompareProcNames(const CProcedure* P) const;
	CProcedure(const CBaseName* parent) : CBaseName(id_CProcedure, parent),
		Receiver(NULL), FormalPars(NULL), DeclSeq(NULL), StatementSeq(NULL), have_return(hr_No) {}
	~CProcedure();
	CBaseName* FindImportedName(const char *module_name, const char *search_name) const;
	//����� ����� � ������� ����
	CBaseName* FindName(const char* search_name) const;
	//���������� ���������� ���������� ��������� (���� ����)
	virtual int GetCommonParsCount() {return 0;};
	//���������� ���������� ���������� ���������
	int GetFormalParsCount() {return FormalPars->FPStore.size();};
	EName_id GetResultId() const;
	int Init(CLexBuf *lb);
	static bool IsProcId(const EName_id id);
	void WriteCPP(CPP_files& f);
	void WriteCPP_RECORD(CPP_files& f, const CRecordType* RT, const bool to_h);
	void WriteDFN(DFN_file& f);
	CReceiver* Receiver;
	//������ ���������� ����������
	CFormalPars *FormalPars;
	//����������
	CDeclSeq *DeclSeq;
	//������������������ ����������
	CStatementSeq *StatementSeq;
protected:
	//������� ������� � ���� ���������(�������) ��������� RETURN
	EHaveRet have_return;
};//CProcedure


//-----------------------------------------------------------------------------
//���������� ��������������� �������������
class CHandlerProc : public CProcedure
{
	//�������������� ���������� ���������� � ����������
	int ConvertParams();
public:
	//���������� ���������� ���������� ���������
	int GetCommonParsCount() {return CommonPars->FPStore.size();};
	CHandlerProc(const CBaseName* parent);
	~CHandlerProc();
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
	//���������� �����. ����-��� �� ��������� � DFN ����
	void WriteDFN(DFN_file& f) {throw error_Internal("CHandlerProc::WriteDFN");};
	//������ ���������� ����������
	CCommonPars *CommonPars;
//private:
	//��� ������ (NULL ��� �������� ������), ��������������� ���������� ���������, � ������� �������� ������ ����������
	char* QualName;
	//����. �������� ��� �������� ����������� (� �������� ������) ����� �����������
	int UID;
};//CHandlerProc


//-----------------------------------------------------------------------------
//������������ �������� ��������� ForwardDecl
class CForwardDecl : public CProcedure
{
public:
	bool CheckSatisfied(CLexBuf *lb) const;
	CForwardDecl(const CBaseName* parent);
	int CompareDeclarations(CProcedure* P);
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
private:
	//������� � ������ ������ �� ������ ���������� ��������� ��� ������� ������������ ��������
	CLexBufPos FDeclPos;
	//������� ������� ��������� ��� ������� ������������ ��������
	bool Satisfied;
};//CForwardDecl


//-----------------------------------------------------------------------------
//���������� ���������
class CCommonProc : public CProcedure
{
protected:
	//������ ���� ������ ��������� ��� ������ ����������� ��������������� �������������
	void WriteCPP_HPar(CPP_files& f, CBaseVar* par);
	//�������� ���������� ���� ���������� � ���������� ����������, ����������� ������� � ���������� ����������
	int CheckParams();
public:
	//���������� ���������� ���������� ���������
	int GetCommonParsCount() {return CommonPars->FPStore.size();};
	CCommonProc(const CBaseName* parent);
	~CCommonProc();
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f);
	CCommonPars *CommonPars;		//������ ���������� ����������
	//������ � .dfn ����
	void WriteDFN(DFN_file& f);
private:
	//������� ������� ����������� �� ���������
	bool DefH;
};//CCommonProc


//-----------------------------------------------------------------------------
//���������, ����������� � DFN
class CDfnProcedure : public CProcedure
{
public:
	CDfnProcedure(const CBaseName* parent);
	int Init(CLexBuf *lb);
};//CDfnProcedure


//-----------------------------------------------------------------------------
//���������� ���������, ����������� � DFN
class CDfnCommonProc : public CCommonProc
{
public:
	CDfnCommonProc(const CBaseName* parent);
	int Init(CLexBuf *lb);
};//CDfnCommonProc


//-----------------------------------------------------------------------------
//��� PTR (��� ������ SYSTEM)
class CPtrType : public CBaseType
{
public:
	CPtrType(const CBaseName* parent);
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	//��� PTR �� ���������������� �� ������ ������
	int Init(CLexBuf *lb) {return 0;};
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
};//CPtrType


//-----------------------------------------------------------------------------
//��� BOOEAN (�������� ���)
class CBooleanType : public CBaseType
{
public:
	CBooleanType(const CBaseName* parent) : CBaseType(id_CBooleanType, parent) {};
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	//��������� ���� �������� ������� ���� � C++
	static const char* GetCPPTypeName() {return "bool";};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
};//CBooleanType


//-----------------------------------------------------------------------------
//��� CHAR (�������� ���)
class CCharType : public CBaseType
{
public:
	CCharType(const CBaseName* parent) : CBaseType(id_CCharType, parent) {};
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	//��������� ���� �������� ������� ���� � C++
	static const char* GetCPPTypeName() {return "char";};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
};//CCharType


//-----------------------------------------------------------------------------
//��� SHORTINT (�������� ���)
class CShortintType : public CBaseType
{
public:
	CShortintType(const CBaseName* parent) : CBaseType(id_CShortintType, parent) {};
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	//��������� ���� �������� ������� ���� � C++
	static const char* GetCPPTypeName() {return "short";};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
};//CShortintType


//-----------------------------------------------------------------------------
//��� INTEGER (�������� ���)
class CIntegerType : public CBaseType
{
public:
	CIntegerType(const CBaseName* parent) : CBaseType(id_CIntegerType, parent) {};
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	//��������� ���� �������� ������� ���� � C++
	static const char* GetCPPTypeName() {return "int";};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
};//CIntegerType


//-----------------------------------------------------------------------------
//��� LONGINT (�������� ���)
class CLongintType : public CBaseType
{
public:
	CLongintType(const CBaseName* parent) : CBaseType(id_CLongintType, parent) {};
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	//��������� ���� �������� ������� ���� � C++
	static const char* GetCPPTypeName() {return "long";};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
};//CLongintType


//-----------------------------------------------------------------------------
//��� REAL (�������� ���)
class CRealType : public CBaseType
{
public:
	CRealType(const CBaseName* parent) : CBaseType(id_CRealType, parent) {};
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	//��������� ���� �������� ������� ���� � C++
	static const char* GetCPPTypeName() {return "double";};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
};//CRealType


//-----------------------------------------------------------------------------
//��� LONGREAL (�������� ���)
class CLongrealType : public CBaseType
{
public:
	CLongrealType(const CBaseName* parent) : CBaseType(id_CLongrealType, parent) {};
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	//��������� ���� �������� ������� ���� � C++
	static const char* GetCPPTypeName() {return "long double";};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
};//CLongrealType


//-----------------------------------------------------------------------------
//��� SET (�������� ���)
class CSetType : public CBaseType
{
public:
	CSetType(const CBaseName* parent) : CBaseType(id_CSetType, parent) {};
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	//��������� ���� �������� ������� ���� � C++
	static const char* GetCPPTypeName() {return "int";};
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
};//CSetType


//-----------------------------------------------------------------------------
//��� Qualident
class CQualidentType : public CBaseType
{
public:
	CQualidentType(const CBaseName* parent) : CBaseType(id_CQualidentType, parent), Qualident(NULL) {};
	~CQualidentType() {delete Qualident;};
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	//��������� ��. �� �������������� ��� (�� QualidentType), ��� �������� �������� QualidentType (� ��������� �������� ����)
	int GetNamedType(CBaseType* &NamedType, const bool check_external) const;
	EName_id GetResultId() const;
	int Init(CLexBuf *lb);
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
	//��� ����
	CQualident* Qualident;
};//CQualidentType


//-----------------------------------------------------------------------------
//��� ARRAY (���������� ������)
class CArrayType : public CBaseType
{
public:
	CArrayType(const CBaseName* parent) : CBaseType(id_CArrayType, parent), size(0), Type(NULL) {};
	~CArrayType() {delete Type;};
	int CheckComplete(CLexBuf *lb) const;
	int CreateType(CBaseType* &BaseType) const;
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const;
	//��������� ��. �� ��� ��-��� ������� (��������� ��� � ������� �����, �� ���������� ��������)
	CBaseType* FindLastType() const;
	//��� ���������� ������ CBaseName::FindName, ������� �� ������ ����������
	CBaseName* FindName(const char*) const {return NULL;};
	//���������� ������������� ���������� ������������ �������
	long GetDimCount();
	EName_id GetResultId(int dimension) const;
	//��������� ���� ��� ��������� ����������� �������, ������ ������������ � 0
	//��� �������� ������������ ����������� ������� NULL
	CBaseType* GetType(const int dimension);
	int Init(CLexBuf *lb);
	//������ ���� ���������� CArrayType
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
	//������ �������, ���� 0 == size, ������ �������� ��������
	long size;
	//��� ��-��� �������: ARRAY OF Type
	CBaseType *Type;
};//CArrayType


//-----------------------------------------------------------------------------
//��� RECORD
class CRecordType : public CBaseType
{
public:
	//���������� ���������� ��-�� � ������� ����
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
	//����� ����� � ������ �����
	CBaseName* FindName(const char* search_name) const;
	//��������� ��. ���� ������� ����������
	const char* GetRuntimeId() {return RuntimeId;};
	int Init(CLexBuf *lb);
	//������������� ���������� ��. ���� ������� ����������
	int InitRuntimeId();
	CRecordType* IsExtension(const char* module_name, const char* type_name);
	//��������� ����
	void WriteCPP(CPP_files& f, const bool to_h);
	//������ � DFN ���������� ����
	void WriteDFN(DFN_file& f);
	//�������� ���������
	CQualident* Qualident;
//private:
	//������ ����� ������ � ������ (�� ��������� �������) �� ��������� � ������ ����� ���������
	mutable CBaseNameVector FieldStore;
	//��� �������� ���������� ������������ ��������� (��. �� ������ ���� �� ����, � ���. �� ��������)
	mutable bool in_checking_complete;
	//��������� ��. ���� ������� ����������
	char* RuntimeId;
};//CRecordType


//-----------------------------------------------------------------------------
//��� ��������������� ���������
class CCommonType : public CBaseType
{
public:
	//�������� ��-�� ���������
	struct SSpec {
		char* Tag;		//������������ ������ ��� TagType != tt_Type
		char* QualName;	//��� ������ � ������ ������� ��-�� ���������
		char* Name;		//��� ��-�� ���������
		bool IsExtended;//������������� ��������� ����� ���������� ���������
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
	//����������� ��-�� ���������
	static void DelSpec(SSpec* sp);
	int Init(CLexBuf *lb);
	int InitExtension(CLexBuf *lb, CDeclSeq* DS);
	//�������� ������ ��-�� ���������
	static SSpec* NewSpec(const char* newTag, const char* newQual, const char* newName, const bool newIsExtended);
	void WriteCPP(CPP_files& f, const bool to_h);
	void WriteDFN(DFN_file& f);
	//������� ����������� ��������� (��������� � ��������� LOCAL)
	bool IsLocal;
	//������ ��-��� ���������
	typedef std::vector<SSpec*> SpecStore_type;
	SpecStore_type SpecStore;
	//��� ���������� ���������
	enum {tt_Default, tt_Type} TagType;
private:
	//���������� ��-�� ��������� � ������
	void AddSpec(const char* newTag, const char* newQual, const char* newName, const bool newIsExtended);
	//�������� ������������ ���� ���������
	int CheckSpecType(const char* pref_ident, const char* ident, const CBaseName* parent) const;
	//������� ������ ���������
	void ClearTmpTagStore();
	//��������� � .2ml ���� ���������� �� ����� ��-�� ���������
	static void WriteCPP_SpecInfo(TFileType* f2ml, const bool IsDef, const CBaseName* parent_element, const SSpec* sp);
	//��� ������ ���������
	typedef StringVector TmpTagStore_type;
	//������������� �� ���������
	SSpec* DefaultSpec;
	//������ ���������
	TmpTagStore_type TmpTagStore;
};


//-----------------------------------------------------------------------------
//���������� ���������������� ���������
//������������ ������ ��� ��������� � .2ml ���� ���������� � ���������� ���������
class CCommonExtensionType : public CBaseType
{
	int CreateType(CBaseType* &BaseType) const {return 0;};
	int Init(CLexBuf *lb) {return 0;};
	int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const {return 0;};
public:
	//���������� ��-�� ��������� � ������
	void AddSpec(const char* newTag, const char* newQual, const char* newName);
	~CCommonExtensionType();
	CCommonExtensionType(const CBaseName* parent) : CBaseType(id_CCommonExtensionType, parent),
		TypeModuleName(NULL),
		TypeName(NULL)
	{
		name = str_new_copy("");
	};
	void WriteCPP(CPP_files& f, const bool to_h);
	//������ ��-��� ���������
	CCommonType::SpecStore_type SpecStore;
	char* TypeModuleName;
	char* TypeName;
};


//-----------------------------------------------------------------------------
//���������� ���, ������������������ ���������� ���������
class CSpecType : public CBaseType
{
public:
	const char* GetQualSpecName() const;
	const char* GetSpecName() const {return TagName;};
	//������ ��������� �� ������ �������������� ��� SpecType
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
	//��� ����������� ����
	CQualident* Qualident;
private:
	//��������� �������� ��������
	char* QualTagName;
	//�������� ��������
	char* TagName;
};


//-----------------------------------------------------------------------------
//��� POINTER
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
	//������� ������������� ��� �� ������������ �����
	bool forward;
	//�� NULL � ������ ��. �� ����������� ���
	CQualident* Qualident;
	//�� NULL � ������ ��. ��  ������������� ��� RECORD
	CBaseType *Type;
	//������� �������� ������������ ���� � ������ ������ (��� ���������� �������� ���� � ��������� �����)
	CLexBufPos TypePos;
};//CPointerType


//-----------------------------------------------------------------------------
//��� PROCEDURE
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
	//������ ���������� ����������
	CFormalPars FormalPars;
};//CProcedureType


//-----------------------------------------------------------------------------
//����������������� ����������� dfn �������
class CDfnModuleSeq
{
public:
	CDfnModuleSeq(const CBaseName* parent);
	~CDfnModuleSeq();
	void EnumDfnModules(CProject &project);
	//���������� ��-�� � ������ �������
	void AddModule(CDfnModule* DM);
	//����� ����� � ������� ����
	CDfnModule* FindName(const char* search_name) const;
	const CBaseName* parent_element;
	void WriteCPP(CPP_files& f);
	typedef std::vector <CDfnModule*> DfnModuleList_type;
	DfnModuleList_type DfnModuleList;
protected:
};//CDfnModuleSeq


#endif	//O2M_Type_h
