//=============================================================================
// ���������� ����������, ������������ ����� �������� �����������
//=============================================================================

#ifndef O2M_Common_h
#define O2M_Common_h

//#ifdef _WIN32
//�������� �������������� Visual C++ 6.0 � ������� ������� 255 ��������
//#pragma warning(disable: 4786)
//#endif

#include "LexAnl.h"


class CBase;
class CBaseName;
class CBaseVar;
class CBaseType;
class CElsifPair;
class CGuardPair;
class CFPElem;
class CSetVarElem;

typedef std::vector <char*> StringVector;
typedef std::vector <CBase*> CBaseVector;
typedef std::vector <CBaseName*> CBaseNameVector;
typedef std::vector <CBaseVar*> CBaseVarVector;


//-----------------------------------------------------------------------------
//������ ������������ ��� ������������ ������������ ������
const char* const comment_line_cpp = "/////////////////////////////////////////////\n";
const char* const comment_line_dfn = "(*******************************************)\n";
const char* const comment_title = "This file was generated by O2M compiler";
const char* const comment_format = "%s// %s //\n%s\n";

//��������� �� ������ �������� ����� ��� ������
const char* const textCannotOpenW = "ERROR: Cannot open file \"%s\" for writing\n";
//��������� �� ������ �������� �����
const char* const textCannotClose = "ERROR: Cannot close file \"%s\"\n";
//�������������� � ������������� �������� ������� ����
const char* const textCheckFolder = "WARNING: Make sure that folder \"%s\" exists in source file folder\n";


//-----------------------------------------------------------------------------
//���������� ���������� � ���������� � ��� ������� ������� � ������ ������
#define DECL_SAVE_POS CLexBufPos CurrLexBufPos = lb->GetCurrPos();
//���������� � ���������� ������� ������� � ������ ������
#define SAVE_POS CurrLexBufPos = lb->GetCurrPos();
//�������������� ������� ������� � ������ ������ �� �������� ����������
#define RESTORE_POS lb->SetCurrPos(CurrLexBufPos);


//-----------------------------------------------------------------------------
//������� ��� ��������� ���������� � �������
#define O2M_SYS_ "O2M_SYS_"


//-----------------------------------------------------------------------------
//����������� ��������� �������� ��-�� ���������
const int SET_MAX = (sizeof(int) << 3) - 1;


//-----------------------------------------------------------------------------
//���� ����������� CBaseName
enum EName_id {
	id_CBaseName, 
	id_CModule,
	id_CDfnModule,
	id_CDfnModuleSystem,
	id_CImportModule,
	id_CWithLoopLink,
	id_CProcedure,
	id_CHandlerProc,
	id_CCommonProc,
	id_CDfnProcedure,
	id_CDfnCommonProc,
	id_CForwardDecl,
	id_CIdentDef, 

	id_CProcedureType,
	id_CSetType,
	id_CQualidentType,
	id_CSpecType,
	id_CArrayType,
	id_CRecordType,
	id_CCommonType,
	id_CCommonExtensionType,
	id_CPointerType,
	id_CLongrealType,
	id_CRealType,
	id_CLongintType, 
	id_CIntegerType,
	id_CShortintType,
	id_CCharType,
	id_CBooleanType,
	id_CPtrType,

	id_CSetVar,
	id_CArrayVar,
	id_CRecordVar,
	id_CCommonVar,
	id_CPointerVar,
	id_CProcedureVar,
	id_CLongrealVar,
	id_CRealVar,
	id_CLongintVar,
	id_CIntegerVar,
	id_CShortintVar,
	id_CCharVar,
	id_CBooleanVar,
	id_CPtrVar
};


//-----------------------------------------------------------------------------
//�������� ���������� (inclusion) ����� id1 ���� id2
bool IsId1IncloseId2(const EName_id id1, const EName_id id2);

//-----------------------------------------------------------------------------
//��������� ��. ����������� ����, ������������ ��� ��������, ��� id_CBaseName
EName_id GetMaxDigitId(const EName_id id1, const EName_id id2);


//-----------------------------------------------------------------------------
//�������� ������� TYPE �� ������ ������
int TypeSelector(CLexBuf *lb, CBaseType* &BaseType, const CBaseName* parent);


//-----------------------------------------------------------------------------
//�������� ��������� �� ������ ������
int ConstSelector(CLexBuf *lb, CBaseVar* &BaseVar, const CBaseName* parent_element);


//-----------------------------------------------------------------------------
//����������� ������ � ���������� ������
char* str_new_copy(const char* source);


//-----------------------------------------------------------------------------
//��������� ����������� ��������� ��������������
int GetGlobalUId();


#endif	//O2M_Common_h