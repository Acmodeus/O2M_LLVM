//=============================================================================
// �������� ������� � ��������������� �������
//=============================================================================

#include "Base.h"
#include "Var.h"
#include "Type.h"


//-----------------------------------------------------------------------------
//����������, ����������� �������� �����
CPP_files::~CPP_files()
{
	//�������� �������� ������
	if (f2ml) {
		fprintf(f2ml, "</Module>\n");
		fclose(f2ml);
	}
	if (fh) {
		fprintf(fh, "}\n#endif\n");
		fclose(fh);
	}
	if (fc) fclose(fc);
	//������� ������� ����� �����
	delete[] f_name;
}


//-----------------------------------------------------------------------------
//�������� ������ ��� ������ � ������� ��������� ��� ������������� ������
int CPP_files::Init(const char *name)
{
	//���������� ������ ��� ������������ ������� ���� � �����
    ext_offs = /*strlen("CPP/.") +*/ strlen(name);
	f_name = new char[ext_offs + strlen("cpp_")];
	//������������ ������� ���� � ����� ��� ����������
    //strcpy(f_name, "CPP/");
    strcpy(f_name, name);

	//���������� ���������� ".2ml"
	strcat(f_name, ".2ml");
	//�������� 2ml ����� ��� ������
	f2ml = fopen(f_name, "w");
	if (!f2ml) {
		fprintf(output, textCheckFolder, "CPP");
		goto fault_exit;
	}

	//����� ���������� �� "cpp"
	f_name[ext_offs] = 0;
	strcat(f_name, "cpp");
	//�������� cpp ����� ��� ������
	fc = fopen(f_name, "w");
	if (!fc) goto fault_exit;

	//����� ���������� �� "h"
	f_name[ext_offs] = 'h';
	f_name[ext_offs + 1] = 0;
	//�������� h ����� ��� ������
	fh = fopen(f_name, "w");
	if (!fh) goto fault_exit;

	//������ ����������� � ������ �����
	fprintf(fc, comment_format, comment_line_cpp, comment_title, comment_line_cpp);
	fprintf(fh, comment_format, comment_line_cpp, comment_title, comment_line_cpp);

	//������ ��������� � 2ml ����
	fprintf(f2ml, "<?xml version=\"1.0\" ?>\n<!-- %s -->\n<Module Name=\"%s\">\n", comment_title, name);

	//�������������� ���������� ����������
	fprintf(fh, "#ifndef O2M_H_FILE_%s\n", name);
	fprintf(fh, "#define O2M_H_FILE_%s\n", name);
	//������� ����� _O2M_sys.h
	fprintf(fh, "#include \"_O2M_sys.h\"\n");

	//�������� ������ ��������� ������
	return 0;

fault_exit:
	//����� ��������� �� ������ �������� ����� ��� ������
	fprintf(output, textCannotOpenW, f_name);
	return s_m_Error;
}


//-----------------------------------------------------------------------------
//������� ��������� � ���� ��������
void CPP_files::tab_fc()
{
	for(int i = tab_level_c; i > 0; i--) fprintf(fc, "\t");
}


//-----------------------------------------------------------------------------
//����������, ����������� �������� ����
DFN_file::~DFN_file()
{
	//�������� ��������� �����
	if (f) fclose(f);
	//������� ������� ����� �����
	delete[] f_name;
}


//-----------------------------------------------------------------------------
//�������� ����� ��� ������ � ������� ��������� ��� ������������� ������
int DFN_file::Init(const char *name)
{
	//���������� ������ ��� ������������ ������� ���� � �����
    f_name = new char[strlen("DFN") +strlen(name) + strlen(".dfn_")];
	//������������ ������� ���� � �����
    strcpy(f_name, "DFN/");
    strcat(f_name, name);
	strcat(f_name, ".dfn");

	//�������� �����
	f = fopen(f_name, "w");
	if (!f) {
        fprintf(output, textCheckFolder, "DFN");
		fprintf(output, textCannotOpenW, f_name);
		return s_m_Error;
	}

	//������ ����������� � ������ �����
	fprintf(f, "%s(* %s *)\n%s\n", comment_line_dfn, comment_title, comment_line_dfn);

	return 0;
}


