//=============================================================================
// �������� ������� ����� (Type)
//=============================================================================

#include "Stat.h"
#include "Type.h"
#include "Var.h"


//-----------------------------------------------------------------------------
//������������� ������ ������������� ������� ���������-�����������
int CIdentList::AssignVars(CLexBuf *lb)
{
	//�������� ������ ���������������
	int err_num = LoadIdentList(lb);
	if (err_num) return err_num;

	//� ������ ������������ ���� ���������� ��������� ��� ������� ��� �������� ���������� ������� ����
	//CBaseType* BT = NULL;	//��. �� ��� ����� �� �������� ���� �� ������� �� �����������

	//�������� ����������� ���������� �� ���������� ����
	CBaseNameVector::iterator vi;
	for (vi = BNVector.begin(); vi != BNVector.end(); ++vi) {
		CIdentDef* IdentDef = static_cast<CIdentDef*>(*vi);
		//�������� ���������� ���������� ����
		CBaseVar* BaseVar;
		err_num = BaseType->CreateVar(BaseVar, parent_element);
		if (err_num) return err_num;
		//��������� ���������� ���������� �� �� �������� � ������ ���������������
		err_num = IdentDef->AssignVar(BaseVar);
		if (err_num) {
			delete BaseVar;
			return err_num;
		}

		//�������� ��������� �������������������� ���������� ����������
		if (id_CCommonVar == BaseVar->name_id && static_cast<CCommonVar*>(BaseVar)->IsPureCommon()) {
			//������� �������� ��� ������������� �� ���������
			//��������� ���� ���������
			CBaseType* CT;
			if (id_CQualidentType == BaseType->name_id) {
				err_num = static_cast<CQualidentType*>(BaseType)->GetNamedType(CT, false);
				if (err_num) return err_num;
			} else
				CT = BaseType;
			if (id_CCommonType != CT->name_id) return s_e_CommonTypeExpected;
			const CCommonType::SSpec* spec = static_cast<CCommonType*>(CT)->GetDefaultSpec();
			if (!spec) {
				//������������� �� ��������� �� �������, ������ ��������� �� ������
				delete BaseVar;
				return s_e_SpecTypeExpected;
			}
			//��������� ����� ������������� �� ���������
			err_num = static_cast<CCommonVar*>(BaseVar)->SetTagName(spec->QualName, spec->Name);
			if (err_num) return err_num;
		}

		//��������� ���������� ���������� � ������
		parent_element->AddName(BaseVar);
	}

	return 0;
}//CIdentList::AssignVars


//-----------------------------------------------------------------------------
//���������� ������� CDeclSeq
CDeclSeq::~CDeclSeq()
{
	CBaseNameVector::const_iterator ci;
	for(ci = BaseNameStore.begin(); ci != BaseNameStore.end(); ++ci)
		delete *ci;
	delete DfnModuleSeq;
}//~CDeclSeq


/**/
/*
//-----------------------------------------------------------------------------
//���������� DeclSeq (������� ����)
void CDeclSeq::debug_PutsDeclSeq()
{
	char *st = new char[MaxStLeng];
	strcpy(st, "===== Name table (");
	strcat(st, parent_element->name);
	strcat(st, ") =====");
	puts(st);
	BaseNameList_type::iterator i;
	for(i = BaseNameList.begin(); i != BaseNameList.end(); ++i)
		switch ((*i)->name_id) {
		case id_CQualidentType:
		case id_CArrayType:
		case id_CRecordType:
		case id_CPointerType:
		case id_CSetType:
		case id_CLongrealType:
		case id_CRealType:
		case id_CLongintType:
		case id_CIntegerType:
		case id_CShortintType:
		case id_CCharType:
		case id_CBooleanType:
		case id_CProcedureType:
			strcpy(st, (*i)->name);
			strcat( st, " = TYPE");
			puts(st);
			break;
		case id_CForwardDecl:
			strcpy(st, (*i)->name);
			strcat( st, " = PROCEDURE^ (Forward)");
			puts(st);
			break;
		case id_CProcedure:
			strcpy(st, (*i)->name);
			strcat( st, " = PROCEDURE");
			puts(st);
			break;
		case id_CDfnProcedure:
			strcpy(st, (*i)->name);
			strcat( st, " = PROCEDURE (DFN)");
			puts(st);
			break;
		case id_CArrayVar:
		case id_CRecordVar:
		case id_CPointerVar:
		case id_CProcedureVar:
		case id_CLongrealVar:
		case id_CRealVar:
		case id_CLongintVar:
		case id_CIntegerVar:
		case id_CShortintVar:
		case id_CCharVar:
		case id_CBooleanVar:
		case id_CSetVar:
			strcpy(st, (*i)->name);
			strcat( st, " = VAR");
			puts(st);
			break;
		case id_CImportModule:
			//Imported table
			const char* r_n = ((CImportModule*)(*i))->real_name;
			CDfnModule* DM = NULL;
			if (DfnModuleSeq) DM = DfnModuleSeq->FindName(r_n);
			if (DM) DM->DfnDeclSeq->debug_PutsDeclSeq();
			break;
		default:
			int num = (*i)->name_id;
			puts((*i)->name);
		};//switch
	strcpy(st, "----- end table (");
	strcat(st, parent_element->name);
	strcat(st, ") ------");
	puts(st);
}//PutsDeclSeq
*/


//-----------------------------------------------------------------------------
//������������� ������� CDeclSeq
int CDeclSeq::Init(CLexBuf *lb)
{
	DECL_SAVE_POS
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;
	//���������� ��� ��������� ������ ������
	int err_num;

	//�������� ������� ��������� ����� (���������� ������ ������)
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;

	//�������� ������� ������-��� ���������� CONST, TYPE, VAR
	while (true) {

		//��� ����� ���������� (���������, ����, ����������)
		enum {dkCONST, dkTYPE, dkVAR} DeclKind;
		//�������� ���������� �������
		if (lex_k_CONST == li.lex) DeclKind = dkCONST;
		else
			if (lex_k_TYPE == li.lex) DeclKind = dkTYPE;
			else
				if (lex_k_VAR == li.lex) DeclKind = dkVAR;
				else
					break;

		//��������� ������-�� ���������� (��� CONST, ��� TYPE, ��� VAR)
		while(true) {

			//����� ������� ��������������� ��-�� ���������� �������
			SAVE_POS

			//����� ��������������� ��-�� (� �����-�� �� ���� ����������)
			switch (DeclKind) {
			case dkCONST:
				err_num	= ConstInit(lb);
				break;
			case dkTYPE:
				err_num	= TypeInit(lb);
				break;
			case dkVAR:
				err_num	= VarInit(lb);
				break;
			}

			//�������� ������������� ��������� ����������
			if (err_num == s_m_IdentDefAbsent) {
				//�����, ���� �� ����� ���������� (�����. �������)
				RESTORE_POS
				break;
			}

			//�������� ���������� ������ ������������� ����������
			if (err_num) return err_num;

			//������ ��. ����� ";"
			if (!lb->ReadLex(li) || lex_k_semicolon != li.lex)
				return s_e_SemicolonMissing;

		}//while

		SAVE_POS

		//�������� ������� ��������� ����� (���������� ������ ������)
		if (!lb->ReadLex(li) || (lex_k_dot > li.lex))
			return s_e_BEGIN;
	}//while

	//���������� ���. �������� ������������ ������� ����
	err_num = CheckCompleteRoot(lb);
	if (err_num) return err_num;

	//�������� ������� ������-��� ���������� PROCEDURE, PROCEDURE ^
	while (lex_k_PROCEDURE == li.lex) {
		err_num = ProcInit(lb);
		if (err_num) return err_num;

		//������ ��. ����� ";"
		if (!lb->ReadLex(li) || lex_k_semicolon != li.lex) return s_e_SemicolonMissing;

		SAVE_POS

		//�������� ������� ��������� ����� (���������� ������ ������)
		if (!lb->ReadLex(li) || (lex_k_dot > li.lex))
			return s_e_BEGIN;
	}

	//��������� ��������� ������� �� �� DeclSeq
	RESTORE_POS

	return 0;
}//CDeclSeq::Init


//-----------------------------------------------------------------------------
//���������� ������������� ������� � ������ ����������
int CDeclSeq::ImportListInit(const CProject *project, CLexBuf *lb)
{
	DECL_SAVE_POS
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� ��. ����� IMPORT
	if (!lb->ReadLex(li) || lex_k_IMPORT != li.lex) {
		//��� ������ ������� - ��������������� �������
		RESTORE_POS
		return 0;
	}
	else {
		while (true) {
			//���� ������ ������� - ��������� ���
			CImportModule *ImpMod = new CImportModule(parent_element);
			int err_num = ImpMod->Init(lb);
			if (err_num) {
				delete ImpMod;
				return err_num;
			}
			//�������� ���������� ����� � ������� ���� (DeclSeq)
			if (FindName(ImpMod->name)) {
				delete ImpMod;
				return s_e_Redefinition;
			}

			//��������� ���������� ������� � ������
			AddName(ImpMod);

			//�������� ���������� �������������� ������ ����
			if (!strcmp(parent_element->name, ImpMod->real_name))
				return s_e_RecursiveImport;

			//�������� dfn ���������������� ������
			err_num = LoadDfnModule(project, ImpMod->real_name, ImpMod->name);
			if (err_num) return err_num;

			//�������� ������� ","
			if (!lb->ReadLex(li) || lex_k_comma != li.lex) break;
		}
		//�������� ������� ";"
		if (lex_k_semicolon != li.lex) return s_e_SemicolonMissing;
	}

	return 0;
}//CDeclSeq::ImportListInit


//-----------------------------------------------------------------------------
//������ ���� CDeclSeq ��� �����
int CDeclSeq::WriteCPP_type(CPP_files& f)
{
	if (id_CModule == parent_element->name_id) fprintf(f.fc, "\n//IMPORT\n");
	CBaseNameVector::const_iterator ci;

	//������ ���� ������������� �������
	for (ci = BaseNameStore.begin(); ci != BaseNameStore.end(); ++ci) {
		if (id_CImportModule == (*ci)->name_id) {
			(*ci)->WriteCPP(f);
			//��������� namespace'�� ���� �� �����
			fprintf(f.fc, "\n");
		}
	}

	//������ ���� ������� ����� (��� ���������� ���� ������� � ������������ ����, �.�.
	//��� ����� �������������� ��� �������� ����������)
	for (ci = BaseNameStore.begin(); ci != BaseNameStore.end(); ++ci)
		if (CBaseType::IsTypeId((*ci)->name_id)) {
			//��������, �������� �� ��� ����������, ���� ��� - �������� �������� ������� '*'
			bool to_h = (id_CModule == (*ci)->parent_element->name_id) || (*ci)->external;
			//��������, �������� �� ��� ������� (��� ������� ������� ������� ������ ����������)
			if (id_CRecordType == (*ci)->name_id)
				fprintf(to_h ? f.fh : f.fc, "struct %s;\n", (*ci)->name);
			else	//������� ������ ���� ���������� ����
				static_cast<CBaseType*>(*ci)->WriteCPP(f, to_h);
		}

	//��������� ���� �������� ���� ������ (�� ����� ���� ������������� ������ ����������,
	//��� ����������, �.�. � ������ ����� �������������� ��� �� ����������� �� ������ ����������
	//������ ����, � ���������� ������ ���������� � ��� ������)
	for (ci = BaseNameStore.begin(); ci != BaseNameStore.end(); ++ci)
		if (id_CRecordType == (*ci)->name_id) {
			//��������, �������� �� ��� ����������, ���� ��� - �������� �������� ������� '*'
			bool to_h = (id_CModule == (*ci)->parent_element->name_id) || (*ci)->external;
			//�������� ���� ������
			static_cast<CRecordType*>(*ci)->WriteCPP(f, to_h);
		}

	return 0;
}//WriteCPP_type


//-----------------------------------------------------------------------------
//������ ���� CDeclSeq ��� ��������
int CDeclSeq::WriteCPP_proc(CPP_files& f)
{
	CBaseNameVector::const_iterator ci;
	for (ci = BaseNameStore.begin(); ci != BaseNameStore.end(); ++ci)
		if (CProcedure::IsProcId((*ci)->name_id)) {
			fprintf(f.fc, "\n%s", comment_line_cpp);
			(*ci)->WriteCPP(f);
		}
	return 0;
}//WriteCPP_proc


//-----------------------------------------------------------------------------
//������ ���� CDeclSeq ��� ����������
int CDeclSeq::WriteCPP_var(CPP_files& f)
{
	//������� ������������ ���� ���������� � ������ ������� ����
	bool is_global = id_CModule == parent_element->name_id;
	//������ ����������� ������ �������� ����������
	fprintf(f.fc, "%s//VAR\n", id_CModule == parent_element->name_id ? "\n" : "");
	//���� �������� ����������
	CBaseNameVector::const_iterator ci;
	for(ci = BaseNameStore.begin(); ci != BaseNameStore.end(); ++ci)
		if (CBaseVar::IsVarId((*ci)->name_id)) {
			//�������� ������������� ��������� ���������� � ���������� ������������ ����
			//(���������� ��� ���������� ������������������ ����������)
			bool req_unnm_space = is_global && !(*ci)->external;
			if (req_unnm_space) fprintf(f.fc, "namespace {");
			//��������� ���� ����������
			(*ci)->WriteCPP(f);
			//��������� ���������� ���� �� ����� � ������� '}' (���� ���������� ������� ������������ ����)
			fprintf(f.fc, ";%s\n", req_unnm_space ? "}" : "");
			//��������� ���������� � .h �����
			if ((*ci)->external) fprintf(f.fh, ";\n");
		}
	return 0;
}//WriteCPP


//-----------------------------------------------------------------------------
//������ � .dfn ����
void CDeclSeq::WriteDFN(DFN_file& f)
{
	//���������� ������� � DFN �����
	f.tab_level++;

	CBaseNameVector::const_iterator i;

	//������� ���������� ��-��� � ���������, ������� ��������� (IMPORT, CONST, TYPE, VAR)
	bool no_elems(true);

	//������ ���� ������������� �������
	for (i = BaseNameStore.begin(); i != BaseNameStore.end(); ++i)
		if ( (*i)->name_id == id_CImportModule )
		{
			//�������� ������� ���������������� ������
			if (no_elems) {
				no_elems = false;
				fprintf(f.f, "\nIMPORT\n\t");
			} else
				fprintf(f.f, ", ");
			//������ ���� �������� ������
			(*i)->WriteDFN(f);
		}
	if (!no_elems) fprintf(f.f, ";\n");

	//������ ���� �������������� ��������
	no_elems = true;
	for (i = BaseNameStore.begin(); i != BaseNameStore.end(); ++i)
		if ( (*i)->external && CBaseVar::IsVarId((*i)->name_id) && static_cast<CBaseVar*>(*i)->is_const ) {
			//�������� 1-� �������������� ���������
			if (no_elems) {
				no_elems = false;
				fprintf(f.f, "\nCONST\n\t");
			} else
				fprintf(f.f, "\t");
			//������ ���� ������� ���������
			(*i)->WriteDFN(f);
			fprintf(f.f, ";\n");
		}

	//������ ���� �������������� �����
	no_elems = true;
	for (i = BaseNameStore.begin(); i != BaseNameStore.end(); ++i)
		if ( (*i)->external && CBaseType::IsTypeId((*i)->name_id) )
		{
			//�������� ������� ��������������� ����
			if (no_elems) {
				no_elems = false;
				fprintf(f.f, "\nTYPE\n\t");
			} else
				fprintf(f.f, "\t");
			//������ ���� �������� ����
			fprintf(f.f, "%s = ", (*i)->name);
			(*i)->WriteDFN(f);
			fprintf(f.f, ";\n");
		}

	//������ ���� �������������� ����������
	no_elems = true;
	for(i = BaseNameStore.begin(); i != BaseNameStore.end(); ++i)
		if ( (*i)->external &&
			((*i)->name_id != id_CImportModule) &&
			!CProcedure::IsProcId((*i)->name_id) &&
			!CBaseType::IsTypeId((*i)->name_id) &&
			!(CBaseVar::IsVarId((*i)->name_id) && static_cast<CBaseVar*>(*i)->is_const) )
		{
			//�������� ������ �������������� ����������
			if (no_elems) {
				no_elems = false;
				fprintf(f.f, "\nVAR\n\t");
			} else
				fprintf(f.f, "\t");
			//������ ���� ������� ����������
			(*i)->WriteDFN(f);
			fprintf(f.f, ";\n");
		}

	//������ ���� �������������� ��������
	for(i = BaseNameStore.begin(); i != BaseNameStore.end(); ++i)
		if ((*i)->external && (id_CProcedure == (*i)->name_id || id_CCommonProc == (*i)->name_id))
		{
			fprintf(f.f, "\n");
			(*i)->WriteDFN(f);
			fprintf(f.f, ";");
		}

	//���������� ������� � DFN �����
	f.tab_level--;

}//WriteDFN


//-----------------------------------------------------------------------------
//����� ����� � ������� ����, NULL - ���� ��� �� �������
CBaseName* CDeclSeq::FindName(const char* search_name) const
{
	CBaseNameVector::const_iterator ci;
	for (ci = BaseNameStore.begin(); ci != BaseNameStore.end(); ci++)
		if (!strcmp((*ci)->name, search_name))
			return *ci;
	return NULL;
}//FindName


//-----------------------------------------------------------------------------
//���������� ���������� ��-�� � ������� ����
void CDeclSeq::AddName(CBaseName* BN) const
{
	BaseNameStore.push_back(BN);
}


//-----------------------------------------------------------------------------
//������������� ������� TYPE �� ������ ������
int CDeclSeq::TypeInit(CLexBuf *lb)
{
	//�������� ����� IdentDef
	CIdentDef IdentDef(parent_element, false);
	int err_num = IdentDef.Init(lb);
	//�������� ������� ����� (����� �� ����) � ���������� ������ (����� ����)
	if (err_num) {
		if (s_e_IdentExpected == err_num)
			return s_m_IdentDefAbsent;	//��� �������������� - ��� �� ������
		else
			return err_num;				//���� ����������� ���������� �����. - ��� ������
	}

	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	DECL_SAVE_POS

	//�������� ������� ��. ����� "=" ��� "+="
	if (!lb->ReadLex(li)) return s_e_EqualSignExpected;

	//��. �� ������ (����������� ��� ����������� ���)
	CBaseType *BaseType = NULL;

	//�������� ���� ���������� �������: "=", "+=", "." (����� �������� ����������)
	switch (li.lex) {

	//������� �������� ����
	case lex_k_eq:
		//�������� ������� ���� �� ������ ������
		err_num = TypeSelector(lb, BaseType, parent_element);
		if (err_num) return err_num;
		//��������� ����� ������� � ��������� ������
		IdentDef.Assign(BaseType);
		//�������� ���������� ����� � ������� ���� (DeclSeq)
		if (FindName(BaseType->name)) return s_e_Redefinition;
		//�������� ������� ������� ��� RECORD � �������� RuntimeId, ���� ����
		if (id_CRecordType == BaseType->name_id) {
			err_num = static_cast<CRecordType*>(BaseType)->InitRuntimeId();
			if (err_num) return err_num;
		}
		//��������� ���������� ������� � ������
		AddName(BaseType);
		return 0;

	//������ �������� ���������� ���������������� ���������
	case lex_k_dot:
		//��������� �������� ���������������� ���������
		if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
		//����� ���������������� ��������� � �������� ����
		BaseType = static_cast<CBaseType*>(parent_element->GetGlobalName(IdentDef.name, li.st));
		if (!BaseType) return s_e_UndeclaredIdent;
		if (id_CCommonType != BaseType->name_id) return s_e_CommonTypeExpected;
		//�������� ������� "+="
		if (!lb->ReadLex(li) || lex_k_add_assign != li.lex) return s_e_AddAssignMissing;
		//�������� ���������� �������� ����������� � ������������ ���������
		if (static_cast<CCommonType*>(BaseType)->IsLocal) return s_e_CommonTypeLocalExt;
		//������������� ���������� ���������������� ���������
		return static_cast<CCommonType*>(BaseType)->InitExtension(lb, this);

	//�������� ���������� ���������
	case lex_k_add_assign:
		//�������� ���������� "*" (������� ���������� �� �����������)
		if (IdentDef.external) {
			RESTORE_POS
			return s_e_IdentWrongMarked;
		}
		//�������� ������� ���������� ������������������ ���������
		if (!BaseType) {
			BaseType = static_cast<CBaseType*>(parent_element->GetGlobalName(IdentDef.name));
			if (!BaseType) {
				RESTORE_POS
				return s_e_UndeclaredIdent;
			}
			if (id_CCommonType != BaseType->name_id) {
				RESTORE_POS
				return s_e_CommonTypeExpected;
			}
		}
		//������������� ���������� ���������� ���������
		return static_cast<CCommonType*>(BaseType)->InitExtension(lb, this);

	//������������ ������
	default:
		return s_e_EqualSignExpected;

	}//switch
}//CDeclSeq::TypeInit


//-----------------------------------------------------------------------------
//�������� ������������� ���� ��������, �������� � ������������ ����
int CDeclSeq::CheckCompleteRoot(CLexBuf *lb)
{
	//��� ��������� ������ ������
	int err_num;
	//���� �������� ���� ����������, �������� � ������������ ����
	CBaseNameVector::const_iterator ci;
	for(ci = BaseNameStore.begin(); ci != BaseNameStore.end(); ++ci)
		switch ((*ci)->name_id) {
		//�������� ���������� � ������� ���� ������������ �������� ��� ��������������� ���������
		case id_CForwardDecl:
			if (!static_cast<CForwardDecl*>(*ci)->CheckSatisfied(lb)) return s_e_ForwardDeclUnsat;
			continue;
		//�������� ������������� ���� ��������� �� ����������� ���
		case id_CPointerType:
			err_num = static_cast<CPointerType*>(*ci)->CheckComplete(lb);
			if (err_num) return err_num;
			continue;
		//�������� ������������� ���� ������
		case id_CRecordType:
			err_num = static_cast<CRecordType*>(*ci)->CheckComplete(lb);
			if (err_num) return err_num;
			continue;
		//�������� ������������� ���� ������
		case id_CArrayType:
			err_num = static_cast<CArrayType*>(*ci)->CheckComplete(lb);
			if (err_num) return err_num;
			continue;
		default:
			//�������� ������������� ������� ����������
			if (CBaseVar::IsVarId((*ci)->name_id)) {
				err_num = static_cast<CBaseVar*>(*ci)->CheckComplete(lb);
				if (err_num) return err_num;
			}
		}//switch
	//�������� ���������
	return 0;
}


//-----------------------------------------------------------------------------
//������������� ������� CONST �� ������ ������
int CDeclSeq::ConstInit(CLexBuf *lb)
{
	//�������� ����� IdentDef
	CIdentDef IdentDef(parent_element, false);
	if (IdentDef.Init(lb)) return s_m_IdentDefAbsent;	//��� ��-� (��� �� ������)

	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� ��. ����� =
	if (!lb->ReadLex(li) || lex_k_eq != li.lex) return s_e_EqualSignExpected;

	//�������� ���������
	CBaseVar* ConstVar;
	int err_num = ConstSelector(lb, ConstVar, parent_element);
	if (err_num) return err_num;

	//������������� ���������� ��������� �������� IdentDef
	err_num = IdentDef.AssignVar(ConstVar);
	if (err_num) return err_num;

	//��������� �������� ���������
	ConstVar->is_const = true;

	//��������� ���������� ��������� � ������� ����
	parent_element->AddName(ConstVar);

	return 0;
}//CDeclSeq::ConstInit


