//=============================================================================
// �������� �������� - ����������� ��������
//=============================================================================

#include "StdProc.h"
#include "Type.h"
#include "Var.h"

#include <math.h>
#include <float.h>


//-----------------------------------------------------------------------------
//�������� ���������
int CAbsStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	//��������� ��������� �� ��������� � ���������� ����������
	int err_num = Expr.CreateConst(BaseConst);
	if (err_num) return err_num;
	//���������� �������� ������� �� ���������� �������� ����������� ���������
	switch (ResultId) {
	case id_CShortintVar:
		static_cast<CShortintVar*>(BaseConst)->ConstValue = abs(static_cast<CShortintVar*>(BaseConst)->ConstValue);
		break;
	case id_CIntegerVar:
		static_cast<CIntegerVar*>(BaseConst)->ConstValue = abs(static_cast<CIntegerVar*>(BaseConst)->ConstValue);
		break;
	case id_CLongintVar:
		static_cast<CLongintVar*>(BaseConst)->ConstValue = labs(static_cast<CLongintVar*>(BaseConst)->ConstValue);
		break;
	case id_CRealVar:
		static_cast<CRealVar*>(BaseConst)->ConstValue = fabs(static_cast<CRealVar*>(BaseConst)->ConstValue);
		break;
	case id_CLongrealVar:
		static_cast<CLongrealVar*>(BaseConst)->ConstValue = fabs(static_cast<CLongrealVar*>(BaseConst)->ConstValue);
	}
	return 0;
}


//-----------------------------------------------------------------------------
//������������� ���������-������� ABS
int CAbsStdProcFunc::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex)
		return s_e_OpBracketMissing;

	//��������� ���������
	int err_num = Expr.Init(lb);
	if (err_num) return err_num;

	//�������� ���� ���������
	ResultId = Expr.GetResultId();
	if (!CBaseVar::IsDigitId(ResultId)) return s_e_Incompatible;

	//�������� ������� ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex)
		return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� ���������-������� ABS
void CAbsStdProcFunc::WriteCPP(CPP_files& f)
{
	if (id_CLongintVar == ResultId) fprintf(f.fc, "l"); else
		if (CBaseVar::IsRealId(ResultId)) fprintf(f.fc, "f");
	fprintf(f.fc, "abs(");
	Expr.WriteCPP(f);
	fprintf(f.fc, ")");
}//WriteCPP


//-----------------------------------------------------------------------------
//�������� ���������
int CAshStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	//��������� ��������� �� x-��������� � ���������� ����������
	CBaseVar* vX;
	int err_num = ExprX.CreateConst(vX);
	if (err_num) return err_num;
	//��������� ��������� �� n-��������� � ���������� ����������
	CBaseVar* vN;
	err_num = ExprN.CreateConst(vN);
	if (err_num) {
		delete vX;
		return err_num;
	}
	//�������� ������������� ���������, ������ � ��������� � ��� �������� �������
	BaseConst = new CLongintVar(parent_element);
	static_cast<CLongintVar*>(BaseConst)->ConstValue = vX->GetIntValue() << vN->GetIntValue();
	//����������� ������������� ��������
	delete vX;
	delete vN;

	return 0;
}


//-----------------------------------------------------------------------------
//������������� ���������-������� ASH
int CAshStdProcFunc::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex)
		return s_e_OpBracketMissing;

	//��������� ������� ���������
	int err_num = ExprX.Init(lb);
	if (err_num) return err_num;

	//�������� ������� ","
	if (!lb->ReadLex(li) || lex_k_comma != li.lex) return s_e_CommaMissing;

	//��������� ������� ���������
	err_num = ExprN.Init(lb);
	if (err_num) return err_num;

	//�������� ����� ���������
	if (!CBaseVar::IsIntId(ExprX.GetResultId())) return s_e_Incompatible;
	if (!CBaseVar::IsIntId(ExprN.GetResultId())) return s_e_Incompatible;
	ResultId = id_CLongintVar;

	//�������� ������� ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex)
		return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� ���������-������� ASH
void CAshStdProcFunc::WriteCPP(CPP_files& f)
{
	fprintf(f.fc, "((");
	ExprX.WriteCPP(f);
	fprintf(f.fc, ") << (");
	ExprN.WriteCPP(f);
	fprintf(f.fc, "))");
}//WriteCPP


//-----------------------------------------------------------------------------
//�������� ���������
int CCapStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	//��������� ��������� �� ��������� � ���������� ����������
	int err_num = Expr.CreateConst(BaseConst);
	if (err_num) return err_num;
	//���������� �������� ������� �� ���������� �������� ����������� ���������
	static_cast<CCharVar*>(BaseConst)->ConstValue = toupper(static_cast<CCharVar*>(BaseConst)->ConstValue);
	return 0;
}


//-----------------------------------------------------------------------------
//������������� ��������� CAP
int CCapStdProcFunc::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//��������� ���������
	int err_num = Expr.Init(lb);
	if (err_num) return err_num;

	//�������� ���� ���������
	if (Expr.GetResultId() != id_CCharVar) return s_e_Incompatible;

	//�������� ������� ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	//CAP ������ ���������� �������� ���� CHAR
	ResultId = id_CCharVar;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� ���������-������� CAP
void CCapStdProcFunc::WriteCPP(CPP_files& f)
{
	//�������� ������� ���������� ����������
	CArrayVar* AV = static_cast<CArrayVar*>(Expr.FindLastName());
	if (AV && (id_CArrayVar == AV->name_id) && AV->ConstString) {
		//����� ������ �� 1 ������� (��������� � Init), ������� 1 ������
		fprintf(f.fc, "toupper('%c')", AV->ConstString[0]);
		return;
	}

	//������ ��������� �������
	fprintf(f.fc, "toupper(");
	Expr.WriteCPP(f);
	fprintf(f.fc, ")");
}//WriteCPP


