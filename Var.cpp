//=============================================================================
// �������� ������� ���������� (Var)
//=============================================================================

#include "Var.h"


//-----------------------------------------------------------------------------
//���������� ������� CArrayVar
CArrayVar::~CArrayVar()
{
	delete ArrayType;
	delete[] ConstString;
}//~CArrayVar


//-----------------------------------------------------------------------------
//�������� ������������� ���� ��-��� �������
int CArrayVar::CheckComplete(CLexBuf *lb)
{
	return ArrayType->CheckComplete(lb);
}


//-----------------------------------------------------------------------------
//�������� ���������, ���������������, ��� is_const == true
CBaseVar* CArrayVar::CreateConst(const CBaseName* parent) const
{
	//����������� �������� ������� �������� ������ ��� �����
	if (!ConstString) return CBaseVar::CreateConst(parent);

	//�������� ����������� ����������
	CArrayVar* AV = static_cast<CArrayVar*>(CArrayVar::CreateVar(parent));

	//����������� ������
	AV->ConstString = str_new_copy(ConstString);
	AV->ConstStringIsChar = ConstStringIsChar;

	return AV;
}


//-----------------------------------------------------------------------------
//�������� ����� ���������� (��� ����������� ������ � ������ ��������)
CBaseVar* CArrayVar::CreateVar(const CBaseName* parent) const
{
	CArrayVar* AV = new CArrayVar(parent);
	Assign(AV);

	//����������� ���� � ����������
	CBaseType *BT = NULL;
	ArrayType->CreateType(BT);
	AV->ArrayType = static_cast<CArrayType*>(BT);

	return AV;
}


//-----------------------------------------------------------------------------
//������������� ������� ������� ��������
int CArrayVar::SetConstValue(const char *st)
{
	is_const = true;
	ConstString = str_new_copy(st);
	return 0;
}


//-----------------------------------------------------------------------------
//������ ���� CArrayVar ��� ���������� ����������
void CArrayVar::WriteCPP(CPP_files& f)
{
	//����� �������� ���� ��-��� �������
	CArrayType *AT = static_cast<CArrayType*>(ArrayType->FindLastType());
	
	//���������� ������� ���������� �������� "[" ������� "]" �� ���������� ���������� => ����� ���������� ����������
	CBaseVar* BV;
	static_cast<CBaseType*>(AT)->CreateVar(BV, parent_element);
	BV->external = external;				//��� ������ � ����� .h/.cpp (��� �������������)
	BV->name = name;						//������������ �������� ������� ��� ������ ������ ����� ���������� �������
	BV->WriteCPP(f);
	BV->name = NULL;						//������������ �������� �� ������� => ��� ������ ������� � ��������� ����������
	delete BV;

	//���������� ���������� ��� �������� => ����� ������� ������� �� ������� ���������
	AT = ArrayType;
	//������� ��������� �����-�������� (���� ����)
	while (AT->name_id == id_CArrayType) {
		//������ �������� ������� (0 ���� �������� ������)
		if (external) fprintf(f.fh, "[%i]", AT->size);
		fprintf(f.fc, "[%i]", AT->size);
		//������� � ����. ������� (������ � ����. ����, ���� ����)
		AT = static_cast<CArrayType*>(AT->Type);
	}
}//WriteCPP


//-----------------------------------------------------------------------------
//������ ���� ���������� ������� � ���������� ��������� (����������� ���������� ���������� ��� �������� �������������� �������� �������� ��������)
void CArrayVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	TFileType *file = to_h ? f.fh : f.fc;
	CArrayType *AT = static_cast<CArrayType*>(ArrayType->FindLastType());

	CBaseVar* BV;
	static_cast<CBaseType*>(AT)->CreateVar(BV, parent_element);

	//��������� �����
	char *formal_name;
	if (!is_var) {//��������� ����� ��� ����������� �������-��������
		formal_name = new char[strlen("O2M_ARR_#") + strlen(name)];
		strcpy(formal_name, "O2M_ARR_");
		strcat(formal_name, name);
	} else formal_name = name;

	//�������� ���� ������� (�������� / �� ��������)
	if (ArrayType->size == 0) {
		if (BV->GetTypeModuleName()) fprintf(file, "%s::", to_h ? BV->GetTypeModuleName() : BV->GetTypeModuleAlias());
		fprintf(file, "%s *%s", BV->GetTypeName(), formal_name);
	} else {
		BV->name = formal_name;
		BV->WriteCPP_fp(f, to_h);
		BV->name = NULL;
	}

	//����������� ���������������� �����
	if (!is_var) delete[] formal_name;
	delete BV;

	//������ �������� �������, ���� �� �������� ������
	AT = ArrayType;
	if (AT->size != 0)
		while (AT->name_id == id_CArrayType) {
			fprintf(file, "[%i]", AT->size);
			AT = static_cast<CArrayType*>(AT->Type);
		}

	//������ ������������ ������� � ���� ���������� ��� ��������� �������
	if (ArrayType->size == 0) {
		AT = ArrayType;
		int dimention = 0;
		while (AT->name_id == id_CArrayType) {
			fprintf(file, ", const int O2M_ARR_%i_%s", dimention, name);
			++dimention;
			AT = static_cast<CArrayType*>(AT->Type);
		}
	}

}//WriteCPP_fp


//-----------------------------------------------------------------------------
//������ ���������� ����-������� � ������ (� ������� �� ��������� ���������� ����� ������ ������������ WriteCPP_fp)
void CArrayVar::WriteCPP_rec(CPP_files& f, const bool to_h)
{
	TFileType *file = to_h ? f.fh : f.fc;
	CArrayType *AT = static_cast<CArrayType*>(ArrayType->FindLastType());

	CBaseVar* BV;
	static_cast<CBaseType*>(AT)->CreateVar(BV, parent_element);

	//�������� ���� ������� (�������� / �� ��������)
	if (ArrayType->size == 0) {
		if (BV->GetTypeModuleName()) fprintf(file, "%s::", to_h ? BV->GetTypeModuleName() : BV->GetTypeModuleAlias());
		fprintf(file, "%s *%s", BV->GetTypeName(), name);
	} else {
		BV->name = name;
		BV->WriteCPP_fp(f, to_h);
		BV->name = NULL;
	}

	delete BV;

	//������ �������� �������, ���� �� �������� ������
	AT = ArrayType;
	if (AT->size != 0) {
		while (AT->name_id == id_CArrayType) {
			fprintf(file, "[%i]", AT->size);
			AT = static_cast<CArrayType*>(AT->Type);
		}
	} else throw error_Internal("CArrayVar::WriteCPP_rec");

}//WriteCPP_rec


//-----------------------------------------------------------------------------
//������ ���� ��� ������������� ������������ ��������, ���������������, ��� is_const == true
void CArrayVar::WriteCPP_ConstValue(CPP_files& f)
{
	//����������� �������� ������� �������� ������ ��� �����
	if (!ConstString) throw error_Internal("CArrayVar::WriteCPP_ConstValue");

	//�������� ������ �� ������� ������������ ��������
	char *new_st = new char[strlen(ConstString)*2 + 1];

	int i = 0;
	int j = 0;
	for (i = 0; i < strlen(ConstString); i++, j++) {
		//�������� ������� '\'
		if (ConstString[i] == '\\') {
			new_st[j] = '\\';
			new_st[++j] = '\\';
			continue;
		}
		//�������� ������� '"'
		if (ConstString[i] == '"') {
			new_st[j] = '\\';
			new_st[++j] = '"';
			continue;
		}
		//������������ ������� �� �������
		new_st[j] = ConstString[i];
	}
	new_st[j] = '\0';

	//���������� ����� ������ (��� �������)
	if (strlen(ConstString) == 1 && ConstStringIsChar)
		fprintf(f.fc, "'%s'", new_st);
	else
		fprintf(f.fc, "\"%s\"", new_st);

	delete[] new_st;
}