//-----------------------------------------------------------------------------
//������������� ������� PROCEDURE ([^]) �� ������ ������
int CDeclSeq::ProcInit(CLexBuf *lb)
{
	DECL_SAVE_POS
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;
	//��� ���������� �������� ������������ �������
	CProcedure *Proc;
	//��� �������� ���������� ����������
	CBaseNameVector::const_iterator ci;

	//�������� ���������� ��. ����� ^
	if (!lb->ReadLex(li) || lex_k_up_arrow != li.lex) {
		//������������� ���������
		RESTORE_POS
		Proc = new CProcedure(parent_element);
	} else	//������������� ������������ �������� ���������
		Proc = new CForwardDecl(parent_element);

	//������������� ��������� � �������� ���������� ��������� � �� ����������
	int err_num = Proc->Init(lb);
	if (err_num && (s_m_CommonProcFound != err_num)) goto fault_exit;

	//�������� ����������� ���������� ���������
	if (s_m_CommonProcFound == err_num) {
		//������� ���������� ��������� ��� ����������
		delete Proc;
		//������� ������������� ���������� ��������������� ���������
		RESTORE_POS
		Proc = new CCommonProc(parent_element);
		err_num = Proc->Init(lb);
		if (err_num && (s_m_HProcFound != err_num)) goto fault_exit;

		//�������� ������� ����������� �����. �������������
		if (s_m_HProcFound == err_num) {
			//������ ���������� �����. ������������� - ���������� CCommonProc
			delete Proc;
			//������������� ����������� �����. �������������
			RESTORE_POS
			Proc = new CHandlerProc(parent_element);
			err_num = Proc->Init(lb);
			if (err_num) goto fault_exit;
		}

		//�������� ������� ������� ����������� � ����������� ������� ���������� ����������� �����������
		AddName(Proc);
		return 0;
	}

	//�������� ������� � ������ ������� ��������� ������ �������� � ����������� ������
	for (ci = BaseNameStore.begin(); ci != BaseNameStore.end(); ci++)
		if (CProcedure::IsProcId((*ci)->name_id)) {
			if (Proc->CompareProcNames(static_cast<CProcedure*>(*ci))) {
				//������� ��� ������� � ������������ ������� (������� ���������),
				//��������� ������� ������������ ������������ �������� (����� - ������)
				if (id_CForwardDecl != (*ci)->name_id) {
					RESTORE_POS
					err_num = s_e_Redefinition;
					goto fault_exit;
				}
				//� ������ �������� ��������� - �������� ������������ ������������ ��������
				if (id_CForwardDecl != Proc->name_id) {
					//�������� ������������ ���������� ��������� �� ������������ ��������
					err_num = static_cast<CForwardDecl*>(*ci)->CompareDeclarations(Proc);
					if (err_num) {
						RESTORE_POS
						goto fault_exit;
					}
				}
			}
		} else {
			//����� ��������, ��������� � �����, �� ����� ������������� � ����������� �������
			if (Proc->Receiver) continue;
			//��������� � ������ �������, �� ���������� ����������
			if (!strcmp(Proc->name, (*ci)->name)) {
				RESTORE_POS
				err_num = s_e_Redefinition;
				goto fault_exit;
			}
		}

	//��������� ���������� ������� � ������
	AddName(Proc);
	return 0;

fault_exit:
	//����������� ���������� ������� � ������� ��������� �� ������
	delete Proc;
	return err_num;

}//CDeclSeq::ProcInit


//-----------------------------------------------------------------------------
//������������� ������� VAR �� ������ ������
int CDeclSeq::VarInit(CLexBuf *lb)
{
	//�������� ������ ���� ���������� (� ���������, �������� �� ������ ������� ��������� ����������)
	CIdentList IdentList(parent_element, (parent_element->name_id != id_CModule) && (parent_element->name_id != id_CDfnModule));
	//�������� ���������� �� ���������� ������ ����
	return IdentList.AssignVars(lb);
}//CDeclSeq::VarInit


//-----------------------------------------------------------------------------
//����������� ������� CDfnModuleSeq
CDfnModuleSeq::CDfnModuleSeq(const CBaseName* parent) : parent_element(parent)
{
}


//-----------------------------------------------------------------------------
//���������� ������� CDfnModuleSeq
CDfnModuleSeq::~CDfnModuleSeq()
{
	DfnModuleList_type::iterator i;
	for(i = DfnModuleList.begin(); i != DfnModuleList.end(); ++i)
		delete (*i);
}//~CDfnModuleSeq


//-----------------------------------------------------------------------------
//������ � project �������� ���� ������ .dfn ������ (����� ����)
void CDfnModuleSeq::EnumDfnModules(CProject &project)
{
	if (DfnModuleList.empty()) return;

	DfnModuleList_type::const_iterator ci;
	for (ci = DfnModuleList.begin(); ci != DfnModuleList.end(); ++ci) {
		//���������� ���������� ����� � ������ ������ (���� ��� ��� ���)
		project.AddMakeFile( (*ci)->full_path, (*ci)->name);
		//������ ������ ��������������� �������
		if ((*ci)->DfnDeclSeq->DfnModuleSeq)
			(*ci)->DfnDeclSeq->DfnModuleSeq->EnumDfnModules(project);
	}
}


//-----------------------------------------------------------------------------
//���������� ��-�� � ������ �������
void CDfnModuleSeq::AddModule(CDfnModule *DM)
{
	DfnModuleList.push_back(DM);
}


//-----------------------------------------------------------------------------
//����� ����� � ������� ����, NULL - ���� ��� �� �������
CDfnModule* CDfnModuleSeq::FindName(const char* search_name) const
{
	DfnModuleList_type::const_iterator i;
	for (i = DfnModuleList.begin(); i != DfnModuleList.end(); ++i)
		if (!strcmp( (*i)->name, search_name ))
			return *i;
	return NULL;
}//FindName


//-----------------------------------------------------------------------------
//������ ���� CDfnModuleSeq
void CDfnModuleSeq::WriteCPP(CPP_files& f)
{
	DfnModuleList_type::const_iterator i;
	for (i = DfnModuleList.begin(); i != DfnModuleList.end(); ++i)
		(*i)->WriteCPP(f);
}


//-----------------------------------------------------------------------------
//���������� ������� MODULE
CModule::~CModule()
{
	delete StatementSeq;
	delete DeclSeq;
}//~CModule


//-----------------------------------------------------------------------------
//������������� ������� MODULE
int CModule::Init(const CProject *project, CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� ��. ����� MODULE
	if (!lb->ReadLex(li) || lex_k_MODULE != li.lex) return s_e_MODULE;

	//�������� ������� �������� ������
	if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	if (!strcmp(li.st, "SYSTEM")) return s_e_MODULE_SYSTEM;
	SetName(li.st);

	//�������� ������� ";"
	if (!lb->ReadLex(li) || lex_k_semicolon != li.lex) return s_e_SemicolonMissing;

	//�������� DeclSeq (��� �������� ������������� �������)
	DeclSeq = new CDeclSeq(this);
	
	//�������� ������ ������������� ������� (���� ����)
	int err_num = DeclSeq->ImportListInit(project, lb);
	if (err_num) return err_num;

	//�������� ������� DeclSeq
	err_num = DeclSeq->Init(lb);
	if (err_num) return err_num;

	//������ ��������� ������� (��������� �����)
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;

	//�������� ������� ����������, ���� ������� BEGIN
	if (lex_k_BEGIN == li.lex) {
		//�������� ������� ������. ����������
		StatementSeq = new CStatementSeq(this);
		err_num = StatementSeq->Init(lb);
		if (err_num) return err_num;
		//������ ��������� ������� (��������� �����)
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;
	}

	//�������� ������� ��. ����� MODULE END
	if (lex_k_END != li.lex) return s_e_END;

	//�������� ������� �������� ������ (� �����)
	if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	if (strcmp(li.st, name)) return s_e_ModuleEndName;

	//�������� ������� "."
	if (!lb->ReadLex(li) || lex_k_dot != li.lex) return s_e_DotMissing;

	return 0;
}//CModule::Init


//-----------------------------------------------------------------------------
//������ ���� CModule
void CModule::WriteCPP(CPP_files& f)
{
	fprintf(f.fc, "//MODULE %s;\n", name);
	fprintf(f.fc, "#include \"%s.h\"\n", name);
	fprintf(f.fc, "using namespace %s;\n", name);

	//��������� ���� ������������� �������
	if (DeclSeq->DfnModuleSeq) DeclSeq->DfnModuleSeq->WriteCPP(f);

	//��������� ���������� � ����� ������������ ����
	fprintf(f.fh, "namespace %s {\n", name);

	//��������� ���� ���������� �����
	DeclSeq->WriteCPP_type(f);
	//��������� ���� ����������
	DeclSeq->WriteCPP_var(f);
	//��������� ���� ���������� ��������
	DeclSeq->WriteCPP_proc(f);

	fprintf(f.fh, "//MODULE INITIALIZATION\nvoid %s%s();\n", O2M_SYS_, name);

	fprintf(f.fc, "\n%s//MODULE INITIALIZATION\n", comment_line_cpp);
	fprintf(f.fc, "void %s::%s%s() {\n", name, O2M_SYS_, name);

	//������������� ������������� �������
	DeclSeq->WriteCPP_mod_init(f);

	//��������� ���� ��� ���������� (���� ����)
	EHaveRet have_return = hr_No;
	if (StatementSeq) {
		have_return = StatementSeq->HaveRet();
		StatementSeq->WriteCPP(f);
	}

	//��� ��� ���������� ��������� RETURN (���� ���� RETURN�)
	if (hr_No != have_return) fprintf(f.fc, "\tO2M_RETURN:;\n");

	fprintf(f.fc, "}\n//END %s.\n", name);
}//WriteCPP


//-----------------------------------------------------------------------------
//����� ����� � ������� ����, NULL - ���� ��� �� �������
CBaseName* CModule::FindName(const char* search_name) const
{
	if (DeclSeq) return DeclSeq->FindName(search_name);
	return NULL;
}


//-----------------------------------------------------------------------------
//���������� ���������� ��-�� � ������� ����
void CModule::AddName(CBaseName* BN) const
{
	DeclSeq->AddName(BN);
}


//-----------------------------------------------------------------------------
//������������� ������� ImportModule
int CImportModule::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//��������� ������� ��-� (����� ������ ��� �������������)
	if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	SetName(li.st);

	DECL_SAVE_POS

	//�������� ������� ��. ����� ":="
	if (!lb->ReadLex(li) || lex_k_assign != li.lex) {
		RESTORE_POS
		real_name = str_new_copy(name);
		return 0;
	}

	//��������� ������� ��-� (��������� ����� ������)
	if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	real_name = str_new_copy(li.st);

	return 0;
}//CImportModule::Init


//-----------------------------------------------------------------------------
//������ ���� CImportModule
void CImportModule::WriteCPP(CPP_files& f)
{
	//�������� ������������ ���� ��� �������������� ������
	if (strcmp(name, real_name)) {
		fprintf(f.fc, "//%s := %s\n", name, real_name);
		fprintf(f.fc, "namespace %s {using namespace %s;}", name, real_name);
	} else
		fprintf(f.fc, "//%s", name);
}//WriteCPP


//-----------------------------------------------------------------------------
//����������� ������� CForwardDecl
CForwardDecl::CForwardDecl(const CBaseName* parent) : CProcedure(parent),
	Satisfied(false)
{
	name_id = id_CForwardDecl;
}


//-----------------------------------------------------------------------------
//�������� ��������� ����� Satisfied � ��������� ������� � ������ ������ �
//������ ������ (���������� ���������)
bool CForwardDecl::CheckSatisfied(CLexBuf *lb) const
{
	//���� ���� �������� - ��� ���������
	if (Satisfied) return true;
	//��������� ������� � ������ ������ �� LexBufPos � ������ ������
	lb->SetCurrPos(FDeclPos);
	//������� �������� ������
	return false;
}


//-----------------------------------------------------------------------------
//��������� ������������ ���������� � ����������� ��������� ��������� �
//��������� ����� Satisfied, ������� - ����� ������ (� ���������� �������
//� ������ ������) ��� 0
int CForwardDecl::CompareDeclarations(CProcedure *P)
{
	//�������� ������������ �������� ���������� � ���������� (���� ����)
	if (Receiver && strcmp(Receiver->name, P->Receiver->name)) return s_e_ParamNotMatch;
	//�������� ������������ ��������� ����������
	if (FormalPars->FPStore.size() != P->FormalPars->FPStore.size()) return s_e_ParamCountNotMatch;

	/**/
	//��������

	Satisfied = true;
	return 0;
}


//-----------------------------------------------------------------------------
//������������� ������� CForwardDecl
int CForwardDecl::Init(CLexBuf *lb)
{
	DECL_SAVE_POS
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;
	//���������� ��� ��������� ������ ������
	int err_num;

	//�������� ������� ���������
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) {
		RESTORE_POS
	} else {
		Receiver = new CReceiver(this);
		err_num = Receiver->Init(lb, this);
		if (err_num) return err_num;
	}

	//�������� ������� ����� � ��������� ��� � ������� ������
	CIdentDef IdentDef(parent_element, false);
	if (err_num = IdentDef.Init(lb)) return err_num;
	IdentDef.Assign(this);

	//���������� ������� � ������ ������ �� ������ ������������� ������
	FDeclPos = lb->GetCurrPos();

	//�������� ������ ���������� ����������
	FormalPars = new CFormalPars;
	err_num = FormalPars->Init(lb, this);
	if (err_num) return err_num;

	//� ������ ������� ��������� ���������� ������� ������ �� ��������� � ������� ����
	//����, � ������� ��� �������
	if (Receiver) {
		//����� ������������ ���� ������ (������ ���� - ��������� � ���������)
		CBaseName* BN = parent_element->GetGlobalName(Receiver->type_name);
		if (id_CPointerType == BN->name_id) BN = static_cast<CPointerType*>(BN)->FindType();
		//��������� � ������ � ��������� ���������� ����������� ����� � �������
		err_num = static_cast<CRecordType*>(BN)->AddProcReference(this);
		if (err_num) return err_num;
	}

	return 0;
}//CForwardDecl::Init


//-----------------------------------------------------------------------------
//������ ���� CForwardDecl
void CForwardDecl::WriteCPP(CPP_files& f)
{
	//������ ������������
	fprintf(f.fc, "//PROCEDURE^ %s%s%s\n", Receiver ? Receiver->type_name : "", Receiver ? "." : "", name);

	//��������� �������������� namespace ��� ��������� �� ��������� � ����� ���������
	bool req_unnm_nmsp = !external && !Receiver;
	if (req_unnm_nmsp) fprintf(f.fc, "namespace {\n");

	//��������� ���� ���� ��������� � ��������
	FormalPars->WriteCPP_type(f, false, parent_element);

	//��������� ���������� ��������� � ������������ ����� � ������ �� ��������
	//(��� ��������, ��������� � �����, ��� �� ���������)
	if (external && !Receiver) {
		fprintf(f.fh, "//PROCEDURE %s%s%s\n", Receiver ? Receiver->type_name : "", Receiver ? "." : "", name);
		//��������� ���� ���� �������������� ��������� � ����������
		FormalPars->WriteCPP_type(f, true, parent_element);
		//��������� �������� ��������� � ����������
		fprintf(f.fh, "%s(", name);
		//��������� namespace ��� �������-� �� ��������� � ����� ��������� � ��������
		fprintf(f.fc, "::%s::", parent_element->name);
	}

	//��������� ����� ���� ��� ���������, ��������� � �����
	if (Receiver) fprintf(f.fc, "%s::", Receiver->type_name);

	//��������� �������� ��������� � ��������
	fprintf(f.fc, "%s(", name);

	//��������� ���� ���������� ���������� � ���� � � ��������� (���� ����)
	FormalPars->WriteCPP_pars(f, false);
	fprintf(f.fc, ");\n");
	if (external && !Receiver) {
		FormalPars->WriteCPP_pars(f, true);
		fprintf(f.fh, ");\n");
	}

	//� ������ ��������� ��������� � ������������� namespace ��������� ������ '}'
	if (req_unnm_nmsp) fprintf(f.fc, "}\n");

}//WriteCPP


//-----------------------------------------------------------------------------
//���������� ������� PROCEDURE
CProcedure::~CProcedure()
{
	delete Receiver;
	delete FormalPars;
	delete DeclSeq;
	delete StatementSeq;
}//~CProcedure


//-----------------------------------------------------------------------------
//������������� ������� PROCEDURE
int CProcedure::Init(CLexBuf *lb)
{
	DECL_SAVE_POS
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;
	//���������� ��� ��������� ������ ������
	int err_num;

	//�������� ������� ���������
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) {
		RESTORE_POS
	} else {
		Receiver = new CReceiver(this);
		err_num = Receiver->Init(lb, this);
		if (err_num) return err_num;
	}

	//�������� ������� ����� � ��������� ��� � ������� ������
	CIdentDef IdentDef(parent_element, false);
	if (err_num = IdentDef.Init(lb)) return err_num;
	IdentDef.Assign(this);

	//�������� ������� ������ ���������� ���������� (����� ��� ���������� �������� ���
	//����������), ������ ������� ��������� � ���������� ��������� ����������� � CCommonProc
	SAVE_POS
	if (!lb->ReadLex(li) || lex_k_op_brace == li.lex || lex_k_dot == li.lex)
		return s_m_CommonProcFound;
	else {
		RESTORE_POS
	}

	//�������� ������ ���������� ����������
	FormalPars = new CFormalPars;
	err_num = FormalPars->Init(lb, this);
	if (err_num) return err_num;

	//�������� ������� ";"
	if (!lb->ReadLex(li) || lex_k_semicolon != li.lex) return s_e_SemicolonMissing;

	//� ������ ������� ��������� ���������� ������� ������ �� ��������� � ������� ����
	//����, � ������� ��� �������
	if (Receiver) {
		//����� ������������ ���� ������ (������ ���� - ��������� � ���������)
		CBaseName* BN = parent_element->GetGlobalName(Receiver->type_name);
		if (id_CPointerType == BN->name_id) BN = static_cast<CPointerType*>(BN)->FindType();
		//��������� � ������ � ��������� ���������� ����������� ����� � �������
		err_num = static_cast<CRecordType*>(BN)->AddProcReference(this);
		if (err_num) {
			RESTORE_POS
			return err_num;
		}
	}

	//�������� ������� DeclSeq
	DeclSeq = new CDeclSeq(this);
	err_num = DeclSeq->Init(lb);
	if (err_num) return err_num;

	//������ ��������� ������� (��������� �����)
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;

	//�������� ������� ����������, ���� ������� BEGIN
	if (lex_k_BEGIN == li.lex) {
		//�������� ������� ������. ����������
		StatementSeq = new CStatementSeq(this);
		err_num = StatementSeq->Init(lb);
		if (err_num) return err_num;
		//������ ��������� ������� (��������� �����)
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;
	}

	//�������� ������� RETURN � ������-�� ����������
	have_return = StatementSeq ? StatementSeq->HaveRet() : hr_No;
	//�������� ����������� RETURN � ������ ���������-�������
	if (FormalPars->Qualident) {
		if (hr_No == have_return)
			return s_e_FuncWithoutRETURN;
		else
			if (hr_NotAll == have_return) WriteWarn(s_w_NotAllPathsRETURN, lb);
	}

	//�������� ������� ��. ����� END
	if (lex_k_END != li.lex) return s_e_END;

	//�������� ������� �������� ��������� (� �����)
	if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	if (strcmp(li.st, name)) return s_e_ProcedureEndName;

	return 0;
}//CProcedure::Init


//-----------------------------------------------------------------------------
//������ ���� CProcedure
void CProcedure::WriteCPP(CPP_files& f)
{
	//������ ������������
	fprintf(f.fc, "//PROCEDURE %s%s%s\n", Receiver ? Receiver->type_name : "", Receiver ? "." : "", name);

	//��������� �������������� namespace ��� ��������� �� ��������� � ����� ���������
	bool req_unnm_nmsp = !external && !Receiver;
	if (req_unnm_nmsp) fprintf(f.fc, "namespace {\n");

	//��������� ���� ���� ��������� � ��������
	FormalPars->WriteCPP_type(f, false, parent_element);

	//��������� ���������� ��������� � ������������ ����� � ������ �� ��������
	//(��� ��������, ��������� � �����, ��� �� ���������)
	if (external && !Receiver) {
		fprintf(f.fh, "//PROCEDURE %s\n", name);
		//��������� ���� ���� �������������� ��������� � ����������
		FormalPars->WriteCPP_type(f, true, parent_element);
		//��������� �������� ��������� � ����������
		fprintf(f.fh, "%s(", name);
		//��������� namespace ��� �������-� �� ��������� � ����� ��������� � ��������
		fprintf(f.fc, "%s::", parent_element->name);
	}

	//��������� ����� ���� ��� ���������, ��������� � �����
	if (Receiver) fprintf(f.fc, "%s::", Receiver->type_name);

	//��������� �������� ��������� � ��������
	fprintf(f.fc, "%s(", name);

	//��������� ���� ���������� ���������� � �������� � � ���������� (���� ����)
	FormalPars->WriteCPP_pars(f, false);
	fprintf(f.fc, ") {\n");
	if (external && !Receiver) {
		FormalPars->WriteCPP_pars(f, true);
		fprintf(f.fh, ");\n");
	}

	//��������� ���� ��������� (���� ����)
	if (Receiver) Receiver->WriteCPP(f);

	//��������� ���������� ��� �������� ����������� ���������-�������
	if (FormalPars->Qualident) {
		fprintf(f.fc, "\t");
		FormalPars->Qualident->WriteCPP_type(f, false, parent_element);
		fprintf(f.fc, " O2M_RESULT;\n");
	}
	//������������� ��������-��������
	FormalPars->WriteCPP_begin(f);

	//��������� ���� ���������� �����
	DeclSeq->WriteCPP_type(f);
	//��������� ���� ����������
	DeclSeq->WriteCPP_var(f);

	//��������� ���� ��� ����������
	if (StatementSeq) {
		fprintf(f.fc, "//BEGIN\n");
		StatementSeq->WriteCPP(f);
	}
	//��������������� �������� ��������-��������
	FormalPars->WriteCPP_end(f, hr_No != have_return);
	//������ ����������� ���������-�������
	if (FormalPars->Qualident) fprintf(f.fc, "\treturn O2M_RESULT;\n");
	//� ������ ��������� ��������� � ������������� namespace ��������� ���. ������ '}'
	if (req_unnm_nmsp) fprintf(f.fc, "}");
	//���������� �������� ���������� ���������
	fprintf(f.fc, "}\n//END %s;\n", name);
}//WriteCPP


//-----------------------------------------------------------------------------
//������ �������� ��������� (��������� � �����) � ���������� ���� ������,
//RT - ���, � ������� ������� ������ ���������
void CProcedure::WriteCPP_RECORD(CPP_files &f, const CRecordType* RT, const bool to_h)
{
	//��� ��������� ������ ������ � ������������ ���� ��� ���
	TFileType* ff = to_h ? f.fh : f.fc;
	//��������� �������� �������������
	fprintf(ff, "virtual ");
	//��������� ���� ���� ���������
	FormalPars->WriteCPP_type(f, to_h, parent_element);
	//��������� �������� ���������
	fprintf(ff, "%s(", name);
	//��������� ���� ���������� ����������
	FormalPars->WriteCPP_pars(f, to_h);
	fprintf(ff, ")");
}


//-----------------------------------------------------------------------------
//������ � .dfn ����
void CProcedure::WriteDFN(DFN_file& f)
{
	fprintf(f.f, "PROCEDURE ");
	//������ ���� ��� ��������� (���� ����)
	if (Receiver) {
		fprintf(f.f, "(");
		if (Receiver->is_var)
			fprintf(f.f, "VAR ");
		fprintf(f.f, "%s : %s) ", Receiver->name, Receiver->type_name);
	}
	//������ �������� ���������
	fprintf(f.f, "%s (\n", name);
	//������ ���������� ���������� ����������
	FormalPars->WriteDFN(f);
	fprintf(f.f, "\n)");
	//������ ���� ��������� (���� ����)
	FormalPars->WriteDFN_type(f);
}//WriteDFN


//-----------------------------------------------------------------------------
//����� ����� ���� ���.��� � ������� ����
CBaseName* CProcedure::FindImportedName(const char *module_name, const char *search_name) const
{
	//�������� ����� ������������ ������ ����� ��������� � ������ ���������, ��������� � �����
	if (Receiver && !strcmp(Receiver->name, module_name) && !strcmp(name, search_name))
		return const_cast<CProcedure*>(this);
	else
		return parent_element->GetGlobalName(module_name, search_name);
}


//-----------------------------------------------------------------------------
//����� ����� � ������� ����, NULL - ���� ��� �� �������
CBaseName* CProcedure::FindName(const char* search_name) const
{
	//�������� ����� ������������ ������ ����� ���������
	if (name && !strcmp(name, search_name)) return const_cast<CProcedure*>(this);
	//�������� ��������� ���������
	CBaseName* BN = NULL;
	//����� ����� � �����������
	if (DeclSeq) BN = DeclSeq->FindName(search_name);
	//����� ����� � ������ ���������� ����������
	if (!BN && FormalPars) BN = FormalPars->FindName(search_name);
	//����� ����� � ���������
	if (!BN && Receiver) BN = Receiver->FindName(search_name);
	//������� ���������� ����� (��� NULL)
	return BN;
}//FindName


//-----------------------------------------------------------------------------
//���������� ���������� ��-�� � ������� ����
void CProcedure::AddName(CBaseName* BN) const
{
	if (DeclSeq) DeclSeq->AddName(BN);
}//AddName


