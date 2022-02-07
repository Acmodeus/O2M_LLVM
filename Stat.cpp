//=============================================================================
// �������� ������� ���������� (Statements)
//=============================================================================

#include "Stat.h"
#include "StdProc.h"
#include "Type.h"
#include "Var.h"


//-----------------------------------------------------------------------------
//���������� ������� ELSIF
CElsifPair::~CElsifPair()
{
	delete StatementSeq;
	delete Expr;
}//~CElsifPair


//-----------------------------------------------------------------------------
//������������� ������� ELSIF �� ������ ������
int CElsifPair::Init(CLexBuf *lb)
{
	//�������� ������� ���������
	Expr = new CExpr(parent_element);
	int err_num = Expr->Init(lb);
	if (err_num) return err_num;

	//�������� ���� ���������
	if (Expr->GetResultId() != id_CBooleanVar) return s_e_IF_ExprType;

	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� ��. ����� THEN
	if (!lb->ReadLex(li) || lex_k_THEN != li.lex) return s_e_THEN;

	//�������� ������� ������. ����������
	StatementSeq = new CStatementSeq(parent_element);
	err_num = StatementSeq->Init(lb);
	if (err_num) return err_num;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� CElsifPair
void CElsifPair::WriteCPP(CPP_files& f)
{
	f.tab_fc();
	fprintf(f.fc, "if(");
	Expr->WriteCPP(f);
	fprintf(f.fc, ") {\n");

	StatementSeq->WriteCPP(f);

	f.tab_fc();
	fprintf(f.fc, "}");
}//WriteCPP


//-----------------------------------------------------------------------------
//���������� ������� IF
CIfStatement::~CIfStatement()
{
	ElsifPairList_type::const_iterator ci;
	for (ci = ElsifPairList.begin(); ci != ElsifPairList.end(); ++ci)
		delete *ci;
	delete ElseStatementSeq;
}//~CIfStatement


//-----------------------------------------------------------------------------
//�������� ������� ��������� RETURN
EHaveRet CIfStatement::HaveRet() const
{
	//�������� ������� ����� ���������� ��� � � RETURN
	bool HaveNo = false;
	bool HaveYes = false;
	//�������� ������� RETURN ����� ����������� ������ ELSIF
	ElsifPairList_type::const_iterator ci;
	for (ci = ElsifPairList.begin(); ci != ElsifPairList.end(); ++ci)
		switch ((*ci)->HaveRet()) {
		case hr_NotAll:
			return hr_NotAll;
		case hr_No:
			HaveNo = true;
			break;
		default:
			HaveYes = true;
		}
	//�������� ������� RETURN � ������ ELSE (���� ����)
	if (ElseStatementSeq)
		switch (ElseStatementSeq->HaveRet()) {
		case hr_NotAll:
			return hr_NotAll;
		case hr_No:
			HaveNo = true;
			break;
		default:
			HaveYes = true;
		}
	//�.�. ��� ������ NotAll ��������� ����, ��������� ������� No � Yes
	return HaveYes ? HaveNo ? hr_NotAll : hr_Yes : hr_No;
}


//-----------------------------------------------------------------------------
//������������� ������� IF �� ������ ������
int CIfStatement::Init(CLexBuf *lb)
{
	//�������� ������ ELSIF � ������ ��������� (��� IF ... THEN)
	CElsifPair* EPair = new CElsifPair(parent_element);
	int err_num = EPair->Init(lb);
	if (err_num) {
		delete EPair;
		return err_num;
	}
	ElsifPairList.push_back(EPair);
	
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//������ ��������� ������� (��������� �����)
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;

	//�������� ������� ELSIF (������ ��� ������)
	while (lex_k_ELSIF == li.lex) {
		//�������� ��������� ���� Expr THEN StatementSeq
		EPair = new CElsifPair(parent_element);
		err_num = EPair->Init(lb);
		if (err_num) {
			delete EPair;
			return err_num;
		}
		ElsifPairList.push_back(EPair);

		//������ ��������� ������� (��������� �����)
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;
	};

	//�������� ������� ELSE
	if (lex_k_ELSE == li.lex) {
		ElseStatementSeq = new CStatementSeq(parent_element);
		err_num = ElseStatementSeq->Init(lb);
		if (err_num) return err_num;
		//������ ��������� ������� (��������� �����)
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;
	};

	//�������� ������� ����� (��������� �������� ���������)
	if (lex_k_END != li.lex) return s_e_END;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� CIfStatement
void CIfStatement::WriteCPP(CPP_files& f)
{
	ElsifPairList_type::iterator i = ElsifPairList.begin();
	//������ ������� ������� (������ ���� ������)
	if (!ElsifPairList.empty()) (*i)->WriteCPP(f);
	//������ ����������� ������� ELSIF (����� �������������)
	for(++i; i != ElsifPairList.end(); ++i) {
		fprintf(f.fc, " else\n");
		(*i)->WriteCPP(f);
	}
	//������ ���������� ������� ELSE
	if (ElseStatementSeq) {
		fprintf(f.fc, " else {\n");
		ElseStatementSeq->WriteCPP(f);
		f.tab_fc();
		fprintf(f.fc, "}");
	}
}//WriteCPP


//-----------------------------------------------------------------------------
//����������
CCaseLabelsSeq::~CCaseLabelsSeq()
{
	CCaseLabelsSeq::CaseLabelsList_type::iterator i;
	for (i = CaseLabelsList.begin(); i != CaseLabelsList.end(); ++i)
		delete *i;
}//CCaseLabelsSeq


//-----------------------------------------------------------------------------
//������������� ������� CaseLabelsSeq �� ������ ������
int CCaseLabelsSeq::Init(CLexBuf *lb, CCaseStatement* const CaseStatement)
{
	DECL_SAVE_POS
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� ������ ����� (����� �� ����, ����� ���� "|", "ELSE", ��� "END")
	if (!lb->ReadLex(li) || lex_k_vertical == li.lex || lex_k_ELSE == li.lex || lex_k_END == li.lex) {
		RESTORE_POS
		return s_m_CaseAbsent;
	}

	RESTORE_POS

	//������ ����� ������������ => ���� ���� Case, �������������� ���
	while (true) {
		//��������� ��������� ����� (���������)
		CCaseLabels* CaseLabels = new CCaseLabels(parent_element);
		int err_num = CaseLabels->Init(lb, CaseStatement, this);
		if (err_num) {
			delete CaseLabels;	//������� ��������� ����������
			return err_num;
		}
		//��������� ��������� ����� � ������
		CaseLabelsList.push_back(CaseLabels);
		//��������� ����. ������� (������ ���� "," ��� ":")
		if (!lb->ReadLex(li)) return s_e_ColonMissing;
		//�������� ���������� �������
		switch (li.lex) {
		case lex_k_comma:
			continue;
		case lex_k_colon:
			break;
		default:
			return s_e_ColonMissing;
		}
		//������� ":", ����� ���������
		break;
	}

	return StatementSeq.Init(lb);
}//Init


//-----------------------------------------------------------------------------
//������ ���� CCaseLabelsSeq
void CCaseLabelsSeq::WriteCPP(CPP_files& f, CExpr* Expr)
{
	CCaseLabelsSeq::CaseLabelsList_type::const_iterator i;

	//�������� ������ �����
	if (CaseLabelsList.empty()) throw error_Internal("CCaseLabelsSeq::WriteCPP");
	i = CaseLabelsList.begin();
	(*i)->WriteCPP(f, Expr);
	
	//���� �������� ���������� �����
	for (++i; i != CaseLabelsList.end(); ++i)
	{
		fprintf(f.fc, ")||(");
		(*i)->WriteCPP(f, Expr);
	}

	//��������� ���� ������-�� ����������
	fprintf(f.fc, ")) {\n");
	StatementSeq.WriteCPP(f);
	f.tab_fc();
	fprintf(f.fc, "}");
}//WriteCPP


//-----------------------------------------------------------------------------
//�������� ������� ���������� �������� � ��� ��������� ������
bool CCaseLabelsSeq::ValueExists(const long Value, const bool IsRng, const long HighValue)
{
	CCaseLabelsSeq::CaseLabelsList_type::const_iterator ci;
	for (ci = CaseLabelsList.begin(); ci != CaseLabelsList.end(); ci++)
		if ((*ci)->ValueExists(Value, IsRng, HighValue)) return true;
	return false;
}


//-----------------------------------------------------------------------------
//������������� ������� CaseLabels �� ������ ������
int CCaseLabels::Init(CLexBuf *lb, CCaseStatement* const CaseStatement, CCaseLabelsSeq* const CaseLabelsSeq)
{
	//�������� ���������
	CBaseVar* BV;
	int err_num = ConstSelector(lb, BV, parent_element);
	if (err_num) return err_num;

	//���������� ��� ���������� ���������
	EName_id ExprResultId = BV->GetResultId();
	//�������� ��� ��������� � CASE
	EName_id CaseExprResultId = CaseStatement->GetExprResultId();

	//���������� �������� ��������� (������������ �������� ���)
	if (id_CCharVar == ExprResultId) ConstValue = static_cast<CCharVar*>(BV)->ConstValue; else
		if (id_CArrayVar == ExprResultId) {
			if (strlen(static_cast<CArrayVar*>(BV)->ConstString) != 1) {
				delete BV;
				return s_e_CASE_LabelType;
			}
			ConstValue = static_cast<CArrayVar*>(BV)->ConstString[0];
		} else
			if (CBaseVar::IsIntId(ExprResultId)) ConstValue = BV->GetIntValue(); else {
				delete BV;
				return s_e_CASE_LabelType;
			}

	//����������� ��������� (�������� ��� ���������)
	delete BV;

	//�������� ������������ ��������� ��������� � CASE � �����
	if (CBaseVar::IsIntId(CaseExprResultId)) {
		//�������� ���������� ���������� ��������� ����� ��������� � CASE
		if (!CBaseVar::IsIntId(ExprResultId) || !IsId1IncloseId2(CaseExprResultId, ExprResultId))
			return s_e_CASE_WrongLabelType;
	} else
		if (id_CCharVar != ExprResultId && id_CArrayVar != ExprResultId) return s_e_CASE_WrongLabelType;

	DECL_SAVE_POS
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� ��. ����� ".."
	if (!lb->ReadLex(li) || lex_k_dots != li.lex) {
		RESTORE_POS
		//�������� ���������� ��� ����������� ����� � ������������ �����������
		if (CaseLabelsSeq->ValueExists(ConstValue, false, 0) || CaseStatement->ValueExists(ConstValue, false, 0)) return s_e_CASE_LabelExists;
		//����� �� �������� ���������, ����� ���������
		return 0;
	}

	//���� ������ ���������
	IsRange = true;

	//�������� ������ ���������
	err_num = ConstSelector(lb, BV, parent_element);
	if (err_num) return err_num;

	//���������� ��� ���������� ���������
	ExprResultId = BV->GetResultId();

	//���������� �������� ��������� (������������ �������� ���)
	if (id_CCharVar == ExprResultId) ConstHighValue = static_cast<CCharVar*>(BV)->ConstValue; else
		if (id_CArrayVar == ExprResultId) {
			if (strlen(static_cast<CArrayVar*>(BV)->ConstString) != 1) {
				delete BV;
				return s_e_CASE_LabelType;
			}
			ConstHighValue = static_cast<CArrayVar*>(BV)->ConstString[0];
		} else
			if (CBaseVar::IsIntId(ExprResultId)) ConstHighValue = BV->GetIntValue(); else {
				delete BV;
				return s_e_CASE_LabelType;
			}

	//����������� ��������� (�������� ��� ���������)
	delete BV;

	//�������� ������������ ��������� ��������� � CASE � �����
	if (CBaseVar::IsIntId(CaseExprResultId)) {
		//�������� ���������� ���������� ��������� ����� ��������� � CASE
		if (!CBaseVar::IsIntId(ExprResultId) || !IsId1IncloseId2(CaseExprResultId, ExprResultId))
			return s_e_CASE_WrongLabelType;
	} else
		if (id_CCharVar != ExprResultId && id_CArrayVar != ExprResultId) return s_e_CASE_WrongLabelType;

	//�������� ������������ ���������
	if (ConstValue > ConstHighValue) return s_e_CASE_LabelType;

	//�������� ���������� ��� ����������� ����� � ������������ �����������
	if (CaseLabelsSeq->ValueExists(ConstValue, true, ConstHighValue) || CaseStatement->ValueExists(ConstValue, true, ConstHighValue)) return s_e_CASE_LabelExists;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� CCaseLabels
void CCaseLabels::WriteCPP(CPP_files& f, CExpr* Expr)
{
	Expr->WriteCPP(f);
	//�������� ������� ���������
	if (IsRange) {
		fprintf(f.fc, " >= %li && ", ConstValue);
		Expr->WriteCPP(f);
		fprintf(f.fc, " <= %li", ConstHighValue);
	} else
		fprintf(f.fc, " == %li", ConstValue);
}//WriteCPP


//-----------------------------------------------------------------------------
//�������� ������� ���������� �������� � ������� �����
bool CCaseLabels::ValueExists(const long Value, const bool IsRng, const long HighValue)
{
	long HVal = IsRng ? HighValue : Value;
	long ConstHVal = IsRange ? ConstHighValue : ConstValue;
	return (Value >= ConstValue && Value <= ConstHVal) ||
			(HVal >= ConstValue && HVal <= ConstHVal) ||
			(ConstValue >= Value && ConstValue <= HVal) ||
			(ConstHVal >= Value && ConstHVal <= HVal);
}


//-----------------------------------------------------------------------------
//���������� ������� CASE
CCaseStatement::~CCaseStatement()
{
	delete ElseStatementSeq;
	delete Expr;
	//������� ������ ������� �����
	CaseLabelsSeqList_type::const_iterator ci;
	for (ci = CaseLabelsSeqList.begin(); ci != CaseLabelsSeqList.end(); ++ci)
		delete *ci;
}//~CCaseStatement


//-----------------------------------------------------------------------------
//�������� ������� ��������� RETURN
EHaveRet CCaseStatement::HaveRet() const
{
	//�������� ������� ����� ���������� ��� � � RETURN
	bool HaveNo = false;
	bool HaveYes = false;
	//�������� ������� RETURN ����� ����������� ������ ELSIF
	CaseLabelsSeqList_type::const_iterator ci;
	for (ci = CaseLabelsSeqList.begin(); ci != CaseLabelsSeqList.end(); ++ci)
		switch ((*ci)->HaveRet()) {
		case hr_NotAll:
			return hr_NotAll;
		case hr_No:
			HaveNo = true;
			break;
		default:
			HaveYes = true;
		}
	//�������� ������� RETURN � ������ ELSE (���� ����)
	if (ElseStatementSeq)
		switch (ElseStatementSeq->HaveRet()) {
		case hr_NotAll:
			return hr_NotAll;
		case hr_No:
			HaveNo = true;
			break;
		default:
			HaveYes = true;
		}
	//�.�. ��� ������ NotAll ��������� ����, ��������� ������� No � Yes
	return HaveYes ? HaveNo ? hr_NotAll : hr_Yes : hr_No;
}


//-----------------------------------------------------------------------------
//������������� ������� CASE �� ������ ������
int CCaseStatement::Init(CLexBuf *lb)
{
	//�������� ������� ���������
	Expr = new CExpr(parent_element);
	int err_num = Expr->Init(lb);
	if (err_num) return err_num;

	//�������� ���� ���������
	EName_id ExprResultId = Expr->GetResultId();
	if (!CBaseVar::IsIntId(ExprResultId) && (id_CCharVar != ExprResultId)) return s_e_CASE_Expr;

	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� ��. ����� OF
	if (!lb->ReadLex(li) || lex_k_OF != li.lex) return s_e_OF;

	//���� ������������� ��-��� Case
	while (true) {
		CCaseLabelsSeq* CLSeq = new CCaseLabelsSeq(parent_element);
		err_num = CLSeq->Init(lb, this);

		//��� ������� Case ������� ��� � ������
		switch (err_num) {
		case 0:
			//������ �� ���������� - ������� Case � ������
			CaseLabelsSeqList.push_back(CLSeq);
			break;
		case s_m_CaseAbsent:
			//������� ���������� Case - ������ �� ������� � ������
			delete CLSeq;
			break;
		default:
			//������ ��� ������������� Case
			delete CLSeq;
			return err_num;
		}

		//��������� ����. ������� (������ ���� �������� �����)
		if (!lb->ReadLex(li)) return s_e_END;

		//�������� ����������� ��������� �����
		switch (li.lex) {
		case lex_k_vertical:
			continue;
		case lex_k_ELSE:
		case lex_k_END:
			break;
		default:
			return s_e_END;
		}

		//������� ELSE ��� END - ����� ������ Case��
		break;
	}//while

	//�������� ������� ������ ELSE
	if (lex_k_ELSE == li.lex) {
		ElseStatementSeq = new CStatementSeq(parent_element);
		err_num = ElseStatementSeq->Init(lb);
		if (err_num) return err_num;
		//��������� ����. �������
		if (!lb->ReadLex(li) || lex_k_END != li.lex) return s_e_END;
	}

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� CCaseStatement
void CCaseStatement::WriteCPP(CPP_files& f)
{
	//���� �������� ������������������� �����
	CCaseStatement::CaseLabelsSeqList_type::const_iterator i;
	for (i = CaseLabelsSeqList.begin(); i != CaseLabelsSeqList.end(); ++i)
	{
		f.tab_fc();
		fprintf(f.fc, "if ((");
		(*i)->WriteCPP(f, Expr);
		fprintf(f.fc, " else\n");
	}//for

	f.tab_fc();

	//�������� ������� ������� ELSE
	if (ElseStatementSeq) {
		fprintf(f.fc, "{\n");
		ElseStatementSeq->WriteCPP(f);
		f.tab_fc();
		fprintf(f.fc, "}");
	} else
		fprintf(f.fc, "\texit(0)");
}//WriteCPP


//-----------------------------------------------------------------------------
//�������� ������� ���������� �������� � ��� ��������� ������� �����
bool CCaseStatement::ValueExists(const long Value, const bool IsRng, const long HighValue)
{
	CaseLabelsSeqList_type::const_iterator ci;
	for (ci = CaseLabelsSeqList.begin(); ci != CaseLabelsSeqList.end(); ci++)
		if ((*ci)->ValueExists(Value, IsRng, HighValue)) return true;
	return false;
}


//-----------------------------------------------------------------------------
//�������� ���������� ���������� ������� ���� (��� ��������� � ������� ���� WithLoopLink)
CBaseVar* CGuard::CreateGuardVar()
{
	//����� ���� � �������� ����������
	CBaseName* BN = parent_element->GetGlobalName(TypeName.pref_ident, TypeName.ident);
	CBaseVar* BV = NULL;
	static_cast<CBaseType*>(BN)->CreateVar(BV, parent_element);
	//��������� ��������� ��������� ����������
	BV->SetName(VarName.ident);
	BV->SetTypeName(TypeName.pref_ident, TypeName.ident);
	//��������� ������ ����� �������������� ������ ��� VAR �������� ���������, ���������
	//�������������� ��������� �������� ���������� ��� ������
	//(��� ��������� �������� ������� ��������� �� ������ ����)
	BV->is_var = true;
	//�������� ���� ���������� ��� ���. ���������
	switch (BV->name_id) {
	case id_CPointerVar:
		//��������� �������� ���������� ���������� ����� QualidentType (���������� ��� ������ � ������ ����� ������)
		static_cast<CPointerVar*>(BV)->qualident_type = true;
		//��� ��������� ��������� �������� ��. �� ������ (��������� � Init)
		static_cast<CPointerVar*>(BV)->SetIsRecord();
		//��������� �������� ���������� ��� �������
		BV->is_guarded = true;
		break;
	case id_CCommonVar:
		//��������� �������� ����������
		static_cast<CCommonVar*>(BV)->SetTagName(spec_name.pref_ident, spec_name.ident);
		break;
	default:
		//��������� �������� ���������� ��� �������
		BV->is_guarded = true;
	}//switch
	return BV;
}


//-----------------------------------------------------------------------------
//��������� �������� ������ (������ ��� ��������������� ����������)
const char* CGuard::GetVarModuleName()
{
	return VarName.pref_ident;
}


//-----------------------------------------------------------------------------
//������������� ������� Guard �� ������ ������
int CGuard::Init(CLexBuf *lb)
{
	//������������� ����� ����������� ����������
	int err_num = VarName.Init(lb, parent_element);
	if (err_num) return err_num;

	//�������� ������� ����������� ����������
	CBaseName* BN = parent_element->GetGlobalName(VarName.pref_ident, VarName.ident);
	if (!BN) {
		//�������� ������ <������>.<����> - ����� �������� ": missing" ������ "undeclared"
		if (parent_element->GetGlobalName(VarName.pref_ident)) return s_e_ColonMissing;
		else return s_e_UndeclaredIdent;
	}

	//��� ���� ������ (��� �������� ������������ � ������ ������ ��� ��. �� ������)
	const char* module_name;
	const char* type_name;

	//�������� ���� ����������� ����������
	switch (BN->name_id) {
	case id_CRecordVar:
		//�������� ���������� ���� ������
		if (!static_cast<CRecordVar*>(BN)->is_var) return s_e_GuardVarNotRecOrP;
		type_id = id_CRecordType;
		//����������� �������� ���� ����������
		module_name = static_cast<CRecordVar*>(BN)->GetTypeModuleAlias();
		type_name = static_cast<CRecordVar*>(BN)->GetTypeName();
		break;
	case id_CCommonVar:
		//�������� ���������� ���� ���������
		if (!static_cast<CCommonVar*>(BN)->is_var) return s_e_GuardVarNotRecOrP;
		if (!static_cast<CCommonVar*>(BN)->IsPureCommon()) return s_e_GuardVarNotRecOrP;
		type_id = id_CCommonType;
		break;
	case id_CPointerVar:
		//��������� ����, �� ������� ��������� ������ ���������
		BN = static_cast<CPointerVar*>(BN)->FindType();
		//�������� ������� ��. �� ���������
		if (id_CCommonType == BN->name_id) {
			type_id = id_CCommonType;
			break;
		}
		//�������� ���������� ���� ��������� �� ������
		type_id = id_CPointerType;
		//����������� �������� ���� ����������
		module_name = static_cast<CBaseType*>(BN)->GetModuleAlias();
		type_name = BN->name;
		break;
	default:
		//�������� ���������� ������������� ����
		return s_e_GuardVarNotRecOrP;
	}//switch

	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� ������� ":"
	if (!lb->ReadLex(li) || lex_k_colon != li.lex) return s_e_ColonMissing;

	//��������� ���� � ������ � ��������� ������������� ��������� "<" �������� ">"
	if (id_CCommonType == type_id) {

		//��������� ���������� ��������� ��� ��. �� ���������

		//�������� ������� "<" - ������ �������� �������������
		if (!lb->ReadLex(li) || lex_k_lt != li.lex) return s_e_OpAngleMissing;
		//������������� �������� ��������
		err_num = spec_name.Init(lb, parent_element);
		if (err_num) return err_num;
		//��������� ���� � ������ ������� ���������� ����������
		if (id_CCommonVar == BN->name_id) BN = static_cast<CCommonVar*>(BN)->FindType();
		//��������� ����� ���� ����������� ���������� ����������
		const char* m_a = static_cast<CBaseType*>(BN)->GetModuleAlias();
		if (m_a) TypeName.pref_ident = str_new_copy(m_a);
		TypeName.ident = str_new_copy(BN->name);
		//��������, �������� �� ������� ���������� ��� ������� ���������
		if (id_CCommonType != BN->name_id) return s_m_Error;
		if (!static_cast<CCommonType*>(BN)->FindSpec(spec_name.pref_ident, spec_name.ident, spec_name.ident)) return s_e_SpecTypeTag;
		//�������� ������� ">" - ����� �������� �������������
		if (!lb->ReadLex(li) || lex_k_gt != li.lex) return s_e_ClAngleMissing;
		//���������� ��������� ���������
		return 0;

	} else {

		//��������� ������ ��� ��. �� ������

		//������������� ����� ���� ����������� ����������
		err_num = TypeName.Init(lb, parent_element);
		if (err_num) return err_num;
		//�������� ������� ���������� ����� ����
		BN = parent_element->GetGlobalName(TypeName.pref_ident, TypeName.ident);
		if (!BN) return s_e_UndeclaredIdent;
		//��������, ��� �� ������ ������������� ����
		if (!CBaseType::IsTypeId(BN->name_id)) return s_e_IdentNotType;
		//��������, ������������� �� ��� � ������ ���������� ����������
		if (id_CRecordType == type_id) {
			//�������� ������� ���� ������
			if (id_CRecordType != BN->name_id) return s_e_GuardTypeNotRec;
		} else {
			//�������� ������� ���� ���������
			if (id_CPointerType != BN->name_id) return s_e_GuardTypeNotP;
			//��������� ����, �� ���. ��������� ���������
			BN = static_cast<CPointerType*>(BN)->FindType();
			//��������, ��������� �� ��. �� ������
			if (id_CRecordType != BN->name_id) return s_e_GuardTypeNotExt;
		}
		//�������� ������� ����� ���� (��� ���������� �������������� ���� ��� ������ ��������� ������)
		if (!type_name) return s_e_GuardTypeNotExt;
		//��������, �������� �� ������ ����������� ���� ���������� ����������
		if (!static_cast<CRecordType*>(BN)->IsExtension(module_name, type_name))
			return s_e_GuardTypeNotExt;
		//���������� ��������� ������ ��� ��. �� ������
		return 0;

	}//else

}//Init


//-----------------------------------------------------------------------------
//������ ���� CGuard
void CGuard::WriteCPP(CPP_files& f)
{
	//������ ������������ ������ Guard, ������� �������� ���������� NIL
	f.tab_fc();
	fprintf(f.fc, "if (");
	if (VarName.pref_ident) fprintf(f.fc, "%s::", VarName.pref_ident);
	fprintf(f.fc, "%s && ", VarName.ident);

	//�������� ������� ����������� ����
	if (id_CCommonType == type_id) {
		//WITH � ����������
		fprintf(f.fc, "(");
		if (VarName.pref_ident) fprintf(f.fc, "%s::", VarName.pref_ident);
		fprintf(f.fc, "%s->O2M_SID == ", VarName.ident);
		if (VarName.pref_ident) fprintf(f.fc, "%s::", VarName.pref_ident);
		fprintf(f.fc, "%s->O2M_SID_", VarName.ident);
	} else {
		//������� ������������ WITH
		fprintf(f.fc, "!strcmp(");
		if (VarName.pref_ident) fprintf(f.fc, "%s::", VarName.pref_ident);
		fprintf(f.fc, "%s->O2M_SYS_ID(), ", VarName.ident);
	}

	//��������� ����-������
	CBaseName* BN = parent_element->GetGlobalName(TypeName.pref_ident, TypeName.ident);

	//��������� ���� ����-������
	switch (type_id) {
	case id_CCommonType:
		//��������� ���������
		{
			const CCommonType::SSpec* spec = static_cast<CCommonType*>(BN)->FindSpec(spec_name.pref_ident, spec_name.ident, spec_name.ident);
			if (!spec) throw error_Internal("CGuard::WriteCPP");
			if (spec->Tag)
				fprintf(f.fc, "%s", spec->Tag);
			else
				fprintf(f.fc, "%s_%s", spec->QualName ? spec->QualName : parent_element->GetParentModule()->name, spec->Name);
		}
		break;
	case id_CPointerType:
		//��������� ��. �� ������
		fprintf(f.fc, "\"%s\"", static_cast<CRecordType*>(static_cast<CPointerType*>(BN)->FindType())->GetRuntimeId());
		break;
	default:
		//��������� ������
		fprintf(f.fc, "\"%s\"", static_cast<CRecordType*>(BN)->GetRuntimeId());
	}

	//������������ ������ Guard � ������������������� ����������
	fprintf(f.fc, ")) {\n");
}//WriteCPP


//-----------------------------------------------------------------------------
//���������� ������� |
CGuardPair::~CGuardPair()
{
	delete StatementSeq;
	delete Guard;
}//~CGuardPair


//-----------------------------------------------------------------------------
//������������� ������� | �� ������ ������
int CGuardPair::Init(CLexBuf *lb)
{
	//���������� ��� ��������� ������ ������
	int err_num = 0;

	//�������� ������� ���������
	Guard = new CGuard(parent_element);
	err_num = Guard->Init(lb);
	if (err_num) return err_num;

	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� ��. ����� DO
	if (!lb->ReadLex(li) || lex_k_DO != li.lex) return s_e_DO;

	//��������� � WithLoopLink ���������� ����������
	WithLoopLink.AddName(Guard->GetVarModuleName(), Guard->CreateGuardVar());

	//�������� ������� ������. ����������
	StatementSeq = new CStatementSeq(&WithLoopLink);
	err_num = StatementSeq->Init(lb);
	if (err_num) return err_num;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//���������� ������� WITH
CWithStatement::~CWithStatement()
{
	if (GuardPairStore) {
		CBaseVector::const_iterator ci;
		for (ci = GuardPairStore->begin(); ci != GuardPairStore->end(); ++ci)
			delete *ci;
		delete GuardPairStore;
	}
	delete ElseStatementSeq;
}//~CWithStatement


//-----------------------------------------------------------------------------
//�������� ������� ��������� RETURN
EHaveRet CWithStatement::HaveRet() const
{
	//�������� ������� ����� ���������� ��� � � RETURN
	bool HaveNo = false;
	bool HaveYes = false;
	//�������� ������� RETURN ����� ����������� ������ ELSIF
	CBaseVector::const_iterator ci;
	for (ci = GuardPairStore->begin(); ci != GuardPairStore->end(); ++ci)
		switch ((*ci)->HaveRet()) {
		case hr_NotAll:
			return hr_NotAll;
		case hr_No:
			HaveNo = true;
			break;
		default:
			HaveYes = true;
		}
	//�������� ������� RETURN � ������ ELSE (���� ����)
	if (ElseStatementSeq)
		switch (ElseStatementSeq->HaveRet()) {
		case hr_NotAll:
			return hr_NotAll;
		case hr_No:
			HaveNo = true;
			break;
		default:
			HaveYes = true;
		}
	//�.�. ��� ������ NotAll ��������� ����, ��������� ������� No � Yes
	return HaveYes ? HaveNo ? hr_NotAll : hr_Yes : hr_No;
}


//-----------------------------------------------------------------------------
//������������� ������� WITH �� ������ ������
int CWithStatement::Init(CLexBuf *lb)
{
	//�������� ������ | � ������ ��������� (��� WITH ... DO)
	CGuardPair* GPair = new CGuardPair(parent_element);
	int err_num = GPair->Init(lb);
	if (err_num) {
		delete GPair;
		return err_num;
	}

	GuardPairStore = new CBaseVector;
	GuardPairStore->push_back(GPair);

	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//������ ��������� ������� (��������� �����)
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;

	//�������� ������� | (������ ��� ������)
	while (lex_k_vertical == li.lex) {
		//�������� ��������� ���� Guard DO StatementSeq
		GPair = new CGuardPair(parent_element);
		err_num = GPair->Init(lb);
		if (err_num) {
			delete GPair;
			return err_num;
		}
		GuardPairStore->push_back(GPair);

		//������ ��������� ������� (��������� �����)
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;
	}

	if (lex_k_ELSE == li.lex) {
		ElseStatementSeq = new CStatementSeq(parent_element);
		err_num = ElseStatementSeq->Init(lb);
		if (err_num) return err_num;
		//������ ��������� ������� (��������� �����)
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;
	}

	//�������� ������� ����� (��������� �������� ���������)
	if (lex_k_END != li.lex) return s_e_END;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� CWithStatement
void CWithStatement::WriteCPP(CPP_files& f)
{
	//������ ���� ������ ������ (������ ������ ��������������)
	CBaseVector::const_iterator ci = GuardPairStore->begin();
	(*ci)->WriteCPP(f);
	//������ ���� ���������� �����
	for (++ci; ci != GuardPairStore->end(); ++ci) {
		fprintf(f.fc, " else\n");
		(*ci)->WriteCPP(f);
	}

	//������ ������������������ ���������� �� ������� ELSE (���� ����)
	fprintf(f.fc, " else\n");
	if (ElseStatementSeq) {
		f.tab_fc();
		fprintf(f.fc, "{\n");

		ElseStatementSeq->WriteCPP(f);

		f.tab_fc();
		fprintf(f.fc, "}");
	} else {
		//������ ���� ������ �� ��������� ��� ���������� ����� ELSE
		f.tab_fc();
		fprintf(f.fc, "\texit(0)");
	}
}//WriteCPP


//-----------------------------------------------------------------------------
//���������� ������� StatementSeq
CStatementSeq::~CStatementSeq()
{
	CBaseVector::const_iterator ci;
	for(ci = StatStore.begin(); ci != StatStore.end(); ++ci)
		delete *ci;
}//~CStatementSeq


//-----------------------------------------------------------------------------
//�������� ������� ��������� RETURN
EHaveRet CStatementSeq::HaveRet() const
{
	CBaseVector::const_reverse_iterator ci;
	for (ci = StatStore.rbegin(); ci != StatStore.rend(); ci++) {
		EHaveRet hr = (*ci)->HaveRet();
		if (hr_No != hr) return hr;
	}
	return hr_No;
}


//-----------------------------------------------------------------------------
//������������� ������� �� ������ ������
int CStatementSeq::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	while(true){
		//������������� ������ ���������
		int err_num	= StatementInit(lb);
		if (err_num) return err_num;
		DECL_SAVE_POS
		//������ ��. ����� ";" (���� ����)
		if (!lb->ReadLex(li) || lex_k_semicolon != li.lex) {
			RESTORE_POS
			return 0;
		}
	}

	return 0;
}//CStatementSeq::Init


//-----------------------------------------------------------------------------
//������������� ������� Statement �� ������ ������
int CStatementSeq::StatementInit(CLexBuf *lb)
{
	DECL_SAVE_POS
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//��������� ��������� ����� ��� ��.
	if (!lb->ReadLex(li) || (lex_k_dot > li.lex && lex_i != li.lex)) return s_e_Statement;

	//���� ������� �������� �� ������
	CBase *Base = NULL;

	//�������� ���� ���������� �������
	switch (li.lex) {

	//�������� ������� ������ ��� ������������
	case lex_i:
	case lex_k_op_brace:
		RESTORE_POS
		//�������� ������� ������ ���������� ���������
		if (lex_k_op_brace == li.lex)
			Base = new CCallStatement(parent_element, NULL, false, true);
		else {
			//��������� Designator, �� �������� ������������ ����� ��������� ��� ������������
			CDesignator* Des = new CDesignator(parent_element, false);
			int err_num = Des->Init(lb);
			if (err_num) {
				delete Des;
				return err_num;
			}
			//�������� ������� ��������� ������ ��� ��������� ������������
			if (Des->IsProcName())
				Base = new CCallStatement(parent_element, Des, true, true);
			else
				Base = new CAssignStatement(parent_element, Des);
		}//else
		break;

	//�������� ������� ����������� ��������� (StdProc)
	case lex_k_ASSERT:
		Base = new CAssertStdProc(parent_element);
		break;
	case lex_k_COPY:
		Base = new CCopyStdProc(parent_element);
		break;
	case lex_k_DEC:
		Base = new CDecStdProc(parent_element);
		break;
	case lex_k_EXCL:
		Base = new CExclStdProc(parent_element);
		break;
	case lex_k_HALT:
		Base = new CHaltStdProc(parent_element);
		break;
	case lex_k_INC:
		Base = new CIncStdProc(parent_element);
		break;
	case lex_k_INCL:
		Base = new CInclStdProc(parent_element);
		break;
	case lex_k_NEW:
		Base = new CNewStdProc(parent_element);
		break;

	//�������� ������� ��������� (Statement)
	case lex_k_IF:
		Base = new CIfStatement(parent_element);
		break;
	case lex_k_CASE:
		Base = new CCaseStatement(parent_element);
		break;
	case lex_k_WHILE:
		Base = new CWhileStatement(parent_element);
		break;
	case lex_k_REPEAT:
		Base = new CRepeatStatement(parent_element);
		break;
	case lex_k_FOR:
		Base = new CForStatement(parent_element);
		break;
	case lex_k_LOOP:
		Base = new CLoopStatement(parent_element);
		break;
	case lex_k_WITH:
		Base = new CWithStatement(parent_element);
		break;
	case lex_k_EXIT:
		Base = new CExitStatement(parent_element);
		break;
	case lex_k_RETURN:
		Base = new CReturnStatement(parent_element);
		break;

	//�������� ������� ������ ���������-������� (������)
	case lex_k_ABS:
	case lex_k_ASH:
	case lex_k_CAP:
	case lex_k_CHR:
	case lex_k_ENTIER:
	case lex_k_LEN:
	case lex_k_LONG:
	case lex_k_MAX:
	case lex_k_MIN:
	case lex_k_ODD:
	case lex_k_ORD:
	case lex_k_SHORT:
	case lex_k_SIZE:
		return s_e_CallFuncAsProc;

	}//switch

	//������������� ���������� ������� (���� �� ��� ������)
	if (Base) {
		int err_num = Base->Init(lb);
		if (err_num) {
			delete Base;
			return err_num;
		}
		//��������� ���������� ������� � ������
		StatStore.push_back(Base);
	} else {
		//������ ������ �������� (������ ���)
		RESTORE_POS
	}

	return 0;
}//CStatementSeq::StatementInit


//-----------------------------------------------------------------------------
//������ ���� CStatementSeq
void CStatementSeq::WriteCPP(CPP_files& f)
{
	f.tab_level_c++;

	CBaseVector::iterator i;
	for(i = StatStore.begin(); i != StatStore.end(); ++i) {
		static_cast<CBase*>(*i)->WriteCPP(f);
		fprintf(f.fc, ";\n");
	}

	f.tab_level_c--;
}//WriteCPP


//-----------------------------------------------------------------------------
//������������� ������� EXIT �� ������ ������
int CExitStatement::Init(CLexBuf *lb)
{
	//�������� ���������� EXIT � StatementSeq �� ��������� LOOP
	const CBaseName* BN = this->parent_element;
	while (BN) {
		//�������� ������� ����� parent_element �������, ��������������� ��� ����� � ���������� LOOP
		if (id_CWithLoopLink == BN->name_id && static_cast<const CWithLoopLink*>(BN)->UnderLoop()) {
			//���������� UID ��������� LOOP (���������� ��� ��������� ���� C++)
			UID = static_cast<const CWithLoopLink*>(BN)->LoopUID;
			return 0;
		}
		BN = BN->parent_element;
	}
	return s_e_EXIT_NotInLoop;
}//Init


//-----------------------------------------------------------------------------
//������ ���� CExitStatement
void CExitStatement::WriteCPP(CPP_files& f)
{
	f.tab_fc();
	fprintf(f.fc, "goto O2M_EXIT_%i", UID);
}//WriteCPP


//-----------------------------------------------------------------------------
//���������� ������� FOR
CForStatement::~CForStatement()
{
	delete[] var_name;
	delete StatementSeq;
	delete ForExpr;
	delete ToExpr;
}//~CForStatement


//-----------------------------------------------------------------------------
//������������� ������� FOR �� ������ ������
int CForStatement::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//��������� ��-��
	if (!lb->ReadLex(li) || (lex_i != li.lex)) return s_e_IdentExpected;
	var_name = str_new_copy(li.st);

	//����� ��-�� � ������� ���� � �������� ��� ����
	CBaseName* BN = parent_element->GetGlobalName(var_name);
	if (!BN) return s_e_UndeclaredIdent;
	if (!CBaseVar::IsIntId(BN->name_id)) return s_e_FOR_VarNotInt;

	//�������� ������� ��. ����� :=
	if (!lb->ReadLex(li) || lex_k_assign != li.lex) return s_e_AssignMissing;

	//�������� ������� ��������� For
	ForExpr = new CExpr(parent_element);
	int err_num = ForExpr->Init(lb);
	if (err_num) return err_num;

	//�������� ������� ��. ����� TO
	if (!lb->ReadLex(li) || lex_k_TO != li.lex) return s_e_TO;

	//�������� ������� ��������� To
	ToExpr = new CExpr(parent_element);
	err_num = ToExpr->Init(lb);
	if (err_num) return err_num;

	//������ ��������� ������� (��������� �����)
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_DO;

	//�������� ������� ConstExpr, ���� ������� BY
	if (lex_k_BY == li.lex) {
		//�������� ������������ ��������� ��� BY
		CBaseVar* ByVar;
		err_num = ConstSelector(lb, ByVar, parent_element);
		if (err_num) return err_num;
		//�������� ��������� ������������� ���������
		if (!CBaseVar::IsIntId(ByVar->name_id)) {
			delete ByVar;
			return s_e_ExprNotIntConst;
		}
		step = ByVar->GetIntValue();
		delete ByVar;
		//�������� ����, ������� 0
		if (!step) return s_e_FOR_BY_Zero;
		//������ ��������� ������� (��������� �����)
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_DO;
	} else
		step = 1;	//��� �� ���������

	//�������� ������� ��. ����� DO
	if (lex_k_DO != li.lex) return s_e_DO;

	//�������� ������� ������. ����������
	StatementSeq = new CStatementSeq(parent_element);
	err_num = StatementSeq->Init(lb);
	if (err_num) return err_num;

	//�������� ������� ��. ����� END
	if (!lb->ReadLex(li) || lex_k_END != li.lex) return s_e_END;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� CForStatement
void CForStatement::WriteCPP(CPP_files& f)
{
	//��������� ���������� (������� TO �� ������ �������� ������ �����)
	f.tab_fc();
	fprintf(f.fc, "{long O2M_FOR=");
	ToExpr->WriteCPP(f);
	fprintf(f.fc, ";\n");

	//��������� ��������� for
	f.tab_fc();
	fprintf(f.fc, "for(%s=", var_name);
	ForExpr->WriteCPP(f);

	//��������� ��������� �������� ����
	fprintf(f.fc, "; %s%c=O2M_FOR; %s+=%li) {\n", var_name, (step > 0) ? '<' : '>', var_name, step);

	//������ ������������������ ����������
	StatementSeq->WriteCPP(f);

	f.tab_fc();
	fprintf(f.fc, "}}");
}//WriteCPP


//-----------------------------------------------------------------------------
//����������� ���������� ��� �������� ����������� ID ��� ������� ��������� LOOP
int CLoopStatement::CurrentUID = 0;


//-----------------------------------------------------------------------------
//���������� ������� LOOP
CLoopStatement::~CLoopStatement()
{
	delete StatementSeq;
}//~CLoopStatement


//-----------------------------------------------------------------------------
//������������� ������� LOOP �� ������ ������
int CLoopStatement::Init(CLexBuf *lb)
{
	//��������� UID ��� �������� � StatementSeq (� ��� ������ ����� ���� ��������� EXIT)
	WithLoopLink.LoopUID = CurrentUID++;

	//�������� ������� ������. ����������
	StatementSeq = new CStatementSeq(&WithLoopLink);
	int err_num = StatementSeq->Init(lb);
	if (err_num) return err_num;

	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� ��. ����� END
	if (!lb->ReadLex(li) || lex_k_END != li.lex) return s_e_END;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� CLoopStatement
void CLoopStatement::WriteCPP(CPP_files& f)
{
	f.tab_fc();
	fprintf(f.fc, "while(true) {\n");

	StatementSeq->WriteCPP(f);

	f.tab_fc();
	fprintf(f.fc, "} O2M_EXIT_%i:", WithLoopLink.LoopUID);
}//WriteCPP


//-----------------------------------------------------------------------------
//���������� ������� REPEAT
CRepeatStatement::~CRepeatStatement()
{
	delete StatementSeq;
	delete Expr;
}//~CRepeatStatement


//-----------------------------------------------------------------------------
//������������� ������� REPEAT �� ������ ������
int CRepeatStatement::Init(CLexBuf *lb)
{
	//�������� ������� ������. ����������
	StatementSeq = new CStatementSeq(parent_element);
	int err_num = StatementSeq->Init(lb);
	if (err_num) return err_num;

	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� ��. ����� UNTIL
	if (!lb->ReadLex(li) || lex_k_UNTIL != li.lex) return s_e_UNTIL;

	//�������� ������� ���������
	Expr = new CExpr(parent_element);
	err_num = Expr->Init(lb);
	if (err_num) return err_num;

	//�������� ���� ���������
	return (Expr->GetResultId() == id_CBooleanVar) ? 0 : s_e_UNTIL_ExprType;
}//Init


//-----------------------------------------------------------------------------
//������ ���� CRepeatStatement
void CRepeatStatement::WriteCPP(CPP_files& f)
{
	f.tab_fc();
	fprintf(f.fc, "do {\n");

	StatementSeq->WriteCPP(f);

	f.tab_fc();
	fprintf(f.fc, "} while(!(");

	Expr->WriteCPP(f);
	fprintf(f.fc, "))");
}//WriteCPP


//-----------------------------------------------------------------------------
//���������� ������� RETURN
CReturnStatement::~CReturnStatement()
{
	delete Expr;
}//~CReturnStatement


//-----------------------------------------------------------------------------
//������������� ������� RETURN �� ������ ������
int CReturnStatement::Init(CLexBuf *lb)
{
	//��������� ��. �� ������������ ������
	const CBaseName *BN = parent_element;
	//����� ��������� (��� ������) ����� �������� ������������ ��������
	while (BN->parent_element && !CProcedure::IsProcId(BN->name_id))
		BN = BN->parent_element;

	//�������� ������������ ��������� � ��������� (�������)
	if (CProcedure::IsProcId(BN->name_id)) {
		//��������� ���������� ��������� � �������� ������������ � �������
		const EName_id ResultId = static_cast<const CProcedure*>(BN)->GetResultId();
		if (id_CBaseName != ResultId) {
			//�������� ������������� ���������������� ��������� ��� ����������
			if (id_CCharVar == ResultId)
				Expr = new CExpr(parent_element, ek_Char, false);
			else
				Expr = new CExpr(parent_element);
			//������������� ���������
			int err_num = Expr->Init(lb);
			if (err_num) return err_num;
			//����� � ������ id_CCharVar == ResultId �������� �� err_num,
			//� ����� ���������� ��������� � ��������������� �� ������������
			/**/
			//����� ���������� �������� ������������� �� ������������ Expr->GetResultId � ResultId
			/**/
			//���������� ��������� ���������
			return 0;
		}
	}

	//������ �������� ��������� � ��������� ��� ������

	DECL_SAVE_POS

	//��������� ���������� ����������� ���������
	Expr = new CExpr(parent_element);
	if (!Expr->Init(lb)) {
		//��������� ��������� - ������� ��������� �� ������ (� ��������� ���������� � ������)
		if (id_CModule == BN->name_id)
			return s_e_RETURN_ExprInModule;
		else
			return s_e_RETURN_ExprInProc;
	}
	delete Expr;
	Expr = NULL;

	RESTORE_POS

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� CReturnStatement
void CReturnStatement::WriteCPP(CPP_files& f)
{
	if (Expr) {
		f.tab_fc();
		fprintf(f.fc, "O2M_RESULT = ");
		Expr->WriteCPP(f);
		fprintf(f.fc, ";\n");
	}
	f.tab_fc();
	fprintf(f.fc, "goto O2M_RETURN");
}//WriteCPP


//-----------------------------------------------------------------------------
//���������� ������� WHILE
CWhileStatement::~CWhileStatement()
{
	delete StatementSeq;
	delete Expr;
}//~CWhileStatement


//-----------------------------------------------------------------------------
//������������� ������� WHILE �� ������ ������
int CWhileStatement::Init(CLexBuf *lb)
{
	//�������� ������� ���������
	Expr = new CExpr(parent_element);
	int err_num = Expr->Init(lb);
	if (err_num) return err_num;

	//�������� ���� ���������
	if (Expr->GetResultId() != id_CBooleanVar) return s_e_WHILE_ExprType;

	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� ��. ����� DO
	if (!lb->ReadLex(li) || lex_k_DO != li.lex) return s_e_DO;

	//�������� ������� ������. ����������
	StatementSeq = new CStatementSeq(parent_element);
	err_num = StatementSeq->Init(lb);
	if (err_num) return err_num;

	//�������� ������� ��. ����� END
	if (!lb->ReadLex(li) || lex_k_END != li.lex) return s_e_END;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� CWhileStatement
void CWhileStatement::WriteCPP(CPP_files& f)
{
	f.tab_fc();
	fprintf(f.fc, "while(");

	Expr->WriteCPP(f);
	fprintf(f.fc, ") {\n");

	StatementSeq->WriteCPP(f);

	f.tab_fc();
	fprintf(f.fc, "}");
}//WriteCPP


//-----------------------------------------------------------------------------
//������������� ������� Assign (�������� ������������) �� ������ ������
int CAssignStatement::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� ��. ����� ":="
	if (!lb->ReadLex(li) || lex_k_assign != li.lex) return s_e_AssignMissing;

	//�������� ���������� ��-��� ������ ��� ������ � ����� ����� ������������
	if (Designator->IsReadOnly()) return s_e_AssignReadonly;

	//��������� �������, �� ������� ��������� �����������
	CBaseName *BN = Designator->FindLastName();
	//�������� ������� ������� � ���������� �������� ���������, ���� �� �����������,
	//������ �����������, �.�. ������ ��������� ����� ���� ��������������
	if (!BN || (CBaseVar::IsVarId(BN->name_id) && static_cast<CBaseVar*>(BN)->is_const)) return s_e_AssignConst;

	//�������� ���������� ���������� ���������� ����� ��. �� ���������� ����������
	if (id_CCommonVar == BN->name_id) {
		cv_compound_name = static_cast<CCommonVar*>(BN)->GetCPPCompoundName();
		if (!cv_compound_name) return s_e_SpecTypeExpected;
	}

	//�������� ���������� ��������� ������ �������
	if (id_CArrayVar == Designator->GetResultId())
		str_to_array = true;
	if (id_CPointerVar == BN->name_id && static_cast<CPointerVar*>(BN)->IsArrayPointer())
		str_to_array = true;

	//�������� ��������� ������������� ��������� 
	switch (Designator->GetResultId()) {
	case id_CCharVar:
		Expr = new CExpr(parent_element, ek_Char, false);
		break;
	case id_CProcedureVar:
		Expr = new CExpr(parent_element, ek_ProcedureVar, false);
		break;
	default:
		Expr = new CExpr(parent_element);
	}

	//������������� ���������
	int err_num = Expr->Init(lb);
	if (err_num) return err_num;
	EName_id ERI = Expr->GetResultId();

	//�������� ���������� ��������� � ������ ���������� ���������� ����������
	if (cv_compound_name && id_CRecordVar != ERI) return s_e_Incompatible;


	////////////////////////////////////////
	//�������� ������������� �� ������������
	/**/

	//���� ��� ���������� ���������� �� ����������� ������� ���������� �����
	if (id_CPointerVar == Designator->GetResultId() && id_CPointerVar != ERI && id_CArrayVar != ERI) return s_e_Incompatible;

	//�������� ���������� ��������� ������ �������
	if (id_CArrayVar == BN->name_id && id_CArrayVar != ERI && id_CCharVar != ERI) return s_e_Incompatible;

	//�������� ������������� �� ������������ ��� �������� �����
	/**/	//��������, ���� �� ����������� ������ ��������
	if (CBaseVar::IsDigitId(BN->name_id))
		if (!IsId1IncloseId2(BN->name_id, ERI))
			return s_e_Incompatible;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//������ ���� CAssignStatement
void CAssignStatement::WriteCPP(CPP_files& f)
{
	////////////////////////////////////////////////////
	//��������� ���� ���������� ��������� ������ �������
	if (str_to_array) {
		WriteCPP_array(f);
		return;
	}

	//////////////////////////////////////
	//��������� ���� �������� ������������
	f.tab_fc();
	//� ������ ���������� ���������� ���������� ���������� ���. �������� ����������
	if (cv_compound_name) fprintf(f.fc, "*");
	//������ ���� ����������� ��� ��������� ���� ������ (���� ���� ������)
	Designator->WriteCPP_Guardless(f);
	//� ������ ���������� ���������� ���������� ���������� ��������� � �����. �����
	if (cv_compound_name) fprintf(f.fc, "->O2M_SPEC_%s", cv_compound_name);
	//������ �������� ����������
	fprintf(f.fc, " = ");
	//������ ���������
	Expr->WriteCPP(f);
}//WriteCPP


//-----------------------------------------------------------------------------
//������ ���� � ������ ���������� ��������� ������ �������
void CAssignStatement::WriteCPP_array(CPP_files &f)
{
	f.tab_fc();
	fprintf(f.fc, "COPY(");

	//������ �������� ���������
	Expr->WriteCPP(f);
	fprintf(f.fc, ", ");
	//��������� ��������� (����������-������� ��� ���������)
	CBaseName* BN = Expr->FindLastName();
	WriteCPP_COPY_Par(f, BN);
	fprintf(f.fc, ", ");

	//������ �������� ���������
	Designator->WriteCPP(f);
	fprintf(f.fc, ", ");
	//��������� ��������� (����������-������� ��� ���������)
	BN = Designator->FindLastName();
	WriteCPP_COPY_Par(f, BN);
	fprintf(f.fc, ")");

}


//-----------------------------------------------------------------------------
//������ ���� CGuardPair
void CGuardPair::WriteCPP(CPP_files &f)
{
	//������ ���� �������� ������������� ���� ����������
	Guard->WriteCPP(f);

	//������ ������������������ ����������
	if (StatementSeq) StatementSeq->WriteCPP(f);

	//������ ����������� ������ ����� ���������� (����������� ������ �������� � Guard->WriteCPP)
	f.tab_fc();
	fprintf(f.fc, "}");
}