//-----------------------------------------------------------------------------
//������ ���� CArrayVar � ���� .dfn
void CArrayVar::WriteDFN(DFN_file& f)
{
	//� ������ ��������� ��������� ����� ������ ������ (������ ��������)
	if (is_const) {
		fprintf(f.f, "%s = \"%s\"", name, ConstString);
		return;
	}

	//������ �������� ����������
	fprintf(f.f, "%s%s : ", name, is_read_only ? "-" : "");
	//������������ ��������� ��������
	CArrayType *AT = ArrayType;
	while (AT->name_id == id_CArrayType) {
		fprintf(f.f, "ARRAY ");
		if (AT->size != 0) fprintf(f.f, "%i ", AT->size);
		fprintf(f.f, "OF ");
		AT = static_cast<CArrayType*>(AT->Type);
	}
	//������ �������� ����
	AT->WriteDFN(f);

}//WriteDFN


//-----------------------------------------------------------------------------
//�������� ���������, ���������������, ��� is_const == true
CBaseVar* CBooleanVar::CreateConst(const CBaseName *parent) const
{
	CBooleanVar* BV = static_cast<CBooleanVar*>(CBooleanVar::CreateVar(parent));

	//����������� ������
	BV->ConstValue = ConstValue;

	return BV;
}


//-----------------------------------------------------------------------------
//�������� ����� ���������� (��� ����������� ������ � ������ ��������)
CBaseVar* CBooleanVar::CreateVar(const CBaseName* parent) const
{
	CBaseVar* BV = new CBooleanVar(parent);
	Assign(BV);
	return BV;
}


//-----------------------------------------------------------------------------
//������ ���� CBooleanVar ��� ���������� ����������
void CBooleanVar::WriteCPP(CPP_files& f)
{
	//������ ���������� ����������
	if (external) {
		fprintf(f.fh, "extern %s %s", CBooleanType::GetCPPTypeName(), name);
		fprintf(f.fc, "%s %s::%s%s", CBooleanType::GetCPPTypeName(), parent_element->name, is_var ? "&" : "", name);
	} else
		fprintf(f.fc, "%s %s%s", CBooleanType::GetCPPTypeName(), is_var ? "&" : "", name);

	//������ ������������� ��������� (���� ����)
	if (is_const) fprintf(f.fc, " = %s", ConstValue ? "true" : "false");
}//WriteCPP


//-----------------------------------------------------------------------------
//������ ���� ���������� � ���������� ���������
void CBooleanVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	//�������� ������� ������������ ����
	if (type_name)
		WriteCPP_fp_named_type(f, to_h);
	else
		fprintf(to_h ? f.fh : f.fc, "%s %s%s", CBooleanType::GetCPPTypeName(), is_var ? "&" : "", name);
}


//-----------------------------------------------------------------------------
//������ ���� ��� ������������� ������������ ��������, ���������������, ��� is_const == true
void CBooleanVar::WriteCPP_ConstValue(CPP_files& f)
{
	fprintf(f.fc, "%s", ConstValue ? "true" : "false");
}


//-----------------------------------------------------------------------------
//������ ���� CBooleanVar � ���� .dfn
void CBooleanVar::WriteDFN(DFN_file& f)
{
	if (is_const) {
		fprintf(f.f, "%s = %s", name, ConstValue ? "TRUE" : "FALSE");
		return;
	}

	if (!type_name)
		fprintf(f.f, "%s%s : BOOLEAN", name, is_read_only ? "-" : "");
	else
		WriteDFN_named_type(f);
}//WriteDFN


//-----------------------------------------------------------------------------
//�������� ���������, ���������������, ��� is_const == true
CBaseVar* CCharVar::CreateConst(const CBaseName *parent) const
{
	CCharVar* CV = static_cast<CCharVar*>(CCharVar::CreateVar(parent));

	//����������� ������
	CV->ConstValue = ConstValue;

	return CV;
}


//-----------------------------------------------------------------------------
//�������� ����� ���������� (��� ����������� ������ � ������ ��������)
CBaseVar* CCharVar::CreateVar(const CBaseName* parent) const
{
	CBaseVar* BV = new CCharVar(parent);
	Assign(BV);
	return BV;
}


//-----------------------------------------------------------------------------
//�������������� ��������� ������ � �������� ����������
void CCharVar::SetConstValue(const char ch)
{
	is_const = true;
	ConstValue = ch;
}


//-----------------------------------------------------------------------------
//������ ���� CCharVar ��� ���������� ����������
void CCharVar::WriteCPP(CPP_files& f)
{
	//������ ���������� ����������
	if (external) {
		fprintf(f.fh, "extern %s %s", CCharType::GetCPPTypeName(), name);
		fprintf(f.fc, "%s %s::%s%s", CCharType::GetCPPTypeName(), parent_element->name, is_var ? "&" : "", name);
	} else
		fprintf(f.fc, "%s %s%s", CCharType::GetCPPTypeName(), is_var ? "&" : "", name);

	//������ ������������� ��������� (���� ����)
	if (is_const) fprintf(f.fc, " = %i", ConstValue);
}//WriteCPP


//-----------------------------------------------------------------------------
//������ ���� ���������� � ���������� ���������
void CCharVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	//�������� ������� ������������ ����
	if (type_name)
		WriteCPP_fp_named_type(f, to_h);
	else
		fprintf(to_h ? f.fh : f.fc, "%s %s%s", CCharType::GetCPPTypeName(), is_var ? "&" : "", name);
}


//-----------------------------------------------------------------------------
//������ ���� ��� ������������� ������������ ��������, ���������������, ��� is_const == true
void CCharVar::WriteCPP_ConstValue(CPP_files& f)
{
	fprintf(f.fc, "%i", ConstValue);
}


//-----------------------------------------------------------------------------
//������ ���� CCharVar � ���� .dfn
void CCharVar::WriteDFN(DFN_file& f)
{
	if (is_const) {
		fprintf(f.f, "%s = %c", name, ConstValue);
		return;
	}

	if (!type_name)
		fprintf(f.f, "%s%s : CHAR", name, is_read_only ? "-" : "");
	else
		WriteDFN_named_type(f);
}//WriteDFN


//-----------------------------------------------------------------------------
//�������� ����� ���������� (��� ����������� ������ � ������ ��������)
CBaseVar* CCommonVar::CreateVar(const CBaseName* parent) const
{
	CCommonVar* CV = new CCommonVar(parent);
	Assign(CV);

	//����������� ���������� �������� � ����������
	if (QualSpecName) CV->QualSpecName = str_new_copy(QualSpecName);
	if (SpecName) CV->SpecName = str_new_copy(SpecName);
	if (Tag) CV->Tag = str_new_copy(Tag);
	if (CPPCompoundName) CV->CPPCompoundName = str_new_copy(CPPCompoundName);

	return CV;
}


//-----------------------------------------------------------------------------
//����� ����� � ������� ����
CBaseName* CCommonVar::FindName(const char *search_name) const
{
	//��������� ���� �������������
	CBaseType* BT = FindType();
	if (!BT) return NULL;
	//����� ����� � ������� ���� ���� �������������
	return BT->FindName(search_name);
}


//-----------------------------------------------------------------------------
//����� ���� ������������� (���� ����)
CBaseType* CCommonVar::FindType() const
{
	//��������� ��. �� ���������� ���
	CCommonType* CT = static_cast<CCommonType*>(parent_element->GetGlobalName(type_module_name, type_name));
	if (!CT || id_CCommonType != CT->name_id) return NULL;

	//��������� �������� ���� ������������� �� ����������� ���� � ��������
	const CCommonType::SSpec* spec = CT->FindSpec(QualSpecName, SpecName, Tag);
	if (!spec) return NULL;

	//��������� �������� ������, ����������� ��� �������������
	const char* ModuleName = spec->QualName ? spec->QualName : (spec->IsExtended ? NULL : CT->GetModuleAlias());

	//��������� ���� ������������� �� ������� ������� ��������� �� �������� ���� �������������
	CBaseName* BN = parent_element->GetGlobalName(ModuleName, spec->Name);
	if (!BN || !CBaseType::IsTypeId(BN->name_id)) return NULL;

	return static_cast<CBaseType*>(BN);
}


//-----------------------------------------------------------------------------
//��������� ����� ��������
void CCommonVar::GetTagName(const char* &QualName, const char* &Name, const char* &TagName)
{
	QualName = QualSpecName;
	Name = SpecName;
	TagName = Tag;
}