//-----------------------------------------------------------------------------
//��������� ����� ��������� � ������ ��������� ��������� � ������ ���������,
//������� - ������� ���������� ����
bool CProcedure::CompareProcNames(const CProcedure *P) const
{
	if (strcmp(name, P->name) || (Receiver && !P->Receiver) || (!Receiver && P->Receiver)) return false;
	if (Receiver && strcmp(Receiver->type_name, P->Receiver->type_name)) return false;
	return true;
}


//-----------------------------------------------------------------------------
//�������� �������������� ���������� ��. � ��-� ���������
bool CProcedure::IsProcId(const EName_id id)
{
	switch (id) {
	case id_CProcedure:
	case id_CDfnProcedure:
	case id_CDfnCommonProc:
	case id_CForwardDecl:
	case id_CCommonProc:
	case id_CHandlerProc:
		return true;
	default:
		return false;
	}//switch
}


//-----------------------------------------------------------------------------
//��������� ���� ��������� (��� ��������� - �������)
EName_id CProcedure::GetResultId() const
{
	if (!FormalPars->Qualident) return id_CBaseName;
	return FormalPars->Qualident->TypeResultId;
}


//-----------------------------------------------------------------------------
//����������� ����������� ��������������� �������������
CCommonProc::CCommonProc(const CBaseName* parent) : CProcedure(parent),
	CommonPars(NULL), DefH(false)
{
	name_id = id_CCommonProc;
}//CCommonProc


//-----------------------------------------------------------------------------
//���������� ������� CCommonProc
CCommonProc::~CCommonProc()
{
	delete CommonPars;
}


//-----------------------------------------------------------------------------
//�������� ���������� ���� ���������� � ���������� ����������, ����������� ������� � ���������� ����������
int CCommonProc::CheckParams() {
	CBaseVarVector::iterator i;
	for (i = CommonPars->FPStore.begin(); i != CommonPars->FPStore.end(); ++i) {
		//�������� ���� ��������� (?)
		if ((*i)->name_id != id_CRecordVar) continue;

		//�������� ���������� ����� ����������� ��������� � ����������
		if (FormalPars->FindName((*i)->name)) return s_e_CommonProcParamRedefined;

		//�������� ����� ���������� ����������
		CCommonVar* CV = new CCommonVar(parent_element);
		//����������� �������� ���� � ��������� ����������
		CV->SetName((*i)->name);
		CV->SetTypeName((*i)->GetTypeModuleAlias(), (*i)->GetTypeName());
		//������ ���������� � ������ ���������� ����������
		delete *i;
		(*i) = CV;
	}
	return 0;
}


//-----------------------------------------------------------------------------
//������������� ������� CCommonProc
int CCommonProc::Init(CLexBuf *lb)
{
	//���������� ��� ��������� ������ ������
	int err_num;

	//�������� ����� IdentDef
	CIdentDef IdentDef(parent_element, false);
	if (err_num = IdentDef.Init(lb)) return err_num;

	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//��������� ���������� �������
	if (!lb->ReadLex(li)) return s_e_CommonProcCommonParam;

	//�������� ������� "." (��������� ������ � ������������ �����. ����-���)
	if (lex_k_dot == li.lex) return s_m_HProcFound;

	//�������� ������� ���������� ����������
	if (lex_k_op_brace != li.lex)
		return s_e_CommonProcCommonParam;
	else {
		CommonPars = new CCommonPars;
		err_num = CommonPars->Init(lb, this);
		if (err_num) return err_num;
	}

	//�������� ������ ���������� ����������
	FormalPars = new CFormalPars;
	err_num = FormalPars->Init(lb, this);
	if (err_num) return err_num;

	//������� �������� ���� ������������� ���������� �� ���������� ���������� � ���������� (���� ����)
	if (CommonPars->Qualident) {
		//�������� �������� ���������� ���� ������������� �������� (� ������ ��������� - �������)
		if (FormalPars->Qualident) return s_e_CommonProcDoubleType;
		FormalPars->Qualident = CommonPars->Qualident;
		CommonPars->Qualident = NULL;
	}

	//�������� ���������� ���� ���������� � ���������� ����������, ����������� ������� � ���������� ����������
	err_num = CheckParams();
	if (err_num) return err_num;

	//��������� ���������� ����� � ������� ������
	IdentDef.Assign(this);

	//�������� ���������� ���� ��������� (����������� �� ���������)
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_SemicolonMissing;
	if (lex_k_assign == li.lex) {
		if (!lb->ReadLex(li) || lex_d != li.lex || strcmp(li.st, "0")) return s_e_ZeroMissing;
		return 0;
	}

	//���������� �� ��������� ������������
	DefH = true;

	//�������� ������� ";"
	if (lex_k_semicolon != li.lex) return s_e_SemicolonMissing;

	//�������� ������� DeclSeq
	DeclSeq = new CDeclSeq(this);
	err_num = DeclSeq->Init(lb);
	if (err_num) return err_num;

	//������ ��������� ������� (��������� �����)
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;

	//�������� ������� ����������, ���� ������� BEGIN
	if (lex_k_BEGIN == li.lex) {
		//�������� ������� ������. ����������
		StatementSeq = new CStatementSeq(this);
		err_num = StatementSeq->Init(lb);
		if (err_num) return err_num;
		//������ ��������� ������� (��������� �����)
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;
	}

	//�������� ������� RETURN � ������-�� ����������
	have_return = StatementSeq ? StatementSeq->HaveRet() : hr_No;
	//�������� ����������� RETURN � ������ ���������-�������
	if (FormalPars->Qualident) {
		if (hr_No == have_return)
			return s_e_FuncWithoutRETURN;
		else
			if (hr_NotAll == have_return) WriteWarn(s_w_NotAllPathsRETURN, lb);
	}

	//�������� ������� ��. ����� END
	if (lex_k_END != li.lex) return s_e_END;

	//�������� ������� �������� ��������� (� �����)
	if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	if (strcmp(li.st, name)) return s_e_ProcedureEndName;

	return 0;
}//CCommonProc::Init


//-----------------------------------------------------------------------------
//������ ���� ������ ��������� ��� ������ ����������� ��������������� �������������
void CCommonProc::WriteCPP_HPar(CPP_files &f, CBaseVar *par)
{
	//�������� ������� �������
	if (id_CArrayVar == par->name_id) {
		//������ ���������� ������� (� ������ �������� - �.�. ������ �� ���������������)
		fprintf(f.fc, "O2M_ARR_%s, ", par->name);

		//��������, �������� �� ������ ��������, ����� ��������� ������ ���. ���������� (������������)
		CArrayType* AT = static_cast<CArrayVar*>(par)->ArrayType;

		//������ ������ ������������ (���� ���� ? -> ArrOfChar + "String")
		if (AT) {
			int dimension = 0;	//������ ������������ � 0
			while (id_CArrayType == AT->name_id) {
				//����� ����������� (� ���� ����� ��� ����� ����������, ���������� �����������)
				if (AT->size)
					fprintf(f.fc, "%i, ", AT->size);
				else
					fprintf(f.fc, "O2M_ARR_%i_%s, ", dimension, par->name);
				//������� � ��������� �����������
				++dimension;
				AT = static_cast<CArrayType*>(AT->Type);
			}//while
		}

	} else	//��������� ���� �������� ��������� (�� �������)
		fprintf(f.fc, "%s, ", par->name);
}


//-----------------------------------------------------------------------------
//������ ���� CCommonProc
void CCommonProc::WriteCPP(CPP_files& f)
{
	///////////////////////////////////////////////////////
	//������ ���������� �� ���������� ��������� � .2ml ����

	//������ ������ �������� ���������� ��������� � .2ml ����
	fprintf(f.f2ml, "\t<CProc Name=\"%s\"%s>\n", name, DefH ? " DefH=\"yes\"" : "");
	//������ ������ ���������� ���������� � .2ml ����
	CBaseVarVector::const_iterator ci;
	for (ci = CommonPars->FPStore.begin(); ci != CommonPars->FPStore.end(); ci++) {
		CCommonType* CT = static_cast<CCommonType*>(parent_element->GetGlobalName((*ci)->GetTypeModuleName(), (*ci)->GetTypeName()));
		if (!CT) continue;
		const char *QN = CT->GetModuleName();
		fprintf(f.f2ml, "\t\t<CPar");
		if (QN) fprintf(f.f2ml, " Qual=\"%s\"", QN);
		fprintf(f.f2ml, " Name=\"%s\" />\n", CT->name);
	}
	//����� �������� ���������� ��������� � .2ml ����
	fprintf(f.f2ml, "\t</CProc>\n");


	//////////////////////////////////
	//������ ���� ���������� ���������

	//������ ������������
	fprintf(f.fc, "//COMMON PROCEDURE %s\n", name);

	//��������� �������������� namespace ��� ��������� �� ��������� � ����� ���������
	bool req_unnm_nmsp = !external && !Receiver;
	if (req_unnm_nmsp) fprintf(f.fc, "namespace {\n");

	//��������� ���� ���� ��������� � ��������
	FormalPars->WriteCPP_type(f, false, parent_element);

	//��������� ���������� ��������� � ������������ ����� � ������ �� ��������
	if (external) {
		fprintf(f.fh, "//COMMON PROCEDURE %s\n", name);
		//��������� ���� ���� �������������� ��������� � ����������
		FormalPars->WriteCPP_type(f, true, parent_element);
		//��������� �������� ��������� � ����������
		fprintf(f.fh, "%s(", name);
		//��������� namespace ��� �������-� �� ��������� � ����� ��������� � ��������
		fprintf(f.fc, "%s::", parent_element->name);
	}

	//��������� �������� ��������� � ��������
	fprintf(f.fc, "%s(", name);

	//��������� ���� ���������� ���������� � �������� (���� ����)
	FormalPars->WriteCPP_pars(f, false);
	if (!FormalPars->FPStore.empty()) fprintf(f.fc, ", ");
	//��������� ���� ���������� ���������� � ���������� (���� ����)
	if (external) {
		FormalPars->WriteCPP_pars(f, true);
		if (!FormalPars->FPStore.empty()) fprintf(f.fh, ", ");
	}

	//��������� ���� ���������� ���������� � ��������
	CommonPars->WriteCPP_pars(f, false);
	fprintf(f.fc, ") {\n");
	//��������� ���� ���������� ���������� � ����������
	if (external) {
		CommonPars->WriteCPP_pars(f, true);
		fprintf(f.fh, ");\n");
	}


	/////////////////////////////////////////////////////////////////
	//��������� ���� ������ ����������� ��������������� �������������

	//��������� ���������� ������������ ���� � ���������� ��� ������ �����������
	fprintf(f.fc, "\t//handler selection\n\ttypedef ");
	FormalPars->WriteCPP_type(f, false, parent_element);
	fprintf(f.fc, "(*O2M_TYPE) (");
	if (!FormalPars->FPStore.empty()) {
		FormalPars->WriteCPP_pars(f, false);
		fprintf(f.fc, ", ");
	}
	CommonPars->WriteCPP_pars(f, false);
	//��������� ��������� ������ ����������� �����. ����-���
	fprintf(f.fc, ");\n\tO2M_TYPE O2M_PROC;\n\tO2M_PROC = (O2M_TYPE)O2M_COMMON_%s_%s(", parent_element->name, name);
	for (ci = CommonPars->FPStore.begin();;) {
		fprintf(f.fc, "%s", (*ci)->name);
		++ci;
		if (CommonPars->FPStore.end() == ci) break;
		fprintf(f.fc, ", ");
	}
	//��������� �������� ������� ������ � ������ ����������� ��� ������� ������
	fprintf(f.fc, ");\n\tif (O2M_PROC) %sO2M_PROC(", FormalPars->Qualident ? "return " : "{\n\t\t");
	//��������� ����������� ���������� ��� ������ ����������� ��������������� �������������
	//��������� ������� ����������� ����������
	for (ci = FormalPars->FPStore.begin(); ci != FormalPars->FPStore.end(); ci++)
		WriteCPP_HPar(f, *ci);
	//��������� ���������� ����������� ����������
	for (ci = CommonPars->FPStore.begin();;) {
		fprintf(f.fc, "%s", (*ci)->name);
		++ci;
		if (CommonPars->FPStore.end() == ci) break;
		fprintf(f.fc, ", ");
	}
	fprintf(f.fc, ");\n");
	if (!FormalPars->Qualident) fprintf(f.fc, "\t\treturn;\n\t}\n");


	/////////////////////////////////////////////////////
	//��������� ���� ����������� �� ��������� (���� ����)

	//�������� ������� ����������� �� ���������
	if (DefH) {

		//��������� ���������� ��� �������� ����������� ���������-�������
		if (FormalPars->Qualident) {
			fprintf(f.fc, "\t");
			FormalPars->Qualident->WriteCPP_type(f, false, parent_element);
			fprintf(f.fc, " O2M_RESULT;\n");
		}
		//������������� ��������-��������
		FormalPars->WriteCPP_begin(f);

		//��������� ���� ����� � ����������
		if (DeclSeq) {
			DeclSeq->WriteCPP_type(f);
			DeclSeq->WriteCPP_var(f);
		}

		//��������� ���� ��� ����������
		if (StatementSeq) {
			fprintf(f.fc, "//BEGIN\n");
			StatementSeq->WriteCPP(f);
		}
		//��������������� �������� ��������-��������
		FormalPars->WriteCPP_end(f, hr_No != have_return);
		//������ ����������� ���������-�������
		if (FormalPars->Qualident) fprintf(f.fc, "\treturn O2M_RESULT;\n");

	} else
		fprintf(f.fc, "\t//default handler absent\n\texit(0);\n");	//��������� ���� ���������� ��������� ��� ���������� ����������� �� ���������

	//� ������ ��������� ��������� � ������������� namespace ��������� ���. ������ '}'
	if (req_unnm_nmsp) fprintf(f.fc, "}");
	//���������� �������� ���������� ���������
	fprintf(f.fc, "}\n//END %s;\n", name);
}//WriteCPP


//-----------------------------------------------------------------------------
//������ � .dfn ����
void CCommonProc::WriteDFN(DFN_file& f)
{
	fprintf(f.f, "PROCEDURE ");
	//������ �������� ���������
	fprintf(f.f, "%s {\n", name);
	//������ ���������� ���������� ����������
	CommonPars->WriteDFN(f);
	fprintf(f.f, "\n}");
	//������ ���������� ���������� ����������
	if (!FormalPars->FPStore.empty()) {
		fprintf(f.f, "(\n");
		FormalPars->WriteDFN(f);
		fprintf(f.f, "\n)");
	}
	//������ ���� ��������� (���� ����)
	FormalPars->WriteDFN_type(f);
}//WriteDFN


//-----------------------------------------------------------------------------
//����������� ����������� ��������������� �������������
CHandlerProc::CHandlerProc(const CBaseName* parent) : CProcedure(parent),
	CommonPars(NULL), QualName(NULL), UID(GetGlobalUId())
{
	name_id = id_CHandlerProc;
}//CHandlerProc


//-----------------------------------------------------------------------------
//���������� ������� CHandlerProc
CHandlerProc::~CHandlerProc()
{
	delete CommonPars;
	delete[] QualName;
}


//-----------------------------------------------------------------------------
//�������������� ���������� ���������� � ����������
int CHandlerProc::ConvertParams() {
	CBaseVarVector::const_iterator ci;
	for (ci = CommonPars->FPStore.begin(); ci != CommonPars->FPStore.end(); ++ci) {
		//�������� ���� ��������� (?)
		if ((*ci)->name_id != id_CCommonVar) continue;
		//�������� ���������� ����� ����������� ��������� � ����������
		if (FormalPars->FindName((*ci)->name)) return s_e_HandlerProcParamRedefined;
		//�������� ����� ����������� ���������
		CBaseVar* BV = static_cast<CCommonVar*>(*ci)->CreateVar(parent_element);
		if (!BV) return s_e_CommonTypeExpected;
		//���������� ���������� � ������ ���������� ����������
		FormalPars->FPStore.push_back(BV);
	}

	return 0;
}


//-----------------------------------------------------------------------------
//������������� ����������� ��������������� �������������
int CHandlerProc::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;
	//���������� ��� ��������� ������ ������
	int err_num;

	//��������� �������������� (��� ������ ��� ��� �����������)
	if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	DECL_SAVE_POS
	//���� ������� ��������� �����. � ���
	name = str_new_copy(li.st);
	//�������� ���������� ������� (���� "." - ��������� ����� ��� ������ ������)
	if (!lb->ReadLex(li)) return s_e_HandlerProcCommonParam;
	if (lex_k_dot == li.lex) {
		//��������� ����� ���������������� ������ �� ����������
		CBaseName* BN = parent_element->GetGlobalName(name);
		if (!BN) {
			RESTORE_POS
			return s_e_UndeclaredIdent;
		}
		if (id_CImportModule != BN->name_id) {
			RESTORE_POS
			return s_e_IdentNotModule;
		}
		//��������� ����� �����������
		if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
		//���������� ����� ������ � ����� �����������
		QualName = name;
		name = str_new_copy(li.st);
		//��������� ���������� �������
		if (!lb->ReadLex(li)) return s_e_HandlerProcCommonParam;
	}

	//����� ���������� ���������, � ������� �������� ������ ����������
	CBaseName* BN = parent_element->GetGlobalName(QualName, name);
	if (!BN || (id_CCommonProc != BN->name_id && id_CDfnCommonProc != BN->name_id)) {
		RESTORE_POS
		return s_e_FreeHProc;
	}

	//�������� ���������� ����� �������� (����������� ��� ������������)
	if (lex_k_asterix == li.lex || lex_k_minus == li.lex) return s_e_IdentWrongMarked;

	//�������� ������� ���������� ����������
	if (lex_k_op_brace != li.lex)
		return s_e_HandlerProcCommonParam;
	else {
		CommonPars = new CCommonPars;
		err_num = CommonPars->InitSpec(lb, this);
		if (err_num) return err_num;
	}

	//�������� ������ ���������� ����������
	FormalPars = new CFormalPars;
	err_num = FormalPars->Init(lb, this);
	if (err_num) return err_num;

	//������� �������� ���� ������������� ���������� �� ���������� ���������� � ���������� (���� ����)
	if (CommonPars->Qualident) {
		//�������� �������� ���������� ���� ������������� �������� (� ������ ��������� - �������)
		if (FormalPars->Qualident) return s_e_CommonProcDoubleType;
		FormalPars->Qualident = CommonPars->Qualident;
		CommonPars->Qualident = NULL;
	}

	//�������������� ���������� ���������� � ����������
	err_num = ConvertParams();
	if (err_num) return err_num;

	//�������� ������� ";"
	if (!lb->ReadLex(li) || lex_k_semicolon != li.lex) return s_e_SemicolonMissing;

	//�������� ������� DeclSeq
	DeclSeq = new CDeclSeq(this);
	err_num = DeclSeq->Init(lb);
	if (err_num) return err_num;

	//������ ��������� ������� (��������� �����)
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;

	//�������� ������� ����������, ���� ������� BEGIN
	if (lex_k_BEGIN == li.lex) {
		//�������� ������� ������. ����������
		StatementSeq = new CStatementSeq(this);
		err_num = StatementSeq->Init(lb);
		if (err_num) return err_num;
		//������ ��������� ������� (��������� �����)
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;
	}

	//�������� ������� RETURN � ������-�� ����������
	have_return = StatementSeq ? StatementSeq->HaveRet() : hr_No;
	//�������� ����������� RETURN � ������ ���������-�������
	if (FormalPars->Qualident) {
		if (hr_No == have_return)
			return s_e_FuncWithoutRETURN;
		else
			if (hr_NotAll == have_return) WriteWarn(s_w_NotAllPathsRETURN, lb);
	}

	//�������� ������� ��. ����� END
	if (lex_k_END != li.lex) return s_e_END;

	//�������� ������� �������� ��������� (� �����)
	if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	if (strcmp(li.st, name)) return s_e_ProcedureEndName;

	return 0;
}


//-----------------------------------------------------------------------------
//������ ���� ����������� ��������������� �������������
void CHandlerProc::WriteCPP(CPP_files& f)
{
	//////////////////////////////////////////////////////////////
	//������ ���������� �� ����������� �����. ����-��� � .2ml ����

	//������ ������ �������� ����������� �����. ����-��� � .2ml ����
	fprintf(f.f2ml, "\t<HProc ");
	if (QualName) {
		CBaseName* BN = parent_element->GetGlobalName(QualName);
		fprintf(f.f2ml, "Qual=\"%s\" ", static_cast<CImportModule*>(BN)->real_name);
	}
	fprintf(f.f2ml, "Name=\"%s\" UID=\"%i\">\n", name, UID);
	//������ ������ ����������������� ���������� ���������� � .2ml ����
	CBaseVarVector::const_iterator ci;
	for (ci = CommonPars->FPStore.begin(); ci != CommonPars->FPStore.end(); ci++) {
		const char* QualSpecName = NULL;
		const char* SpecName = NULL;
		const char* TagName = NULL;
		static_cast<CCommonVar*>(*ci)->GetTagName(QualSpecName, SpecName, TagName);
		fprintf(f.f2ml, "\t\t<SPar");
		if (TagName)
			fprintf(f.f2ml, " Tag=\"%s\"", TagName);
		else {
			if (QualSpecName) fprintf(f.f2ml, " Qual=\"%s\"", QualSpecName);
			fprintf(f.f2ml, " Name=\"%s\"", SpecName);
		}
		fprintf(f.f2ml, " />\n");
	}
	//����� �������� ����������� � .2ml ����
	fprintf(f.f2ml, "\t</HProc>\n");


	///////////////////////////////////////////////////////
	//������ ���� ����������� ��������������� �������������

	//������ ������������
	fprintf(f.fc, "//HANDLER PROCEDURE %s\n", name);
	fprintf(f.fh, "//HANDLER PROCEDURE %s\n", name);

	//��������� ���� ���� ��������� � �������� � ����������
	FormalPars->WriteCPP_type(f, false, parent_element);
	FormalPars->WriteCPP_type(f, true, parent_element);

	//��������� namespace � �������� ���������
	fprintf(f.fc, "%s::", parent_element->name);

	//��������� �������� ��������� � �������� � ����������
	fprintf(f.fc, "O2M_HANDLER_%s_%i(", name, UID);
	fprintf(f.fh, "O2M_HANDLER_%s_%i(", name, UID);

	//��������� ���� ���������� ���������� � �������� � ����������
	FormalPars->WriteCPP_pars(f, false);
	fprintf(f.fc, ") {\n");
	FormalPars->WriteCPP_pars(f, true);
	fprintf(f.fh, ");\n");

	//��������� ���������� ��� �������� ����������� ���������-�������
	if (FormalPars->Qualident) {
		fprintf(f.fc, "\t");
		FormalPars->Qualident->WriteCPP_type(f, false, parent_element);
		fprintf(f.fc, " O2M_RESULT;\n");
	}
	//������������� ��������-��������
	FormalPars->WriteCPP_begin(f);

	//��������� ���� ���������� �����
	DeclSeq->WriteCPP_type(f);
	//��������� ���� ����������
	DeclSeq->WriteCPP_var(f);

	//��������� ���� ��� ����������
	if (StatementSeq) {
		fprintf(f.fc, "//BEGIN\n");
		StatementSeq->WriteCPP(f);
	}
	//��������������� �������� ��������-��������
	FormalPars->WriteCPP_end(f, hr_No != have_return);
	//������ ����������� ���������-�������
	if (FormalPars->Qualident) fprintf(f.fc, "\treturn O2M_RESULT;\n");
	//���������� �������� ���������� ���������
	fprintf(f.fc, "}\n//END %s;\n", name);
}


//-----------------------------------------------------------------------------
//����������� ������� CPtrType
CPtrType::CPtrType(const CBaseName* parent) : CBaseType(id_CPtrType, parent)
{
	//���������� ����� ���� (PTR)
	SetName("PTR");
}//CPtrType


//-----------------------------------------------------------------------------
//�������� ����� ������� ����
int CPtrType::CreateType(CBaseType* &BaseType) const
{
	BaseType = new CPtrType(parent_element);
	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//�������� ���������� �� ����
int CPtrType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	BaseVar = new CPtrVar(parent);
	//��������� �������� ���� � ������ ������������ ����
	BaseVar->SetTypeName(GetModuleName(), name);
	return 0;
}//CPtrType::CreateVar


//-----------------------------------------------------------------------------
//������ ���� CPtrType
void CPtrType::WriteCPP(CPP_files& f, const bool to_h)
{
	fprintf(to_h ? f.fh : f.fc, "/* PTR TYPE %s */", name);
	fprintf(to_h ? f.fh : f.fc, "typedef bool %s;\n", name);
}


//-----------------------------------------------------------------------------
//������ � DFN ���������� ����
void CPtrType::WriteDFN(DFN_file& f)
{
	throw error_Internal("CPtrType::WriteDFN");
}//WriteDFN