//-----------------------------------------------------------------------------
//�������� ���������
int CChrStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	//��������� ��������� �� ��������� � ���������� ����������
	int err_num = Expr.CreateConst(BaseConst);
	if (err_num) return err_num;
	//��������� �������������� ��������
	long int_data = BaseConst->GetIntValue();
	//������ ������������� ��������� �� ����������
	delete BaseConst;
	BaseConst = new CCharVar(parent_element);
	//���������� �������� ������� �� ���������� �������� ����������� ���������
	static_cast<CCharVar*>(BaseConst)->ConstValue = int_data;
	return 0;
}


//-----------------------------------------------------------------------------
//������������� ���������-������� CHR
int CChrStdProcFunc::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//��������� ���������
	int err_num = Expr.Init(lb);
	if (err_num) return err_num;

	//�������� ���� ���������
	if (!CBaseVar::IsIntId(Expr.GetResultId())) return s_e_Incompatible;
	ResultId = id_CCharVar;

	//�������� ������� ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� ���������-������� CHR
void CChrStdProcFunc::WriteCPP(CPP_files& f)
{
	Expr.WriteCPP(f);
}//WriteCPP


//-----------------------------------------------------------------------------
//�������� ���������
int CEntierStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	//��������� ��������� �� ��������� � ���������� ����������
	int err_num = Expr.CreateConst(BaseConst);
	if (err_num) return err_num;
	//��������� ������������� ��������
	double real_data;
	switch (BaseConst->GetResultId()) {
	case id_CRealVar:
		real_data = static_cast<CRealVar*>(BaseConst)->ConstValue;
		break;
	case id_CLongrealVar:
		real_data = static_cast<CLongrealVar*>(BaseConst)->ConstValue;
	default:
		//������ ���� �� ������, ��. CEntierStdProcFunc::Init
		real_data = 0;
	}
	//������ ������������ ��������� �� ������������� (LONGINT)
	delete BaseConst;
	BaseConst = new CLongintVar(parent_element);
	//���������� �������� ������� �� ���������� �������� ����������� ���������
	static_cast<CLongintVar*>(BaseConst)->ConstValue = static_cast<long>(floor(real_data));
	return 0;
}


//-----------------------------------------------------------------------------
//������������� ���������-������� ENTIER
int CEntierStdProcFunc::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//��������� ������� ���������
	int err_num = Expr.Init(lb);
	if (err_num) return err_num;

	//�������� ���� ���������
	if (!CBaseVar::IsRealId(Expr.GetResultId())) return s_e_Incompatible;
	ResultId = id_CLongintVar;

	//�������� ������� ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� ���������-������� ENTIER
void CEntierStdProcFunc::WriteCPP(CPP_files& f)
{
	fprintf(f.fc, "long(floor(");
	Expr.WriteCPP(f);
	fprintf(f.fc, "))");
}//WriteCPP


//-----------------------------------------------------------------------------
//�����������
CLenStdProcFunc::CLenStdProcFunc(const CBaseName *parent) : CStdProcFunc(parent),
	array_name(NULL),
	dimension(0),
	array_size(0)
{
}


//-----------------------------------------------------------------------------
//����������
CLenStdProcFunc::~CLenStdProcFunc()
{
	delete[] array_name;
}


//-----------------------------------------------------------------------------
//�������� ���������
int CLenStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	//�������� ���� ������� (��� ��������� ������� ������ ������� ���������)
	if (!array_size) return s_e_ExprNotConst;
	//�������� ��������� (��� LONGINT)
	BaseConst = new CLongintVar(parent_element);
	static_cast<CLongintVar*>(BaseConst)->ConstValue = array_size;
	return 0;
}


//-----------------------------------------------------------------------------
//������������� ��������� LEN
int CLenStdProcFunc::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//��������� ��������� � ����� ������ id_CLongintVar
	ResultId = id_CLongintVar;

	//�������� ������� "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//�������� ������� �������� ������� ��� ��������� ������
	if (!lb->ReadLex(li) || (lex_i != li.lex && lex_s != li.lex)) return s_e_LEN_NotArray;

	//��������� ��������� ������
	if (lex_s == li.lex) {
		//����������� ����� ������
		array_size = strlen(li.st);
		if (array_size <= 1) return s_e_LEN_NotArray;
		//�������� ������� ")"
		if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;
		//��������� ��������� ������ ���������
		return 0;
	}

	//���������� �������� �������
	array_name = str_new_copy(li.st);

	//��������� ������� ������� � ��������� ������
	CBaseName *BN = parent_element->GetGlobalName(array_name);
	if (!BN || (id_CArrayVar != BN->name_id)) return s_e_LEN_NotArray;

	//�������� ������� "," ��� ")"
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_ClBracketMissing;
	if (lex_k_comma == li.lex) {
		//�������� ���������
		CBaseVar* BV;
		int err_num = ConstSelector(lb, BV, parent_element);
		if (err_num) return err_num;
		//��������� ������ ��������� ����������� �������
		if (CBaseVar::IsIntId(BV->name_id))
			dimension = static_cast<CBaseVar*>(BV)->GetIntValue();
		else {
			delete BV;
			return s_e_ParamNotIntConst;
		}
		//����������� ��������� (������ �� �����)
		delete BV;
		//�������� ������� ����. �������
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_ClBracketMissing;
	}
	//�������� ������� ")"
	if (lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	//������� ������������ �������� ��������� (���������� ����������� ���������� ���������)
	if (dimension < 0 ||
		dimension >= static_cast<CArrayVar*>(BN)->ArrayType->GetDimCount())
		return s_e_LEN_Dimension;

	//��������� �������� ��������� ����������� �������
	BN = static_cast<CArrayVar*>(BN)->ArrayType->GetType(dimension);
	if (!BN || (id_CArrayType != BN->name_id)) return s_m_Error;	//?!

	//���������� ������� ������� � ��������� �����������
	array_size = static_cast<CArrayType*>(BN)->size;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� ��������� LEN
void CLenStdProcFunc::WriteCPP(CPP_files& f)
{
	//�������� ���� ������� (�������� / �������)
	if (array_size)
		fprintf(f.fc, "%i", array_size);
	else
		fprintf(f.fc, "O2M_ARR_%i_%s", dimension, array_name);
}//WriteCPP


//-----------------------------------------------------------------------------
//�������� ���������
int CLongStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	//��������� ��������� �� ��������� � ���������� ����������
	int err_num = Expr.CreateConst(BaseConst);
	if (err_num) return err_num;
	//���������� ��� ���������� �������� �������� ��������
	int int_data;
	double real_data;
	//�������� ���� ��������� � ������ ��������� �� ��������� ������� ����
	switch (BaseConst->GetResultId()) {
	case id_CShortintVar:
		int_data = static_cast<CShortintVar*>(BaseConst)->ConstValue;
		delete BaseConst;
		BaseConst = new CIntegerVar(parent_element);
		static_cast<CIntegerVar*>(BaseConst)->ConstValue = int_data;
		break;
	case id_CIntegerVar:
		int_data = static_cast<CIntegerVar*>(BaseConst)->ConstValue;
		delete BaseConst;
		BaseConst = new CLongintVar(parent_element);
		static_cast<CLongintVar*>(BaseConst)->ConstValue = int_data;
		break;
	case id_CRealVar:
		real_data = static_cast<CRealVar*>(BaseConst)->ConstValue;
		delete BaseConst;
		BaseConst = new CLongrealVar(parent_element);
		static_cast<CLongrealVar*>(BaseConst)->ConstValue = real_data;
		break;
	}
	return 0;
}