//-----------------------------------------------------------------------------
//�������� ���������� �������������� ���������� ���������� �����
bool CCommonVar::IsPureCommon()
{
	return (NULL == Tag) && (NULL == SpecName);
}


//-----------------------------------------------------------------------------
//��������� ����� ��������
int CCommonVar::SetTagName(const char *QualName, const char *Name)
{
	//������� ��������� ����� ������ ���������
	if (QualSpecName || Tag) return s_e_CommonTypeExpected;

	//��������� ��. �� ���������� ���
	CCommonType* CT = static_cast<CCommonType*>(parent_element->GetGlobalName(type_module_name, type_name));
	if (!CT || id_CCommonType != CT->name_id) return s_m_Error;

	//��������� �������� ���� ������������� �� ����������� ���� � ��������
	const CCommonType::SSpec* spec = CT->FindSpec(QualName, Name, Name);
	if (!spec) return s_e_SpecTypeTag;

	//����������� ���������� ��������
	if (spec->Tag) Tag = str_new_copy(Name);
	if (spec->QualName) QualSpecName = str_new_copy(spec->QualName);
	if (spec->Name) SpecName = str_new_copy(spec->Name);	//NULL == spec->Name ��� ���� NIL

	//��������� ���������� �����, ������������� ��� ��������� ����
	if (Tag)
		CPPCompoundName = str_new_copy(Tag);
	else {
		int len;
		if (QualSpecName)
			len = strlen(QualSpecName);
		else
			len = strlen(CT->GetParentModule()->name);
		CPPCompoundName = new char[len + strlen(SpecName) + 2];
		if (QualSpecName)
			strcpy(CPPCompoundName, QualSpecName);
		else
			strcpy(CPPCompoundName, CT->GetParentModule()->name);
		strcat(CPPCompoundName, "_");
		strcat(CPPCompoundName, SpecName);
	}//else

	return 0;
}


//-----------------------------------------------------------------------------
//������ ���� CCommonVar ��� ���������� ����������
void CCommonVar::WriteCPP(CPP_files& f)
{
	//�������� ������� ����� ����
	if (type_name) {
		//������ ����� ������ ����� ����� ����������		
		if (type_module_name) fprintf(f.fc, "%s::", type_module_name);

		//��������, �������������� �� ����������
		if (external) {
			fprintf(f.fh, "extern ");
			if (type_module_name) {
				CBaseName* BN = parent_element->GetGlobalName(type_module_name);
				if (BN) fprintf(f.fh, "%s::", static_cast<CImportModule*>(BN)->real_name);
			}
			fprintf(f.fh, "%s %s", type_name, name);
			fprintf(f.fc, "%s %s::%s(", type_name, parent_element->name, name);
		} else
			fprintf(f.fc, "%s %s(", type_name, name);

		//��������� ���� ������ ������������, ����������������� ���������
		if (type_module_name) fprintf(f.fc, "%s::", type_module_name);
		fprintf(f.fc, "%s::O2M_INIT_%s)", type_name, CPPCompoundName);
	}
}//WriteCPP


//-----------------------------------------------------------------------------
//������ ���� ���������� � ���������� ���������
void CCommonVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	if (to_h) {//������ ���� � .h ����
		if (type_module_name) {
			//����� ������ �� ��� ����������
			CBaseName* BN = parent_element->GetGlobalName(type_module_name);
			//������ �������� �� ���������� ����� ������
			if ( BN && (BN->name_id == id_CImportModule) )
				fprintf(f.fh, "%s::", static_cast<CImportModule*>(BN)->real_name);
		}
		//������ ���� � �������� ����������
		fprintf(f.fh, "%s* %s", type_name, name);
	} else {//������ ���� � .cpp ����
		if (type_module_name) fprintf(f.fc, "%s::", type_module_name);
		fprintf(f.fc, "%s* %s", type_name, name);
	}//else
}


//-----------------------------------------------------------------------------
//������ ���� CCommonVar � ���� .dfn
void CCommonVar::WriteDFN(DFN_file& f)
{
	//���������� ���������� �� ����� ���� ���������� � ������ ������������ ����
	WriteDFN_named_type(f);
}//WriteDFN


//-----------------------------------------------------------------------------
//�������� ���������, ���������������, ��� is_const == true
CBaseVar* CIntegerVar::CreateConst(const CBaseName *parent) const
{
	CIntegerVar* IV = static_cast<CIntegerVar*>(CIntegerVar::CreateVar(parent));

	//����������� ������
	IV->ConstValue = ConstValue;
	
	return IV;
}


//-----------------------------------------------------------------------------
//�������� ����� ���������� (��� ����������� ������ � ������ ��������)
CBaseVar* CIntegerVar::CreateVar(const CBaseName* parent) const
{
	CBaseVar* BV = new CIntegerVar(parent);
	Assign(BV);
	return BV;
}


//-----------------------------------------------------------------------------
//�������������� ��������� ������ � �������� ����������
void CIntegerVar::SetConstValue(const char *st)
{
	ConstValue = atoi(st);
}


//-----------------------------------------------------------------------------
//������ ���� CIntegerVar ��� ���������� ����������
void CIntegerVar::WriteCPP(CPP_files& f)
{
	//������ ���������� ����������
	if (external) {
		fprintf(f.fh, "extern %s %s", CIntegerType::GetCPPTypeName(), name);
		fprintf(f.fc, "%s %s::%s%s", CIntegerType::GetCPPTypeName(), parent_element->name, is_var ? "&" : "", name);
	} else
		fprintf(f.fc, "%s %s%s", CIntegerType::GetCPPTypeName(), is_var ? "&" : "", name);

	//������ ������������� ��������� (���� ����)
	if (is_const) fprintf(f.fc, " = %i", ConstValue);
}//WriteCPP


//-----------------------------------------------------------------------------
//������ ���� ���������� � ���������� ���������
void CIntegerVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	//�������� ������� ������������ ����
	if (type_name)
		WriteCPP_fp_named_type(f, to_h);
	else
		fprintf(to_h ? f.fh : f.fc, "%s %s%s", CIntegerType::GetCPPTypeName(), is_var ? "&" : "", name);
}


//-----------------------------------------------------------------------------
//������ ���� ��� ������������� ������������ ��������, ���������������, ��� is_const == true
void CIntegerVar::WriteCPP_ConstValue(CPP_files& f)
{
	if (INT_MIN == ConstValue)
		fprintf(f.fc, "(%i - 1)", ConstValue + 1);
	else
		fprintf(f.fc, "%i", ConstValue);
}


//-----------------------------------------------------------------------------
//������ ���� CIntegerVar � ���� .dfn
void CIntegerVar::WriteDFN(DFN_file& f)
{
	if (is_const) {
		fprintf(f.f, "%s = %i", name, ConstValue);
		return;
	}

	if (!type_name)
		fprintf(f.f, "%s%s : INTEGER", name, is_read_only ? "-" : "");
	else
		WriteDFN_named_type(f);
}//WriteDFN


//-----------------------------------------------------------------------------
//�������� ���������, ���������������, ��� is_const == true
CBaseVar* CLongintVar::CreateConst(const CBaseName *parent) const
{
	CLongintVar* LV = static_cast<CLongintVar*>(CLongintVar::CreateVar(parent));

	//����������� ������
	LV->ConstValue = ConstValue;

	return LV;
}


//-----------------------------------------------------------------------------
//�������� ����� ���������� (��� ����������� ������ � ������ ��������)
CBaseVar* CLongintVar::CreateVar(const CBaseName* parent) const
{
	CBaseVar* BV = new CLongintVar(parent);
	Assign(BV);
	return BV;
}


//-----------------------------------------------------------------------------
//������ ���� CLongintVar ��� ���������� ����������
void CLongintVar::WriteCPP(CPP_files& f)
{
	//������ ���������� ����������
	if (external) {
		fprintf(f.fh, "extern %s %s", CLongintType::GetCPPTypeName(), name);
		fprintf(f.fc, "%s %s::%s%s", CLongintType::GetCPPTypeName(), parent_element->name, is_var ? "&" : "", name);
	} else
		fprintf(f.fc, "%s %s%s", CLongintType::GetCPPTypeName(), is_var ? "&" : "", name);

	//������ ������������� ��������� (���� ����)
	if (is_const) fprintf(f.fc, " = %i", ConstValue);
}//WriteCPP