//-----------------------------------------------------------------------------
//������������� ������� CBooleanType
int CBooleanType::Init(CLexBuf *lb)
{
	return 0;
}//Init CBooleanType


//-----------------------------------------------------------------------------
//������ ���� CBooleanType
void CBooleanType::WriteCPP(CPP_files& f, const bool to_h)
{
	fprintf(to_h ? f.fh : f.fc, "typedef %s %s;\n", GetCPPTypeName(), name);
}


//-----------------------------------------------------------------------------
//������ � DFN ���������� ����
void CBooleanType::WriteDFN(DFN_file& f)
{
	fprintf(f.f, "BOOLEAN");
}//WriteDFN


//-----------------------------------------------------------------------------
//�������� ����� ������� ����
int CBooleanType::CreateType(CBaseType* &BaseType) const
{
	BaseType = new CBooleanType(parent_element);
	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//�������� ���������� �� ����
int CBooleanType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	BaseVar = new CBooleanVar(parent);
	//��������� �������� ���� � ������ ������������ ����
	BaseVar->SetTypeName(GetModuleName(), name);
	return 0;
}//CBooleanType::CreateVar


//-----------------------------------------------------------------------------
//������������� ������� CCharType
int CCharType::Init(CLexBuf *lb)
{
	return 0;
}//Init CCharType


//-----------------------------------------------------------------------------
//������ ���� CCharType
void CCharType::WriteCPP(CPP_files& f, const bool to_h)
{
	fprintf(to_h ? f.fh : f.fc, "typedef %s %s;\n", GetCPPTypeName(), name);
}


//-----------------------------------------------------------------------------
//������ � DFN ���������� ����
void CCharType::WriteDFN(DFN_file& f)
{
	fprintf(f.f, "CHAR");
}//WriteDFN


//-----------------------------------------------------------------------------
//�������� ����� ������� ����
int CCharType::CreateType(CBaseType* &BaseType) const
{
	BaseType = new CCharType(parent_element);
	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//�������� ���������� �� ����
int CCharType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	BaseVar = new CCharVar(parent);
	//��������� �������� ���� � ������ ������������ ����
	BaseVar->SetTypeName(GetModuleName(), name);
	return 0;
}//CCharType::CreateVar


//-----------------------------------------------------------------------------
//������������� ������� CShortintType
int CShortintType::Init(CLexBuf *lb)
{
	return 0;
}//Init CShortintType


//-----------------------------------------------------------------------------
//������ ���� CShortintType
void CShortintType::WriteCPP(CPP_files& f, const bool to_h)
{
	fprintf(to_h ? f.fh : f.fc, "typedef %s %s;\n", GetCPPTypeName(), name);
}


//-----------------------------------------------------------------------------
//������ � DFN ���������� ����
void CShortintType::WriteDFN(DFN_file& f)
{
	fprintf(f.f, "SHORTINT");
}//WriteDFN


//-----------------------------------------------------------------------------
//�������� ����� ������� ����
int CShortintType::CreateType(CBaseType* &BaseType) const
{
	BaseType = new CShortintType(parent_element);
	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//�������� ���������� �� ����
int CShortintType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	BaseVar = new CShortintVar(parent);
	//��������� �������� ���� � ������ ������������ ����
	BaseVar->SetTypeName(GetModuleName(), name);
	return 0;
}//CShortintType::CreateVar


//-----------------------------------------------------------------------------
//������������� ������� CIntegerType
int CIntegerType::Init(CLexBuf *lb)
{
	return 0;
}//Init CIntegerType


//-----------------------------------------------------------------------------
//������ ���� CIntegerType
void CIntegerType::WriteCPP(CPP_files& f, const bool to_h)
{
	fprintf(to_h ? f.fh : f.fc, "typedef %s %s;\n", GetCPPTypeName(), name);
}


//-----------------------------------------------------------------------------
//������ � DFN ���������� ����
void CIntegerType::WriteDFN(DFN_file& f)
{
	fprintf(f.f, "INTEGER");
}//WriteDFN


//-----------------------------------------------------------------------------
//�������� ����� ������� ����
int CIntegerType::CreateType(CBaseType* &BaseType) const
{
	BaseType = new CIntegerType(parent_element);
	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//�������� ���������� �� ����
int CIntegerType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	BaseVar = new CIntegerVar(parent);
	//��������� �������� ���� � ������ ������������ ����
	BaseVar->SetTypeName(GetModuleName(), name);
	return 0;
}//CIntegerType::CreateVar


//-----------------------------------------------------------------------------
//������������� ������� CLongintType
int CLongintType::Init(CLexBuf *lb)
{
	return 0;
}//Init CLongintType


//-----------------------------------------------------------------------------
//������ ���� CLongintType
void CLongintType::WriteCPP(CPP_files& f, const bool to_h)
{
	fprintf(to_h ? f.fh : f.fc, "typedef %s %s;\n", GetCPPTypeName(), name);
}


//-----------------------------------------------------------------------------
//������ � DFN ���������� ����
void CLongintType::WriteDFN(DFN_file& f)
{
	fprintf(f.f, "LONGINT");
}//WriteDFN


//-----------------------------------------------------------------------------
//�������� ����� ������� ����
int CLongintType::CreateType(CBaseType* &BaseType) const
{
	BaseType = new CLongintType(parent_element);
	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//�������� ���������� �� ����
int CLongintType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	BaseVar = new CLongintVar(parent);
	//��������� �������� ���� � ������ ������������ ����
	BaseVar->SetTypeName(GetModuleName(), name);
	return 0;
}//CLongintType::CreateVar


//-----------------------------------------------------------------------------
//������������� ������� CRealType
int CRealType::Init(CLexBuf *lb)
{
	return 0;
}//Init CRealType


//-----------------------------------------------------------------------------
//������ ���� CRealType
void CRealType::WriteCPP(CPP_files& f, const bool to_h)
{
	fprintf(to_h ? f.fh : f.fc, "typedef %s %s;\n", GetCPPTypeName(), name);
}


//-----------------------------------------------------------------------------
//������ � DFN ���������� ����
void CRealType::WriteDFN(DFN_file& f)
{
	fprintf(f.f, "REAL");
}//WriteDFN


//-----------------------------------------------------------------------------
//�������� ����� ������� ����
int CRealType::CreateType(CBaseType* &BaseType) const
{
	BaseType = new CRealType(parent_element);
	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//�������� ���������� �� ����
int CRealType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	BaseVar = new CRealVar(parent);
	//��������� �������� ���� � ������ ������������ ����
	BaseVar->SetTypeName(GetModuleName(), name);
	return 0;
}//CRealType::CreateVar


//-----------------------------------------------------------------------------
//������������� ������� CLongrealType
int CLongrealType::Init(CLexBuf *lb)
{
	return 0;
}//Init CLongrealType


//-----------------------------------------------------------------------------
//������ ���� CLongrealType
void CLongrealType::WriteCPP(CPP_files& f, const bool to_h)
{
	fprintf(to_h ? f.fh : f.fc, "typedef %s %s;\n", GetCPPTypeName(), name);
}


//-----------------------------------------------------------------------------
//������ � DFN ���������� ����
void CLongrealType::WriteDFN(DFN_file& f)
{
	fprintf(f.f, "LONGREAL");
}//WriteDFN


//-----------------------------------------------------------------------------
//�������� ����� ������� ����
int CLongrealType::CreateType(CBaseType* &BaseType) const
{
	BaseType = new CLongrealType(parent_element);
	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//�������� ���������� �� ����
int CLongrealType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	BaseVar = new CLongrealVar(parent);
	//��������� �������� ���� � ������ ������������ ����
	BaseVar->SetTypeName(GetModuleName(), name);
	return 0;
}//CLongrealType::CreateVar


//-----------------------------------------------------------------------------
//������������� ������� CSetType
int CSetType::Init(CLexBuf *lb)
{
	return 0;
}//Init CSetType


//-----------------------------------------------------------------------------
//������ ���� CSetType
void CSetType::WriteCPP(CPP_files& f, const bool to_h)
{
	fprintf(to_h ? f.fh : f.fc, "typedef %s %s;\n", GetCPPTypeName(), name);
}


//-----------------------------------------------------------------------------
//������ � DFN ���������� ����
void CSetType::WriteDFN(DFN_file& f)
{
	fprintf(f.f, "SET");
}//WriteDFN


//-----------------------------------------------------------------------------
//�������� ����� ������� ����
int CSetType::CreateType(CBaseType* &BaseType) const
{
	BaseType = new CSetType(parent_element);
	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//�������� ���������� �� ����
int CSetType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	BaseVar = new CSetVar(parent);
	//��������� �������� ���� � ������ ������������ ����
	BaseVar->SetTypeName(GetModuleName(), name);
	return 0;
}//CSetType::CreateVar


//-----------------------------------------------------------------------------
//������������� ������� CQualidentType
int CQualidentType::Init(CLexBuf *lb)
{
	//������������� Qualident (����� ����)
	Qualident = new CQualident;
	return Qualident->InitTypeName(lb, parent_element);
}//Init CQualidentType


//-----------------------------------------------------------------------------
//������ ���� CQualidentType
void CQualidentType::WriteCPP(CPP_files& f, const bool to_h)
{
	if (Qualident) {
		if (Qualident->pref_ident) {
			//��������� ���� �� ��� ����� (� ������ ������� ���������� �� ����� �������������)
			CBaseType* BT = static_cast<CBaseType*>(parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident));
			//TYPE type1 = module1.type1
			fprintf(to_h ? f.fh : f.fc, "typedef %s::%s %s;\n", to_h ? BT->GetModuleName() : BT->GetModuleAlias(), Qualident->ident, name);
		} else	//TYPE type1 = type2;
			fprintf(to_h ? f.fh : f.fc, "typedef %s %s;\n", Qualident->ident, name);
	} else	//�������� � ���� ������
		fprintf(f.fc, "%s", name);
}//WriteCPP


//-----------------------------------------------------------------------------
//������ � DFN ���������� ����
void CQualidentType::WriteDFN(DFN_file& f)
{
	if (Qualident) Qualident->WriteDFN(f);
	else fprintf(f.f, "BYTE");
}//WriteDFN


//-----------------------------------------------------------------------------
//�������� ����� ������� ����
int CQualidentType::CreateType(CBaseType* &BaseType) const
{
	CQualidentType *QT = new CQualidentType(parent_element);
	QT->Qualident = new CQualident();
	Qualident->Assign(QT->Qualident);

	BaseType = QT;
	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//�������� ���������� �� ����
int CQualidentType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	//����� ���� �� ���������� �����
	CBaseType *NamedType = NULL;
	int err_num = GetNamedType(NamedType, false);
	if (err_num) return err_num;
	//����� ��������� �������� ���������� ���������� ����
	err_num = NamedType->CreateVar(BaseVar, parent);
	if (err_num) return err_num;
	//�������� �������� ��������� (��� ���� ���������� ���� ������� ������� QualidentType)
	if (id_CPointerVar == BaseVar->name_id)
		static_cast<CPointerVar*>(BaseVar)->qualident_type = true;
	//��������� ����� ���� � ����������
	/**/
	//������������� NamedType->GetModuleAlias() �������� � ������ � ������ ������������� ���� ���������,
	//���������������� �� ������, ������� ��� � ���� ������� �����������
	//BaseVar->SetTypeName(Qualident->pref_ident ? NamedType->GetModuleAlias() : NULL, Qualident->ident);
	BaseVar->SetTypeName(Qualident->pref_ident, Qualident->ident);
	return 0;
}//CQualidentType::CreateVar


//-----------------------------------------------------------------------------
//��������� ��. �� �������������� ��� (�� QualidentType), ��� �������� �������� QualidentType (� ��������� �������� ����)
int CQualidentType::GetNamedType(CBaseType* &NamedType, const bool check_external) const
{
	CBaseName* BN = parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident);
	//�������� �������� ���������� ���� (��� ��� ���������� � DFN ������)
	if (check_external && BN && !BN->external && (BN->parent_element->name_id != id_CDfnModule)) return s_e_TypeNotExternal;

	while (BN && (BN->name_id == id_CQualidentType)) {
		BN = BN->parent_element->GetGlobalName(static_cast<CQualidentType*>(BN)->Qualident->pref_ident, static_cast<CQualidentType*>(BN)->Qualident->ident);
		//�������� �������� ���������� ���� (��� ��� ���������� � DFN ������)
		if (check_external && BN && !BN->external && (BN->parent_element->name_id != id_CDfnModule)) return s_e_TypeNotExternal;
	}

	//��� ������ ���� ������
	if (!BN || !CBaseType::IsTypeId(BN->name_id)) return s_m_Error;

	NamedType = static_cast<CBaseType*>(BN);
	return 0;
}


//-----------------------------------------------------------------------------
//��������� ���������� ��������� ��� ����, ��� �������� �������� QualidentType
EName_id CQualidentType::GetResultId() const
{
	CBaseType* BT;
	int err_num = GetNamedType(BT, false);
	if (err_num) return id_CBaseName;
	return BT->GetResultId();
}


//-----------------------------------------------------------------------------
//������������� ������� CArrayType
int CArrayType::Init(CLexBuf *lb)
{
	DECL_SAVE_POS
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	CExpr* CountExpr = new CExpr(parent_element);
	int err_num = CountExpr->Init(lb);

	if (err_num) {
		//������ �� ����� �������, ��������������� ������� � �����
		RESTORE_POS
		//���������� ��������� ���������
		delete CountExpr;
		//��������� ������� "OF"
		if (!lb->ReadLex(li) || lex_k_OF != li.lex) return s_e_OF;
		//�������� ������� � �������� ����
		return TypeSelector(lb, Type, parent_element);
	} else {
		//������ ����� ������, ���������� ���
		CBaseVar* BV = NULL;
		err_num = CountExpr->CreateConst(BV);
		delete CountExpr;
		if (err_num || !CBaseVar::IsIntId(BV->name_id)) {
			delete BV;
			return s_e_ExprNotPosIntConst;	//�������� ��� ��������� � ������� �������
		}
		//��������� � �������� ������������ ������� �������
		size = BV->GetIntValue();
		delete BV;
		if (size <= 0) return s_e_ExprNotPosIntConst;
	}//else

	//������ ����� ������, ������ ����. ������� ("," ��� "OF")
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_OF;

	//�������� ������� ","
	if (lex_k_comma == li.lex) {
		//������� ��������� ���������, �������������� ��� ��� ��� ��-��� �������, ���������� ��������
		Type = new CArrayType(parent_element);
		err_num = static_cast<CArrayType*>(Type)->Init(lb);
		if (err_num) return err_num;
		//���������, �������� �� ������ �������� (��� �����������)
		if (0 == static_cast<CArrayType*>(Type)->size) return s_e_OpenArrayNotAllowed;
		return 0;
	}

	//�������� ������� "OF"
	if (lex_k_OF == li.lex) {
		//����� ������ ������������ �������, �������� ���� ��-��� �������
		err_num = TypeSelector(lb, Type, parent_element);
		if (err_num) return err_num;
		//� ������, ���� ����� ��-��� ������� �������� ������, ���������, �������� �� �� �������� (��� �����������)
		if (id_CArrayType == Type->name_id && 0 == static_cast<CArrayType*>(Type)->size) return s_e_OpenArrayNotAllowed;
		return 0;
	}

	return s_e_OF;
}//Init CArrayType


//-----------------------------------------------------------------------------
//������ ���� ���������� CArrayType
void CArrayType::WriteCPP(CPP_files& f, const bool to_h)
{
	//��� ������ ����� �� ��� ��. �� ������ - ������� ������������ ������ ��� ���� �������,
	//����������� (�����������) ������������� ������ ��� ���������� ����������, ����� �� �������
	//������������� ��� ��� NEW()

	//��������� ����� (��� �������� RuntimeId � ��������� ����) - ������� ��������, �.�. InitRuntimeId ��� WriteCPP ����� ������������� ����������
	Type->name = name;
	try {
		//��� ���� RECORD ���������� RuntimeId
		if (id_CRecordType == Type->name_id) static_cast<CRecordType*>(Type)->InitRuntimeId();
		Type->WriteCPP(f, to_h);
	}
	catch(error_Internal) {
		Type->name = NULL;
		throw;
	};

	//������� ������������� ���
	Type->name = NULL;

}//WriteCPP


//-----------------------------------------------------------------------------
//������ � DFN ���������� ����
void CArrayType::WriteDFN(DFN_file &f)
{
	fprintf(f.f, "ARRAY");

	//����� �������� �� ������� ��������� (���� ���� ������)
	CArrayType *AT = this;
	while (AT->name_id == id_CArrayType) {
		if (AT->size != 0) fprintf(f.f, "%s%i", AT == this ? " " : ",", AT->size);
		AT = static_cast<CArrayType*>(AT->Type);
	}

	//������ "OF" <���������� ���� ��-��� �������>
	fprintf(f.f, " OF ");
	AT->WriteDFN(f);

}


//-----------------------------------------------------------------------------
//�������� ������������� ���� ��-��� �������
int CArrayType::CheckComplete(CLexBuf *lb) const
{
	//����� ���� ��-��� �������
	CBaseType *BT = FindLastType();
	switch (BT->name_id) {
	case id_CPointerType:
		return static_cast<CPointerType*>(BT)->CheckComplete(lb);
	case id_CRecordType:
		return static_cast<CRecordType*>(BT)->CheckComplete(lb);
	default:
		return 0;
	}//switch
}


//-----------------------------------------------------------------------------
//�������� ����� ������� ����
int CArrayType::CreateType(CBaseType* &BaseType) const
{
	CArrayType* AT = new CArrayType(parent_element);
	//����������� �������
	AT->size = size;
	//����������� ����� (���� ����)
	if (name) AT->SetName(name);
	//����������� ���� ��-��� �������
	int err_num = Type->CreateType( AT->Type );
	//���������� ��������� ����� ����
	BaseType = AT;
	return err_num;
}//CreateType


//-----------------------------------------------------------------------------
//�������� ���������� �� ����
int CArrayType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	//�������� ����������
	CArrayVar* AV = new CArrayVar(parent);
	CBaseType* BT;
	//����������� � ���������� �������� ����
	CreateType(BT);
	AV->ArrayType = static_cast<CArrayType*>(BT);
	//������� ��������� ����������
	BaseVar = AV;
	//��������� �������� ���� � ������ ������������ ����
	BaseVar->SetTypeName(GetModuleName(), name);

	return 0;
}//CArrayType::CreateVar


//-----------------------------------------------------------------------------
//���������� ������� CTypeRecord
CRecordType::~CRecordType()
{
	delete Qualident;
	delete[] RuntimeId;
	//������� ������ ����� ������ (�� ����������� ��������� � ����� ��������)
	CBaseNameVector::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci)
		if (!CProcedure::IsProcId((*ci)->name_id)) delete *ci;
}//~CRecordType


//-----------------------------------------------------------------------------
//������������� ������� CRecordType
int CRecordType::Init(CLexBuf *lb)
{
	DECL_SAVE_POS
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;
	//���������� ��� ��������� ������ ������
	int err_num;

	//�������� ������� Qualident - �������� ������� "("
	if (lb->ReadLex(li) && lex_k_op_bracket == li.lex) {
		Qualident = new CQualident;
		err_num = Qualident->Init(lb, parent_element);
		if (err_num) return err_num;
		//�������� ������� �������� ���� ������, ���������������� Qualident
		CBaseType* BaseRec = static_cast<CBaseType*>( parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident) );
		if (!BaseRec) return s_e_UndeclaredIdent;
		//��������� ���� QualidentType (��������� ����, �� ���. �� ���������)
		if (BaseRec->name_id == id_CQualidentType) err_num = static_cast<CQualidentType*>(BaseRec)->GetNamedType(BaseRec, external);
		else err_num = 0;
		if (err_num) return err_num;
		if (!BaseRec) return s_e_UndeclaredIdent;
		//�������� ������������ �������� ���� (������ ���� ��� ������)
		if (BaseRec->name_id != id_CRecordType) return s_e_IdentNotRecordType;
		//�������� ������� ��. ����� ")"
		if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;
	}
	else {
		RESTORE_POS
	}

	//��������, ������ �� ������ ��� � ��������� ������� ��������� (����� parent_element ���� ������ CModule, CDfnModule ��� CRecordType)
	const CBaseName* BN = parent_element;
	while (BN->parent_element && BN->name_id == id_CRecordType) BN = BN->parent_element;
	bool is_local = (BN->name_id != id_CModule) && (BN->name_id != id_CDfnModule);

	//������������� ������� ����� (���� ����)
	while (true) {
		CIdentList IdentList(this, is_local);	//�������� ������ ���� ����������
		SAVE_POS
		err_num = IdentList.AssignVars(lb);
		if (err_num == s_m_IdentDefAbsent) {
			RESTORE_POS
		} else
			if (err_num) return err_num;

		//�������� ������� ��. ����� ";"
		SAVE_POS
		if (!lb->ReadLex(li) || lex_k_semicolon != li.lex) {
			RESTORE_POS
			break;
		}
	}

	//�������� ������� ��. ����� "END"
	if (!lb->ReadLex(li) || lex_k_END != li.lex) return s_e_END;

	return 0;
}//Init CRecordType


//-----------------------------------------------------------------------------
//������������� ���������� ��. ���� ������� ����������
int CRecordType::InitRuntimeId()
{
	//�������� ���������� ���������� RuntimeId
	if (RuntimeId) return s_m_Error;

	//���������� � RuntimeId �������� ���� + �������� ������
	if (name) {
		//��������� ��. �� ������ (��� Dfn-������)
		const CBaseName* top_parent = parent_element;
		while ((top_parent->parent_element) && (top_parent->name_id != id_CDfnModule)) top_parent = top_parent->parent_element;
		//�������� ������ RuntimeId (������� ���� ��� ���� ��� ��������� �������� ��������� �����)
		RuntimeId = new char[strlen(top_parent->name) + strlen(name) + 2];
		strcpy(RuntimeId, name);
		strcat(RuntimeId, "@");
		strcat(RuntimeId, top_parent->name);
	} else {
		//��������� RuntimeId ��� ������������� �����
		RuntimeId = new char[16];
		strcpy(RuntimeId, "O2M_SYS_UNNAMED");
	}

	return 0;
}//InitRuntimeId


//-----------------------------------------------------------------------------
//������ ���� CRecordType
void CRecordType::WriteCPP(CPP_files& f, const bool to_h)
{
	//////////////////
	//������ � .h ����
	if (to_h) {
		fprintf(f.fh, "struct %s", name);
		//������ �������� ���� (���� ����)
		if (Qualident) {
			fprintf(f.fh, " : ");
			CBaseName* BN = parent_element->GetGlobalName(Qualident->pref_ident);
			if (BN) fprintf(f.fh, "%s::", static_cast<CImportModule*>(BN)->real_name);
			fprintf(f.fh, "%s", Qualident->ident);
		}
		//������ �������� ���� ����� ������
		fprintf(f.fh, " {\n");
		//������ ���� ������ ��� ��������� ��. ���� ������� ���������� �� ����� ����������
		fprintf(f.fh, "\t%sconst char* O2M_SYS_ID() {return \"%s\";};\n", Qualident ? "" : "virtual ", RuntimeId);
		//������ ���� ����� ������
		CBaseNameVector::const_iterator ci;
		for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci) {
			fprintf(f.fh, "\t");
			if (id_CArrayVar == (*ci)->name_id)	//������ ���� �������
				static_cast<CArrayVar*>(*ci)->WriteCPP_rec(f, true);
			else {
				//�������� ������� ��������� � ����� ���������
				if (CProcedure::IsProcId((*ci)->name_id))
					static_cast<CProcedure*>(*ci)->WriteCPP_RECORD(f, this, to_h);
				else
					static_cast<CBaseVar*>(*ci)->WriteCPP_fp(f, true);
			}
			fprintf(f.fh, ";\n");
		}//for
		//���������� �������� ���� ������
		fprintf(f.fh, "};\n");
		return;
	}//if

	////////////////////
	//������ � .cpp ����
	fprintf(f.fc, "struct %s", name);
	//������ �������� ���� (���� ����)
	if (Qualident) {
		fprintf(f.fc, " : ");
		Qualident->WriteCPP_type(f, false, parent_element);
	}
	//������ �������� ���� ����� ������
	fprintf(f.fc, " {\n");
	//������ ���� ������ ��� ��������� ��. ���� ������� ���������� �� ����� ����������
	fprintf(f.fc, "\t%sconst char* O2M_SYS_ID() {return \"%s\";};\n", Qualident ? "" : "virtual ", RuntimeId);
	//������ ���� ����� ������
	CBaseNameVector::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci) {
		fprintf(f.fc, "\t");
		if ((*ci)->name_id == id_CArrayVar)	//������ ���� �������
			static_cast<CArrayVar*>(*ci)->WriteCPP_rec(f, false);
		else {
			//�������� ������� ��������� � ����� ���������
			if (CProcedure::IsProcId((*ci)->name_id))
				static_cast<CProcedure*>(*ci)->WriteCPP_RECORD(f, this, false);
			else
				static_cast<CBaseVar*>(*ci)->WriteCPP_fp(f, false);
		}
		fprintf(f.fc, ";\n");
	}//for
	//���������� �������� ���� ������
	fprintf(f.fc, "};\n");

}//WriteCPP


