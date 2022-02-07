//=============================================================================
// ���������� ������� � ��������������� �������
//=============================================================================

#ifndef O2M_Base_h
#define O2M_Base_h

#include "Common.h"


//-----------------------------------------------------------------------------
//��� ��������� ��������� ����� ������� RETURN (���, �� ��� ������-��, ����)
enum EHaveRet {hr_No, hr_NotAll, hr_Yes};


//-----------------------------------------------------------------------------
//���������� ��� ��������� �� ���������� ������
class error_Internal
{
public:
	error_Internal(char *message) : error_message(message) {};
	char *error_message;
};


//-----------------------------------------------------------------------------
//����� ��� �������� ���������� � WriteCPP
class CPP_files
{
public:
	CPP_files() : f2ml(NULL), fc(NULL), fh(NULL), tab_level_c(0), ext_offs(0), f_name(NULL) {};
	//����������, ����������� �������� �����
	~CPP_files();
	//�������� ������ ��� ������ � ������� ��������� ��� ������������� ������
	int Init(const char* name);
	//������� ��������� � ���� ��������
	void tab_fc();
	//���� ��� �������� ���������
	TFileType *f2ml;
	//���� ��������
	TFileType *fc;
	//���� ����������
	TFileType *fh;
	//���������� ��������� ��� ������� ������ ����� ��������
	int tab_level_c;
private:
	//�������� �������� ���������� �� ������ ������� ����� �����
	int ext_offs;
	//��� �������� ������ ���� ������ ������� ���� � ����������
	char* f_name;
};//CPP_files


//-----------------------------------------------------------------------------
//����� ��� �������� ���������� � WriteDFN
class DFN_file
{
public:
	DFN_file() : f(NULL), tab_level(0), f_name(NULL) {};
	//����������, ����������� �������� ����
	~DFN_file();
	//�������� ����� ��� ������ � ������� ��������� ��� ������������� ������
	int Init(const char* name);
	void tab();
	//DFN ����
	TFileType *f;
	//���������� ��������� ��� ������� ������ DFN �����
	int tab_level;
private:
	//��� �������� ������� ����� ����� ������� ���� � ����������
	char* f_name;
};//DFN_file


//-----------------------------------------------------------------------------
//������� ����� ��� ����������
class CBase
{
public:
	CBase(const CBaseName* parent) : parent_element(parent) {};
	const CBaseName* parent_element;
	virtual ~CBase() {};
	//�������� ������� ��������� RETURN (������ ��� ����������)
	virtual EHaveRet HaveRet() const {return hr_No;};
	virtual int Init(CLexBuf *lb) = 0;
	virtual void WriteCPP(CPP_files& f) = 0;
};//CBase


//-----------------------------------------------------------------------------
//������� ����� ��� ����������� ��������
class CBaseName
{
public:
	CBaseName(EName_id id, const CBaseName* parent) : external(false),
		name(NULL),
		name_id(id),
		parent_element(parent) {};
	virtual ~CBaseName() {delete[] name;};
	//���������� ���������� ��-�� � ������� ����
	virtual void AddName(CBaseName*) const {throw error_Internal("CBaseName::AddName");};
	//����������� ����� � ��������� � ��������� ������ (BaseName)
	void Assign(CBaseName* BaseName) const;
	//����� ����� � ������ ���� ������� (���������� ������ �� �������, � ������� ��������������)
	virtual CBaseName* FindName(const char*) const {throw error_Internal("CBaseName::FindName");};
	//��������� �� �������� ������� �� ���������� ������� ���������
	CBaseName* GetGlobalName(const char* global_name) const;
	//��������� �� ����������� �������� ������� �� ���������� ������� ���������
	CBaseName* GetGlobalName(const char* module_name, const char* global_name) const;
	//��������� ������ (�������� ��� DFN) �� ��-��� parent_element
	const CBaseName* GetParentModule() const;
	//��������� id ����������, ���� return id_CBaseName => ��������� ������������
	virtual EName_id GetResultId() const {return id_CBaseName;};
	//������������� ������� �� ������ ������
	virtual int Init(CLexBuf *lb) = 0;
	//����������� � ������ ���������� �����
	void SetName(const char* NewName);
	//������ ����������������� ���� C++ (������ ���������� �������)
	virtual void WriteCPP(CPP_files& f) = 0;
	//������ � .dfn ���� (������ ��� �������������� ��������)
	virtual void WriteDFN(DFN_file& f) {};
	//���� '*' (������ ��������������)
	bool external;
	//��� �������
	char *name;
	//��������� ���������� ����� ����������� ��������
	EName_id name_id;
	//������������ ��-� (��������� ��� ������ � �������� ����)
	const CBaseName* parent_element;
};//CBaseName