//-----------------------------------------------------------------------------
//������ ���� ���������� � ���������� ���������
void CLongintVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	//�������� ������� ������������ ����
	if (type_name)
		WriteCPP_fp_named_type(f, to_h);
	else
		fprintf(to_h ? f.fh : f.fc, "%s %s%s", CLongintType::GetCPPTypeName(), is_var ? "&" : "", name);
}


//-----------------------------------------------------------------------------
//������ ���� ��� ������������� ������������ ��������, ���������������, ��� is_const == true
void CLongintVar::WriteCPP_ConstValue(CPP_files& f)
{
	if (LONG_MIN == ConstValue)
		fprintf(f.fc, "(%li - 1)", ConstValue + 1);
	else
		fprintf(f.fc, "%li", ConstValue);
}


//-----------------------------------------------------------------------------
//������ ���� CLongintVar � ���� .dfn
void CLongintVar::WriteDFN(DFN_file& f)
{
	if (is_const) {
		fprintf(f.f, "%s = %i", name, ConstValue);
		return;
	}

	if (!type_name)
		fprintf(f.f, "%s%s : LONGINT", name, is_read_only ? "-" : "");
	else
		WriteDFN_named_type(f);
}//WriteDFN


//-----------------------------------------------------------------------------
//�������� ���������, ���������������, ��� is_const == true
CBaseVar* CLongrealVar::CreateConst(const CBaseName *parent) const
{
	CLongrealVar* LV = static_cast<CLongrealVar*>(CLongrealVar::CreateVar(parent));

	//����������� ������
	LV->ConstValue = ConstValue;

	return LV;
}


//-----------------------------------------------------------------------------
//�������� ����� ���������� (��� ����������� ������ � ������ ��������)
CBaseVar* CLongrealVar::CreateVar(const CBaseName* parent) const
{
	CBaseVar* BV = new CLongrealVar(parent);
	Assign(BV);
	return BV;
}


//-----------------------------------------------------------------------------
//�������������� ��������� ������ � �������� ����������
void CLongrealVar::SetConstValue(const char *st)
{
	is_const = true;
	ConstValue = atof(st);
}//StoreData


//-----------------------------------------------------------------------------
//������ ���� CLongrealVar ��� ���������� ����������
void CLongrealVar::WriteCPP(CPP_files& f)
{
	//������ ���������� ����������
	if (external) {
		fprintf(f.fh, "extern %s %s", CLongrealType::GetCPPTypeName(), name);
		fprintf(f.fc, "%s %s::%s%s", CLongrealType::GetCPPTypeName(), parent_element->name, is_var ? "&" : "", name);
	} else
		fprintf(f.fc, "%s %s%s", CLongrealType::GetCPPTypeName(), is_var ? "&" : "", name);

	//������ ������������� ��������� (���� ����)
	if (is_const) fprintf(f.fc, " = %g", ConstValue);
}//WriteCPP


//-----------------------------------------------------------------------------
//������ ���� ���������� � ���������� ���������
void CLongrealVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	//�������� ������� ������������ ����
	if (type_name)
		WriteCPP_fp_named_type(f, to_h);
	else
		fprintf(to_h ? f.fh : f.fc, "%s %s%s", CLongrealType::GetCPPTypeName(), is_var ? "&" : "", name);
}


//-----------------------------------------------------------------------------
//������ ���� ��� ������������� ������������ ��������, ���������������, ��� is_const == true
void CLongrealVar::WriteCPP_ConstValue(CPP_files& f)
{
	fprintf(f.fc, "%g", ConstValue);
}


//-----------------------------------------------------------------------------
//������ ���� CLongrealVar � ���� .dfn
void CLongrealVar::WriteDFN(DFN_file& f)
{
	if (is_const) {
		fprintf(f.f, "%s = %E", name, ConstValue);
		return;
	}

	if (!type_name)
		fprintf(f.f, "%s%s : LONGREAL", name, is_read_only ? "-" : "");
	else
		WriteDFN_named_type(f);
}//WriteDFN


//-----------------------------------------------------------------------------
//�������� ������������� ���� ������ ����������
int CPointerVar::CheckComplete(CLexBuf *lb)
{
	//����� ����
	CBaseName* BN = FindType();
	//�������� ������� ������������ ����
	if (!BN) {
		lb->SetCurrPos(TypePos);
		return s_e_UndeclaredIdent;
	}
	//�������� ������������ ���� (����������� ������ ���� ������, ������ � ���������)
	switch (BN->name_id) {
	case id_CArrayType:
		IsArray = true;
		return static_cast<CArrayType*>(BN)->CheckComplete(lb);
	case id_CRecordType:
		IsRecord = true;
		return static_cast<CRecordType*>(BN)->CheckComplete(lb);
	case id_CCommonType:
		return 0;
	case id_CSpecType:
		IsRecord = true;
		return 0;
	default:
		lb->SetCurrPos(TypePos);
		return s_e_PointerTypeKind;
	}//switch
}


//-----------------------------------------------------------------------------
//�������� ���������, ���������������, ��� is_const == true
CBaseVar* CPointerVar::CreateConst(const CBaseName *parent) const
{
	//����������� ��������� ��������� ����� ���� ������ NIL => �� ��������� ����������� ������
	return CPointerVar::CreateVar(parent);
}


//-----------------------------------------------------------------------------
//�������� ����� ���������� (��� ����������� ������ � ������ ��������)
CBaseVar* CPointerVar::CreateVar(const CBaseName* parent) const
{
	//�������� ���������� � ����������� �������� ���������
	CPointerVar* PV = new CPointerVar(parent);
	Assign(PV);
	//����������� ���� (� ������ �������������� ����)
	if (Type) Type->CreateType(PV->Type);
	//����������� ���. ���������
	PV->IsArray = IsArray;
	PV->IsRecord = IsRecord;
	PV->qualident_type = qualident_type;
	PV->TypePos = TypePos;
	return PV;
}


//-----------------------------------------------------------------------------
//����� ����� � ������� ���� ������������ ����, ����� ����� ������ ��� ��. �� ������
CBaseName* CPointerVar::FindName(const char *search_name) const
{
	//�������� ������� ���� ������ (������-�� � ������ ������ ������� �� ������ ����������)
	if (!IsRecord) return NULL;
	//��������� ����
	CBaseType* BT = FindType();
	//����� ����� � ������ ���� ����
	if (id_CSpecType == BT->name_id)
		return static_cast<CSpecType*>(BT)->FindName(search_name);
	else
		return static_cast<CRecordType*>(FindType())->FindName(search_name);
}


//-----------------------------------------------------------------------------
//����� ���� (�� QualidentType, �� PointerType), �� ���. ��������� ���������
CBaseType* CPointerVar::FindType() const
{
	//�������� ������� �������������� ����
	if (Type) return Type;

	//����� �� ������� ����������� �����
	CBaseName* BN = parent_element->GetGlobalName(type_module_name, type_name);
	while (BN && id_CQualidentType == BN->name_id)
		BN = BN->parent_element->GetGlobalName(static_cast<CQualidentType*>(BN)->Qualident->pref_ident, static_cast<CQualidentType*>(BN)->Qualident->ident);

	//��� ����������� ���� ��. ������������ ��� ����� ������ ����
	//(������ � ������, ���� ���������� ���� ��������� ����� QualidentType)
	if (qualident_type && BN && id_CPointerType == BN->name_id)
		return static_cast<CPointerType*>(BN)->FindType();

	//������� ���������� ���� (��� NULL)
	return static_cast<CBaseType*>(BN);
}


//-----------------------------------------------------------------------------
//������ ���� ���. ����������, ���������� ����������� � ������ ��������� �������
void CPointerVar::WriteCPP_array(CPP_files &f)
{
	//�������� ���������� �������
	if (!IsArray) return;
	//��������� ����, �� ���. ��������� POINTER
	CArrayType* AT = static_cast<CArrayType*>(FindType());
	//��������� ���������� ��� ������ �������� �����������
	int dimension = 0;	//������ ������������ � 0
	while (id_CArrayType == AT->name_id) {
		//��� ���������� �������� ����������� ���������� ���������
		if (AT->size) break;
		//��������� ���������� ��� �������� ������� �����������
		if (external) {
			fprintf(f.fh, ";\nextern int O2M_ARR_%i_%s", dimension, name);
			fprintf(f.fc, ";\nint %s::O2M_ARR_%i_%s", parent_element->name, dimension, name);
		} else
			fprintf(f.fc, ";\nint O2M_ARR_%i_%s", dimension, name);
		//������� � ��������� �����������
		++dimension;
		AT = static_cast<CArrayType*>(AT->Type);
	}//while
}