//-----------------------------------------------------------------------------
//������ � DFN ���������� ����
void CRecordType::WriteDFN(DFN_file &f)
{
	fprintf(f.f, "RECORD");
	//��������� ���� �������� ����
	if (Qualident) {
		fprintf(f.f, " (");
		Qualident->WriteDFN(f);
		fprintf(f.f, ")");
	}

	//���������� ������� � DFN �����
	f.tab_level++;

	fprintf(f.f, "\n");
	f.tab();

	//��������� ���� ����� ������
	CBaseNameVector::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci)
		if ((*ci)->external) {
			//�������� ���������� ��������� � ����� ���������
			if (!CProcedure::IsProcId((*ci)->name_id)) {
				(*ci)->WriteDFN(f);
				fprintf(f.f, ";\n");
				f.tab();
			}//if
		}//if

	//����� ��������� ���� ������
	fprintf(f.f, "END");

	//���������� ������� � DFN �����
	f.tab_level--;
}


//-----------------------------------------------------------------------------
//��������, �������� �� ������ ��� ����������� ���� � ��������� ������,
//������� - ��. �� ������� ��� � ��������� ������
CRecordType* CRecordType::IsExtension(const char *module_name, const char *type_name)
{
	//��������, �������� �� ������ ��� � ��� � ��������� ������ ������� ������
	if (IsSame(module_name, type_name)) return this;
	//�������� ������� ����������������� �������� ���� (direct extension)
	if (!Qualident) return NULL;
	//��������� �������� ���� (������������� �����������������)
	CBaseType* BT = static_cast<CBaseType*>( parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident) );
	if (!BT) return NULL;
	int err_num = 0;
	if (id_CQualidentType == BT->name_id)
		err_num = static_cast<CQualidentType*>(BT)->GetNamedType(BT, external);
	if (err_num || id_CRecordType != BT->name_id) return NULL;
	//��������, �������� �� ���������������� ������� ��� ����������� ���� � ��������� ������
	return static_cast<CRecordType*>(BT)->IsExtension(module_name, type_name);
}


//-----------------------------------------------------------------------------
//����� ����� � ������ �����, NULL - ���� ��� �� �������
CBaseName* CRecordType::FindName(const char* search_name) const
{
	//����� ����� � ����������� ������ ����� ����
	CBaseNameVector::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci)
		if (!strcmp( (*ci)->name, search_name ))
			return *ci;

	//�������� ������� �������� ����
	if (Qualident) {
		//��������� ��. �� ������ �������� ����
		CBaseName* BN = parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident);
		return static_cast<CRecordType*>(BN)->FindName(search_name);
	}

	//��� �� �������
	return NULL;
}//FindName


//-----------------------------------------------------------------------------
//���������� ���������� ��-�� � ������� ����
void CRecordType::AddName(CBaseName* BN) const
{
	FieldStore.push_back(BN);
}//AddName


//-----------------------------------------------------------------------------
//���������� ��. �� ��������� � ��������� ������� ���� � ������� ForwardDecl
//� ��������� ��������� ������ (�� ForwardDecl, ������������ ���������� �
//���������������� ����������)
int CRecordType::AddProcReference(CProcedure* P) const
{
	/**/
	//�������� �������� ���������� ���������� � ���������������� ����������

	//����� �������� � ������, ����������� � ���������� ��������
	CBaseNameVector::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci)
		if (!strcmp(P->name, (*ci)->name)) {
			//�������� ������� ������������ �������� (����� ��������� ������ �� ���������)
			if (id_CForwardDecl == (*ci)->name_id) return 0;
			//��������� ������ ������������� ����
			return s_e_Redefinition;
		}
	//����������� �������� �� ���������� - ������� ���������� � ������� ����
	FieldStore.push_back(P);
	return 0;
}


//-----------------------------------------------------------------------------
//�������� ������������� ���� ����� ��������� � ������� ���� ������
int CRecordType::CheckComplete(CLexBuf *lb) const
{
	//�������� ������������ ������ (���� ����� ����� ���� ����������� ���������)
	if (in_checking_complete)
		return 0;
	else
		in_checking_complete = true;
	//���� �������� ����� ������ � ������ CheckComplete ��� ������ ����������
	CBaseNameVector::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci)
		if (CBaseVar::IsVarId((*ci)->name_id)) {
			int err_num = static_cast<CBaseVar*>(*ci)->CheckComplete(lb);
			if (err_num) return err_num;
		}
	return 0;
}


//-----------------------------------------------------------------------------
//�������� ����� ������� ����
int CRecordType::CreateType(CBaseType* &BaseType) const
{
	BaseType = new CRecordType(parent_element);
	CRecordType* RT = static_cast<CRecordType*>(BaseType);
	//����������� ������ ����� (���� ����)
	CBaseNameVector::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci) {
		//�������� ������� ��������� � ����� ��������� (�� ���������� � ���)
		if (CProcedure::IsProcId((*ci)->name_id)) continue;
		//����������� ����������
		CBaseVar* TempVar = static_cast<CBaseVar*>(*ci);
		TempVar = TempVar->CreateVar(RT);
		RT->AddName(TempVar);
	}
	//����������� �������� �������� ���� (���� ����)
	if (Qualident) Qualident->CreateCopy(RT->Qualident);
	//����������� RuntimeId (���� ����)
	if (RuntimeId) RT->RuntimeId = str_new_copy(RuntimeId);
	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//�������� ���������� �� ����
int CRecordType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	BaseVar = new CRecordVar(parent);
	//��������� �������� ���� � ������ ������������ ����
	BaseVar->SetTypeName(GetModuleName(), name);
	//����������� ������ ���������� (���� ����)
	CBaseNameVector::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci) {
		//�������� ������� ��������� � ����� ��������� (�� ���������� � ����������)
		if (CProcedure::IsProcId((*ci)->name_id)) continue;
		//����������� ����������
		CBaseVar* TempVar = static_cast<CBaseVar*>(*ci);
		TempVar = TempVar->CreateVar(BaseVar);
		BaseVar->AddName(TempVar);
	}
	//����������� �������� �������� ���� (���� ����)
	if (Qualident)
		Qualident->CreateCopy(static_cast<CRecordVar*>(BaseVar)->Qualident);

	return 0;
}//CRecordType::CreateVar


//-----------------------------------------------------------------------------
//���������� ��-�� ��������� � ������
void CCommonType::AddSpec(const char *newTag, const char *newQual, const char *newName, const bool newIsExtended)
{
	SpecStore.push_back(NewSpec(newTag, newQual, newName, newIsExtended));
}


//-----------------------------------------------------------------------------
//�������� ������������ ���� ���������
int CCommonType::CheckSpecType(const char *pref_ident, const char *ident, const CBaseName* parent) const
{
	//�������� ������� ������������ ���� �������������
	CBaseName* BN = parent->GetGlobalName(pref_ident, ident);
	if (!BN) return s_e_UndeclaredIdent;
	//�������� ������������ ���� �������������
	switch (BN->name_id) {
	case id_CRecordType:
	case id_CArrayType:
	case id_CCommonType:
		return 0;
	default:
		return s_e_CommonTypeSpecType;
	}//switch
}


//-----------------------------------------------------------------------------
//������� ������ ���������
void CCommonType::ClearTmpTagStore()
{
	TmpTagStore_type::const_iterator ci_t;
	for (ci_t = TmpTagStore.begin(); ci_t != TmpTagStore.end(); ci_t++) delete[] *ci_t;
	TmpTagStore.clear();
}


//-----------------------------------------------------------------------------
//����������� ��-�� ���������
void CCommonType::DelSpec(CCommonType::SSpec *sp)
{
	delete sp->Tag;
	delete sp->QualName;
	delete sp->Name;
	delete sp;
}


//-----------------------------------------------------------------------------
//����� ������������� �� �������� (� ������ ���� ��������)
const CCommonType::SSpec* CCommonType::FindSpec(const char *QualName, const char *Name, const char *Tag)
{
	if (tt_Type == TagType)
		return FindSpecByName(QualName, Name);
	else
		return FindSpecByTag(Tag);
}


//-----------------------------------------------------------------------------
//����� ������������� �� ��������
const CCommonType::SSpec* CCommonType::FindSpecByTag(const char *Tag) const
{
	//����� � ������ �������������
	SpecStore_type::const_iterator ci;
	for (ci = SpecStore.begin(); ci != SpecStore.end(); ci++)
		if (!strcmp(Tag, (*ci)->Tag)) return *ci;
	//�������� ������������� �� ��������� (���� ����)
	if (DefaultSpec && !strcmp(Tag, DefaultSpec->Tag)) return DefaultSpec;
	//������������� �� �������
	return NULL;
}


//-----------------------------------------------------------------------------
//����� ������������� �� ����� ����
const CCommonType::SSpec* CCommonType::FindSpecByName(const char *QualName, const char *Name) const
{
	//� ������ ������������� ��-�� - DFN ������, �������� ������������� ����������� ��� ������
	if (QualName && id_CDfnModule == parent_element->name_id)
		if (static_cast<const CDfnModule*>(parent_element)->alias_name && !strcmp(QualName, static_cast<const CDfnModule*>(parent_element)->alias_name))
			QualName = NULL;
	//����� � ������ �������������
	SpecStore_type::const_iterator ci;
	for (ci = SpecStore.begin(); ci != SpecStore.end(); ci++) {
		bool IsEqual = (QualName && (*ci)->QualName) || !(QualName || (*ci)->QualName);
		if (IsEqual && QualName) IsEqual = !strcmp(QualName, (*ci)->QualName);
		if (IsEqual) IsEqual = (Name && (*ci)->Name) || !(Name || (*ci)->Name);
		if (IsEqual && Name) IsEqual = !strcmp(Name, (*ci)->Name);
		if (IsEqual) return *ci;
	}//for
	//�������� ������������� �� ��������� (���� ����)
	if (DefaultSpec) {
		bool IsEqual = (QualName && DefaultSpec->QualName) || !(QualName || DefaultSpec->QualName);
		if (IsEqual && QualName) IsEqual = !strcmp(QualName, DefaultSpec->QualName);
		if (IsEqual) IsEqual = (Name && DefaultSpec->Name) || !(Name || DefaultSpec->Name);
		if (IsEqual && Name) IsEqual = !strcmp(Name, DefaultSpec->Name);
		if (IsEqual) return DefaultSpec;
	}//if
	//������������� �� �������
	return NULL;
}


//-----------------------------------------------------------------------------
//�������� ������ ��-�� ���������
CCommonType::SSpec* CCommonType::NewSpec(const char *newTag, const char *newQual, const char *newName, const bool newIsExtended)
{
	//�������� ��-�� ���������
	SSpec* sp = new SSpec;
	//������� (Tag)
	if (newTag)
		sp->Tag = str_new_copy(newTag);
	else
		sp->Tag = NULL;
	//��������� (��� ���������������� �����)
	if (newQual)
		sp->QualName = str_new_copy(newQual);
	else
		sp->QualName = NULL;
	//��� ���� (������)
	if (newName)
		sp->Name = str_new_copy(newName);
	else
		sp->Name = NULL;
	//������� ���������� ����� ���������
	sp->IsExtended = newIsExtended;
	//������� ���������� � ������������������� ��-�� ���������
	return sp;
}


//-----------------------------------------------------------------------------
//���������� CCommonType
CCommonType::~CCommonType()
{
	//������� ������ ���������
	ClearTmpTagStore();
	//������� ������ ��-��� ���������
	SpecStore_type::const_iterator ci_s;
	for (ci_s = SpecStore.begin(); ci_s != SpecStore.end(); ci_s++) DelSpec(*ci_s);
	//����������� ELSE - ������������� (���� ����)
	if (DefaultSpec) DelSpec(DefaultSpec);
}


//-----------------------------------------------------------------------------
//�������� ����� ������� ����
int CCommonType::CreateType(CBaseType *&BaseType) const
{
	BaseType = new CCommonType(parent_element);
	/**/
	return 0;
}


//-----------------------------------------------------------------------------
//�������� ���������� �� ����
int CCommonType::CreateVar(CBaseVar *&BaseVar, const CBaseName *parent) const
{
	BaseVar = new CCommonVar(parent);
	//��������� �������� ���� � ������ ������������ ����
	BaseVar->SetTypeName(GetModuleName(), name);
	return 0;
}


//-----------------------------------------------------------------------------
//������������� ������� CCommonType
int CCommonType::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;
	//���������� ��� ��������� ������ ������
	int err_num;

	//�������� ������� ���� ���������, �������� ����������� ��� OF
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_OF;
	switch (li.lex) {
	case lex_k_TYPE:
		TagType = tt_Type;
		break;
	case lex_k_LOCAL:
		IsLocal = true;
		break;
	case lex_k_OF:
		goto OF_received;
	default:
		return s_e_OF;
	}//switch

	//�������� ������� �������� ����������� ��� OF
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_OF;
	switch (li.lex) {
	case lex_k_LOCAL:
		IsLocal = true;
		break;
	case lex_k_OF:
		goto OF_received;
	default:
		return s_e_OF;
	}//switch

	//�������� ������� OF
	if (!lb->ReadLex(li) || lex_k_OF != li.lex) return s_e_OF;

OF_received:

	DECL_SAVE_POS
	//������� �� ������ ������������� � ���������
	bool is_not_first = false;
	//��� ��������� Qualident
	CQualident qual;

	//������������� ������ ������������� (��-��� ���������)
	while (true) {
		//��������� ������ ������� ������������� ��� |, ELSE, END
		SAVE_POS
		if (!lb->ReadLex(li)) return s_e_END;
		//�������� ������� | (���� ����), ���� ���� - ��������� ����. �������
		if (is_not_first) {
			if (lex_k_vertical == li.lex) {
				SAVE_POS
				if (!lb->ReadLex(li)) return s_e_IdentExpected;
			}
		} else
			is_not_first = true;
		//�������� �������������� (������� ��� �������������), ELSE, END
		switch (li.lex) {
		case lex_i:
			break;
		case lex_k_ELSE:
			goto ELSE_received;
		case lex_k_END:
			goto END_received;
		default:
			return s_e_IdentExpected;
		}//switch
		//�������� ������������� ��������� ������ ���������
		if (tt_Default == TagType) {

			//������� ������ ��������� �� ���������� ��������
			ClearTmpTagStore();

			//���� �������� ������ ���������
			while (true) {
				//�������� ���������� ������������� � ������ ���������
				if (FindSpecByTag(li.st)) return s_e_SpecRedefining;
				//����������� ���������� ��������
				TmpTagStore.push_back(str_new_copy(li.st));
				//��������� ����. ������� ("," ��� ":")
				if (!lb->ReadLex(li)) return s_e_ColonMissing;
				if (lex_k_colon == li.lex) break;
				if (lex_k_comma != li.lex) return s_e_ColonMissing;
				//��������� ����. ��������������
				if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
			}

			//��������� ����. ������� (ident ��� NIL)
			SAVE_POS
			if (!lb->ReadLex(li)) return s_e_IdentExpected;

			//� ������ NIL ���������� ��������� �������� ����� �������������
			if (lex_k_NIL == li.lex) {
				TmpTagStore_type::const_iterator ci;
				for (ci = TmpTagStore.begin(); ci != TmpTagStore.end(); ci++) AddSpec(*ci, NULL, NULL, false);
				continue;
			} else {
				RESTORE_POS
			}

			//��������� ����� ����
			qual.Clear();
			err_num = qual.Init(lb, parent_element);
			if (err_num) return err_num;
			//�������� ���� �������������
			err_num = CheckSpecType(qual.pref_ident, qual.ident, parent_element);
			if (err_num) return err_num;

			//���������� ��������� �������� ����� �������������
			TmpTagStore_type::const_iterator ci;
			for (ci = TmpTagStore.begin(); ci != TmpTagStore.end(); ci++) AddSpec(*ci, qual.pref_ident, qual.ident, false);
			continue;

		}//if

		//��������� ����� ����
		RESTORE_POS
		qual.Clear();
		err_num = qual.Init(lb, parent_element);
		if (err_num) return err_num;
		//�������� ���������� ������������� � ������ ������
		if (FindSpecByName(qual.pref_ident, qual.ident)) return s_e_SpecRedefining;
		//�������� ���� �������������
		err_num = CheckSpecType(qual.pref_ident, qual.ident, parent_element);
		if (err_num) return err_num;

		//���������� ��������� �������� ����� �������������
		AddSpec(NULL, qual.pref_ident, qual.ident, false);

	}//while

ELSE_received:
	//��������� �������������� (������� ��� �������������)
	qual.Clear();
	err_num = qual.Init(lb, parent_element);
	if (err_num) return err_num;
	//�������� ���� ���������
	if (tt_Type == TagType) {
		//�������� ���������� ������������� � ������ ������
		if (FindSpecByName(qual.pref_ident, qual.ident)) return s_e_SpecRedefining;
		//�������� ���� �������������
		err_num = CheckSpecType(qual.pref_ident, qual.ident, parent_element);
		if (err_num) return err_num;
		//�������� �������� ������������� �� ���������
		DefaultSpec = NewSpec(NULL, qual.pref_ident, qual.ident, false);
	} else {
		//�������� ���������� ����������� �����
		if (qual.pref_ident) return s_e_IdentExpected;
		//�������� ���������� ������������� � ������ ���������
		if (FindSpecByTag(qual.ident)) return s_e_SpecRedefining;
		//�������� ������� ":"
		if (!lb->ReadLex(li) || lex_k_colon != li.lex) return s_e_ColonMissing;
		SAVE_POS
		//��������� ����. �������
		if (!lb->ReadLex(li)) return s_e_IdentExpected;
		if (lex_k_NIL == li.lex) {
			//�������� �������� ������������� �� ���������
			DefaultSpec = NewSpec(qual.ident, NULL, NULL, false);
		} else {
			RESTORE_POS
			//���������� ��������������
			char* ident = str_new_copy(qual.ident);
			qual.Clear();
			err_num = qual.Init(lb, parent_element);
			if (err_num) {
				delete ident;
				return err_num;
			}
			//�������� �������� ������������� �� ���������
			DefaultSpec = NewSpec(ident, qual.pref_ident, qual.ident, false);
			//����������� ������������ ��������������
			delete[] ident;
		}
	}

	//�������� ������� END
	if (!lb->ReadLex(li) || lex_k_END != li.lex) return s_e_END;

END_received:

	return 0;
}


//-----------------------------------------------------------------------------
//������������� ���������� � ��������� ����� �������������
int CCommonType::InitExtension(CLexBuf *lb, CDeclSeq* DS)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;
	//���������� ��� ��������� ������ ������
	int err_num;

	//������� �� ������ ������������� � ���������
	bool is_not_first = false;
	//��� ��������� Qualident
	CQualident qual;

	//������ ��� �������� ���������� � ���������� ���������������� ��������� (��� NULL)
	CCommonExtensionType* CET = NULL;
	if (GetModuleAlias()) {
		CET = new CCommonExtensionType(parent_element);
		CET->TypeName = str_new_copy(name);
		CET->TypeModuleName = str_new_copy(GetModuleName());
		DS->AddName(CET);
	}

	//������������� ������ ������������� (��-��� ���������)
	while (true) {
		//��������� ������ ������� ������������� ��� |, ";"
		DECL_SAVE_POS
		if (!lb->ReadLex(li)) return s_e_SemicolonMissing;
		//�������� ������� | (���� ����), ���� ���� - ��������� ����. �������
		if (is_not_first) {
			if (lex_k_vertical == li.lex) {
				SAVE_POS
				if (!lb->ReadLex(li)) return s_e_IdentExpected;
			}
		} else
			is_not_first = true;
		//�������� �������������� (������� ��� �������������), ";"
		switch (li.lex) {
		case lex_i:
			break;
		case lex_k_semicolon:
			RESTORE_POS
			goto semicolon_received;
		default:
			return s_e_IdentExpected;
		}//switch
		//�������� ������������� ��������� ������ ���������
		if (tt_Default == TagType) {

			//������� ������ ��������� �� ���������� ��������
			ClearTmpTagStore();

			//���� �������� ������ ���������
			while (true) {
				//�������� ���������� ������������� � ������ ���������
				if (FindSpecByTag(li.st)) return s_e_SpecRedefining;
				//����������� ���������� ��������
				TmpTagStore.push_back(str_new_copy(li.st));
				//��������� ����. ������� ("," ��� ":")
				if (!lb->ReadLex(li)) return s_e_ColonMissing;
				if (lex_k_colon == li.lex) break;
				if (lex_k_comma != li.lex) return s_e_ColonMissing;
				//��������� ����. ��������������
				if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
			}

			//��������� ����. ������� (ident ��� NIL)
			SAVE_POS
			if (!lb->ReadLex(li)) return s_e_IdentExpected;

			//� ������ NIL ���������� ��������� �������� ����� �������������
			if (lex_k_NIL == li.lex) {
				TmpTagStore_type::const_iterator ci;
				for (ci = TmpTagStore.begin(); ci != TmpTagStore.end(); ci++) {
					AddSpec(*ci, NULL, NULL, true);
					if (CET) CET->AddSpec(*ci, NULL, NULL);
				}
				continue;
			} else {
				RESTORE_POS
			}

			//��������� ����� ����
			qual.Clear();
			err_num = qual.Init(lb, parent_element);
			if (err_num) return err_num;
			//�������� ���� �������������
			err_num = CheckSpecType(qual.pref_ident, qual.ident, DS->parent_element);
			if (err_num) return err_num;

			//���������� ��������� �������� ����� �������������
			TmpTagStore_type::const_iterator ci;
			for (ci = TmpTagStore.begin(); ci != TmpTagStore.end(); ci++) {
				AddSpec(*ci, qual.pref_ident, qual.ident, true);
				if (CET) CET->AddSpec(*ci, qual.pref_ident, qual.ident);
			}
			continue;

		}//if

		//��������� ����� ����
		RESTORE_POS
		qual.Clear();
		err_num = qual.Init(lb, parent_element);
		if (err_num) return err_num;
		//�������� ���������� ������������� � ������ ������
		if (FindSpecByName(qual.pref_ident, qual.ident)) return s_e_SpecRedefining;
		//�������� ���� �������������
		err_num = CheckSpecType(qual.pref_ident, qual.ident, DS->parent_element);
		if (err_num) return err_num;

		//���������� ��������� �������� ����� �������������
		AddSpec(NULL, qual.pref_ident, qual.ident, true);
		if (CET) CET->AddSpec(NULL, qual.pref_ident, qual.ident);

	}//while

semicolon_received:

	//����� �������� ���������� ���������
	return 0;
}


//-----------------------------------------------------------------------------
//��������� �������� ��������� � .2ml ����
void CCommonType::WriteCPP(CPP_files& f, const bool to_h)
{
	//������ ������ �������� ���������
	fprintf(f.f2ml, "\t<Case Name=\"%s\"%s%s>\n", name, (tt_Type == TagType) ? " TagType=\"type\"" : "", IsLocal ? " Local=\"yes\"" : "");
	//��������� ������������� ��������� (��-��� ���������)
	SpecStore_type::const_iterator ci;
	for (ci = SpecStore.begin(); ci != SpecStore.end(); ci++)
		WriteCPP_SpecInfo(f.f2ml, false, parent_element, *ci);
	//��������� ������������� �� ���������
	if (DefaultSpec)
		WriteCPP_SpecInfo(f.f2ml, true, parent_element, DefaultSpec);
	//����� �������� ���������
	fprintf(f.f2ml, "\t</Case>\n");
}


//-----------------------------------------------------------------------------
//��������� � .2ml ���� ���������� �� ����� ��-�� ���������
void CCommonType::WriteCPP_SpecInfo(TFileType* f2ml, const bool IsDef, const CBaseName* parent_element, const CCommonType::SSpec *sp)
{
	fprintf(f2ml, "\t\t<Spec");
	if (IsDef) fprintf(f2ml, " Def=\"yes\"");
	if (sp->Tag) fprintf(f2ml, " Tag=\"%s\"", sp->Tag);
	if (sp->QualName) {
		//����� ����� ������ �� ����������
		CBaseName* BN = parent_element->GetGlobalName(sp->QualName);
		const char* st = sp->QualName;
		if (BN && id_CImportModule == BN->name_id) st = static_cast<CImportModule*>(BN)->real_name;
		//������ ���������� �������� ������
		fprintf(f2ml, " Qual=\"%s\"", st);
	}
	if (sp->Name) fprintf(f2ml, " Name=\"%s\"", sp->Name);
	fprintf(f2ml, " />\n");
}