//-----------------------------------------------------------------------------
//������� ����� ����������� ��������-�������
class CStdProcFunc : public CBase
{
public:
	CStdProcFunc(const CBaseName *parent) : CBase(parent), ResultId(id_CBaseName) {};
	//��������� ������������ id ���������� ���������
	EName_id GetResultId() {return ResultId;};
	//�������� ���������
	virtual int CreateConst(CBaseVar* &BaseConst) {return s_m_Error;};
protected:
	EName_id ResultId;	//��� �������� ������������ id ���������� (��� id_CBaseName)
};


//-----------------------------------------------------------------------------
//������� ����� ��� �����
class CBaseType : public CBaseName
{
public:
	CBaseType(EName_id id, const CBaseName* parent) : CBaseName(id, parent) {};
	//�������� ����� ������� ����
	virtual int CreateType(CBaseType* &BaseType) const = 0;
	//�������� ���������� ������� ����
	virtual int CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const = 0;
	//��������� ���� �������� ������� ���� � C++
	static const char* GetCPPTypeName(const EName_id ResultId);
	//��������� ���������� DFN ������, ��������������� ������ ��� (���� ����)
	const char* GetModuleAlias() const;
	const char* GetModuleName() const;
	//��������� id ���������� (�������������� id ���� � id ��������������� ����������)
	EName_id GetResultId() const;
	bool IsSame(const char *module_name, const char *type_name) const;
	static bool IsTypeId(const EName_id id);
	virtual void WriteCPP(CPP_files &f, const bool to_h) = 0;
private:
	//�������� WriteCPP, �.�. � ����������� ����� ������� ������ ������ �������������
	void WriteCPP(CPP_files &f) {};
};//CBaseType


//-----------------------------------------------------------------------------
//������� ����� ��� ����������
class CBaseVar : public CBaseName
{
public:
	//����������� ��������� �������� ������� � ��������� ����������
	void Assign(CBaseVar *BaseVar) const;
	CBaseVar(EName_id id, const CBaseName* parent) : CBaseName(id, parent),
		is_const(false),
		is_guarded(false),
		is_read_only(false),
		is_var(false),
		type_module_name(NULL),
		type_name(NULL) {};
	virtual ~CBaseVar();
	//�������� ������������� ���� �����, �������� � ������ ����������
	//��� ������ ����������, ������� ����� �������� ��� (����) ���������, ������ �������������� ������ �����
	virtual int CheckComplete(CLexBuf *lb) {return 0;};
	//�������� ���������, ���������������, ��� is_const == true
	//��� ������ ����������, ������� ����������� �����, ������ �������������� ������ �����
	virtual CBaseVar* CreateConst(const CBaseName* parent) const {throw error_Internal("CBaseVar::CreateConst");};
	//�������� ��������������� ��������
	static bool IsRealId(const EName_id id);
	static bool IsIntId(const EName_id id);
	static bool IsDigitId(const EName_id id);
	static bool IsVarId(const EName_id id);
	//��������� �������������� �������� ����������
	virtual long GetIntValue() const {throw error_Internal("CBaseVar::GetIntValue");};
	//��� ���������� � ���������� ���������
	virtual void WriteCPP_fp(CPP_files &f, const bool to_h) = 0;
	//������ ���������� ������, ��������������� ��� ���������� (�������� � ��� ������������� �����)
	const char* GetTypeModuleAlias();
	const char* GetTypeModuleName();
	//������ ����� ���� ���������� (����������� ��� ��� ��� CPP)
	const char* GetTypeName();
	void SetTypeName(const char *TypeModuleName, const char* TypeName);
	//�������� ����� ���������� (��� ����������� ������ � ������ ��������)
	virtual CBaseVar* CreateVar(const CBaseName* parent) const = 0;
	//������ ���� ��� ������������� ������������ ��������, ���������������, ��� is_const == true
	//��� ������ ����������, ������� ����������� �����, ������ �������������� ������ �����
	virtual void WriteCPP_ConstValue(CPP_files& f) {throw error_Internal("CBaseVar::WriteCPP_ConstValue");};
	//���� ���������� ��������� ��� ���������
	bool is_const;
	//���� ���������� ��� ������� (������ WITH), ��� ��������� ���� ���������� ���������� ����
	bool is_guarded;
	//���� '-' (������ �������������� ������ ��� ������)
	bool is_read_only;
	//���� ���������� �������� �������� ��� VAR
	bool is_var;
	//��������� id ���������� (� ������ ������ id ����������)
	EName_id GetResultId() const {return name_id;};
protected:
	//������ ���� ���������� ������������ ���� � ��������� ���������
	void WriteCPP_fp_named_type(CPP_files &f, const bool to_h);
	void WriteDFN_named_type(DFN_file &f);
	//�������� ������, ����������� ����������� ��� ����������, ��� NULL
	char* type_module_name;
	//�������� ���� ���������� � ������ ������������ ����, ��� NULL
	char* type_name;
private:
	//���������� �� ���������������� (������ ��� ��������� ����� Type->CreateVar)
	int Init(CLexBuf *lb) {throw error_Internal("CBaseVar::Init");};
};//CBaseVar