//-----------------------------------------------------------------------------
//������� ��������� � DFN ����
void DFN_file::tab()
{
	for(int i = tab_level; i > 0; i--) fprintf(f, "\t");
}


//-----------------------------------------------------------------------------
//����������� ����� � ��������� � ��������� ������ (BaseName)
void CBaseName::Assign(CBaseName* BaseName) const
{
	//����������� ����� (���� ����)
	if (name) BaseName->SetName(name);
	//����������� ���������
	BaseName->external = external;
}//Assign


//-----------------------------------------------------------------------------
//����������� � ������ ���������� �����
void CBaseName::SetName(const char *NewName)
{
	name = str_new_copy(NewName);
}


//-----------------------------------------------------------------------------
//��������� �� �������� ������� �� ���������� ������� ���������
CBaseName* CBaseName::GetGlobalName(const char* global_name) const
{
	//�������� ������� �������� �������
	if (!global_name) return NULL;
	//����� ���������� �����, ������� � ������� ������� ����
	const CBaseName* Scope = this;
	while (true) {
		//�������� ������� ����� � ������� ������� ����
		if (CBaseName* GlobalName = Scope->FindName(global_name)) return GlobalName;
		//������� � ����� ���������� ������� ���������
		Scope = Scope->parent_element;
		//�������� ����� ���������� ������� ���������
		if (!Scope) return NULL;
	}//while
}//GetGlobalName


//-----------------------------------------------------------------------------
//��������� �� ����������� �������� ������� �� ���������� ������� ���������
CBaseName* CBaseName::GetGlobalName(const char* module_name, const char* global_name) const
{
	//�������� ������� �������� ������, ���� ��� ��� - ���� ��������� ���
	if (!module_name) return GetGlobalName(global_name);

	//����� �������, ���������� ������� ������� � ��������� ������ (���������������)
	const CBaseName* BaseName = this;
	while (BaseName)
		switch (BaseName->name_id) {
		//����� ��������� ����������� ������ ����� � ����������� �� ���� BaseName,
		//��� ������� � ����� �������� ������� � ������ ������������ ��������
		case id_CModule:
			return static_cast<const CModule*>(BaseName)->FindImportedName(module_name, global_name);
		case id_CDfnModule:
			return static_cast<const CDfnModule*>(BaseName)->FindImportedName(module_name, global_name);
		case id_CWithLoopLink:
			return static_cast<const CWithLoopLink*>(BaseName)->FindImportedName(module_name, global_name);
		default:
			//�������� ������� ���������
			if (CProcedure::IsProcId(BaseName->name_id))
				return static_cast<const CProcedure*>(BaseName)->FindImportedName(module_name, global_name);
			//������� � ����� �������� ������� � ������ ������������ ��������
			BaseName = BaseName->parent_element;
		}//switch

	//����� ������� parent_element CModule ��� CDfnModule ������ ���� ������
	throw error_Internal("::GetGlobalName");
}//GetGlobalName


//-----------------------------------------------------------------------------
//��������� ������ (�������� ��� DFN) �� ��-��� parent_element
const CBaseName* CBaseName::GetParentModule() const
{
	//������� parent_element ���� �� ����� ������ ������ ������-���� ����
	const CBaseName* BN = parent_element;
	while (BN)
		switch (BN->name_id) {
		case id_CModule:
		case id_CDfnModule:
		case id_CDfnModuleSystem:
			return BN;
		default:
			BN = BN->parent_element;
		}//switch
	//��� ������ ������� ������ � ������ parent_element �����������
	return this;
}