//-----------------------------------------------------------------------------
//������ � DFN ���������� ����
void CCommonType::WriteDFN(DFN_file& f)
{
	//�������� ���������� ������ ��������� (���������� ��������� �� �����
	//���������������� => WriteDFN ���������� ������ ��� ���������� ���������)
	fprintf(f.f, "CASE ");
	//��������� ���� ��������
	if (tt_Type == TagType) fprintf(f.f, "TYPE ");
	if (IsLocal) fprintf(f.f, "LOCAL ");
	fprintf(f.f, "OF\n\t\t");

	//��������� ��-��� ���������
	bool is_not_first = false;
	SpecStore_type::const_iterator ci;
	for (ci = SpecStore.begin(); ci != SpecStore.end(); ci++) {
		//�������� ������������� ������ �����������
		if (is_not_first) fprintf(f.f, "\n\t\t| "); else is_not_first = true;
		//�������� ������������� ������ ��������
		if ((*ci)->Tag) fprintf(f.f, "%s : ", (*ci)->Tag);
		//�������� ������������� ������ ���������
		if ((*ci)->QualName) fprintf(f.f, "%s.", (*ci)->QualName);
		//������ ����� ��� NIL
		fprintf(f.f, "%s", (*ci)->Name ? (*ci)->Name : "NIL");
	}

	//��� ���������� ���������� ������ ���������
	//�������� ������� ������������� �� ���������
	if (DefaultSpec) {
		fprintf(f.f, "\n\t\tELSE ");
		if (DefaultSpec->Tag) fprintf(f.f, "%s : ", DefaultSpec->Tag);
		if (DefaultSpec->QualName) fprintf(f.f, "%s.", DefaultSpec->QualName);
		fprintf(f.f, "%s", DefaultSpec->Name ? DefaultSpec->Name : "NIL");
	}
	//���������� ������ ���������� ���������� ���������
	fprintf(f.f, "\n\t\tEND");
}


//-----------------------------------------------------------------------------
//���������� ��-�� ��������� � ������
void CCommonExtensionType::AddSpec(const char *newTag, const char *newQual, const char *newName)
{
	SpecStore.push_back(CCommonType::NewSpec(newTag, newQual, newName, true));
}


//-----------------------------------------------------------------------------
//���������� CCommonExtensionType
CCommonExtensionType::~CCommonExtensionType()
{
	delete[] TypeModuleName;
	delete[] TypeName;
	//������� ������ ��-��� ���������
	CCommonType::SpecStore_type::const_iterator ci_s;
	for (ci_s = SpecStore.begin(); ci_s != SpecStore.end(); ci_s++) CCommonType::DelSpec(*ci_s);
}


//-----------------------------------------------------------------------------
//��������� �������� ���������� ��������� � .2ml ����
void CCommonExtensionType::WriteCPP(CPP_files &f, const bool to_h)
{
	//������ ������ �������� ���������
	fprintf(f.f2ml, "\t<Case Qual=\"%s\" Name=\"%s\">\n", TypeModuleName, TypeName);
	//��������� ������������� ��������� (��-��� ���������)
	CCommonType::SpecStore_type::const_iterator ci;
	for (ci = SpecStore.begin(); ci != SpecStore.end(); ci++) {
		fprintf(f.f2ml, "\t\t<Spec");
		if ((*ci)->Tag) fprintf(f.f2ml, " Tag=\"%s\"", (*ci)->Tag);
		if ((*ci)->QualName) fprintf(f.f2ml, " Qual=\"%s\"", (*ci)->QualName);
		if ((*ci)->Name) fprintf(f.f2ml, " Name=\"%s\"", (*ci)->Name);
		fprintf(f.f2ml, " />\n");
	}

	//����� �������� ���������
	fprintf(f.f2ml, "\t</Case>\n");
}


//-----------------------------------------------------------------------------
//������������� ������� CSpecType
int CSpecType::Init(CLexBuf *lb)
{
	//��������� ��������� (Qualident ���������������� � TypeSelector)
	CBaseName* BN = parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident);
	if (!BN || (id_CCommonType != BN->name_id)) return s_e_CommonTypeExpected;

	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//��������� "<" (�� ������� ��������� � TypeSelector)
	if (!lb->ReadLex(li) || lex_k_lt != li.lex) return s_e_OpAngleMissing;

	//�������� ������� ">" (������������ ������������� �� ���������)
	DECL_SAVE_POS
	if (!lb->ReadLex(li) || lex_k_gt != li.lex) {
		RESTORE_POS
	} else {
		//��������� ���������� ������������� �� ���������
		const CCommonType::SSpec* spec = static_cast<CCommonType*>(BN)->GetDefaultSpec();
		if (!spec) return s_e_NoDefaultSpec;
		//���������� ���������� ������������� �� ���������
		if (spec->Tag)
			TagName = str_new_copy(spec->Tag);
		else {
			TagName = str_new_copy(spec->Name);
			if (spec->QualName) QualTagName = str_new_copy(spec->QualName);
		}
		//������������� ���������
		return 0;
	}

	//��������� �������� (� ����������� �� ���� ��������)
	if (CCommonType::tt_Type == static_cast<CCommonType*>(BN)->TagType) {
		//������������� �������� ��������
		CQualident qual;
		int err_num = qual.Init(lb, parent_element);
		if (err_num) return err_num;
		//�������� ����������� ��������
		if (!static_cast<CCommonType*>(BN)->FindSpecByName(qual.pref_ident, qual.ident)) return s_e_SpecTypeTag;
		//���������� �������� ��������
		QualTagName = qual.pref_ident;
		qual.pref_ident = NULL;
		TagName = qual.ident;
		qual.ident = NULL;
	} else {
		//��������� �������� ��������
		if (!lb->ReadLex(li)) return s_e_SpecTypeTag;
		//�������� ����������� ��������
		if (!static_cast<CCommonType*>(BN)->FindSpecByTag(li.st)) return s_e_SpecTypeTag;
		//���������� �������� ��������
		TagName = str_new_copy(li.st);
	}

	//�������� ����������� ������ ">"
	if (!lb->ReadLex(li) || lex_k_gt != li.lex) return s_e_ClAngleMissing;

	return 0;
}//Init CSpecType


//-----------------------------------------------------------------------------
//������ ���� CSpecType
void CSpecType::WriteCPP(CPP_files& f, const bool to_h)
{
	if (Qualident->pref_ident)
		//TYPE type1 = module1.type1
		fprintf(to_h ? f.fh : f.fc, "typedef %s::%s %s;\n", Qualident->pref_ident, Qualident->ident, name);
	else
		//TYPE type1 = type2;
		fprintf(to_h ? f.fh : f.fc, "typedef %s %s;\n", Qualident->ident, name);
}//WriteCPP


//-----------------------------------------------------------------------------
//������ � DFN ���������� ����
void CSpecType::WriteDFN(DFN_file &f)
{
	Qualident->WriteDFN(f);
	fprintf(f.f, "<");
	if (QualTagName) fprintf(f.f, "%s.", QualTagName);
	fprintf(f.f, "%s>", TagName);
}


//-----------------------------------------------------------------------------
//�������� ����� ������� ����
int CSpecType::CreateType(CBaseType *&BaseType) const
{
	CSpecType* ST = new CSpecType(parent_element);
	Qualident->CreateCopy(ST->Qualident);
	if (QualTagName) ST->QualTagName = str_new_copy(QualTagName);
	if (TagName) ST->TagName = str_new_copy(TagName);
	BaseType = ST;
	return 0;
}


//-----------------------------------------------------------------------------
//�������� ���������� �� ����
int CSpecType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	BaseVar = new CCommonVar(parent);
	//��������� �������� ����������� ����
	BaseVar->SetTypeName(Qualident->pref_ident, Qualident->ident);
	//��������� �������� ��������
	return static_cast<CCommonVar*>(BaseVar)->SetTagName(QualTagName, TagName);
}//CSpecType::CreateVar


//-----------------------------------------------------------------------------
//����� ����� � ������ ����� ������, NULL - ���� ��� �� �������
CBaseName* CSpecType::FindName(const char *search_name) const
{
	//��������� ��. �� ���������� ���
	CCommonType* CT = static_cast<CCommonType*>(parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident));
	if (!CT || id_CCommonType != CT->name_id) return NULL;

	//��������� �������� ���� ������������� �� ����������� ���� � ��������
	const CCommonType::SSpec* spec = CT->FindSpec(QualTagName, TagName, TagName);
	if (!spec) return NULL;

	//��������� �������� ������, ����������� ��� �������������
	const char* ModuleName = spec->QualName ? spec->QualName : (spec->IsExtended ? NULL : CT->GetModuleAlias());

	//��������� ���� ������������� �� ������� ������� ��������� �� �������� ���� �������������
	CBaseName* BN = parent_element->GetGlobalName(ModuleName, spec->Name);
	if (!BN || id_CRecordType != BN->name_id) return NULL;

	return static_cast<CRecordType*>(BN)->FindName(search_name);
}


//-----------------------------------------------------------------------------
//����� ����� � ������ ����� ������, NULL - ���� ��� �� �������
const char* CSpecType::GetQualSpecName() const
{
	//��������� ��. �� ���������� ���
	CCommonType* CT = static_cast<CCommonType*>(parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident));
	if (!CT || id_CCommonType != CT->name_id) return NULL;

	//�������� ���� ���������
	if (CCommonType::tt_Type != CT->TagType) return NULL;

	//�������� ��������� ���������
	if (!QualTagName) return GetParentModule()->name;

	//����� ����� ������ �� ����������
	CBaseName* BN = parent_element->GetGlobalName(QualTagName);
	const char* st = QualTagName;
	if (BN && id_CImportModule == BN->name_id) st = static_cast<CImportModule*>(BN)->real_name;

	//������� ���������� �������� ������
	return st;
}


//-----------------------------------------------------------------------------
//�������� ������������� ���� (������� ������������ ����, �� ���. ��������� ��.)
//������� - 0 ��� ��� ������ (��� �� ��������, ������������ ���) c ����������
//������� �� ������ � ������ ������
int CPointerType::CheckComplete(CLexBuf *lb) const
{
	//����������� ������ ������� ����, ������� ��� �� ��� �������� �� ������ ���������� ��.
	if (!forward) return 0;
	//��������� ����
	CBaseName* BN = FindType();
	//�������� ������� ������������ ����
	if (!BN) {
		lb->SetCurrPos(TypePos);
		return s_e_UndeclaredIdent;
	}
	//�������� ������������ ���� (����������� ������ ���� ������, ������ � ���������)
	switch (BN->name_id) {
	case id_CArrayType:
		return static_cast<CArrayType*>(BN)->CheckComplete(lb);
	case id_CRecordType:
		return static_cast<CRecordType*>(BN)->CheckComplete(lb);
	case id_CCommonType:
	case id_CSpecType:
		return 0;
	default:
		lb->SetCurrPos(TypePos);
		return s_e_PointerTypeKind;
	}//switch
}


//-----------------------------------------------------------------------------
//���������� ������� CPointerType
CPointerType::~CPointerType()
{
	delete Type;
	delete Qualident;
}//~CPointerType


//-----------------------------------------------------------------------------
//������������� ������� CPointerType
int CPointerType::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� ��. ����� TO
	if (!lb->ReadLex(li) || lex_k_TO != li.lex) return s_e_TO;

	//��� �������� ������� QualidentType
	DECL_SAVE_POS

	//��������� ����. �������
	if (!lb->ReadLex(li)) return s_e_PointerTypeKind;

	//�������� ������� Qualident, ARRAY ��� RECORD
	switch (li.lex) {

	//������ �����. (��� Qualident), ��� ��. �� ����������� ���
	case lex_i:
		{
			RESTORE_POS
			//�������� � ������������� ����� ����
			Qualident = new CQualident;
			int err_num = Qualident->Init(lb, parent_element);
			if (err_num) return err_num;
			//�������� ���������� �������� � Qualident �������� ����
			if (id_CBaseName != Qualident->TypeResultId) return s_e_PointerTypeKind;
			//����� ����� ����, �� ���. ��������� ���������
			CBaseType* BT = FindType();
			//�������� �������� ��� ������������ ����
			if (BT) {
				//�������� �������� ������������� ����
				switch (BT->name_id) {
				case id_CArrayType:
				case id_CRecordType:
				case id_CSpecType:
					break;
				case id_CCommonType:
					//�������� ������� �������������� SpecType
					SAVE_POS
					err_num = !lb->ReadLex(li) || lex_k_lt != li.lex;
					RESTORE_POS
					if (!err_num) {
						//������ SpecType, �������������� ��� (���������� ��� ����������� Qualident)
						Type = new CSpecType(parent_element);
						static_cast<CSpecType*>(Type)->Qualident = Qualident;
						Qualident = NULL;
						err_num = Type->Init(lb);
						if (err_num) return err_num;
					}
					break;
				default:
					return s_e_PointerTypeKind;
				}//switch
			} else {
				//��� ��� �� ��������, ������������� ������� ������������� ������������ ��������
				forward = true;
				//���������� ������� � ������ ������ (�� ������ ������)
				TypePos = lb->GetCurrPos();
			}
			return 0;
		}

	//������� �������� �������������� ���� ARRAY
	case lex_k_ARRAY:
		Type = new CArrayType(parent_element);
		return Type->Init(lb);

	//������� �������� �������������� ���� RECORD (������ ��� ���������)
	case lex_k_RECORD:
		SAVE_POS
		if (!lb->ReadLex(li) || lex_k_CASE != li.lex) {
			RESTORE_POS
			Type = new CRecordType(parent_element);
		} else
			Type = new CCommonType(parent_element);
		//������������� ���������� ����
		return Type->Init(lb);

	//������������ ��� ���������
	default:
		return s_e_PointerTypeKind;
	}//switch
}//CPointerType::Init


//-----------------------------------------------------------------------------
//������ ���� CPointerType
void CPointerType::WriteCPP(CPP_files& f, const bool to_h)
{
	//� ���� C++ ��������� �������������� ���� ������ �� ����� ���� �������� ��������
	//��������� �������������� ���� ������ �������� ��������� (?)

	//��� ��������� ���� ��. �� ������������� ��� ��� ����� ������� ��������, ����� ������������� �����������
	//��������, � ����������� �� ����

	if (Type) {
		//��������� ���� ����, �� ���. ��������� ���������
		//��������� ����� (��� �������� RuntimeId � ��������� ����) - ������� ��������, �.�. InitRuntimeId ��� WriteCPP ����� ������������� ����������
		Type->name = name;
		try {
			//��� ���� RECORD ���������� RuntimeId
			if (id_CRecordType == Type->name_id) static_cast<CRecordType*>(Type)->InitRuntimeId();
			Type->WriteCPP(f, to_h);
		}
		catch(error_Internal) {
			Type->name = NULL;
			throw;
		};
		//������� ������������� ���
		Type->name = NULL;
	} else {
		//����������� ��� - ���������� ��� ��� ���������� ���� POINTER TO

		//��������� ���� �� ��� ����� (� ������ ������� ���������� �� ����� �������������)
		CBaseType* BT = static_cast<CBaseType*>(parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident));
		const char* name_alias;
		TFileType* ff;
		//��������, ���� ����� �������� ����������
		if (to_h) {
			name_alias = BT->GetModuleName();
			ff = f.fh;
		} else {
			name_alias = BT->GetModuleAlias();
			ff = f.fc;
		}

		//�������� ������������� ��������� ������������ ��������
		if (forward) {
			if (id_CRecordType == BT->name_id || id_CCommonType == BT->name_id) {
				//��������� ���������� ���� � ������ ������ (���������)
				fprintf(ff, "struct ");
				if (name_alias) fprintf(ff, "%s::", name_alias);
				fprintf(ff, "%s;\n", Qualident->ident);
			} else {
				//� ������ ������� ��������� ���������� ��� (� ����� �� ����� ������������ ������)
				if (id_CArrayType != BT->name_id) throw error_Internal("CPointerType::WriteCPP");
				static_cast<CArrayType*>(BT)->WriteCPP(f, to_h);
			}
		}

		//���������� ��������� ���� ���������� ����
		fprintf(ff, "typedef ");
		if (name_alias) fprintf(ff, "%s::", name_alias);
		fprintf(ff, "%s %s;\n", Qualident->ident, name);
	}//else
}


//-----------------------------------------------------------------------------
//������ � DFN ���������� ����
void CPointerType::WriteDFN(DFN_file& f)
{
	fprintf(f.f, "POINTER TO ");
	//������ ���������� �������������� ��� ����� ������������ ����
	if (Type)
		Type->WriteDFN(f);
	else
		Qualident->WriteDFN(f);
}//WriteDFN


//-----------------------------------------------------------------------------
//�������� ����� ������� ����
int CPointerType::CreateType(CBaseType* &BaseType) const
{
	//�������� ������ ����
	BaseType = new CPointerType(parent_element);
	//����������� ���������� ��������� �������
	static_cast<CPointerType*>(BaseType)->forward = forward;
	static_cast<CPointerType*>(BaseType)->TypePos = TypePos;
	if (Qualident)
		Qualident->CreateCopy(static_cast<CPointerType*>(BaseType)->Qualident);
	if (Type)
		Type->CreateType(static_cast<CPointerType*>(BaseType)->Type);

	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//�������� ���������� �� ����
int CPointerType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	//�������� �������� ������������ ���� (� ���� ������ �������� ��� ����, ��� ��� ����� ��� �� ���� ������)
	if (Qualident) {
		//������� ����������
		BaseVar = new CPointerVar(parent);
		//������������� �������� ����
		BaseVar->SetTypeName(Qualident->pref_ident, Qualident->ident);
	} else {
		//��� ��������� �� ������������� ���, ������� ��� ����� � ������� � ����������
		CBaseType* BT;
		int err_num = Type->CreateType(BT);
		if (err_num) return err_num;
		//������� ���������� � ��������� �������� �� ����
		BaseVar = new CPointerVar(parent, BT);
	}

	//��������� ������� ������� � ������ ������ (���� ����) � ����������
	if (forward) static_cast<CPointerVar*>(BaseVar)->TypePos = TypePos;

	return 0;
}//CPointerType::CreateVar


//-----------------------------------------------------------------------------
//��������� ����, �� ���. ��������� ���������
CBaseType* CPointerType::FindType() const
{
	//�������� ������� �������������� ����
	if (Type) return Type;

	//����� �� ������� ����������� �����
	CBaseName* BN = parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident);
	while (BN && id_CQualidentType == BN->name_id)
		BN = BN->parent_element->GetGlobalName(static_cast<CQualidentType*>(BN)->Qualident->pref_ident, static_cast<CQualidentType*>(BN)->Qualident->ident);

	//������� ���������� ���� (��� NULL)
	return static_cast<CBaseType*>(BN);
}


//-----------------------------------------------------------------------------
//��������� ���� ���������� ��������� (��� ��������� - �������) ��� id_CBaseName
EName_id CProcedureType::GetResultId() const
{
	//�������� ����������-��������� (�� ����� ���� ����������)
	if (!FormalPars.Qualident) return id_CBaseName;
	//������� ���� ���������� ���������
	return FormalPars.Qualident->TypeResultId;
}


//-----------------------------------------------------------------------------
//������������� ������� CProcedureType
int CProcedureType::Init(CLexBuf *lb)
{
	return FormalPars.Init(lb, parent_element);
}


//-----------------------------------------------------------------------------
//������ ���� ���������� ������������ ���� (CProcedureType)
void CProcedureType::WriteCPP(CPP_files& f, const bool to_h)
{
	//��� ��������� ������ �������� ����� (.h ��� .cpp)
	TFileType* const ff = to_h ? f.fh : f.fc;

	fprintf(ff, "typedef ");
	//������ ���� ��� ������� (���� ����) ��� void
	if (FormalPars.Qualident) FormalPars.Qualident->WriteCPP_type(f, to_h, parent_element);
	else fprintf(ff, "void");
	fprintf(ff, " (*%s) (", name);
	FormalPars.WriteCPP_pars(f, to_h);
	fprintf(ff, ");\n");
}


//-----------------------------------------------------------------------------
//������ � DFN ���������� ����
void CProcedureType::WriteDFN(DFN_file& f)
{
	fprintf(f.f, "PROCEDURE");

	//������ ���������� ���������� ����������
	fprintf(f.f, " (\n\t\t");
	FormalPars.WriteDFN(f);
	fprintf(f.f, ")");
	//������ ���� ��������� (���� ����)
	FormalPars.WriteDFN_type(f);
}//WriteDFN


//-----------------------------------------------------------------------------
//�������� ����� ������� ����
int CProcedureType::CreateType(CBaseType* &BaseType) const
{
	//�������� ������ ����
	BaseType = new CProcedureType(parent_element);
	//����������� ������ ���������� ����������
	FormalPars.Assign(static_cast<CProcedureType*>(BaseType)->FormalPars, parent_element);
	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//�������� ���������� �� ����
int CProcedureType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	//�������� ����������� ����������
	BaseVar = new CProcedureVar(parent);
	//��������� �������� ���� � ������ ������������ ����
	BaseVar->SetTypeName(GetModuleName(), name);

	//����������� ���������� ����������
	FormalPars.Assign(static_cast<CProcedureVar*>(BaseVar)->FormalPars, parent_element);

	return 0;
}//CProcedureType::CreateVar


//-----------------------------------------------------------------------------
//���������� ������� CReceiver
CReceiver::~CReceiver()
{
	delete[] name;
	delete[] type_name;
	delete Recv;
}


//-----------------------------------------------------------------------------
//����� ����� � ������� ����, NULL - ���� ��� �� �������
CBaseName* CReceiver::FindName(const char *search_name) const
{
	if (!strcmp(search_name, name)) return Recv;
	return NULL;
}


//-----------------------------------------------------------------------------
//������������� ������� CReceiver
int CReceiver::Init(CLexBuf *lb, const CBaseName* parent_element)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� �������
	if (!lb->ReadLex(li)) return s_e_IdentExpected;

	//�������� ������� VAR
	if (lex_k_VAR == li.lex) {
		is_var = true;
		if (!lb->ReadLex(li)) return s_e_IdentExpected;
	}

	//�������� ������� ��.
	if (lex_i != li.lex) return s_e_IdentExpected;

	//���������� ���������� ���
	name = str_new_copy(li.st);

	//�������� ������� ":"
	if (!lb->ReadLex(li) || lex_k_colon != li.lex) return s_e_ColonMissing;

	//�������� ������� ��. ��� ��. �����
	if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;

	//�������� ������������� ������� ���� � ���������� ������� ���������
	CBaseName* RType = parent_element->GetGlobalName(li.st);
	if (!RType) return s_e_UndeclaredIdent;

	//�������� ���� � ������� ���������� (VAR) ���������
	switch (RType->name_id) {
	case id_CRecordType:	//��� ������, ������ ���� �������� ��� VAR
		if (!is_var) return s_e_ReceiverVAR;
		break;
	case id_CPointerType:	//��� ��., ������ ���� �������� ��� VAR
		if (is_var) return s_e_ReceiverVAR;
		//�������� ���� ��������� (������ ���� ��. �� ������)
		if (id_CRecordType != static_cast<CPointerType*>(RType)->FindType()->name_id) return s_e_ReceiverVAR;
	}

	//����������� ���������� - ��������� (��� ���������, ��� ������ ���)
	int err_num = static_cast<CBaseType*>(RType)->CreateVar(Recv, parent_element);
	if (err_num) return err_num;
	//� ������ ��. ��������� �������� ��. �� ������ (��� ���������)
	if (id_CPointerVar == Recv->name_id) static_cast<CPointerVar*>(Recv)->SetIsRecord();
	//����������� �������� ��������
	Recv->external = RType->external;
	//����������� � Recv ����� ��������� (��� ���������� ������ � Recv ��� � ������� ����������� ����������)
	Recv->SetName(name);
	//��������� �������� VAR ��� ������
	Recv->is_var = is_var;

	//���������� ���������� ��� ����
	type_name = str_new_copy(li.st);

	//�������� ������� ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init CReceiver


//-----------------------------------------------------------------------------
//������ ���� ��������� (���������� �������)
void CReceiver::WriteCPP_fp(CPP_files &f, const bool external, const bool single_param)
{
	fprintf(f.fc, "%s* %s%s", type_name, name, single_param ? "" : ", ");
	if (external) fprintf(f.fh, "%s* %s%s", type_name, name, single_param ? "" : ", ");
}