//-----------------------------------------------------------------------------
//������ ���� CPointerVar ��� ���������� ����������
void CPointerVar::WriteCPP(CPP_files& f)
{
	//�������� ������������ ���� (���� ��� ����)
	if (type_name) {
		//������ ���� ���������� ����������
		if (external) {
			fprintf(f.fh, "extern ");
			if (type_module_name) {
				fprintf(f.fh, "%s::", type_module_name);
				fprintf(f.fc, "%s::", type_module_name);
			}
			fprintf(f.fh, "%s* %s", type_name, name);
			fprintf(f.fc, "%s* %s::%s", type_name, parent_element->name, name);
		} else {
			if (type_module_name) fprintf(f.fc, "%s::", type_module_name);
			fprintf(f.fc, "%s* %s", type_name, name);
		}
		//� ������ ��������� ������� ��������� ��������� ���. ����������
		WriteCPP_array(f);
		return;
	}

	//������������� ��� (������������ � ���������� �������������� ����� ���� O2M_UNNM_<��� ����������>)
	if (Type) {
		//��������� ���� ����, �� ���. ��������� ���������
		Type->name = new char[strlen("O2M_UNNM_") + strlen(name) + 1];
		strcpy(Type->name, "O2M_UNNM_");
		strcat(Type->name, name);
		//��� ���� RECORD ���������� RuntimeId
		if (id_CRecordType == Type->name_id) static_cast<CRecordType*>(Type)->InitRuntimeId();
		//������ ���� ����
		Type->WriteCPP(f, external);
		//������ ���� ���������� ����������
		if (external) {
			fprintf(f.fh, "extern O2M_UNNM_%s* %s", name, name);
			fprintf(f.fc, "O2M_UNNM_%s* %s::%s", name, parent_element->name, name);
		} else {
			//���������� ������ ���� ���������� ����������
			fprintf(f.fc, "O2M_UNNM_%s* %s", name, name);
		}
		//� ������ ��������� ������� ��������� ��������� ���. ����������
		WriteCPP_array(f);
		return;
	}

	//���������� ���������
	if (is_const) {
		fprintf(f.fc, "void %s = 0", name);
		return;
	}

}//WriteCPP


//-----------------------------------------------------------------------------
//������ ���� ���������� � ���������� ���������
void CPointerVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	//��� ��������� ������ ����������
	TFileType* ff = to_h ? f.fh : f.fc;

	//�������� ������������ ���� (���� ��� ����)
	if (type_name) {
		if (type_module_name) fprintf(ff, "%s::", type_module_name);
		fprintf(ff, "%s* %s%s", type_name, is_var ? "&" : "", name);
		return;
	} else {

		//������������� ��� (������������ � ���������� �������������� ����� ���� O2M_UNNM_<��� ����������>)
		if (Type) {
			//��������� ���� ����, �� ���. ��������� ���������
			Type->name = new char[strlen("O2M_UNNM_") + strlen(name) + 1];
			strcpy(Type->name, "O2M_UNNM_");
			strcat(Type->name, name);
			//��� ���� RECORD ���������� RuntimeId
			if (id_CRecordType == Type->name_id) static_cast<CRecordType*>(Type)->InitRuntimeId();
			//������ ���� ����
			Type->WriteCPP(f, to_h);
			//������ ���� ���������� ����������
			fprintf(ff, "O2M_UNNM_%s* %s", name, name);
			return;
		}

	}//else

	/**/
	//� ����������, ��� ��. �� ������������� ���, ����������� � ���������� ���������,
	//��������� ��� � ������������� ������ � ���. ������������ ���� ���������
}


//-----------------------------------------------------------------------------
//������ ���� ��� ������������� ������������ ��������, ���������������, ��� is_const == true
void CPointerVar::WriteCPP_ConstValue(CPP_files& f)
{
	//����������� ��������� ��������� ����� ���� ������ NIL
	fprintf(f.fc, "0");
}


//-----------------------------------------------------------------------------
//������ ���� CPointerVar � ���� .dfn
void CPointerVar::WriteDFN(DFN_file& f)
{
	//������ � ������ ������������ ����
	if (type_name) {
		fprintf(f.f, "%s : ", name);
		//� ������ ��. �� ����������� ��� ����� ��������� "POINTER TO"
		if (!qualident_type) fprintf(f.f, "POINTER TO ");
		if (type_module_name) fprintf(f.f, "%s.", type_module_name);
		fprintf(f.f, "%s", type_name);
		return;
	}//if

	//������ ������ ���������� �������������� ���� ���������
	fprintf(f.f, "%s%s : POINTER TO ", name, is_read_only ? "-" : "");

	//��������� � ������ ���������� �������������� ����
	CBaseType* BT = FindType();
	BT->WriteDFN(f);

}//WriteDFN


//-----------------------------------------------------------------------------
//�������� ����� ���������� (��� ����������� ������ � ������ ��������)
CBaseVar* CProcedureVar::CreateVar(const CBaseName* parent) const
{
	//�������� ����������
	CBaseVar* BV = new CProcedureVar(parent);
	Assign(BV);

	//����������� ���������� ���������� � ��������� ����������
	FormalPars.Assign(static_cast<CProcedureVar*>(BV)->FormalPars, parent);

	return BV;
}


//-----------------------------------------------------------------------------
//��������� ���� ���������� ��������� (��� ��������� - �������) ��� id_CBaseName
EName_id CProcedureVar::GetResultId() const
{
	//�������� ����������-��������� (�� ����� ���� ����������)
	if (!FormalPars.Qualident) return id_CBaseName;
	//������� ���� ���������� ���������
	return FormalPars.Qualident->TypeResultId;
}


//-----------------------------------------------------------------------------
//������ ���� CProcedureVar ��� ���������� ����������
void CProcedureVar::WriteCPP(CPP_files& f)
{
	//�������� ������� ����� ����
	if (type_name) {
		if (type_module_name) fprintf(f.fc, "%s::", type_module_name);

		if (external) {
			fprintf(f.fh, "extern ");
			if (type_module_name) {
				CBaseName* BN = parent_element->GetGlobalName(type_module_name);
				if (BN) fprintf(f.fh, "%s::", static_cast<CImportModule*>(BN)->real_name);
			}
			fprintf(f.fh, "%s %s", type_name, name);
			fprintf(f.fc, "%s %s::%s", type_name, parent_element->name, name);
		} else
			fprintf(f.fc, "%s %s", type_name, name);

		return;
	}

	//��� ��������� ������ �������� ����� (.h ��� .cpp)
	TFileType* const ff = external ? f.fh : f.fc;

	//������ ���� ��� ������� (���� ����) ��� void
	if (FormalPars.Qualident) FormalPars.Qualident->WriteCPP_type(f, external, parent_element);
	else fprintf(ff, "void");
	fprintf(ff, " (*%s) (", name);
	FormalPars.WriteCPP_pars(f, external);
	fprintf(ff, ")");

}//WriteCPP


//-----------------------------------------------------------------------------
//������ ���� ���������� � ���������� ���������
void CProcedureVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	//�������� ������� ����� ����
	if (type_name)
		WriteCPP_fp_named_type(f, to_h);
	else {
		//��� ��������� ������ �������� ����� (.h ��� .cpp)
		TFileType* const ff = external ? f.fh : f.fc;

		//������ ���� ��� ������� (���� ����) ��� void
		if (FormalPars.Qualident) FormalPars.Qualident->WriteCPP_type(f, external, parent_element);
		else fprintf(ff, "void");
		fprintf(ff, " (*%s) (", name);
		FormalPars.WriteCPP_pars(f, external);
		fprintf(ff, ")");
	}//else
}