//-----------------------------------------------------------------------------
//����������� ����� � ��������� � ��������� ������ (BaseVar)
int CIdentDef::AssignVar(CBaseVar *BaseVar) const
{
	//�������� ������� ���������� ���������� (���� ���� <>)
	if (is_common && id_CCommonVar != BaseVar->name_id) return s_e_CommonTypeExpected;
	//����������� ���������, �������������� �� CBaseName
	CBaseName::Assign(BaseVar);
	//����������� �������������� ���������
	BaseVar->is_read_only = is_read_only;
	//��������� �������� � ������ ������������������ ���������� ����������
	if (is_common && TagName)
		return static_cast<CCommonVar*>(BaseVar)->SetTagName(QualTagName, TagName);
	else
		return 0;
}


//-----------------------------------------------------------------------------
//������������� ������� CIdentDef
int CIdentDef::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� ��.
	if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;

	//���������� ���������� ���
	SetName(li.st);

	DECL_SAVE_POS

	//��������� ����. ������� ("*", "-", "<")
	if (!lb->ReadLex(li)) {
		RESTORE_POS
		return 0;
	}

	//�������� ���������� �������
	switch (li.lex) {
	case lex_k_minus:
		if (!read_only_enabled) return s_e_IdentWrongMarked;
		is_read_only = true;
	case lex_k_asterix:
		external = true;
		break;
	case lex_k_lt:
		goto lt_received;
	default:
		RESTORE_POS
		return 0;
	}//switch

	SAVE_POS

	//�������� ������� "<"
	if (!lb->ReadLex(li) || lex_k_lt != li.lex) {
		RESTORE_POS
		return 0;
	}

lt_received:

	//��������� �������� ������� ���������
	is_common = true;

	//�������� ������� ">" (������������ ������������� �� ���������)
	SAVE_POS
	if (!lb->ReadLex(li) || lex_k_gt != li.lex) {
		RESTORE_POS
	} else
		return 0;

	//��������� ��������
	CQualident qual;
	int err_num = qual.Init(lb, parent_element);
	//�������� ���������� �������������� (��� �� ������)
	if (err_num && s_e_IdentExpected != err_num) return err_num;

	//���������� �������� �������� (���� ����)
	if (qual.pref_ident) QualTagName = str_new_copy(qual.pref_ident);
	if (qual.ident) TagName = str_new_copy(qual.ident);

	//�������� ������� ">"
	if (!lb->ReadLex(li) || lex_k_gt != li.lex) return s_e_ClAngleMissing;

	return 0;
}//Init CIdentDef


//-----------------------------------------------------------------------------
//���������� ������� CIdentList
CIdentList::~CIdentList()
{
	delete BaseType;
	CBaseNameVector::iterator i;
	for(i = BNVector.begin(); i != BNVector.end(); ++i)
		delete *i;
}//~CIdentList