//-----------------------------------------------------------------------------
//���������� ������� CFPSection
CFPSection::~CFPSection()
{
	//������� ������ �����
	TmpIdents_type::const_iterator ci;
	for (ci = tmp_idents.begin(); ci != tmp_idents.end(); ci++)
		delete[] *ci;
	//������� ����
	delete BaseType;
}//~CFPSection


//-----------------------------------------------------------------------------
//������������� ������ �������� FPVector, � ReceiverName - ��� ��������� ��� NULL
int CFPSection::AssignFPElems(CLexBuf *lb, CBaseVarVector &FPVector, const char* ReceiverName)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������ ������� (������ ���� ������������� ��� VAR)
	if (!lb->ReadLex(li) || (lex_k_VAR != li.lex && lex_i != li.lex)) return s_e_IdentExpected;

	//�������� ������� "VAR", �� ��� ������ ��������� �������������
	if (is_var = (lex_k_VAR == li.lex))
		if (!lb->ReadLex(li) || lex_i != li.lex)
			return s_e_IdentExpected;

	while (true) {
		//�������� ���������� ���������� ����� � ������ ��������� (���� ���� ��� ���������)
		if (ReceiverName)
			if (!strcmp(ReceiverName, li.st))
				return s_e_Redefinition;

		//�������� ���������� ����� � ������ ���� FPSection
		TmpIdents_type::const_iterator ci;
		for (ci = tmp_idents.begin(); ci != tmp_idents.end(); ++ci)
			if (!strcmp(*ci, li.st))
				return s_e_Redefinition;

		//�������� ���������� ����� � ������ ���� FormalPars
		CBaseVarVector::const_iterator fp_ci;
		for (fp_ci = FPVector.begin(); fp_ci != FPVector.end(); ++fp_ci)
			if ( !strcmp((*fp_ci)->name, li.st) )
				return s_e_Redefinition;

		//���������� ��������� ��������� ���
		tmp_idents.push_back(str_new_copy(li.st));

		//�������� ���������� ","
		if (!lb->ReadLex(li) || lex_k_comma != li.lex) break;

		//���������� ��������� �������
		if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	}//while

	//�������� ������� ":"
	if (lex_k_colon != li.lex) return s_e_ColonMissing;

	//�������� ������� ���� �� ������ ������
	int err_num = TypeSelector(lb, BaseType, parent_element);
	if (err_num) return err_num;
	if (!BaseType) return s_m_Error;

	//�������� ����������� ���������� ���������� �� ���������� ����
	TmpIdents_type::iterator it;
	for (it = tmp_idents.begin(); it != tmp_idents.end(); ++it) {
		CBaseVar* FPVar = NULL;
		//�������� ������ ��-�� ������ ���������� ����������
		err_num = BaseType->CreateVar(FPVar, parent_element);
		if (err_num) {
			delete FPVar;
			return err_num;
		}

		//�������� ������� ������������������� ���������� ����������
		if (id_CCommonVar == FPVar->name_id && static_cast<CCommonVar*>(FPVar)->IsPureCommon()) {
			//����������� ���������� ����������
			delete FPVar;
			//��� ���������� VAR - ������� ���������� ������������������� ���������� ����������,
			//��� ���������� QualidentType - ������� ���������� �������������� ����������� ���� � ���������� ���������
			if (!is_var || id_CQualidentType != BaseType->name_id) return s_e_SpecTypeExpected;
			//�������� ���������� ���� ��������� �� ������������������� ���������
			FPVar = new CPointerVar(parent_element);
			const CQualident* Q = static_cast<CQualidentType*>(BaseType)->Qualident;
			FPVar->SetTypeName(Q->pref_ident, Q->ident);
		}

		//������������� ������ ��-�� ������ ���������� ����������
		FPVar->is_var = is_var;
		FPVar->name = *it;
		*it = NULL;
		//��������� ���������� ��-�� � ������
		FPVector.push_back(FPVar);
	}

	return 0;
}


//-----------------------------------------------------------------------------
//���������� ������� CFormalPars
CFormalPars::~CFormalPars()
{
	delete Qualident;
	CBaseVarVector::const_iterator ci;
	for (ci = FPStore.begin(); ci != FPStore.end(); ++ci) delete *ci;
}//~CFormalPars


//-----------------------------------------------------------------------------
//����������� ����������� ������� � ��������� ���������� ���������
void CFormalPars::Assign(CFormalPars& FP, const CBaseName* parent_element) const
{
	//����������� ������ ����������
	CBaseVarVector::const_iterator ci = FPStore.begin();
	for (; ci != FPStore.end(); ci++)
		FP.FPStore.push_back((*ci)->CreateVar(parent_element));

	//����������� �������� ���� (���� ����)
	if (Qualident) Qualident->CreateCopy(FP.Qualident);
}


//-----------------------------------------------------------------------------
//������������� ������� CFormalPars
int CFormalPars::Init(CLexBuf *lb, const CBaseName* parent_element)
{
	DECL_SAVE_POS
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� "("
	if (lb->ReadLex(li) && lex_k_op_bracket == li.lex) {
		SAVE_POS

		//��� �������� ����� ��������� � ������ ���������, ��������� � �����
		const char* ReceiverName = NULL;
		if (CProcedure::IsProcId(parent_element->name_id))
			if (static_cast<const CProcedure*>(parent_element)->Receiver)
				ReceiverName = static_cast<const CProcedure*>(parent_element)->Receiver->name;

		//�������� ���������� ")" (���� ������ ���������� ����������)
		if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) {
			RESTORE_POS

			while (true) {
				//�������� ������� FPSection
				CFPSection FPSection (parent_element);
				int err_num = FPSection.AssignFPElems(lb, FPStore, ReceiverName);
				if (err_num) return err_num;
	
				//�������� ������� ";"
				if (!lb->ReadLex(li) || lex_k_semicolon != li.lex) break;
			}

			//�������� ������� ")" (� ����� ������ ���������� ����������)
			if (lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

			//���������� ���. �������� ������������ ���������� ����������
			int err_num = CheckCompleteRoot(lb);
			if (err_num) return err_num;

		}//if

	} else {
		//����������� "(" => ��� ������ ���������� ����������
		RESTORE_POS
		//��������� ���������, �.�. ��� ���������� ����� ���� ������� ������ ����� "()"
		return 0;
	}

	//�������� ������� ����������-�������� ���� ������ (��������� ��� ����������� ������������� ������������� ��������)
	CBaseVarVector::const_iterator ci;
	for (ci = FPStore.begin(); ci != FPStore.end(); ++ci)
		if (id_CArrayVar == (*ci)->name_id && !(*ci)->is_var) {
			have_arrays = true;
			break;
		}

	//�������� ������� ":" (���������� ���� ������������� ����������)
	SAVE_POS
	if (!lb->ReadLex(li) || lex_k_colon != li.lex) {
		RESTORE_POS
		return 0;
	}

	//�������� �������� ����� ���� ������������� ����������
	Qualident = new CQualident;
	return Qualident->InitTypeName(lb, parent_element);

}//Init CFormalPars


//-----------------------------------------------------------------------------
//������ ���� CFormalPars (��� ���������)
void CFormalPars::WriteCPP_type(CPP_files& f, const bool to_h, const CBaseName* parent_element)
{
	//���������� ������ ���� ��������� (�������)
	if (!Qualident) {
		fprintf(to_h ? f.fh : f.fc, "void ");
	} else {
		Qualident->WriteCPP_type(f, to_h, parent_element);
		fprintf(to_h ? f.fh : f.fc, " ");
	}
}//WriteCPP_type


//-----------------------------------------------------------------------------
//������ ���������� ���������� ���������� ��������� � dfn ����
void CFormalPars::WriteDFN(DFN_file& f)
{
	//������ ���� ���������� (���� ����) ���������
	if (!FPStore.empty()) {
		CBaseVarVector::const_iterator ci = FPStore.begin();
		while (true) {
			f.tab();
			if ((*ci)->is_var) fprintf(f.f, "VAR ");
			(*ci)->WriteDFN(f);
			++ci;
			if (FPStore.end() == ci) break;
			fprintf(f.f, ";\n");
		}
	}
}//WriteDFN


//-----------------------------------------------------------------------------
//������ ���� ��������� � dfn ����
void CFormalPars::WriteDFN_type(DFN_file& f)
{
	//������ ���� ���� ��������� (���� ����)
	if (Qualident) {
		fprintf(f.f, " : ");
		Qualident->WriteDFN(f);
	}
}//WriteDFN


//-----------------------------------------------------------------------------
//������ ���� CFormalPars (���������� ���������)
void CFormalPars::WriteCPP_pars(CPP_files& f, const bool to_h)
{
	//�������� ������� ���������� ����������
	if (FPStore.empty()) return;
	//������ ������� ����������� ���������
	CBaseVarVector::const_iterator i = FPStore.begin();
	(*i)->WriteCPP_fp(f, to_h);
	//������ ��������� ���������� ����������
	for(++i; i != FPStore.end(); ++i) {
		fprintf(to_h ? f.fh : f.fc, ", ");
		(*i)->WriteCPP_fp(f, to_h);
	}
}//WriteCPP_pars


//-----------------------------------------------------------------------------
//������ ������ ���� ���������� ���������� ����� ","
void CFormalPars::WriteCPP_names(CPP_files& f, const bool to_h)
{
	//�������� ������� ���������� ����������
	if (FPStore.empty()) return;
	//������ ������� ����������� ���������
	CBaseVarVector::const_iterator ci = FPStore.begin();
	fprintf(to_h ? f.fh : f.fc, "%s", (*ci)->name);
	//������ ��������� ���������� ����������
	for(++ci; ci != FPStore.end(); ++ci) {
		fprintf(to_h ? f.fh : f.fc, ", ");
		fprintf(to_h ? f.fh : f.fc, "%s", (*ci)->name);
	}
}


//-----------------------------------------------------------------------------
//��������� ���� ��� ������������� ��������-��������
void CFormalPars::WriteCPP_begin(CPP_files& f)
{
	//�������� ���������� �������� � �������� ����������-��������
	if (!have_arrays) return;

	f.tab_level_c++;
	f.tab_fc();
	fprintf(f.fc, "//arrays initialisation\n");

	//������� ������ ���������� ����������
	CBaseVarVector::const_iterator ci;
	for (ci = FPStore.begin(); ci != FPStore.end(); ++ci)
		//�������� ������� ���������-�������� ���� ������
		if ( ((*ci)->name_id == id_CArrayVar)&&(!(*ci)->is_var) ) {

			//��������� ��. �� ��� ������� ��� ������� ����������
			CArrayType *AT = static_cast<CArrayVar*>(*ci)->ArrayType;
			//�������� ������� ��������� �������
			if (AT->size == 0) {	//�������� ��������� ������� � ������������� ���������� ��������
				AT = static_cast<CArrayType*>(AT->FindLastType());
				//���������� ��������� ���������� ��� ����������� ����������
				CBaseVar *BV;
				AT->CreateVar(BV, AT->parent_element);
				f.tab_fc();
				const char* module_alias = BV->GetTypeModuleAlias();
				if (module_alias) fprintf(f.fc, "%s::", module_alias);
				fprintf(f.fc, "%s *%s;\n{\n", BV->GetTypeName(), (*ci)->name);
				//��������� ���� ��������� ������ ��� ������
				fprintf(f.fc, "\tconst int O2M_COUNT = O2M_ARR_0_%s", (*ci)->name);
				int dimention = 1;
				AT = static_cast<CArrayType*>( static_cast<CArrayVar*>(*ci)->ArrayType->Type );
				while (AT->name_id == id_CArrayType) {
					fprintf(f.fc, "*O2M_ARR_%i_%s", dimention, (*ci)->name);
					++dimention;
					AT = static_cast<CArrayType*>(AT->Type);
				}
				fprintf(f.fc, ";\n\t%s = new ", (*ci)->name);
				if (module_alias) fprintf(f.fc, "%s::", module_alias);
				fprintf(f.fc, "%s[O2M_COUNT];\n", BV->GetTypeName());
				//��������� ����������������� ����
				fprintf(f.fc, "\tfor(int O2M_I=0; O2M_I<O2M_COUNT; ++O2M_I) %s[O2M_I] = O2M_ARR_%s[O2M_I];\n}\n", (*ci)->name, (*ci)->name);
				delete BV;

			} else {	//�������� �������� ������� � ������������� ���������� ��������

				//���������� ��������� ���������� ����������� ����������
				f.tab_fc();
				(*ci)->WriteCPP(f);
				fprintf(f.fc, ";\n{");
				//��������� ����������������� ����
				int dimention = 0;
				while (AT->name_id == id_CArrayType) {
					//��������� ���� ����� �������� ������� �� ���������� ���������
					fprintf(f.fc, "\n\tfor(int O2M_I_%i = 0; O2M_I_%i < %i; ++O2M_I_%i)", dimention, dimention, AT->size, dimention);
					++dimention;
					AT = static_cast<CArrayType*>(AT->Type);
				}
				//��������� ���� ������������� �������� ��-�� �������
				fprintf(f.fc, " %s", (*ci)->name);
				int i;
				for (i = 0; i < dimention; i++) fprintf(f.fc, "[O2M_I_%i]", i);
				fprintf(f.fc, " = O2M_ARR_%s", (*ci)->name);
				for (i = 0; i < dimention; i++) fprintf(f.fc, "[O2M_I_%i]", i);
				fprintf(f.fc, ";\n}\n");
			}//else
		}//if

	f.tab_level_c--;
}//WriteCPP_arrays


//-----------------------------------------------------------------------------
//��������� ���� ��� ��������������� ��������-��������
void CFormalPars::WriteCPP_end(CPP_files& f, const bool ret_present)
{
	//� ������ ������� ��������� RETURN ��������� ����� ��� ��������� ��� ������
	if (ret_present) {
		f.tab_fc();
		fprintf(f.fc, "\tO2M_RETURN:;\n");
	}

	//�������� ���������� �������� � �������� ����������-��������
	if (!have_arrays) return;

	f.tab_level_c++;

	f.tab_fc();
	fprintf(f.fc, "//delete arrays\n");

	//������� ������ ���������� ����������
	CBaseVarVector::const_iterator ci;
	for (ci = FPStore.begin(); ci != FPStore.end(); ++ci)
		//�������� ������� ���������-�������� ���� ������
		if ( ((*ci)->name_id == id_CArrayVar) && (!(*ci)->is_var) )
			if (static_cast<CArrayVar*>(*ci)->ArrayType->size == 0) {
				f.tab_fc();
				fprintf(f.fc, "delete[] %s;\n", (*ci)->name);
			}

	f.tab_level_c--;
}//WriteCPP_delete


//-----------------------------------------------------------------------------
//������������� ������ �������� FPVector, � ReceiverName - ��� ��������� ��� NULL
int CCommonFPSection::AssignFPElems(CLexBuf *lb, CBaseVarVector &FPVector)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������ ������� (������ ���� ������������� ��� VAR)
	if (!lb->ReadLex(li) || (lex_k_VAR != li.lex && lex_i != li.lex)) return s_e_IdentExpected;

	//��� ��������� ������� ������ ��� �������� ����������/������� VAR
	DECL_SAVE_POS

	//�������� ������� "VAR", �� ��� ������ ��������� �������������
	if (is_var = (lex_k_VAR == li.lex))
		if (!lb->ReadLex(li) || lex_i != li.lex)
			return s_e_IdentExpected;

	while (true) {

		//�������� ���������� ����� � ������ ���� FPSection
		TmpIdents_type::const_iterator ci;
		for (ci = tmp_idents.begin(); ci != tmp_idents.end(); ++ci)
			if (!strcmp(*ci, li.st))
				return s_e_Redefinition;

		//�������� ���������� ����� � ������ ���� FormalPars
		CBaseVarVector::const_iterator fp_ci;
		for (fp_ci = FPVector.begin(); fp_ci != FPVector.end(); ++fp_ci)
			if ( !strcmp((*fp_ci)->name, li.st) )
				return s_e_Redefinition;

		//���������� ��������� ��������� ���
		tmp_idents.push_back(str_new_copy(li.st));

		//�������� ���������� ","
		if (!lb->ReadLex(li) || lex_k_comma != li.lex) break;

		//���������� ��������� �������
		if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	}//while

	//�������� ������� "<" (������� ������� ����������� �����. ����-���)
	if (lex_k_lt == li.lex) return s_m_HProcFound;

	//�������� ������� ":"
	if (lex_k_colon != li.lex) return s_e_ColonMissing;

	//�������� ������� ���� �� ������ ������
	int err_num = TypeSelector(lb, BaseType, parent_element);
	if (err_num) return err_num;
	if (!BaseType) return s_m_Error;


	/////////////////////////////////////
	//�������� ��������� ����������� ����

	//��� ��������� ����������� ���� ����� ����������� ��� ��� ���������
	CBaseType* BT;

	//�������� ����������� ����
	switch (BaseType->name_id) {
	case id_CQualidentType:
		//��������� ���� ����� ����������� ���
		err_num = static_cast<CQualidentType*>(BaseType)->GetNamedType(BT, false);
		if (err_num) return err_num;
		break;
	case id_CPointerType:
		//��������� BT ��� ���������� �������� ���� ���������
		BT = BaseType;
		break;
	case id_CSpecType:
		//��������� ���������� �����. ����-���
		return s_m_HProcFound;
	default:
		return s_e_CommonTypeExpected;
	}

	//�������� ����, ����������� �� ������������, ��� �������� ���� ���������
	switch (BT->name_id) {
	case id_CPointerType:
		//��������� ���� ����� ��� ���������
		BT = static_cast<CPointerType*>(BT)->FindType();
		if (id_CHandlerProc == BT->name_id) return s_m_HProcFound;
		if (id_CCommonType != BT->name_id) return s_e_CommonTypeExpected;
		if (is_var) {
			RESTORE_POS
			return s_e_CommonParVAR;
		}
		break;
	case id_CCommonType:
		if (!is_var) {
			RESTORE_POS
			return s_e_CommonParVAR;
		}
		break;
	case id_CSpecType:
		return s_m_HProcFound;
	default:
		return s_e_CommonTypeExpected;
	}


	///////////////////////////////////////////////////////////////
	//�������� ����������� ���������� ���������� �� ���������� ����
	TmpIdents_type::iterator it;
	for (it = tmp_idents.begin(); it != tmp_idents.end(); ++it) {
		CBaseVar* FPVar = NULL;
		//������������� ������ ��-�� ������ ���������� ����������
		err_num = BaseType->CreateVar(FPVar, parent_element);
		if (err_num) {
			delete FPVar;
			return err_num;
		}
		if (!FPVar)
			return s_m_Error;
		FPVar->is_var = is_var;
		FPVar->name = *it;
		*it = NULL;
		//��������� ���������� ��-�� � ������
		FPVector.push_back(FPVar);
	}

	return 0;
}


//-----------------------------------------------------------------------------
//����������
CSpecFPSection::~CSpecFPSection()
{
	//������� ������ ����������� ������������������ ����������
	TSFPElemStore::const_iterator ci;
	for (ci = SFPElemStore.begin(); ci != SFPElemStore.end(); ci++)
		delete *ci;
	//������� ����
	delete BaseType;
}


//-----------------------------------------------------------------------------
//������������� ������ �������� FPVector
int CSpecFPSection::AssignFPElems(CLexBuf *lb, CBaseVarVector &FPVector)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������ ������� (������ ���� ������������� ��� VAR)
	if (!lb->ReadLex(li) || (lex_k_VAR != li.lex && lex_i != li.lex)) return s_e_IdentExpected;

	//�������� ������� "VAR", �� ��� ������ ��������� �������������
	if (is_var = (lex_k_VAR == li.lex))
		if (!lb->ReadLex(li) || lex_i != li.lex)
			return s_e_IdentExpected;

	//������� ������������� ������� ����������� (��������������������) ����
	bool CommonTypeNeed = false;

	while (true) {
		//�������� ���������� ����� � ������ ���� SpecFPSection
		TSFPElemStore::const_iterator ci;
		for (ci = SFPElemStore.begin(); ci != SFPElemStore.end(); ++ci)
			if (!strcmp((*ci)->ident, li.st)) return s_e_Redefinition;

		//�������� ���������� ����� � ������ ���� FormalPars
		CBaseVarVector::const_iterator fp_ci;
		for (fp_ci = FPVector.begin(); fp_ci != FPVector.end(); ++fp_ci)
			if (!strcmp((*fp_ci)->name, li.st)) return s_e_Redefinition;

		//�������� ��-�� ��� �������� ���������� � ������� ���������
		SSFPElem* fpe = new SSFPElem;
		fpe->ident = str_new_copy(li.st);
		fpe->IsNeedDefaultSpec = false;
		fpe->QualTagName = NULL;
		fpe->TagName = NULL;
		//���������� ������� ������� (��� ����������� ������� ������ ��� �������� �������� ��������)
		fpe->pos = lb->GetCurrPos();
		//���������� ���������� �������������� � ������ ��-���
		SFPElemStore.push_back(fpe);

		//��������� ����. �������
		if (!lb->ReadLex(li)) return s_e_ColonMissing;

		//�������� ������� "," (������� � ��������� ����. ��-��)
		if (lex_k_comma == li.lex) {
			if (CommonTypeNeed) return s_e_OpAngleMissing;
			if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
			continue;
		}

		//�������� ���������� "<" (����� ��������� ������ ����������)
		if (lex_k_lt != li.lex) break;

		//���� "<", ��������� ���������� (�������������������) ���
		CommonTypeNeed = true;

		//�������� ������� ">" (������������ ������������� �� ���������)
		DECL_SAVE_POS
		if (!lb->ReadLex(li) || lex_k_gt != li.lex) {
			//">" �����������, ��������������� ������� � ������ ������
			RESTORE_POS
			//������������� �������� ��������
			CQualident qual;
			int err_num = qual.Init(lb, parent_element);
			if (err_num) return err_num;
			//���������� �������� ��������
			fpe->QualTagName = qual.pref_ident;
			fpe->TagName = qual.ident;
			qual.pref_ident = NULL;
			qual.ident = NULL;
			//�������� ������� ">"
			if (!lb->ReadLex(li) || lex_k_gt != li.lex) return s_e_ClAngleMissing;
		} else	//��������� �������� ������������� ������������� ����-��� �� ���������
			fpe->IsNeedDefaultSpec = true;

		//�������� ���������� ","
		if (!lb->ReadLex(li) || lex_k_comma != li.lex) break;

		//���������� ��������� �������
		if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	}//while

	//�������� ������� ":"
	if (lex_k_colon != li.lex) return s_e_ColonMissing;

	//�������� ������� ���� �� ������ ������
	int err_num = TypeSelector(lb, BaseType, parent_element);
	if (err_num) return err_num;
	//��. �� ��� ��� ��������� ������ QualidentType
	CBaseType* BT = BaseType;
	if (id_CQualidentType == BT->name_id) {
		//� ������ QualidentType �������� ����������� ���
		err_num = static_cast<CQualidentType*>(BT)->GetNamedType(BT, false);
		if (err_num) return err_num;
	}

	//�������� ������� ���������� ����������� (�������� �������������������) ����
	if (CommonTypeNeed) {
		if (id_CCommonType != BT->name_id) return s_e_CommonTypeExpected;
	} else
		if (id_CSpecType != BT->name_id) return s_e_SpecTypeExpected;

	//�������� ����������� ���������� ���������� �� ���������� ����
	TSFPElemStore::const_iterator ci;
	for (ci = SFPElemStore.begin(); ci != SFPElemStore.end(); ++ci) {
		//�������� ���������� ���������� �������� � �������� ��-�� (������� �������� ����� �� �� ��������� ��������� ����)
		if (!(*ci)->TagName && !(*ci)->IsNeedDefaultSpec && CommonTypeNeed) {
			lb->SetCurrPos((*ci)->pos);
			lb->ReadLex(li);
			return s_e_OpAngleMissing;
		}
		//��. �� ����������� ����������
		CBaseVar* FPVar = NULL;
		//������������� ������ ��-�� ������ ���������� ����������
		err_num = BT->CreateVar(FPVar, parent_element);
		if (err_num) {
			delete FPVar;
			return err_num;
		}
		//��������� ������� ��������� ����������
		FPVar->is_var = is_var;
		FPVar->name = (*ci)->ident;
		(*ci)->ident = NULL;

		//��� ������� ����������� ���� ��������� ��������� ��������
		if (CommonTypeNeed) {
			//�������� ������������� ��������� �������� �� ���������
			if ((*ci)->IsNeedDefaultSpec) {
				const CCommonType::SSpec* spec = static_cast<CCommonType*>(BT)->GetDefaultSpec();
				if (!spec) {
					//������������� �� ��������� �� �������, ������ ��������� �� ������
					delete FPVar;
					lb->SetCurrPos((*ci)->pos);
					lb->ReadLex(li);
					return s_e_SpecTypeExpected;
				}
				//��������� ����� ������������� �� ���������
				err_num = static_cast<CCommonVar*>(FPVar)->SetTagName(spec->QualName, spec->Name);
			} else
				err_num = static_cast<CCommonVar*>(FPVar)->SetTagName((*ci)->QualTagName, (*ci)->TagName);
			//�������� ������� ������ ����� ��������� ��������
			if (err_num) {
				delete FPVar;
				lb->SetCurrPos((*ci)->pos);
				lb->ReadLex(li);
				return err_num;
			}
		}

		//��������� ���������� ��-�� � ������
		FPVector.push_back(FPVar);
	}

	return 0;
}