//-----------------------------------------------------------------------------
//������ ���� CProcedureVar � ���� .dfn
void CProcedureVar::WriteDFN(DFN_file& f)
{
	if (is_const) throw error_Internal("CProcedureVar::WriteDFN");

	if (!type_name)
		fprintf(f.f, "%s%s : PROCEDURE", name, is_read_only ? "-" : "");
	else
		WriteDFN_named_type(f);
}//WriteDFN


//-----------------------------------------------------------------------------
//�������� ����� ���������� (��� ����������� ������ � ������ ��������)
CBaseVar* CPtrVar::CreateVar(const CBaseName* parent) const
{
	CBaseVar* BV = new CPtrVar(parent);
	Assign(BV);
	return BV;
}


//-----------------------------------------------------------------------------
//������ ���� CPtrVar ��� ���������� ����������
void CPtrVar::WriteCPP(CPP_files& f)
{
	if (external) {
		fprintf(f.fh, "extern bool %s", name);
		fprintf(f.fc, "bool %s::%s%s", parent_element->name, is_var ? "&" : "", name);
	}
	else
		fprintf(f.fc, "bool %s%s", is_var ? "&" : "", name);
}//WriteCPP


//-----------------------------------------------------------------------------
//������ ���� ���������� � ���������� ���������
void CPtrVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	//�������� ������� ����� ����
	if (type_name) {
		WriteCPP_fp_named_type(f, to_h);
		return;
	}
	//������ ���������� � ����
	fprintf(to_h ? f.fh : f.fc, "bool %s%s", is_var ? "&" : "", name);
}


//-----------------------------------------------------------------------------
//������ ���� CPtrVar � ���� .dfn
void CPtrVar::WriteDFN(DFN_file& f)
{
	if (is_const) throw error_Internal("CPtrVar::WriteDFN");

	if (!type_name)
		fprintf(f.f, "%s%s : BOOLEAN", name, is_read_only ? "-" : "");
	else
		WriteDFN_named_type(f);
}//WriteDFN


//-----------------------------------------------------------------------------
//�������� ���������, ���������������, ��� is_const == true
CBaseVar* CRealVar::CreateConst(const CBaseName *parent) const
{
	CRealVar* RV = static_cast<CRealVar*>(CRealVar::CreateVar(parent));

	//����������� ������
	RV->ConstValue = ConstValue;

	return RV;
}


//-----------------------------------------------------------------------------
//�������� ����� ���������� (��� ����������� ������ � ������ ��������)
CBaseVar* CRealVar::CreateVar(const CBaseName* parent) const
{
	CBaseVar* BV = new CRealVar(parent);
	Assign(BV);
	return BV;
}


//-----------------------------------------------------------------------------
//�������������� ��������� ������ � �������� ����������
void CRealVar::SetConstValue(const char *st)
{
	is_const = true;
	ConstValue = atof(st);
}//StoreData


//-----------------------------------------------------------------------------
//������ ���� CRealVar ��� ���������� ����������
void CRealVar::WriteCPP(CPP_files& f)
{
	//������ ���������� ����������
	if (external) {
		fprintf(f.fh, "extern %s %s", CRealType::GetCPPTypeName(), name);
		fprintf(f.fc, "%s %s::%s%s", CRealType::GetCPPTypeName(), parent_element->name, is_var ? "&" : "", name);
	} else
		fprintf(f.fc, "%s %s%s", CRealType::GetCPPTypeName(), is_var ? "&" : "", name);

	//������ ������������� ��������� (���� ����)
	if (is_const) fprintf(f.fc, " = %g", ConstValue);
}//WriteCPP


//-----------------------------------------------------------------------------
//������ ���� ���������� � ���������� ���������
void CRealVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	//�������� ������� ������������ ����
	if (type_name)
		WriteCPP_fp_named_type(f, to_h);
	else
		fprintf(to_h ? f.fh : f.fc, "%s %s%s", CRealType::GetCPPTypeName(), is_var ? "&" : "", name);
}


//-----------------------------------------------------------------------------
//������ ���� ��� ������������� ������������ ��������, ���������������, ��� is_const == true
void CRealVar::WriteCPP_ConstValue(CPP_files& f)
{
	fprintf(f.fc, "%g", ConstValue);
}


//-----------------------------------------------------------------------------
//������ ���� CRealVar � ���� .dfn
void CRealVar::WriteDFN(DFN_file& f)
{
	if (is_const) {
		fprintf(f.f, "%s = %E", name, ConstValue);
		return;
	}

	if (!type_name)
		fprintf(f.f, "%s%s : REAL", name, is_read_only ? "-" : "");
	else
		WriteDFN_named_type(f);
}//WriteDFN


//-----------------------------------------------------------------------------
//���������� ���������� ��-�� � ������ �����
void CRecordVar::AddName(CBaseName* BN) const
{
	if (!CBaseVar::IsVarId(BN->name_id)) throw error_Internal("CRecordVar::AddName");
	FieldStore.push_back(static_cast<CBaseVar*>(BN));
}


//-----------------------------------------------------------------------------
//�������� ������������� ����� � ���� ����� ������
int CRecordVar::CheckComplete(CLexBuf *lb)
{
	//�������� ����������������� ���� ���������� (����� �� ��� �������� ��� ������������� DFN ������)
	if (GetTypeModuleAlias()) return 0;
	//�������� ������������� ����� ������
	TFieldStore::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci) {
		int err_num = (*ci)->CheckComplete(lb);
		if (err_num) return err_num;
	}
	return 0;
}


//-----------------------------------------------------------------------------
//�������� ����� ���������� (��� ����������� ������ � ������ ��������)
CBaseVar* CRecordVar::CreateVar(const CBaseName* parent) const
{
	CBaseVar* BV = new CRecordVar(parent);
	Assign(BV);
	//����������� ����� ������
	TFieldStore::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci)
		BV->AddName((*ci)->CreateVar(BV));
	return BV;
}


//-----------------------------------------------------------------------------
//���������� ������� CRecordVar
CRecordVar::~CRecordVar()
{
	//������� ������ ����� ������
	TFieldStore::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci)
		delete *ci;
	//������� ����� �������� ����
	delete Qualident;
}//~CRecordVar


//-----------------------------------------------------------------------------
//����� ����� � ������ �����, NULL - ���� ��� �� �������
CBaseName* CRecordVar::FindName(const char* search_name) const
{
	//�������� ������� ������������ ����
	if (type_name) {
		//������������ ����� ����� ����� ���� �� ������ ������� ��������� � ����� ���������
		CBaseName* BN = parent_element->GetGlobalName(type_module_name, type_name);
		return static_cast<CRecordType*>(BN)->FindName(search_name);
	}

	//����� ����� � ����������� ������ ����� ������
	//(������������ ��� ���������� �������������� ����)
	TFieldStore::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci)
		if (!strcmp( (*ci)->name, search_name ))
			return *ci;

	//�������� ������� �������� ����
	if (Qualident) {
		//��������� ��. �� ������ �������� ���� (������ ��������������)
		CBaseName* BN = parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident);
		return static_cast<CRecordType*>(BN)->FindName(search_name);
	}

	//��� �� �������
	return NULL;
}//FindName


//-----------------------------------------------------------------------------
//��������� ���� ��� ���������� ����������
void CRecordVar::WriteCPP(CPP_files& f)
{
	//�������� ������� ����� ����
	if (type_name) {
		if (type_module_name) fprintf(f.fc, "%s::", type_module_name);

		if (external) {
			fprintf(f.fh, "extern ");
			if (type_module_name) {
				CBaseName* BN = parent_element->GetGlobalName(type_module_name);
				if (BN) fprintf(f.fh, "%s::", static_cast<CImportModule*>(BN)->real_name);
			}
			fprintf(f.fh, "%s %s", type_name, name);
			fprintf(f.fc, "%s %s::%s", type_name, parent_element->name, name);
		} else
			fprintf(f.fc, "%s %s", type_name, name);

		return;
	}

	//��������� ���������� ���������� �������������� ����

	//��� ���������� ���������� ��� extern ��������� ������� ��� ����
	if (external)
		fprintf(f.fh, "struct O2M_UNNM_%s {\n", name);
	else
		fprintf(f.fc, "struct {\n");
	//��������� ���� ����� ������
	TFieldStore::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci) {
		fprintf(external ? f.fh : f.fc, "\t");
		(*ci)->WriteCPP_fp(f, external);
		fprintf(external ? f.fh : f.fc, ";\n");
	}
	//��������� ���������� ���������� � ������ ��������
	if (external) {
		fprintf(f.fh, "};\nextern O2M_UNNM_%s %s", name, name);
		fprintf(f.fc, "O2M_UNNM_%s %s::%s", name, parent_element->name, name);
	} else
		fprintf(f.fc, "} %s", name);
}