//-----------------------------------------------------------------------------
//�������� ������ ��������������� ��� �������� ����� ��������
int CIdentList::LoadIdentList(CLexBuf *lb)
{
	//�������� ����� IdentDef
	CIdentDef* IdentDef = new CIdentDef(parent_element, true);
	//������ ������� ����� (����� �������������)
	int err_num = IdentDef->Init(lb);

	//�������� ������� ������ ��� �������� ���������� ��������������
	if (err_num)
		if (s_e_IdentExpected == err_num) {
			delete IdentDef;			//���������� ��������� ��-�
			return s_m_IdentDefAbsent;	//��� ��-� (��� �� ������)
		} else return err_num;

	//�������� ���������� ����������� �������� � ������� ����
	if (parent_element->FindName(IdentDef->name)) {
		delete IdentDef;
		return s_e_Redefinition;
	}

	//��������� ������������ �������� ����� (���� ��������������)
	if (is_local && IdentDef->external) {
		delete IdentDef;
		return s_e_IdentWrongMarked;
	}

	//���������� ��������� �������� � ������ ��������
	BNVector.push_back(IdentDef);

	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//���� ������ ���������� ��������� ������
	while(true) {
		DECL_SAVE_POS

		//�������� ������� ","
		if (!lb->ReadLex(li) || lex_k_comma != li.lex) {
			RESTORE_POS
			break;
		}

		//������ ���������� ����� (������ ��������������)
		IdentDef = new CIdentDef(parent_element, true);
		if (err_num = IdentDef->Init(lb)) {
			delete IdentDef;
			return err_num;
		}

		//�������� ���������� ����������� �������� � ������� ����
		if (parent_element->FindName(IdentDef->name)) {
			delete IdentDef;
			return s_e_Redefinition;
		}

		//�������� ���������� ����������� �������� � ������ ��������
		CBaseNameVector::const_iterator ci;
		for (ci = BNVector.begin(); ci != BNVector.end(); ci++)
			if (!strcmp( (*ci)->name, IdentDef->name)) {
				delete IdentDef;
				return s_e_Redefinition;
			}

		//��������� ������������ �������� ����� (���� ��������������)
		if (is_local && IdentDef->external) {
			delete IdentDef;
			return s_e_IdentWrongMarked;
		}

		//���������� ��������� �������� � ������ ��������
		BNVector.push_back(IdentDef);

	}//while

	//�������� ������� ��. ����� ":"
	if (!lb->ReadLex(li) || lex_k_colon != li.lex) return s_e_ColonMissing;

	//�������� ������� ���� �� ������ ������
	err_num = TypeSelector(lb, BaseType, parent_element);
	if (err_num) return err_num;

	//��������� ���� (��� �������� ���������� ���� ������)
	CBaseType* BT;
	if (id_CQualidentType == BaseType->name_id)
		static_cast<CQualidentType*>(BaseType)->GetNamedType(BT, false);
	else
		BT = BaseType;

	//� ������ ������� ��������, �������� �� ������ �������� (��� �����������)
	if (BT && id_CArrayType == BT->name_id && 0 == static_cast<CArrayType*>(BT)->size)
		return s_e_OpenArrayNotAllowed;

	return 0;
}//CIdentList::LoadIdentList


//-----------------------------------------------------------------------------
//���������� ������� Qualident
CQualident::~CQualident()
{
	delete[] pref_ident;
	delete[] ident;
}//~CQualident


//-----------------------------------------------------------------------------
//������������� ������� Qualident (�� ����� ����) �� ������ ������ � ���������
//������� ������������ ������� �������������� (������ � ������ ������� ���������)
int CQualident::Init(CLexBuf *lb, const CBaseName *parent_element)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//��������� � �������� ������� ���������� ��-�
	if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	ident = str_new_copy(li.st);

	DECL_SAVE_POS

	//�������� ������� ��. ����� "."
	if (!lb->ReadLex(li) || lex_k_dot != li.lex) {
		RESTORE_POS
	} else {
		//�������� ������� ������� ��������������
		if (!parent_element->GetGlobalName(ident)) {
			RESTORE_POS
			return s_e_UndeclaredIdent;
		}
		//��������� ������� ��-� (��������� �����)
		if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
		//ident �� ����� ���� ��� pref_ident-��, ������� ��� ���� 
		pref_ident = ident;
		//���������� ����� ��-�
		ident = str_new_copy(li.st);
	}

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������������� ������� Qualident (� ������ ����� ����) �� ������ ������
//� ������������ ���� ���������� ���������
int CQualident::InitTypeName(CLexBuf *lb, const CBaseName *parent_element)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//��������� ��-� ��� ��������� �����
	if (!lb->ReadLex(li) || (lex_i != li.lex && lex_k_dot > li.lex))
		return s_e_IdentExpected;

	//�������� ������������� �������� ����
	if (lex_i != li.lex) {
		switch (li.lex) {
		case lex_k_BOOLEAN:
			TypeResultId = id_CBooleanVar;
			break;
		case lex_k_CHAR:
			TypeResultId = id_CCharVar;
			break;
		case lex_k_SHORTINT:
			TypeResultId = id_CShortintVar;
			break;
		case lex_k_INTEGER:
			TypeResultId = id_CIntegerVar;
			break;
		case lex_k_LONGINT:
			TypeResultId = id_CLongintVar;
			break;
		case lex_k_REAL:
			TypeResultId = id_CRealVar;
			break;
		case lex_k_LONGREAL:
			TypeResultId = id_CLongrealVar;
			break;
		case lex_k_SET:
			TypeResultId = id_CSetVar;
			break;
		default:
			return s_e_IdentExpected;
		}
		//������� ��� ���������, ���������� ident �� ���������
		return 0;
	}

	ident = str_new_copy(li.st);

	DECL_SAVE_POS

	//�������� ������� ��. ����� "." (����� ��������� ������������� - ��� ������)
	if (!lb->ReadLex(li) || lex_k_dot != li.lex) {
		RESTORE_POS
	} else {
		//�������� ������� ������� �������������� - ����� ������
		CBaseName* BN = parent_element->GetGlobalName(ident);
		if (!BN) {
			RESTORE_POS
			return s_e_UndeclaredIdent;
		}
		if (id_CImportModule != BN->name_id) {
			RESTORE_POS
			return s_e_TypeDefinition;
		}
		//��������� ������� ��-� (��������� �����)
		if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
		//ident �� ����� ���� ��� pref_ident-��, ������� ��� ���� 
		pref_ident = ident;
		//���������� ����� ��-�
		ident = str_new_copy(li.st);
	}

	//����� ���� � ������� ����
	CBaseName* BN = parent_element->GetGlobalName(pref_ident, ident);
	if (!BN) return s_e_UndeclaredIdent;
	if (!CBaseType::IsTypeId(BN->name_id)) return s_e_IdentNotType;

	//��������� ���� ���������� �� ����
	TypeResultId = BN->GetResultId();

	return 0;
}