//-----------------------------------------------------------------------------
//������������� ���������-������� LONG
int CLongStdProcFunc::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//��������� ������� ���������
	int err_num = Expr.Init(lb);
	if (err_num) return err_num;

	//�������� ���� ���������
	switch (Expr.GetResultId()) {
	case id_CShortintVar:
		ResultId = id_CIntegerVar;
		break;
	case id_CIntegerVar:
		ResultId = id_CLongintVar;
		break;
	case id_CRealVar:
		ResultId = id_CLongrealVar;
		break;
	default:
		return s_e_Incompatible;
	}

	//�������� ������� ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� ���������-������� LONG
void CLongStdProcFunc::WriteCPP(CPP_files& f)
{
	Expr.WriteCPP(f);
}//WriteCPP


//-----------------------------------------------------------------------------
//�������� ���������
int CMaxStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	//�������� ����, ��� �������� ����������� ������������ ��������
	switch (ResultId) {
	case id_CBooleanVar:
		BaseConst = new CBooleanVar(parent_element);
		static_cast<CBooleanVar*>(BaseConst)->ConstValue = true;
		break;
	case id_CCharVar:
		BaseConst = new CCharVar(parent_element);
		static_cast<CCharVar*>(BaseConst)->ConstValue = (char)255;
		break;
	case id_CShortintVar:
		BaseConst = new CShortintVar(parent_element);
		static_cast<CShortintVar*>(BaseConst)->ConstValue = SHRT_MAX;
		break;
	case id_CIntegerVar:
		BaseConst = new CIntegerVar(parent_element);
		//����� ����� SET � INTEGER
		static_cast<CIntegerVar*>(BaseConst)->ConstValue = AppliedToSET ? SET_MAX : INT_MAX;
		break;
	case id_CLongintVar:
		BaseConst = new CLongintVar(parent_element);
		static_cast<CLongintVar*>(BaseConst)->ConstValue = LONG_MAX;
		break;
	case id_CRealVar:
		BaseConst = new CRealVar(parent_element);
		static_cast<CRealVar*>(BaseConst)->ConstValue = DBL_MAX;
		break;
	case id_CLongrealVar:
		BaseConst = new CLongrealVar(parent_element);
		static_cast<CLongrealVar*>(BaseConst)->ConstValue = DBL_MAX;
		break;
	}
	return 0;
}


//-----------------------------------------------------------------------------
//������������� ���������-������� MAX
int CMaxStdProcFunc::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//������������� ��������� - ��������� ����
	CQualident TmpQual;
	int err_num = TmpQual.InitTypeName(lb, parent_element);
	if (err_num) return err_num;

	//����������� �������� ������ �������� ����
	switch (TmpQual.TypeResultId) {
	case id_CBooleanVar:
	case id_CCharVar:
	case id_CShortintVar:
	case id_CIntegerVar:
	case id_CLongintVar:
	case id_CRealVar:
	case id_CLongrealVar:
		ResultId = TmpQual.TypeResultId;
		break;
	case id_CSetVar:
		ResultId = id_CIntegerVar;
		AppliedToSET = true;
		break;
	default:
		return s_e_MAX_NotType;
	}//switch

	//�������� ������� ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� ���������-������� MAX
void CMaxStdProcFunc::WriteCPP(CPP_files& f)
{
	switch (ResultId) {
	case id_CBooleanVar:
		fprintf(f.fc, "true");
		break;
	case id_CCharVar:
		fprintf(f.fc, "(char)(255)");	//0FFX
		break;
	case id_CShortintVar:
		fprintf(f.fc, "%hi", SHRT_MAX);
		break;
	case id_CIntegerVar:
		//����� ����� SET � INTEGER
		fprintf(f.fc, "%i", AppliedToSET ? SET_MAX : INT_MAX);
		break;
	case id_CLongintVar:
		fprintf(f.fc, "%li", LONG_MAX);
		break;
	case id_CRealVar:
	case id_CLongrealVar:
		//double �� ������� ��������� � long double
		fprintf(f.fc, "%g", DBL_MAX);
		break;
	}//switch
}