//-----------------------------------------------------------------------------
//������������� ���������� ���������� ���������� ���������
int CCommonPars::Init(CLexBuf *lb, CBaseName* parent_element)
{
	DECL_SAVE_POS
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ���������� "}" (���� ������ ���������� ����������)
	if (!lb->ReadLex(li) || lex_k_cl_brace != li.lex) {
		RESTORE_POS

		while (true) {
			//�������� ������� FPSection
			CCommonFPSection CommonFPSection(parent_element);
			int err_num = CommonFPSection.AssignFPElems(lb, FPStore);
			if (err_num) return err_num;

			//�������� ������� ";"
			if (!lb->ReadLex(li) || lex_k_semicolon != li.lex) break;
		}

		//�������� ������� "}" (� ����� ������ ���������� ����������)
		if (lex_k_cl_brace != li.lex) return s_e_ClBraceMissing;
	}//if

	//�������� ������� ":" (���������� ���� ������������� ����������)
	SAVE_POS
	if (!lb->ReadLex(li) || lex_k_colon != li.lex) {
		RESTORE_POS
		return 0;
	}

	//�������� �������� ����� ���� ������������� ����������
	Qualident = new CQualident;
	return Qualident->InitTypeName(lb, parent_element);

}//Init CCommonPars


//-----------------------------------------------------------------------------
//������������� ���������� ���������� ����������� �����. ����-���
int CCommonPars::InitSpec(CLexBuf *lb, CBaseName *parent_element)
{
	DECL_SAVE_POS
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ���������� "}" (���� ������ ���������� ����������)
	if (!lb->ReadLex(li) || lex_k_cl_brace != li.lex) {
		RESTORE_POS

		while (true) {
			//�������� ������� SpecFPSection
			CSpecFPSection SpecFPSection(parent_element);
			int err_num = SpecFPSection.AssignFPElems(lb, FPStore);
			if (err_num) return err_num;

			//�������� ������� ";"
			if (!lb->ReadLex(li) || lex_k_semicolon != li.lex) break;
		}

		//�������� ������� "}" (� ����� ������ ���������� ����������)
		if (lex_k_cl_brace != li.lex) return s_e_ClBraceMissing;
	}//if

	//�������� ������� ":" (���������� ���� ������������� ����������)
	SAVE_POS
	if (!lb->ReadLex(li) || lex_k_colon != li.lex) {
		RESTORE_POS
		return 0;
	}

	//�������� �������� ����� ���� ������������� ����������
	Qualident = new CQualident;
	return Qualident->InitTypeName(lb, parent_element);

}


//-----------------------------------------------------------------------------
//������������� ������������� ������� (���������� ������ ��� �������)
void CDeclSeq::WriteCPP_mod_init(CPP_files &f)
{
	//�������� ���������� ���������� ������ ��������� �������������
	fprintf(f.fc, "\t//imports initialisation\n");
	fprintf(f.fc, "\tstatic bool %swas_started(false);\n", O2M_SYS_);
	fprintf(f.fc, "\tif (%swas_started) return;\n", O2M_SYS_);
	fprintf(f.fc, "\t%swas_started = true;\n", O2M_SYS_);

	//����� ������. ������� � ����� �������� �� �������������
	CBaseNameVector::const_iterator i;
	for (i = BaseNameStore.begin(); i != BaseNameStore.end(); ++i)
		if ((*i)->name_id == id_CImportModule)
			fprintf(f.fc, "\t%s::%s%s();\n", 
				static_cast<CImportModule*>(*i)->name,
				O2M_SYS_,
				static_cast<CImportModule*>(*i)->real_name);
	//����������� - ������ ������������ ���� ������
	fprintf(f.fc, "//BEGIN\n");
}


//-----------------------------------------------------------------------------
//�������� ����� _O2M_main.cpp � �������� O2M_SYS_main_init
bool CModule::WriteCPP_main()
{
	//��� .h ����� _O2M_main
	const char* const _O2M_main_h = "CPP/_O2M_main.h";
	//��� .cpp ����� _O2M_main
	const char* const _O2M_main_cpp = "CPP/_O2M_main.cpp";

	//�������� ������������� ����� _O2M_main.h
	FILE *f = fopen(_O2M_main_h, "w");
	if (!f) {
		fprintf(output, textCannotOpenW, _O2M_main_h);
		return false;
	}
	//������ �����������
	fprintf(f, comment_format, comment_line_cpp, comment_title, comment_line_cpp);

	//�������������� ���������� ����������
	fprintf(f, "#ifndef O2M_H_FILE__O2M_main\n");
	fprintf(f, "#define O2M_H_FILE__O2M_main\n\n");

	//������ ����� ������� ������������� ������
	fprintf(f, "#include \"%s.h\"\n\n", name);
	fprintf(f, "void %smain_init();\n\n", O2M_SYS_);
	fprintf(f, "#endif\n");

	//�������� �����
	if (fclose(f)) {
		fprintf(output, textCannotClose, _O2M_main_h);
		return false;
	}

	//�������� ����� _O2M_main.cpp
	f = fopen(_O2M_main_cpp, "w");
	if (!f) {
		fprintf(output, textCannotOpenW, _O2M_main_cpp);
		return false;
	}
	//������ �����������
	fprintf(f, comment_format, comment_line_cpp, comment_title, comment_line_cpp);

	//������ ��������� ������������� ����� �������� ����� �������, � ����� � ����������� PPP
	fprintf(f, "#include \"%s.h\"\n#include \"_O2M_ppp.cpp\"\n\n", name);

	//������ ������� ������������� ������
	fprintf(f, "void %smain_init() {\n", O2M_SYS_);
	fprintf(f, "\t%s::%s%s();\n}\n", name, O2M_SYS_, name);

	//�������� �����
	if (fclose(f)) {
		fprintf(output, textCannotClose, _O2M_main_cpp);
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
//������ � .dfn ����
void CModule::WriteDFN(DFN_file &f)
{
	fprintf(f.f, "DEFINITION %s;\n", name);
	DeclSeq->WriteDFN(f);
	fprintf(f.f, "\n\nEND %s.\n", name);
}


//-----------------------------------------------------------------------------
//���������� ������� CDfnModule
CDfnModule::~CDfnModule()
{
	delete DfnDeclSeq;
	delete[] full_path;
	delete[] alias_name;
}//~CDfnModule


//-----------------------------------------------------------------------------
//������ ���� CDfnModule
void CDfnModule::WriteCPP(CPP_files& f)
{
	fprintf(f.fh, "#include \"%s%s.h\"\n", full_path ? full_path : "", name);
}


//-----------------------------------------------------------------------------
//������������� ������� CDfnModule (�������� .dfn �����)
int CDfnModule::Init(const CProject *project, CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� ��. ����� DEFINITION
	if (!lb->ReadLex(li) || lex_k_DEFINITION != li.lex) return s_e_DEFINITION;

	//�������� ������� �������� ������
	if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	SetName(li.st);

	//�������� ������� ";"
	if (!lb->ReadLex(li) || lex_k_semicolon != li.lex) return s_e_SemicolonMissing;

	//�������� DfnDeclSeq (��� �������� ������������� �������)
	DfnDeclSeq = new CDfnDeclSeq(this);
	
	//�������� ������ ������������� ������� (���� ����)
	int err_num = DfnDeclSeq->ImportListInit(project, lb);
	if (err_num) return err_num;

	//�������� ������� DfnDeclSeq
	err_num = DfnDeclSeq->Init(lb);
	if (err_num) return err_num;

	//�������� ������� ��. ����� END
	if (!lb->ReadLex(li) || lex_k_END != li.lex) return s_e_END;

	//�������� ������� �������� ������ (� �����)
	if (!lb->ReadLex(li) || lex_i != li.lex || strcmp(li.st, name)) return s_e_ModuleEndName;

	//�������� ������� "."
	if (!lb->ReadLex(li) || lex_k_dot != li.lex) return s_e_DotMissing;

	return 0;
}//CDfnModule::Init


//-----------------------------------------------------------------------------
//���������� ���������� ��-�� � ������� ����
void CDfnModule::AddName(CBaseName* BN) const
{
	DfnDeclSeq->AddName(BN);
}


//-----------------------------------------------------------------------------
//����� ���������������� ����� � ������� ����, NULL - ���� ��� �� �������
CBaseName* CDfnModule::FindImportedName(const char *module_name, const char *search_name) const
{
	//����� ����� (����������) ���������������� ������
	const CBaseName* BaseIM = FindName(module_name);
	if (!BaseIM || (BaseIM->name_id != id_CImportModule))
		return NULL;	//��������� ��������� �� ������
	//�������� ������� ��������������� �������
	if (!DfnDeclSeq->DfnModuleSeq) return NULL;
	//����� ���������������� ������ �� ��������� �����, ����������� �� ����������
	CDfnModule* DM = DfnDeclSeq->DfnModuleSeq->FindName(static_cast<const CImportModule*>(BaseIM)->real_name);
	if (!DM) return NULL;	//��� ������������ dfn ������
	//���������� ����� ���������� ����������� �����
	return DM->FindName(search_name);	
}


//-----------------------------------------------------------------------------
//����� ����� � ������� ����, NULL - ���� ��� �� �������
CBaseName* CDfnModule::FindName(const char* search_name) const
{
	if (DfnDeclSeq) return DfnDeclSeq->FindName(search_name);
	return NULL;
}


//-----------------------------------------------------------------------------
//����������� ������� CDfnModuleSystem
CDfnModuleSystem::CDfnModuleSystem(const CBaseName* parent) : CDfnModule(parent)
{
	name_id = id_CDfnModuleSystem;
}//CDfnModuleSystem


//-----------------------------------------------------------------------------
//��������� ������������� ������ SYSTEM (����. ������� ������������ ��� .dfn)
int CDfnModuleSystem::Init(const CProject *project, CLexBuf *lb)
{
	//����� ������� ��������� �������������
	CDfnModule::Init(project, lb);

	//�������� ����. ��������

	//�������� ���� PTR
	CBaseType *BT = new CPtrType(this);
	AddName(BT);

	return 0;
}


//-----------------------------------------------------------------------------
//������������� ������� PROCEDURE �� ������ ������ � dfn ������
int CDfnDeclSeq::ProcInit(CLexBuf *lb)
{
	DECL_SAVE_POS
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;
	//����������� ������
	CBaseName *BaseName = NULL;

	//�������� ���������� ��. ����� ^
	if (!lb->ReadLex(li) || lex_k_up_arrow != li.lex) {
		//������������� ���������
		RESTORE_POS
		BaseName = new CDfnProcedure(parent_element);

		//������������� ��������� � �������� ���������� ��������� � �� ����������
		int err_num = BaseName->Init(lb);
		if (err_num && (s_m_CommonProcFound != err_num)) {
			delete BaseName;
			return err_num;
		}

		//�������� ����������� ���������� ���������
		if (s_m_CommonProcFound == err_num) {
			//������� ���������� ��������� ��� ����������
			delete BaseName;
			//������� ������������� ���������� ��������������� ���������
			RESTORE_POS
			BaseName = new CDfnCommonProc(parent_element);
			err_num = BaseName->Init(lb);
			//� DFN �� ����������� ������� �����������
			if (s_m_HProcFound == err_num) err_num = s_e_HProcInDfn;
			//�������� ������� ������ ��� ��������� ���������� ���������
			if (err_num) {
				delete BaseName;
				return err_num;
			}
		}

		//��������� ���������� ������� � ������
		AddName(BaseName);
	} else	//������� ����������� ���������� (^)
		return s_e_ForwardDeclDfnFile;

	return 0;
}//CDfnDeclSeq::ProcInit


//-----------------------------------------------------------------------------
//����������� ������� CDfnProcedure
CDfnProcedure::CDfnProcedure(const CBaseName* parent) : CProcedure(parent)
{
	name_id = id_CDfnProcedure;
}


//-----------------------------------------------------------------------------
//������������� ������� CDfnProcedure
int CDfnProcedure::Init(CLexBuf *lb)
{
	DECL_SAVE_POS
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;
	//���������� ��� ��������� ������ ������
	int err_num;

	//�������� ������� ���������
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) {
		RESTORE_POS
	} 
	else {
		Receiver = new CReceiver(this);
		err_num = Receiver->Init(lb, this);
		if (err_num) return err_num;
	}

	//�������� ������� ����� � ��������� ��� � ������� ������
	CIdentDef IdentDef(parent_element, false);
	if (err_num = IdentDef.Init(lb)) return err_num;
	IdentDef.Assign(this);

	//�������� ������� ������ ���������� ���������� (����� ��� ���������� �������� ��� ����������)
	SAVE_POS
	if (!lb->ReadLex(li) || lex_k_op_brace == li.lex)
		return s_m_CommonProcFound;
	else {
		RESTORE_POS
	}

	//�������� ������ ���������� ����������
	FormalPars = new CFormalPars;
	err_num = FormalPars->Init(lb, this);
	if (err_num) return err_num;

	//� ������ ������� ��������� ���������� ������� ������ �� ��������� � ������� ����
	//����, � ������� ��� �������
	if (Receiver) {
		//����� ������������ ���� ������ (������ ���� - ��������� � ���������)
		CBaseName* BN = parent_element->GetGlobalName(Receiver->type_name);
		if (id_CPointerType == BN->name_id) BN = static_cast<CPointerType*>(BN)->FindType();
		//��������� � ������ � ��������� ���������� ����������� ����� � �������
		return static_cast<CRecordType*>(BN)->AddProcReference(this);
	}

	return 0;
}//CDfnProcedure::Init


//-----------------------------------------------------------------------------
//����������� ������� CDfnCommonProc
CDfnCommonProc::CDfnCommonProc(const CBaseName* parent) : CCommonProc(parent)
{
	name_id = id_CDfnCommonProc;
}


//-----------------------------------------------------------------------------
//������������� ������� CDfnCommonProc
int CDfnCommonProc::Init(CLexBuf *lb)
{
	//���������� ��� ��������� ������ ������
	int err_num;

	//�������� ����� IdentDef
	CIdentDef IdentDef(parent_element, false);
	if (err_num = IdentDef.Init(lb)) return err_num;

	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� ���������� ����������
	if (!lb->ReadLex(li) || lex_k_op_brace != li.lex) {
		return s_e_CommonProcCommonParam;
	} else {
		CommonPars = new CCommonPars;
		err_num = CommonPars->Init(lb, this);
		if (err_num) return err_num;
	}

	//�������� ������ ���������� ����������
	FormalPars = new CFormalPars;
	err_num = FormalPars->Init(lb, this);
	if (err_num) return err_num;

	//������� �������� ���� ������������� ���������� �� ���������� ���������� � ���������� (���� ����)
	if (CommonPars->Qualident) {
		//�������� �������� ���������� ���� ������������� �������� (� ������ ��������� - �������)
		if (FormalPars->Qualident) return s_e_CommonProcDoubleType;
		FormalPars->Qualident = CommonPars->Qualident;
		CommonPars->Qualident = NULL;
	}

	//�������� ���������� ���� ���������� � ���������� ����������, ����������� ������� � ���������� ����������
	err_num = CheckParams();
	if (err_num) return err_num;

	//��������� ���������� ����� � ������� ������
	IdentDef.Assign(this);

	return 0;
}//CDfnCommonProc::Init


//-----------------------------------------------------------------------------
//�������� ���������� dfn ������
int CDeclSeq::LoadDfnModule(const CProject *project, const char* module_name, const char* alias_name)
{
	//����� dfn ������ � ������ ���������
	char *full_name = new char[project->PathMaxLen + strlen(module_name) + strlen(".dfn_")];
	CProject::Paths_type::const_iterator ci;
	for (ci = project->Paths.begin(); ci != project->Paths.end(); ++ci) {
		strcpy(full_name, *ci);
		strcat(full_name, module_name);
		strcat(full_name, ".dfn");
		FILE *f = fopen(full_name, "r");
		if (f) {
			fclose(f);
			break;
		}
	}
	//�������� ������� ���������� ������
	if (ci == project->Paths.end()) {
		fprintf(output, "ERROR: required file \"%s.dfn\" not found\n", module_name);
		delete[] full_name;
		return s_e_DfnFileNotFound;
	}
	//������ ����. ������� ���������� ������
	CLexBuf *lb = CLexBuf::AnalyseFile(full_name, project->TabSize);
	if (!lb) {
		delete[] full_name;
		return s_e_DfnFileError;
	}
	//�������� ������ dfn ������
	CDfnModule *DM;
	//��������, ��� ������ SYSTEM ��������� ������ CDfnModuleSystem
	if (strcmp(module_name, "SYSTEM"))
		DM = new CDfnModule(parent_element);
	else
		DM = new CDfnModuleSystem(parent_element);
	int err_num = DM->Init(project, lb);
	//�������� ������� ������
	if (err_num) {
		//����� ��������� �� ������
		WriteErr(err_num, lb);
		fprintf(output, "Syntactic analysis of \"%s\" - FAILED\n", full_name);
		//����������� ��������� ��������
		delete lb;
		delete[] full_name;
		delete DM;
		//������� �������� ������
		return s_e_DfnFileError;
	}
	//����������� ������ ������
	delete lb;
	delete[] full_name;
	//�������� ���������� ������
	DM->alias_name = str_new_copy(alias_name);
	//�������� (��� �������������) ������� ���� � ������
	if (strcmp("DFN/", (*ci))) DM->full_path = str_new_copy(*ci);
	//�������� (��� �������������) ���������� dfn �������
	if (!DfnModuleSeq) DfnModuleSeq = new CDfnModuleSeq(parent_element);
	//���������� ��-�� � ������ ����������� �������
	DfnModuleSeq->AddModule(DM);

	return 0;
}


//-----------------------------------------------------------------------------
//����� ���������������� ����� � ������� ����, NULL - ���� ��� �� �������
CBaseName* CModule::FindImportedName(const char *module_name, const char *search_name) const
{
	//����� ����� (����������) ���������������� ������
	const CBaseName* BaseIM = FindName(module_name);
	if (!BaseIM || (BaseIM->name_id != id_CImportModule))
		return NULL;	//��������� ��������� �� ������
	//�������� ������� ��������������� �������
	if (!DeclSeq->DfnModuleSeq) return NULL;
	//����� ���������������� ������ �� ��������� �����, ����������� �� ����������
	CDfnModule* DM = DeclSeq->DfnModuleSeq->FindName(static_cast<const CImportModule*>(BaseIM)->real_name);
	if (!DM) return NULL;	//��� ������������ dfn ������
	//���������� ����� ���������� ����������� �����
	return DM->FindName(search_name);	
}


//-----------------------------------------------------------------------------
//�������� ������������� ���� ��������, �������� � ������� ����������
int CFormalPars::CheckCompleteRoot(CLexBuf *lb)
{
	//��� ��������� ������ ������
	int err_num;
	//���� �������� ���� ���������� (� ���������� ���������� ����� ���� ������ ���������� ����������)
	CBaseVarVector::const_iterator ci;
	for(ci = FPStore.begin(); ci != FPStore.end(); ++ci)
		if (CBaseVar::IsVarId((*ci)->name_id)) {
			err_num = static_cast<CBaseVar*>(*ci)->CheckComplete(lb);
			if (err_num) return err_num;
		}
	//�������� ���������
	return 0;
}


//-----------------------------------------------------------------------------
//����� ���������� ����� � ������ ���������� ����������
CBaseName* CFormalPars::FindName(const char *search_name) const
{
	CBaseVarVector::const_iterator ci;
	for (ci = FPStore.begin(); ci != FPStore.end(); ++ci)
		if ( !strcmp((*ci)->name, search_name) )
			return *ci;
	return NULL;
}


//-----------------------------------------------------------------------------
//����� ����� � ������ ���������� ���������� �� ������
CBaseName* CFormalPars::GetNameByIndex(int index)
{
	if (index >= FPStore.size()) return NULL;
	return FPStore[index];
}


//-----------------------------------------------------------------------------
//������ � .dfn ����
void CImportModule::WriteDFN(DFN_file &f)
{
	if (strcmp(name, real_name)) fprintf(f.f, "%s := ", name);
	fprintf(f.f, "%s", real_name);
}


//-----------------------------------------------------------------------------
//������ ���� ���������� ������������ ���� � ��������� ���������
void CBaseVar::WriteCPP_fp_named_type(CPP_files &f, const bool to_h)
{
	if (!to_h) {//������ ���� � .cpp ���� (� ����������� "&" ��� VAR ����������)
		if (type_module_name) fprintf(f.fc, "%s::", type_module_name);
		fprintf(f.fc, "%s %s%s", type_name, is_var ? "&" : "", name);
	}
	else {//������ ���� � .h ����
		if (type_module_name) {
			//����� ������ �� ��� ����������
			CBaseName* BN = parent_element->GetGlobalName(type_module_name);
			//������ �������� �� ���������� ����� ������
			if ( BN && (BN->name_id == id_CImportModule) )
				fprintf(f.fh, "%s::", static_cast<CImportModule*>(BN)->real_name);
		}
		//������ ���� � �������� ���������� (� "&" ��� VAR ����������)
		fprintf(f.fh, "%s %s%s", type_name, is_var ? "&" : "", name);
	}
}


//-----------------------------------------------------------------------------
//���������� ������������� ���������� ������������ �������
long CArrayType::GetDimCount()
{
	long DimCount = 1;
	CArrayType *AT = static_cast<CArrayType*>(Type);
	while (AT->name_id == id_CArrayType) {
		++DimCount;
		AT = static_cast<CArrayType*>(AT->Type);
	}
	return DimCount;
}


//-----------------------------------------------------------------------------
//��������� ���������� ��������� ��� ���������� ���������, ������ ��������� � 0
//��� ������������ ���������� ��������� ������� id_CBaseName
EName_id CArrayType::GetResultId(int dimension) const
{
	//������� ���� ���� ���������, ��� ���� �� �������� ���������� ���������
	CBaseType* BT = const_cast<CArrayType*>(this);
	while (id_CArrayType == BT->name_id && dimension) {
		--dimension;
		BT = static_cast<CArrayType*>(BT)->Type;
	}
	//�������� ������, ����� ������������ ������ ��� �����������
	if (dimension) return id_CBaseName;
	//�������� ��������� QualidentType
	if (id_CQualidentType == BT->name_id) {
		//������� QualidentType, ��������� �� ���� ���
		int err_num = static_cast<CQualidentType*>(BT)->GetNamedType(BT, false);
		if (err_num) return id_CBaseName;
	}
	//��������� id ���������� �� ��������� ����
	return BT->GetResultId();
}


//-----------------------------------------------------------------------------
//��������� ��. �� ��� ��-��� ������� (��������� ��� � ������� �����, �� ���������� ��������)
CBaseType* CArrayType::FindLastType() const
{
	CBaseType *BT = Type;
	while (BT->name_id == id_CArrayType) BT = static_cast<CArrayType*>(BT)->Type;
	return BT;
}


//-----------------------------------------------------------------------------
//��������� ���� ��� ��������� ����������� �������, ������ ������������ � 0
//��� �������� ������������ ����������� ������� NULL
CBaseType* CArrayType::GetType(const int dimension)
{
	//������ ������ � �������� �������
	CBaseType* BT = this;
	//������� ����� � ��������� ������ �����������
	int i = 0;
	while (i < dimension) {
		i++;
		BT = static_cast<CArrayType*>(BT)->Type;
		if (id_CArrayType != BT->name_id) break;
	}
	//��������, ��� ������� ����� �� ��������� ������ ���������� ��������� �����������
	if (dimension != i) return NULL;
	//�������� ���������� ������������ ���� (��������� ����� ������ ������ ����)
	if (id_CQualidentType == BT->name_id)
		static_cast<CQualidentType*>(BT)->GetNamedType(BT, false);
	//������ ���������� ����
	return BT;
}