//-----------------------------------------------------------------------------
//����������� ��������� �������� Qualident'� � ��������� ������
void CQualident::Assign(CQualident *Qualident) const
{
	Qualident->Clear();
	if (pref_ident) {
		Qualident->pref_ident = str_new_copy(pref_ident);
	}
	if (ident) {
		Qualident->ident = str_new_copy(ident);
	}
	Qualident->TypeResultId = TypeResultId;
}


//-----------------------------------------------------------------------------
//������� ����������� CQualident ��� ���������� ������ Init
void CQualident::Clear()
{
	delete[] pref_ident;
	pref_ident = NULL;
	delete[] ident;
	ident = NULL;
	TypeResultId = id_CBaseName;
}


//-----------------------------------------------------------------------------
//������ ���� ��� CQualident, ����������� ��� ����
void CQualident::WriteCPP_type(CPP_files& f, const bool to_h, const CBaseName* parent_element)
{
	//����� ����� ��� ������ ����������
	TFileType* ff = to_h ? f.fh : f.fc;
	//��������� �������� ���� (���� ����)
	const char* TypeName = CBaseType::GetCPPTypeName(TypeResultId);
	if (TypeName) fprintf(ff, TypeName);
	else {
		//��������� ���� �� ��� ����� (� ������ ������� ���������� �� ����� �������������)
		CBaseType* BT = static_cast<CBaseType*>(parent_element->GetGlobalName(pref_ident, ident));
		//������ ��������� ����� (������������ ����)
		if (pref_ident) fprintf(ff, "%s::", to_h ? BT->GetModuleName() : BT->GetModuleAlias());
		//��������, �������� �� Qualident ��� ���� ��� �������� ��������� ����
		switch (TypeResultId) {
		case id_CPointerVar:
			fprintf(ff, "%s*", ident);
			break;
		default:	//id_CQualidentType, id_CRecordType, � �.�.
			fprintf(ff, "%s", ident);
		}//switch
	}//else
}


