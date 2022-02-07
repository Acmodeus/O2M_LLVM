//=============================================================================
// �������� ����������, ������������ ����� �������� �����������
//=============================================================================

#include "Common.h"
#include "Type.h"
#include "Stat.h"


//-----------------------------------------------------------------------------
//�������� ���������� (inclusion) ����� id1 ���� id2
bool IsId1IncloseId2(const EName_id id1, const EName_id id2) {
	if (id1 == id2) return true;
	switch (id1) {
	case id_CLongrealVar:
		switch (id2) {
		case id_CRealVar:
		case id_CLongintVar:
		case id_CIntegerVar:
		case id_CShortintVar:
			return true;
		default:
			return false;
		}
	case id_CRealVar:
		switch (id2) {
		case id_CLongintVar:
		case id_CIntegerVar:
		case id_CShortintVar:
			return true;
		default:
			return false;
		}
	case id_CLongintVar:
		switch (id2) {
		case id_CIntegerVar:
		case id_CShortintVar:
			return true;
		default:
			return false;
		}
	case id_CIntegerVar:
		if (id2 == id_CShortintVar) return true;
	default:
		return false;
	}
}


//-----------------------------------------------------------------------------
//�������� ���������� ����� (Same Types)
bool IsSameTypes(const CBaseVar* v1, const CBaseVar* v2) {
	if (v1->name_id != id_CArrayVar)
		if (v1->name_id == v2->name_id) return true; else return false;
	else
		/**/
		//������ �������� �� �������� ��������
		return false;
}


//-----------------------------------------------------------------------------
//�������� ������ ����� (Equal Types)
bool IsEqualTypes(const CBaseVar* v1, const CBaseVar* v2) {
	if (IsSameTypes(v1, v2)) return true;
	/**/
	//������ �������� �������� �������� � ������� ������ ��-��� � ����������� ����� � ������������ �������� ���������� ����������
	return false;
}


//-----------------------------------------------------------------------------
//�������� ������������� �� ������������ (Assign Compatible)
bool IsEAssignCompatibleWithV(const CBaseVar* ve, const CBaseVar* vv) {
	if (IsSameTypes(ve, vv)) return true;
	if (CBaseVar::IsDigitId(ve->name_id) && CBaseVar::IsDigitId(vv->name_id) && IsId1IncloseId2(vv->name_id, ve->name_id)) return true;

	/**/

	return false;
}


//-----------------------------------------------------------------------------
//��������� ��. ����������� ����, ������������ ��� ��������, ��� id_CBaseName
EName_id GetMaxDigitId(const EName_id id1, const EName_id id2) {
	if (IsId1IncloseId2(id1, id2)) return id1;
	if (IsId1IncloseId2(id2, id1)) return id2;
	return id_CBaseName;
}


//-----------------------------------------------------------------------------
//�������� ������� TYPE �� ������ ������
int TypeSelector(CLexBuf *lb, CBaseType* &BaseType, const CBaseName* parent)
{
	//��� �������� ������� QualidentType ����� ��������� ������� � �����
	DECL_SAVE_POS
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//��������� ��������� ����� ��� ��.
	if (!lb->ReadLex(li) || (lex_k_dot > li.lex && lex_i != li.lex)) return s_e_TypeDefinition;

	BaseType = NULL;				//��������� �� ���

	//�������� ���� ������� ��� ������ ����������������� ����
	switch (li.lex) {
	case lex_i:
		//�������� ������� QualidentType
		RESTORE_POS
		BaseType = new CQualidentType(parent);
		break;
	case lex_k_ARRAY:
		BaseType = new CArrayType(parent);
		break;
	case lex_k_RECORD:
		BaseType = new CRecordType(parent);
		break;
	case lex_k_CASE:
		BaseType = new CCommonType(parent);
		break;
	case lex_k_POINTER:
		BaseType = new CPointerType(parent);
		break;
	case lex_k_PROCEDURE:
		BaseType = new CProcedureType(parent);
		break;
	case lex_k_SET:
		BaseType = new CSetType(parent);
		break;
	case lex_k_LONGREAL:
		BaseType = new CLongrealType(parent);
		break;
	case lex_k_REAL:
		BaseType = new CRealType(parent);
		break;
	case lex_k_LONGINT:
		BaseType = new CLongintType(parent);
		break;
	case lex_k_INTEGER:
		BaseType = new CIntegerType(parent);
		break;
	case lex_k_SHORTINT:
		BaseType = new CShortintType(parent);
		break;
	case lex_k_CHAR:
		BaseType = new CCharType(parent);
		break;
	case lex_k_BOOLEAN:
		BaseType = new CBooleanType(parent);
	}

	//������������� ���������� �������
	if (BaseType) {
		int err_num = BaseType->Init(lb);
		//�������� ���������� ������ ��� �������������
		if (err_num) {
			delete BaseType;
			BaseType = NULL;
			return err_num;	//��� ������
		}
		//�������� ������� SpecType
		if (BaseType->name_id == id_CQualidentType) {
			SAVE_POS
			err_num = !lb->ReadLex(li) || lex_k_lt != li.lex;
			RESTORE_POS
			if (!err_num) {
				//������ SpecType, �������������� ���
				//�������� ��� ����������� Qualident �� CQualidentType
				CQualident* q = static_cast<CQualidentType*>(BaseType)->Qualident;
				static_cast<CQualidentType*>(BaseType)->Qualident = NULL;
				delete BaseType;
				BaseType = new CSpecType(parent);
				static_cast<CSpecType*>(BaseType)->Qualident = q;
				err_num = BaseType->Init(lb);
				if (err_num) {
					delete BaseType;
					BaseType = NULL;
					return err_num;
				}
			}
		}
		//��� ���������������
		return 0;
	}

	return s_e_TypeDefinition;
}//TypeSelector


//-----------------------------------------------------------------------------
//�������� ��������� �� ������ ������
int ConstSelector(CLexBuf *lb, CBaseVar* &BaseVar, const CBaseName* parent_element)
{
	//�������� ���������
	CExpr* Expr = new CExpr(parent_element);
	int err_num = Expr->Init(lb);
	if (err_num) {
		delete Expr;
		BaseVar = NULL;
		return err_num;
	}
	//��������� ���������
	err_num = Expr->CreateConst(BaseVar);
	delete Expr;
	return err_num;
}


//-----------------------------------------------------------------------------
//����������� ������ � ���������� ������
char* str_new_copy(const char* source)
{
	char* dest = new char[strlen(source) + 1];
	return strcpy(dest, source);
}


//-----------------------------------------------------------------------------
//��������� ����������� ��������� ��������������
int GetGlobalUId()
{
	static int GlobalUId = 0;
	return GlobalUId++;
}