//-----------------------------------------------------------------------------
//�������� ���������
int CMinStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	//�������� ����, ��� �������� ����������� ����������� ��������
	switch (ResultId) {
	case id_CBooleanVar:
		BaseConst = new CBooleanVar(parent_element);
		static_cast<CBooleanVar*>(BaseConst)->ConstValue = false;
		break;
	case id_CCharVar:
		BaseConst = new CCharVar(parent_element);
		static_cast<CCharVar*>(BaseConst)->ConstValue = 0;
		break;
	case id_CShortintVar:
		BaseConst = new CShortintVar(parent_element);
		static_cast<CShortintVar*>(BaseConst)->ConstValue = SHRT_MIN;
		break;
	case id_CIntegerVar:
		BaseConst = new CIntegerVar(parent_element);
		//����� ����� SET � INTEGER
		static_cast<CIntegerVar*>(BaseConst)->ConstValue = AppliedToSET ? 0 : INT_MIN;
		break;
	case id_CLongintVar:
		BaseConst = new CLongintVar(parent_element);
		static_cast<CLongintVar*>(BaseConst)->ConstValue = LONG_MIN;
		break;
	case id_CRealVar:
		BaseConst = new CRealVar(parent_element);
		static_cast<CRealVar*>(BaseConst)->ConstValue = DBL_MIN;
		break;
	case id_CLongrealVar:
		BaseConst = new CLongrealVar(parent_element);
		static_cast<CLongrealVar*>(BaseConst)->ConstValue = DBL_MIN;
		break;
	}
	return 0;
}


//-----------------------------------------------------------------------------
//������������� ���������-������� MIN
int CMinStdProcFunc::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//������������� ��������� - ��������� ����
	CQualident TmpQual;
	int err_num = TmpQual.InitTypeName(lb, parent_element);
	if (err_num) return err_num;

	//����������� �������� ������ �������� ����
	switch (TmpQual.TypeResultId) {
	case id_CBooleanVar:
	case id_CCharVar:
	case id_CShortintVar:
	case id_CIntegerVar:
	case id_CLongintVar:
	case id_CRealVar:
	case id_CLongrealVar:
		ResultId = TmpQual.TypeResultId;
		break;
	case id_CSetVar:
		ResultId = id_CIntegerVar;
		AppliedToSET = true;
		break;
	default:
		return s_e_MIN_NotType;
	}//switch

	//�������� ������� ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� ���������-������� MIN
void CMinStdProcFunc::WriteCPP(CPP_files& f)
{
	switch (ResultId) {
	case id_CBooleanVar:
		fprintf(f.fc, "false");
		break;
	case id_CCharVar:
		fprintf(f.fc, "0");	//0X
		break;
	case id_CShortintVar:
		fprintf(f.fc, "%hi", SHRT_MIN);
		break;
	case id_CIntegerVar:
		//����� ����� SET � INTEGER
		if (AppliedToSET)
			fprintf(f.fc, "0");
		else
			fprintf(f.fc, "(%i - 1)", INT_MIN + 1);
		break;
	case id_CLongintVar:
		fprintf(f.fc, "(%li - 1)", LONG_MIN + 1);
		break;
	case id_CRealVar:
	case id_CLongrealVar:
		//double �� ������� ��������� � long double
		fprintf(f.fc, "%g", DBL_MIN);
		break;
	}//switch
}


//-----------------------------------------------------------------------------
//�������� ���������
int COddStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	//��������� ��������� �� ��������� � ���������� ����������
	int err_num = Expr.CreateConst(BaseConst);
	if (err_num) return err_num;
	//��������� �������������� ��������
	long int_data = BaseConst->GetIntValue();
	//������ ������������� ��������� �� ���������
	delete BaseConst;
	BaseConst = new CBooleanVar(parent_element);
	//���������� �������� ������� �� ���������� �������� ����������� ���������
	static_cast<CBooleanVar*>(BaseConst)->ConstValue = int_data % 2 == 1;
	return 0;
}


//-----------------------------------------------------------------------------
//������������� ���������-������� ODD
int COddStdProcFunc::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//��������� ������� ���������
	int err_num = Expr.Init(lb);
	if (err_num) return err_num;

	//�������� ���� ���������
	if (!CBaseVar::IsIntId(Expr.GetResultId())) return s_e_Incompatible;
	ResultId = id_CBooleanVar;

	//�������� ������� ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� ���������-������� ODD
void COddStdProcFunc::WriteCPP(CPP_files& f)
{
	fprintf(f.fc, "((");
	Expr.WriteCPP(f);
	fprintf(f.fc, ")%%2 == 1)");
}//WriteCPP


//-----------------------------------------------------------------------------
//�������� ���������
int COrdStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	//��������� ��������� �� ��������� � ���������� ����������
	int err_num = Expr.CreateConst(BaseConst);
	if (err_num) return err_num;
	//��������� ����������� ��������
	char ch = static_cast<CCharVar*>(BaseConst)->ConstValue;
	//������ ���������� ��������� �� �������������
	delete BaseConst;
	BaseConst = new CIntegerVar(parent_element);
	//���������� �������� ������� �� ���������� �������� ����������� ���������
	static_cast<CIntegerVar*>(BaseConst)->ConstValue = (ch < 0) ? (ch + 256) : ch;
	return 0;
}


//-----------------------------------------------------------------------------
//������������� ��������� ORD
int COrdStdProcFunc::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//��������� ���������
	int err_num = Expr.Init(lb);
	if (err_num) return err_num;

	//�������� ���� ���������
	if (Expr.GetResultId() != id_CCharVar) return s_e_Incompatible;

	//�������� ������� ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	ResultId = id_CIntegerVar;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� ��������� ORD
void COrdStdProcFunc::WriteCPP(CPP_files& f)
{
	//�������� ������� ���������� ����������
	CArrayVar* AV = static_cast<CArrayVar*>(Expr.FindLastName());
	if (AV && (id_CArrayVar == AV->name_id) && AV->ConstString) {
		//����� ������ �� 1 ������� (��������� � Init), ������� 1 ������
		fprintf(f.fc, "ORD('%c')", AV->ConstString[0]);
		return;
	}

	//������ ��������� �������
	fprintf(f.fc, "ORD(");
	Expr.WriteCPP(f);
	fprintf(f.fc, ")");
}//WriteCPP