//-----------------------------------------------------------------------------
//������ ���� CQualident � .dfn ����
void CQualident::WriteDFN(DFN_file& f)
{
	//��������, �������� �� Qualident ��� ���� ��� �������� ��������� ����
	switch (TypeResultId) {
	case id_CBooleanVar:
		fprintf(f.f, "BOOLEAN");
		break;
	case id_CCharVar:
		fprintf(f.f, "CHAR");
		break;
	case id_CSetVar:
		fprintf(f.f, "SET");
		break;
	case id_CIntegerVar:
		fprintf(f.f, "INTEGER");
		break;
	case id_CLongintVar:
		fprintf(f.f, "LONGINT");
		break;
	case id_CShortintVar:
		fprintf(f.f, "SHORTINT");
		break;
	case id_CRealVar:
		fprintf(f.f, "REAL");
		break;
	case id_CLongrealVar:
		fprintf(f.f, "LONGREAL");
		break;
	default:	//id_CQualidentType, id_CRecordType, � �.�.
		if (pref_ident)	fprintf(f.f, "%s.", pref_ident);
		fprintf(f.f, "%s", ident);
	}//switch
}//WriteDFN


//-----------------------------------------------------------------------------
//��������� ���� �������� ������� ���� � C++
const char* CBaseType::GetCPPTypeName(const EName_id ResultId)
{
	switch (ResultId) {
	case id_CBooleanVar:
		return CBooleanType::GetCPPTypeName();
	case id_CCharVar:
		return CCharType::GetCPPTypeName();
	case id_CIntegerVar:
		return CIntegerType::GetCPPTypeName();
	case id_CLongintVar:
		return CLongintType::GetCPPTypeName();
	case id_CLongrealVar:
		return CLongrealType::GetCPPTypeName();
	case id_CRealVar:
		return CRealType::GetCPPTypeName();
	case id_CSetVar:
		return CSetType::GetCPPTypeName();
	case id_CShortintVar:
		return CShortintType::GetCPPTypeName();
	default:
		return NULL;
	}//switch
}


//-----------------------------------------------------------------------------
//��������� ���������� DFN ������, ��������������� ������ ��� (���� ����)
const char* CBaseType::GetModuleAlias() const
{
	//��������� ������
	const CBaseName* BN = GetParentModule();
	//��������, �������� �� ������ DFN �������
	if (id_CDfnModule == BN->name_id)
		return static_cast<const CDfnModule*>(BN)->alias_name;
	else
		return NULL;
}


//-----------------------------------------------------------------------------
//��������� ����� DFN ������, ��������������� ������ ��� (���� ����)
const char* CBaseType::GetModuleName() const
{
	//��������� ������
	const CBaseName* BN = GetParentModule();
	//��������, �������� �� ������ DFN �������
	if (id_CDfnModule == BN->name_id)
		return static_cast<const CDfnModule*>(BN)->name;
	else
		return NULL;
}


//-----------------------------------------------------------------------------
//��������� id ���������� (�������������� id ���� � id ��������������� ����������)
EName_id CBaseType::GetResultId() const
{
	switch(name_id) {
	case id_CArrayType:		return id_CArrayVar;
	case id_CBooleanType:	return id_CBooleanVar;
	case id_CCharType:		return id_CCharVar;
	case id_CIntegerType:	return id_CIntegerVar;
	case id_CLongintType:	return id_CLongintVar;
	case id_CLongrealType:	return id_CLongrealVar;
	case id_CPointerType:	return id_CPointerVar;
	case id_CProcedureType:	return id_CProcedureVar;
	case id_CPtrType:		return id_CPtrVar;
	case id_CRealType:		return id_CRealVar;
	case id_CRecordType:	return id_CRecordVar;
	case id_CCommonType:
	case id_CSpecType:		return id_CCommonVar;
	case id_CSetType:		return id_CSetVar;
	case id_CShortintType:	return id_CShortintVar;
	default:
		return id_CBaseName;
	}//switch
}