//-----------------------------------------------------------------------------
//������ ���� ���������� � ���������� ���������
void CRecordVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	//�������� ������� ����� ���� � ���������� �������� ���������-����������
	if (type_name && !is_var) {
		WriteCPP_fp_named_type(f, to_h);
		return;
	}

	//�������� �������� ���������-����������
	if (is_var) {
		if (!to_h) {//������ ���� � .cpp ����
			if (type_module_name) fprintf(f.fc, "%s::", type_module_name);
			fprintf(f.fc, "%s* %s", type_name, name);
		}
		else {//������ ���� � .h ����
			if (type_module_name) {
				//����� ������ �� ��� ����������
				CBaseName* BN = parent_element->GetGlobalName(type_module_name);
				//������ �������� �� ���������� ����� ������
				if ( BN && (BN->name_id == id_CImportModule) )
					fprintf(f.fh, "%s::", static_cast<CImportModule*>(BN)->real_name);
			}
			//������ ���� � �������� ����������
			fprintf(f.fh, "%s* %s", type_name, name);
		}//else

		return;
	}//if


	//������������� ��� ������ (��������, ������ ������ ������)
	//��������� �������� ���� (���������� �������������� ����)
	fprintf(to_h ? f.fh : f.fc, "struct {\n\t");
	//������ ���� ����� ������
	TFieldStore::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci) {
		(*ci)->WriteCPP_fp(f, to_h);
		fprintf(to_h ? f.fh : f.fc, ";\n\t");
	}
	fprintf(to_h ? f.fh : f.fc, "} %s", name);

}


//-----------------------------------------------------------------------------
//������ ���� ��� ������������� CRecordVar ��� �������� ���� ���������
void CRecordVar::WriteCPP_pointer(CPP_files& f)
{
	//������ � .h ����
	if (external) {
		fprintf(f.fh, "struct %s", name);
		
		//������ �������� ���� (���� ����)
		if (Qualident) {
			fprintf(f.fh, " : ");
			CBaseName* BN = parent_element->GetGlobalName(Qualident->pref_ident);
			if (BN) fprintf(f.fh, "%s::", static_cast<CImportModule*>(BN)->real_name);
			fprintf(f.fh, "%s", Qualident->ident);
		}
		fprintf(f.fh, " {\n");

		//������ ���� ������ ��� ��������� ��. ���� ������� ���������� �� ����� ����������
		fprintf(f.fh, "\t%sconst char* O2M_SYS_ID() {return \"%s\";};\n", Qualident ? "" : "virtual ", name);

		//������ ���� ����� ������
		TFieldStore::const_iterator ci;
		for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci) {
			(*ci)->WriteCPP_fp(f, true);
			fprintf(f.fh, ";\n");
		}
		//���������� ������ ���� ������
		fprintf(f.fh, "};\n");

		return;
	}


	//������ � .cpp ����
	fprintf(f.fc, "struct %s", name);

	//������ �������� ���� (���� ����)
	if (Qualident) {
		fprintf(f.fc, " : ");
		Qualident->WriteCPP_type(f, false, parent_element);
	}
	fprintf(f.fc, " {\n");

	//������ ���� ������ ��� ��������� ��. ���� ������� ���������� �� ����� ����������
	fprintf(f.fc, "\t%sconst char* O2M_SYS_ID() {return \"%s\";};\n", Qualident ? "" : "virtual ", name);

	//������ ���� ����� ������
	TFieldStore::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci) {
		(*ci)->WriteCPP_fp(f, false);
		fprintf(f.fc, ";\n");
	}
	fprintf(f.fc, "};\n");

}//WriteCPP_pointer


//-----------------------------------------------------------------------------
//������ ���� CRecordVar � ���� .dfn
void CRecordVar::WriteDFN(DFN_file& f)
{
	if (is_const) throw error_Internal("CRecordVar::WriteDFN");

	if (!type_name) {

		fprintf(f.f, "%s%s : RECORD", name, is_read_only ? "-" : "");
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
		TFieldStore::const_iterator ci;
		for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci)
			if ((*ci)->external)
			{
				(*ci)->WriteDFN(f);
				fprintf(f.f, ";\n");
				f.tab();
			}
		//����� ��������� ���� ������
		fprintf(f.f, "END");

		//���������� ������� � DFN �����
		f.tab_level--;

	} else
		WriteDFN_named_type(f);
}//WriteDFN


//-----------------------------------------------------------------------------
//�������� ���������, ���������������, ��� is_const == true
CBaseVar* CSetVar::CreateConst(const CBaseName *parent) const
{
	//�������� ����������
	CSetVar* SV = static_cast<CSetVar*>(CSetVar::CreateVar(parent));
	//����������� ������������ ��������
	SV->ConstValue = ConstValue;
	//��������� �������
	return SV;
}


//-----------------------------------------------------------------------------
//�������� ����� ���������� (��� ����������� ������ � ������ ��������)
CBaseVar* CSetVar::CreateVar(const CBaseName* parent) const
{
	//�������� ����� ���������� � ����������
	CSetVar* SV = new CSetVar(parent);
	Assign(SV);
	return SV;
}


//-----------------------------------------------------------------------------
//���������� ������� CSetVar
CSetVar::~CSetVar()
{
	CBaseVector::iterator vi;
	for (vi = SetElems.begin(); vi != SetElems.end(); vi++)
		delete *vi;
}//~CSetVar


//-----------------------------------------------------------------------------
//��������� ������������ ���������, ���� ���� (����� �� ����), ��� ������������� ������
//������ �������������� ����������� ���� ��������� ��������
int CSetVar::SetInit(CLexBuf *lb)
{
	DECL_SAVE_POS
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//��� �������� ������������� ������������ ���������
	is_const = true;
	
	//�������� ������� "}" (���������� ��������� - ����������� ����������� ������� ���������)
	if (lb->ReadLex(li) && lex_k_cl_brace == li.lex) return 0;

	RESTORE_POS

	while (true) {
		//�������� ��-�� ���������
		CSetVarElem *SE = new CSetVarElem(parent_element);
		int err_num = SE->Init(lb);
		if (err_num) {
			delete SE;
			return err_num;
		}
		//��������� ��-�� ��������� � ������
		SetElems.push_back(SE);

		//��������, �������� �� ������ ����������� ��������� ����������� (��� ��-�� ������ ���� �����������)
		is_const = is_const && SE->IsConst();

		//�������� ������� ","
		if (!lb->ReadLex(li) || lex_k_comma != li.lex) break;
	}

	//�������� ������� "}" (����� ������������ ���������)
	if (lex_k_cl_brace != li.lex) return s_e_ClBraceMissing;

	//� ������, ���� ��� ��-�� ��������� ����������, ��������� ConstValue
	if (is_const) {
		CBaseVector::iterator vi;
		for (vi = SetElems.begin(); vi != SetElems.end(); vi++) {
			//���������� ���������� ConstValue
			ConstValue |= static_cast<CSetVarElem*>(*vi)->GetConstValue();
			//��-� SetVarElem ��� �� �����
			delete *vi;
		}
		//������� ������ (�� ��� �� �����)
		SetElems.clear();
	}

	return 0;
}//Init CSetVar


//-----------------------------------------------------------------------------
//������ ���� CSetVar ��� ���������� ����������
void CSetVar::WriteCPP(CPP_files& f)
{
	//������ ���������� ����������
	if (external) {
		fprintf(f.fh, "extern %s %s", CSetType::GetCPPTypeName(), name);
		fprintf(f.fc, "%s %s::%s%s", CSetType::GetCPPTypeName(), parent_element->name, is_var ? "&" : "", name);
	} else
		fprintf(f.fc, "%s %s%s", CSetType::GetCPPTypeName(), is_var ? "&" : "", name);

	//������ ������������� ��������� (���� ����)
	if (is_const) {
		fprintf(f.fc, " = ");
		CSetVar::WriteCPP_ConstValue(f);
	}
}//WriteCPP