//-----------------------------------------------------------------------------
//�������� ���������
int CShortStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	//��������� ��������� �� ��������� � ���������� ����������
	int err_num = Expr.CreateConst(BaseConst);
	if (err_num) return err_num;
	//���������� ��� ���������� �������� �������� ��������
	int int_data;
	double real_data;
	//�������� ���� ��������� � ������ ��������� �� ��������� ������� ����
	switch (BaseConst->GetResultId()) {
	case id_CLongintVar:
		int_data = static_cast<CLongintVar*>(BaseConst)->ConstValue;
		delete BaseConst;
		BaseConst = new CIntegerVar(parent_element);
		static_cast<CIntegerVar*>(BaseConst)->ConstValue = int_data;
		break;
	case id_CIntegerVar:
		int_data = static_cast<CIntegerVar*>(BaseConst)->ConstValue;
		delete BaseConst;
		BaseConst = new CShortintVar(parent_element);
		static_cast<CShortintVar*>(BaseConst)->ConstValue = int_data;
		break;
	case id_CLongrealVar:
		real_data = static_cast<CLongrealVar*>(BaseConst)->ConstValue;
		delete BaseConst;
		BaseConst = new CRealVar(parent_element);
		static_cast<CRealVar*>(BaseConst)->ConstValue = real_data;
		break;
	}
	return 0;
}


//-----------------------------------------------------------------------------
//������������� ���������-������� SHORT
int CShortStdProcFunc::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//��������� ������� ���������
	int err_num = Expr.Init(lb);
	if (err_num) return err_num;

	//�������� ���� ���������
	switch (Expr.GetResultId()) {
	case id_CLongintVar:
		ResultId = id_CIntegerVar;
		break;
	case id_CIntegerVar:
		ResultId = id_CShortintVar;
		break;
	case id_CLongrealVar:
		ResultId = id_CRealVar;
		break;
	default:
		return s_e_Incompatible;
	}

	//�������� ������� ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� ���������-������� SHORT
void CShortStdProcFunc::WriteCPP(CPP_files& f)
{
	Expr.WriteCPP(f);
}//WriteCPP


//-----------------------------------------------------------------------------
//�������� ���������
int CSizeStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	/**/
	//�� �� ����� ���������� ����������
	//����������� � ����������

	//��� ���������������� ���������� ������� ����
	int sz;

	switch (Qualident.TypeResultId) {
	case id_CArrayType:
		/**/
		//�������� �� ����� �-���
		sz = 10000;
		break;
	case id_CRecordType:
		/**/
		//�������� �� ����� �-���
		sz = 100;
		break;
	case id_CPointerType:
		sz = sizeof(void*);
		break;
	case id_CLongrealType:
		sz = sizeof(long double);
		break;
	case id_CRealType:
		sz = sizeof(double);
		break;
	case id_CLongintType:
		sz = sizeof(long);
		break;
	case id_CSetVar:
	case id_CIntegerVar:
		sz = sizeof(int);
		break;
	case id_CShortintVar:
		sz = sizeof(short);
		break;
	case id_CCharVar:
		sz = sizeof(char);
		break;
	case id_CBooleanType:
		sz = sizeof(bool);
		break;
	default:
		sz = 0;
		break;
	}//switch

	//����� ������������ ������ ����, ������������ ��� �������� ��������
	if (SHRT_MAX >= sz) {
		BaseConst = new CShortintVar(parent_element);
		static_cast<CShortintVar*>(BaseConst)->ConstValue = sz;
	} else {
		BaseConst = new CIntegerVar(parent_element);
		static_cast<CIntegerVar*>(BaseConst)->ConstValue = sz;
	}

	return 0;
}


//-----------------------------------------------------------------------------
//������������� ���������-������� SIZE
int CSizeStdProcFunc::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//�������� ������� �������� ������ ����
	int err_num = Qualident.InitTypeName(lb, parent_element);
	if (err_num) return err_num;

	//�������� ������� ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	//�� ����� ���� ����� ����� ���� ����� ����� ���
	ResultId = id_CIntegerVar;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� ���������-������� SIZE
void CSizeStdProcFunc::WriteCPP(CPP_files& f)
{
	fprintf(f.fc, "sizeof(");
	Qualident.WriteCPP_type(f, false, parent_element);
	fprintf(f.fc, ")");
}//WriteCPP