//-----------------------------------------------------------------------------
//��������, �������� �� ������ ��� � ��� ��� �������� �������. �����������
bool CBaseType::IsSame(const char *module_name, const char *type_name) const
{
	//��������� �������� ������, ��������������� ������ ��� (���� ����)
	const char* m_n = GetModuleAlias();
	//��������� ����������� ����� ���� � ��������� ������
	if (m_n && module_name) {
		if (!strcmp(m_n, module_name) && !strcmp(name, type_name)) return true;
	} else
		if (!m_n && !module_name && !strcmp(name, type_name)) return true;
	//���� �� �����
	return false;
}


//-----------------------------------------------------------------------------
//�������� �������������� ���������� ��. � ��-� ����
bool CBaseType::IsTypeId(const EName_id id)
{
	switch (id) {
	case id_CProcedureType:
	case id_CSetType:
	case id_CQualidentType:
	case id_CArrayType:
	case id_CRecordType:
	case id_CCommonType:
	case id_CCommonExtensionType:
	case id_CSpecType:
	case id_CPointerType:
	case id_CLongrealType:
	case id_CRealType:
	case id_CLongintType:
	case id_CIntegerType:
	case id_CShortintType:
	case id_CCharType:
	case id_CBooleanType:
	case id_CPtrType:
		return true;
	default:
		return false;
	}//switch
}


//-----------------------------------------------------------------------------
//����������� ����� ���� � ����������
void CBaseVar::SetTypeName(const char *TypeModuleName, const char *TypeName)
{
	//�.�. � ��������� ������� ��� ���� ����� ���� ��� �����������, ��� ���� ������
	if (type_module_name) {
		delete[] type_module_name;
		type_module_name = NULL;
	}
	if (type_name) {
		delete[] type_name;
		type_name = NULL;
	}

	//���������� ��������� ����� ����
	if (TypeModuleName) type_module_name = str_new_copy(TypeModuleName);
	if (TypeName) type_name = str_new_copy(TypeName);
}//SetTypeName


//-----------------------------------------------------------------------------
//����������� ��������� �������� ������� � ��������� ����������
void CBaseVar::Assign(CBaseVar *BaseVar) const
{
	//����������� ���������, �������������� �� CBaseName
	CBaseName::Assign(BaseVar);
	//����������� ����������� ���������
	BaseVar->is_const = is_const;
	BaseVar->is_guarded = is_guarded;
	BaseVar->is_read_only = is_read_only;
	BaseVar->is_var = is_var;
	//����������� �������� ����
	BaseVar->SetTypeName(type_module_name, type_name);
}


//-----------------------------------------------------------------------------
//������ ���������� ������, ��������������� ��� ���������� (�������� � ��� ������������� �����)
const char* CBaseVar::GetTypeModuleAlias()
{
	//� ��������������� ���������� ����������� ��� ������ � �����, ���� ��� ���� �� ���� �� ������, ��� � ����������
	if (!type_module_name) {
		//� ������ ���� ������ ���������� ��������� parent_element � ������
		const CBaseName* BN = GetParentModule();
		//��������, ���� �� ������������� ����������
		if (id_CDfnModule == BN->name_id) return static_cast<const CDfnModule*>(BN)->alias_name;
	}
	//������ �������� ������
	return type_module_name;
}


//-----------------------------------------------------------------------------
//������ ����� ������, ��������������� ��� ���������� (�������� � ��� ������������� �����)
const char* CBaseVar::GetTypeModuleName()
{
	//� ��������������� ���������� ����������� ��� ������ � �����, ���� ��� ���� �� ���� �� ������, ��� � ����������
	if (!type_module_name) {
		//� ������ ���� ������ ���������� ��������� parent_element � ������
		const CBaseName* BN = GetParentModule();
		//��������, ���� �� ������������� ����������
		if (id_CDfnModule == BN->name_id) return static_cast<const CDfnModule*>(BN)->name;
	}
	//������ �������� ������
	return type_module_name;
}