//-----------------------------------------------------------------------------
//IdentDef - ��������� ������ ��� ������������� ������������ �������
//	(����������, ����, � �.�.)
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
	//�� ������������
	void WriteCPP(CPP_files& f) {};
	//������� ������� ������� ������ => ������ ��������� � ����������������� ����������
	bool is_common;
	//������� ������� '-' - ���������� ������ ��� ������ (��� ��������� �������� �� ������������)
	bool is_read_only;
	//��������� �������� ��������
	char* QualTagName;
	//������� ������������ �������� is_read_only
	bool read_only_enabled;
	//�������� ��������
	char* TagName;
};//CIdentDef


//-----------------------------------------------------------------------------
//IdentList - ��������� ������ ��� ������������� ����� ������� 
//	� ���������� ���������� �������� � ����������� ��������
class CIdentList
{
	CBaseType* BaseType;		//��� ���� �����-��� � ������
	CBaseNameVector BNVector;	//������ ����������� ��-��
	const CBaseName* parent_element;
	bool is_local;				//������� �������� ���������� IdentList (�������� ������� ���������������)
	//�������� ������ ��������������� ��� �������� ����� ��������
	int LoadIdentList(CLexBuf *lb);
public:
	CIdentList(const CBaseName* parent, const bool is_local_list) : BaseType(NULL),
		parent_element(parent), is_local(is_local_list) {};
	~CIdentList();
	//������������� ���������� ������ ���������-�����������
	int AssignVars(CLexBuf *lb);
};//CIdentList


//-----------------------------------------------------------------------------
//Qualident - [�������� ������] "." �������� ����������
class CQualident
{
public:
	void Assign(CQualident* Qualident) const;
	void Clear();
	CQualident() : ident(NULL), pref_ident(NULL), TypeResultId(id_CBaseName) {};
	~CQualident();
	//�������� ����� CQualident
	void CreateCopy(CQualident *&qual) const;
	int Init(CLexBuf *lb, const CBaseName* parent_element);
	int InitTypeName(CLexBuf *lb, const CBaseName* parent_element);
	//������ ���� ��� CQualident, ����������� ��� ����
	void WriteCPP_type(CPP_files& f, const bool to_h, const CBaseName* parent_element);
	void WriteDFN(DFN_file& f);
	//�������� �����
	char *ident;
	//������ �����
	char *pref_ident;
	//ResultId ��� ���� (���� Qualident �������� ������ ����), ��� id_CBaseName
	EName_id TypeResultId;
};//CQualident


//-----------------------------------------------------------------------------
//����� ��� ����� � ����������� LOOP � WITH ����� ������� parent_element
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
	//�� ��������� ��� ����� � LOOP
	int Init(CLexBuf *lb) {return 0;};
	//������ ��� ������ � �������� ���� (�������� ������ 1 ����������)
	void AddName(const char* ModuleName, CBaseName* GuardVar) const;
	CBaseName* FindName(const char* search_name) const;
	//��������� �������� IsUnderLoop (������� �������������� � LOOP)
	bool UnderLoop() const {return IsUnderLoop;};
	//��� �������� UID ��� LOOP ����� parent_element
	int LoopUID;
private:
	//������� CWithLoopLink ��� ��������� LOOP
	bool IsUnderLoop;
	//����������, ��� ������� ����������� ������ ���� (������ ��� WITH)
	mutable CBaseName* Var;
	//��� ������ � ������ ������������� ��������������� ����������
	mutable char* VarModuleName;
	//�������� ������ ��������� ����
	void WriteCPP(CPP_files& f) {};
};


#endif	//O2M_Base_h