//-----------------------------------------------------------------------------
//������������� ��������� NEW
int CNewStdProc::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//��������� �������� ���������
	int err_num = Des.Init(lb);
	if (err_num) return err_num;

	//��������� ����������-��������� �� �����������
	if (Des.IsReadOnly()) return s_e_OperandReadOnly;
	CPointerVar* pVar = static_cast<CPointerVar*>(Des.FindLastName());
	if (!pVar) return s_e_UndeclaredIdent;
	if (id_CPointerVar != pVar->name_id) return s_e_OperandInappl;


	//////////////////////////////
	//�������� ��������� ���������

	CBaseType* BT = pVar->FindType();
	if (id_CCommonType == BT->name_id) {

		//�������� ������� "<"
		if (!lb->ReadLex(li) || lex_k_lt != li.lex) return s_e_OpAngleMissing;

		//�������� ������� ">" (������������ ������������� �� ���������)
		DECL_SAVE_POS
		if (!lb->ReadLex(li) || lex_k_gt != li.lex) {
			RESTORE_POS
		} else {
			//��������� ���������� ������������� �� ���������
			const CCommonType::SSpec* spec = static_cast<CCommonType*>(BT)->GetDefaultSpec();
			if (!spec) return s_e_NoDefaultSpec;
			//���������� ���������� ������������� �� ���������
			if (spec->Tag)
				qual.ident = str_new_copy(spec->Tag);
			else {
				qual.ident = str_new_copy(spec->Name);
				if (spec->QualName) qual.pref_ident = str_new_copy(spec->QualName);
			}
			//�������� ������� ")"
			if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;
			return 0;
		}

		//������������� �������� ��������
		int err_num = qual.Init(lb, parent_element);
		if (err_num) return err_num;
		//�������� ����������� ��������
		if (!static_cast<CCommonType*>(BT)->FindSpec(qual.pref_ident, qual.ident, qual.ident)) return s_e_SpecTypeTag;

		//�������� ������� ">"
		if (!lb->ReadLex(li) || lex_k_gt != li.lex) return s_e_ClAngleMissing;

		//�������� ������� ")"
		if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

		return 0;
	}


	///////////////////////////////////
	//�������� ������� ������ ���������

	//�������� ������� "," ��� ")"
	if (!lb->ReadLex(li) || lex_k_comma != li.lex)
		if (lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;
		else {
			//NEW �������� ������ 1 �������� => �� ������ ���� ������� ��� ��. �� ������ ������������� �����
			if (pVar->IsArrayPointer()) {
				CArrayType *AT = static_cast<CArrayType*>(pVar->FindType());
				while (id_CArrayType == AT->name_id) {
					if (!AT->size) return s_e_ParsFewer;
					//������� � ����. ����������� (���� ����)
					AT = static_cast<CArrayType*>(AT->Type);
				}//while
			}//if
			return 0;
		}

	while (true) {
		//�������� ������� Expr
		CExpr* Ex = new CExpr(parent_element);
		int err_num = Ex->Init(lb);
		if (err_num) {
			delete Ex;
			return err_num;
		}

		//��������� Expr � ������ ���������
		ExprStore.push_back(Ex);

		//�������� ������� ","
		if (!lb->ReadLex(li) || lex_k_comma != li.lex) break;
	}

	//�������� ������� ")"
	if (lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	//�������� ����, �� ���. ��������� ����������-��������� (������ ���� ������)
	if (!pVar->IsArrayPointer()) return s_e_OperandInappl;
	//�������� ������������ ���������� ������������ ���������� ��������� � ������
	int act_par_count = ExprStore.size();
	int form_par_count = static_cast<CArrayType*>(pVar->FindType())->GetDimCount();
	if (act_par_count != form_par_count)
		if (act_par_count < form_par_count)
			return s_e_ParsFewer;
		else
			return s_e_ParsMore;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� ��������� NEW
void CNewStdProc::WriteCPP(CPP_files& f)
{
	//��������� ���������� �� �����������
	CPointerVar* pVar = static_cast<CPointerVar*>(Des.FindLastName());
	//��������� ���� �� ����������
	CBaseType* BT = pVar->FindType();

	//��������� �������
	f.tab_fc();


	////////////////////////////////////////////////////
	//���������� ������������ � ������ ��������� �������
	if (!ExprStore.empty()) {
		//������ ������������ � 0
		int dimension = 0;
		//��������� ���� ���������� ������������ � ����������
		TExprStore::const_iterator ci;
		for (ci = ExprStore.begin(); ci != ExprStore.end(); ++ci) {
			//��������� ����� ����������
			fprintf(f.fc, "O2M_ARR_%i_%s = ", dimension, pVar->name);
			//��������� �������� �����������
			(*ci)->WriteCPP(f);
			//��������� �������� ������ � �������
			fprintf(f.fc, ";\n");
			f.tab_fc();
			//������� � ��������� �����������
			++dimension;
		}//for
	}//if


	//������ ����� ���������� (������� ���������)
	Des.WriteCPP(f);


	////////////////////////////
	//�������� ������� ���������
	if (id_CCommonType == BT->name_id) {
		//��������� ���� ������ �-��� new
		fprintf(f.fc, " = new ");
		//��������� ����� ����
		const char* ma = BT->GetModuleAlias();
		if (ma) fprintf(f.fc, "%s::", ma);
		fprintf(f.fc, "%s(", BT->name);
		//��������� ��������� ������������ ����
		if (ma) fprintf(f.fc, "%s::", ma);
		fprintf(f.fc, "%s::O2M_INIT_", BT->name);
		//�������� ���������� ��� ��������� CompoundName
		CBaseVar* BV;
		static_cast<CCommonType*>(BT)->CreateVar(BV, parent_element);
		static_cast<CCommonVar*>(BV)->SetTagName(qual.pref_ident, qual.ident);
		fprintf(f.fc, "%s)", static_cast<CCommonVar*>(BV)->GetCPPCompoundName());
		delete BV;
		return;
	}

	///////////////////////////////////////////////////////////////
	//�������� ������� ������������� (������������������ ���������)
	if (id_CSpecType == BT->name_id) {
		//��������� ���� ������ �-��� new
		fprintf(f.fc, " = new ");
		//��������� ����� ����
		const char* ma = BT->GetModuleAlias();
		if (ma) fprintf(f.fc, "%s::", ma);
		fprintf(f.fc, "%s(", BT->name);
		//��������� ��������� ������������ ����
		if (ma) fprintf(f.fc, "%s::", ma);
		fprintf(f.fc, "%s::O2M_INIT_", BT->name);
		//�������� ���������� ��� ��������� CompoundName
		CBaseVar* BV;
		static_cast<CSpecType*>(BT)->CreateVar(BV, parent_element);
		fprintf(f.fc, "%s)", static_cast<CCommonVar*>(BV)->GetCPPCompoundName());
		delete BV;
		return;
	}


	////////////////////////////////////////////////////
	//��������� �����������, ��������� ���� �������� new
	fprintf(f.fc, " = new(");

	//������ �������� ������, ���� ���� (����� �������������� � � ������ �������������� ����)
	if (pVar->GetTypeModuleAlias()) fprintf(f.fc, "%s::", pVar->GetTypeModuleAlias());

	//�������� ������� ������������ ����
	if (pVar->GetTypeName())
		fprintf(f.fc, "%s", pVar->GetTypeName());
	else //������������� ��� => ���������� ������������� ��� (O2M_UNNM_<��� ����������>)
		fprintf(f.fc, "O2M_UNNM_%s", pVar->name);

	//�������� ������� ������ ��������� � �������������
	if (ExprStore.empty()) {
		//������ ������������ � ������ ��. �� ������ ������������� �����
		if (pVar->IsArrayPointer()) {
			CArrayType *AT = static_cast<CArrayType*>(pVar->FindType());
			while (AT->name_id == id_CArrayType) {
				fprintf(f.fc, "[%i]", AT->size);
				//������� � ����. ����������� (���� ����)
				AT = static_cast<CArrayType*>(AT->Type);
			}//while
		}//if
	} else {
		//������ ������������ � ������ ��. �� �������� ������
		fprintf(f.fc, "[");
		//���������� ����������, ������������� ����, �������������
		for (int dim = 0; dim < ExprStore.size(); dim++)
			fprintf(f.fc, "%sO2M_ARR_%i_%s", dim ? " * " : "", dim, pVar->name);
		fprintf(f.fc, "]");
	}//else

	fprintf(f.fc, ")");

}//WriteCPP


//-----------------------------------------------------------------------------
//������������� ��������� COPY
int CCopyStdProc::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//�������� ������� ���������
	int err_num = ExprSource.Init(lb);
	if (err_num) return err_num;

	//�������� ������� ����������� ������� (������)
	if (ExprSource.GetResultId() != id_CArrayVar) return s_e_ExprCompatibility;

	//�������� ������� ","
	if (!lb->ReadLex(li) || lex_k_comma != li.lex) return s_e_CommaMissing;

	//�������� ������� ���������
	err_num = ExprDest.Init(lb);
	if (err_num) return err_num;

	//�������� ������� l-value �� ������ ���������
	CBaseName* BN = ExprDest.FindLValue();
	if (!BN) return s_e_OperandNotVariable;
	if (ExprDest.IsReadOnly()) return s_e_OperandReadOnly;
	//�������� ������������ ������� ���������
	switch (BN->GetResultId()) {
	case id_CArrayVar:
		/**/
		//������ �������� ���� ��-��� �������
		break;
	case id_CPointerVar:
		/**/
		//������ �������� ���� ���������
		break;
	default:
		return s_e_OperandInappl;
	}//switch

	//�������� ������� ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� ��������� COPY
void CCopyStdProc::WriteCPP(CPP_files& f)
{
	f.tab_fc();
	fprintf(f.fc, "COPY(");

	//������ �������� ������� ���������
	ExprSource.WriteCPP(f);
	fprintf(f.fc, ", ");
	//��������� ������� ��������� ��������� (����������-������� ��� ���������)
	CBaseName* BN = ExprSource.FindLastName();
	WriteCPP_COPY_Par(f, BN);
	fprintf(f.fc, ", ");

	//������ �������� ������� ���������
	ExprDest.WriteCPP(f);
	fprintf(f.fc, ", ");
	//��������� ������� ��������� ��������� (����������-������� ��� ���������)
	BN = ExprDest.FindLastName();
	WriteCPP_COPY_Par(f, BN);
	fprintf(f.fc, ")");

}//WriteCPP


//-----------------------------------------------------------------------------
//������������� ��������� ASSERT
int CAssertStdProc::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//�������� ������� �����. ���������
	int err_num = Expr.Init(lb);
	if (err_num) return err_num;

	//�������� ���� ����������� ���������
	if (Expr.GetResultId() != id_CBooleanVar) return s_e_ASSERT_ExprType;

	//�������� ������� "," ��� ")"
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_ClBracketMissing;
	if (lex_k_comma == li.lex) {
		//�������� ��������� (������� ���������)
		CBaseVar* BV;
		err_num = ConstSelector(lb, BV, parent_element);
		if (err_num) return err_num;
		//�������� ���� ������� ���������
		if (!CBaseVar::IsIntId(BV->GetResultId())) {
			delete BV;
			return s_e_ParamNotIntConst;
		}
		//��������� �������� ������������� ��������� � �� �����������
		AssertVal = BV->GetIntValue();
		delete BV;
		//�������� ������� ����. �������
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_ClBracketMissing;
	}
	//�������� ������� ")"
	if (lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� ��������� ASSERT
void CAssertStdProc::WriteCPP(CPP_files& f)
{
	f.tab_fc();
	fprintf(f.fc, "if (!(");
	Expr.WriteCPP(f);
	fprintf(f.fc, ")) exit(%li)", AssertVal);
}//WriteCPP


//-----------------------------------------------------------------------------
//������������� ��������� HALT
int CHaltStdProc::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//�������� ������� ����� ���������
	CBaseVar* BV;
	int err_num = ConstSelector(lb, BV, parent_element);
	if (err_num) return err_num;

	//�������� ���� ������� ���������
	if (!CBaseVar::IsIntId(BV->GetResultId())) {
		delete BV;
		return s_e_ParamNotIntConst;
	}
	//��������� �������� ������������� ��������� � �� �����������
	HaltVal = BV->GetIntValue();
	delete BV;

	//�������� ������� ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� ��������� HALT
void CHaltStdProc::WriteCPP(CPP_files& f)
{
	f.tab_fc();
	fprintf(f.fc, "exit(%li)", HaltVal);
}//WriteCPP


//-----------------------------------------------------------------------------
//������������� ��������� DEC
int CDecStdProc::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//��������� ������� ���������
	int err_num = Expr1.Init(lb);
	if (err_num) return err_num;

	//�������� ������� l-value � ������ ���������
	if (!Expr1.FindLValue()) return s_e_OperandNotVariable;
	if (Expr1.IsReadOnly()) return s_e_OperandReadOnly;
	//�������� ������������ ������� ���������
	if (!CBaseVar::IsIntId(Expr1.GetResultId())) return s_e_OperandInappl;

	//�������� ������� ������� ��� ")"
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_ClBracketMissing;
	if (lex_k_comma == li.lex) {
		//�������� ��������� ��� ������� ���������
		Expr2 = new CExpr(parent_element);
		err_num = Expr2->Init(lb);
		if (err_num) return err_num;
		if (!CBaseVar::IsIntId(Expr2->GetResultId())) return s_e_OperandInappl;
		//�������� ����. �������
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_ClBracketMissing;
	}
	//�������� ������� ")"
	if (lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� ��������� DEC
void CDecStdProc::WriteCPP(CPP_files& f)
{
	//������ ����� ���������� (������� ���������)
	f.tab_fc();
	Expr1.WriteCPP(f);
	fprintf(f.fc, " -= ");

	//������ ������� ��������� (���� ����)
	if (Expr2) Expr2->WriteCPP(f); else fprintf(f.fc, "1");
}//WriteCPP


//-----------------------------------------------------------------------------
//������������� ��������� EXCL
int CExclStdProc::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//��������� ������� ���������
	int err_num = Expr1.Init(lb);
	if (err_num) return err_num;

	//�������� ������� l-value � ������ ���������
	if (!Expr1.FindLValue()) return s_e_OperandNotVariable;
	if (Expr1.IsReadOnly()) return s_e_OperandReadOnly;
	//�������� ������������ ������� ���������
	if (id_CSetVar != Expr1.GetResultId()) return s_e_OperandInappl;

	//�������� ������� ","
	if (!lb->ReadLex(li) || lex_k_comma != li.lex) return s_e_CommaMissing;

	//�������� ��������� ��� ������� ���������
	err_num = Expr2.Init(lb);
	if (err_num) return err_num;
	if (!CBaseVar::IsIntId(Expr2.GetResultId())) return s_e_Incompatible;

	//�������� ������� ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� ��������� EXCL
void CExclStdProc::WriteCPP(CPP_files& f)
{
	f.tab_fc();
	Expr1.WriteCPP(f);
	fprintf(f.fc, " &= ~(1 << (");
	Expr2.WriteCPP(f);
	fprintf(f.fc, "))");
}//WriteCPP


//-----------------------------------------------------------------------------
//������������� ��������� INC
int CIncStdProc::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//��������� ������� ���������
	int err_num = Expr1.Init(lb);
	if (err_num) return err_num;

	//�������� ������� l-value � ������ ���������
	if (!Expr1.FindLValue()) return s_e_OperandNotVariable;
	if (Expr1.IsReadOnly()) return s_e_OperandReadOnly;
	//�������� ������������ ������� ���������
	if (!CBaseVar::IsIntId(Expr1.GetResultId())) return s_e_OperandInappl;

	//�������� ������� ������� ��� ")"
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_ClBracketMissing;
	if (lex_k_comma == li.lex) {
		//�������� ��������� ��� ������� ���������
		Expr2 = new CExpr(parent_element);
		err_num = Expr2->Init(lb);
		if (err_num) return err_num;
		if (!CBaseVar::IsIntId(Expr2->GetResultId())) return s_e_Incompatible;
		//�������� ����. �������
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_ClBracketMissing;
	}

	//�������� ������� ")"
	if (lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� ��������� INC
void CIncStdProc::WriteCPP(CPP_files& f)
{
	//������ ����� ���������� (������� ���������)
	f.tab_fc();
	Expr1.WriteCPP(f);
	fprintf(f.fc, " += ");

	//������ ������� ��������� (���� ����)
	if (Expr2) Expr2->WriteCPP(f); else fprintf(f.fc, "1");
}//WriteCPP


//-----------------------------------------------------------------------------
//������������� ��������� INCL
int CInclStdProc::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//��������� ������� ���������
	int err_num = Expr1.Init(lb);
	if (err_num) return err_num;

	//�������� ������� l-value � ������ ���������
	if (!Expr1.FindLValue()) return s_e_OperandNotVariable;
	if (Expr1.IsReadOnly()) return s_e_OperandReadOnly;
	//�������� ������������ ������� ���������
	if (id_CSetVar != Expr1.GetResultId()) return s_e_OperandInappl;

	//�������� ������� ","
	if (!lb->ReadLex(li) || lex_k_comma != li.lex) return s_e_ColonMissing;

	//�������� ��������� ��� ������� ���������
	err_num = Expr2.Init(lb);
	if (err_num) return err_num;
	if (!CBaseVar::IsIntId(Expr2.GetResultId())) return s_e_Incompatible;

	//�������� ������� ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� ��������� INCL
void CInclStdProc::WriteCPP(CPP_files& f)
{
	f.tab_fc();
	Expr1.WriteCPP(f);
	fprintf(f.fc, " |= 1 << (");
	Expr2.WriteCPP(f);
	fprintf(f.fc, ")");
}//WriteCPP


//-----------------------------------------------------------------------------
//����������
CNewStdProc::~CNewStdProc()
{
	//������� ������ ��������� (������������)
	TExprStore::iterator i;
	for (i = ExprStore.begin(); i != ExprStore.end(); ++i)
		delete *i;
}


//-----------------------------------------------------------------------------
//������ ���� ����������� ������� � �������� ��������� ��� ������ COPY
//��������������, ��� BN �������� �������� �������� ��� ��. �� ������ ��������
void WriteCPP_COPY_Par(CPP_files &f, CBaseName* BN)
{
	const char *arr_name = BN->name;
	//� ������ ��������� �������� ��� ������������ �������
	if (id_CPointerVar == BN->name_id) BN = static_cast<CPointerVar*>(BN)->FindType();
	//��������� ������� ��� ���������� ��� ����
	int arr_size;
	if (id_CArrayVar == BN->name_id)
		arr_size = static_cast<CArrayVar*>(BN)->ArrayType->size;
	else
		arr_size = static_cast<CArrayType*>(BN)->size;
	//������ ������� (���� ������ �������� (������ 0) - ������������ ����������)
	if (arr_size)
		fprintf(f.fc, "%i", arr_size);
	else
		fprintf(f.fc, "O2M_ARR_0_%s", arr_name);
}