//-----------------------------------------------------------------------------
//������ ���� ���������� � ���������� ���������
void CSetVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	//�������� ������� ������������ ����
	if (type_name)
		WriteCPP_fp_named_type(f, to_h);
	else
		fprintf(to_h ? f.fh : f.fc, "%s %s%s", CSetType::GetCPPTypeName(), is_var ? "&" : "", name);
}


//-----------------------------------------------------------------------------
//������ ���� ��� ������������� ������������ ��������, ���������������, ��� is_const == true
void CSetVar::WriteCPP_ConstValue(CPP_files& f)
{
	//�������� ������� ������������� ������������ ��������
	if (is_const)
		fprintf(f.fc, "%i", ConstValue);
	else {
		//�������� ������� ��-��� � ���������
		if (SetElems.empty())
			fprintf(f.fc, "0");
		else {
			fprintf(f.fc, "(");
			CBaseVector::const_iterator ci = SetElems.begin();
			static_cast<CSetVarElem*>(*ci)->CSetVarElem::WriteCPP(f);
			for (ci++; ci != SetElems.end(); ci++) {
				fprintf(f.fc, " | ");
				static_cast<CSetVarElem*>(*ci)->CSetVarElem::WriteCPP(f);
			}
			fprintf(f.fc, ")");
		}
	}//else
}


//-----------------------------------------------------------------------------
//������ ���� CSetVar � ���� .dfn
void CSetVar::WriteDFN(DFN_file& f)
{
	if (is_const) throw error_Internal("CSetVar::WriteDFN");

	if (!type_name)
		fprintf(f.f, "%s%s : SET", name, is_read_only ? "-" : "");
	else
		WriteDFN_named_type(f);
}//WriteDFN


//-----------------------------------------------------------------------------
//���������� ������� CSetVarElem
CSetVarElem::~CSetVarElem()
{
	delete LowExpr;
	delete HighExpr;
}//~CSetVarElem


//-----------------------------------------------------------------------------
//������������� ������� CSetVarElem
int CSetVarElem::Init(CLexBuf *lb)
{
	//������������� ������� ���������
	LowExpr = new CExpr(parent_element);
	int err_num = LowExpr->Init(lb);
	if (err_num) return err_num;

	//�������� ���� ���������
	if (!CBaseVar::IsIntId(LowExpr->GetResultId())) return s_e_SetElemType;

	//������� ���������� ���������
	CBaseVar* BV;
	err_num = LowExpr->CreateConst(BV);
	if (!err_num) {
		LowBoundValue = BV->GetIntValue();
		delete BV;
		delete LowExpr;
		LowExpr = NULL;
		//�������� ����������� �������� ���������
		if (SET_MAX < LowBoundValue || 0 > LowBoundValue) return s_e_SetElemRange;
		//�������������� ��������� �������� � ���������
		SetValue = 1 << LowBoundValue;
	}

	DECL_SAVE_POS
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� ".." (������� ���������)
	if (!lb->ReadLex(li) || lex_k_dots != li.lex) {
		RESTORE_POS
		return 0;
	}

	IsRange = true;

	//������������� ������� ���������
	HighExpr = new CExpr(parent_element);
	err_num = HighExpr->Init(lb);
	if (err_num) return err_num;

	//�������� ���� ������� ���������
	if (!CBaseVar::IsIntId(HighExpr->GetResultId())) return s_e_SetElemType;

	//������� ���������� ��������� ��� ������� ���������
	err_num = HighExpr->CreateConst(BV);
	if (!err_num) {
		HighBoundValue = BV->GetIntValue();
		delete BV;
		delete HighExpr;
		HighExpr = NULL;
		//�������� ����������� �������� ���������
		if (SET_MAX < HighBoundValue || 0 > HighBoundValue) return s_e_SetElemRange;
		//�������� ����������� ��������� ������������ ���������
		if (!LowExpr) {
			//���������� ��������� - ��������� � ������ ����������� �������� ���������
			if (LowBoundValue > HighBoundValue) return s_e_SetRange;
			for (int i = LowBoundValue + 1; i <= HighBoundValue; i++) SetValue |= 1 << i;
		}
	}//if

	return 0;
}//Init CSetVarElem


//-----------------------------------------------------------------------------
//������ ���� CSetVarElem
void CSetVarElem::WriteCPP(CPP_files &f)
{
	//�������� ����������� ������������� ������������ ��������
	if (IsRange && (LowExpr || HighExpr)) {
		fprintf(f.fc, "O2M_SET_RANGE(");	//��������� ������ ������� - ������������� ������� ��������� (�� ���������)
		if (LowExpr) LowExpr->WriteCPP(f); else fprintf(f.fc, "%i", LowBoundValue);	//����� ���. ������������ ������ ������� ���������
		fprintf(f.fc, ",");
		if (HighExpr) HighExpr->WriteCPP(f); else fprintf(f.fc, "%i", HighBoundValue);	//����� ���. ������������ ������� ������� ���������
		fprintf(f.fc, ")");
	} else
		if (LowExpr) {
			//��� ��������� ��� ������������ ���������, ��������� �������������� �������������� ��������� � ���������
			fprintf(f.fc, "1 << (");
			LowExpr->WriteCPP(f);
			fprintf(f.fc, ")");
		} else
			fprintf(f.fc, "%i", SetValue);	//����� ������������ ��������� ������������ ���������
}


//-----------------------------------------------------------------------------
//�������� ���������, ���������������, ��� is_const == true
CBaseVar* CShortintVar::CreateConst(const CBaseName *parent) const
{
	CShortintVar* SV = static_cast<CShortintVar*>(CShortintVar::CreateVar(parent));

	//����������� ������ � ��������� ����������
	SV->ConstValue = ConstValue;

	return SV;
}


//-----------------------------------------------------------------------------
//�������� ����� ���������� (��� ����������� ������ � ������ ��������)
CBaseVar* CShortintVar::CreateVar(const CBaseName* parent) const
{
	CBaseVar* BV = new CShortintVar(parent);
	Assign(BV);
	return BV;
}


//-----------------------------------------------------------------------------
//������ ���� CShortintVar ��� ���������� ����������
void CShortintVar::WriteCPP(CPP_files& f)
{
	//������ ���������� ����������
	if (external) {
		fprintf(f.fh, "extern %s %s", CShortintType::GetCPPTypeName(), name);
		fprintf(f.fc, "%s %s::%s%s", CShortintType::GetCPPTypeName(), parent_element->name, is_var ? "&" : "", name);
	} else
		fprintf(f.fc, "%s %s%s", CShortintType::GetCPPTypeName(), is_var ? "&" : "", name);

	//������ ������������� ��������� (���� ����)
	if (is_const) fprintf(f.fc, " = %i", ConstValue);
}//WriteCPP


//-----------------------------------------------------------------------------
//������ ���� ���������� � ���������� ���������
void CShortintVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	//�������� ������� ������������ ����
	if (type_name)
		WriteCPP_fp_named_type(f, to_h);
	else
		fprintf(to_h ? f.fh : f.fc, "%s %s%s", CShortintType::GetCPPTypeName(), is_var ? "&" : "", name);
}


//-----------------------------------------------------------------------------
//������ ���� ��� ������������� ������������ ��������, ���������������, ��� is_const == true
void CShortintVar::WriteCPP_ConstValue(CPP_files& f)
{
	fprintf(f.fc, "%hi", ConstValue);
}


//-----------------------------------------------------------------------------
//������ ���� CShortintVar � ���� .dfn
void CShortintVar::WriteDFN(DFN_file& f)
{
	if (is_const) {
		fprintf(f.f, "%s = %i", name, ConstValue);
		return;
	}

	if (!type_name)
		fprintf(f.f, "%s%s : SHORTINT", name, is_read_only ? "-" : "");
	else
		WriteDFN_named_type(f);
}//WriteDFN