//-----------------------------------------------------------------------------
//������ ����� ���� ���������� (����������� ��� ��� ��� CPP)
const char* CBaseVar::GetTypeName()
{
	//�������� ������������ ����
	if (type_name) return type_name;
	//��� ������� �����
	return CBaseType::GetCPPTypeName(name_id);
}


//-----------------------------------------------------------------------------
//�������� �������������� ���������� ��. � ��-� ����������
bool CBaseVar::IsVarId(const EName_id id)
{
	switch (id) {
	case id_CArrayVar:
	case id_CBooleanVar:
	case id_CCharVar:
	case id_CIntegerVar:
	case id_CLongintVar:
	case id_CLongrealVar:
	case id_CPointerVar:
	case id_CProcedureVar:
	case id_CRealVar:
	case id_CRecordVar:
	case id_CCommonVar:
	case id_CSetVar:
	case id_CShortintVar:
	case id_CPtrVar:
		return true;
	default:
		return false;
	}//switch
}


//-----------------------------------------------------------------------------
//�������� �������������� ���������� ��. � ��-� ���������� ��������� ����
bool CBaseVar::IsDigitId(const EName_id id)
{
	switch (id) {
	case id_CIntegerVar:
	case id_CLongintVar:
	case id_CShortintVar:
	case id_CRealVar:
	case id_CLongrealVar:
		return true;
	default:
		return false;
	}//switch
}


//-----------------------------------------------------------------------------
//�������� �������������� ���������� ��. � ��-� ���������� ������ ����
bool CBaseVar::IsIntId(const EName_id id)
{
	switch (id) {
	case id_CIntegerVar:
	case id_CLongintVar:
	case id_CShortintVar:
		return true;
	default:
		return false;
	}//switch
}


//-----------------------------------------------------------------------------
//�������� �������������� ���������� ��. � ��-� ���������� ��������������� (�������������) ����
bool CBaseVar::IsRealId(const EName_id id)
{
	switch (id) {
	case id_CRealVar:
	case id_CLongrealVar:
		return true;
	default:
		return false;
	}//switch
}


//-----------------------------------------------------------------------------
//���������� ������� CBaseVar
CBaseVar::~CBaseVar()
{
	delete[] type_module_name;
	delete[] type_name;
}


//-----------------------------------------------------------------------------
//�������� ����� CQualident
void CQualident::CreateCopy(CQualident *&qual) const
{
	qual = new CQualident;
	//����������� ���� ����������
	qual->TypeResultId = TypeResultId;
	//����������� �������������� (���� ����)
	if (ident) qual->ident = str_new_copy(ident);
	//����������� �������� (���� ����)
	if (pref_ident) qual->pref_ident = str_new_copy(pref_ident);
}


//-----------------------------------------------------------------------------
//������ ���� � dfn, ���� ���������� ������������ ����
void CBaseVar::WriteDFN_named_type(DFN_file &f)
{
	fprintf(f.f, "%s : ", name);
	if (type_module_name) fprintf(f.f, "%s.", type_module_name);
	fprintf(f.f, "%s", type_name);
}


//-----------------------------------------------------------------------------
//���������� ����� � ������� ����
void CWithLoopLink::AddName(const char* ModuleName, CBaseName *GuardVar) const
{
	//����������� �������� ������ (���� ����)
	if (ModuleName) VarModuleName = str_new_copy(ModuleName);
	//����������� ����������
	Var = GuardVar;
}


//-----------------------------------------------------------------------------
//����� ����� ���� ���_������.��� � ������� ����
CBaseName* CWithLoopLink::FindImportedName(const char *module_name, const char *search_name) const
{
	if (Var && VarModuleName && !strcmp(search_name, Var->name) && !strcmp(module_name, VarModuleName)) return Var;
	return parent_element->GetGlobalName(module_name, search_name);
}


//-----------------------------------------------------------------------------
//����� ����� � ������� ����
CBaseName* CWithLoopLink::FindName(const char* search_name) const
{
	if (Var && !VarModuleName && !strcmp(search_name, Var->name)) return Var;
	return parent_element->FindName(search_name);
}


