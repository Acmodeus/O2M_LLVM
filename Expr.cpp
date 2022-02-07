//=============================================================================
// �������� ���������� ��� ���������� ���������
//=============================================================================

#include "Expr.h"
#include "Type.h"
#include "Var.h"
#include "StdProc.h"


//-----------------------------------------------------------------------------
//����������
CExprList::~CExprList()
{
	CExprVector::iterator i;
	for(i = ExprVector->begin(); i != ExprVector->end(); ++i)
		delete *i;
	delete ExprVector;
}//~CExprList


//-----------------------------------------------------------------------------
//������������� ������� ExprList �� ������ ������
int CExprList::Init(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//������� ����� ��������� � ������ ����������� ����������
	int curr_fp_number = 0;

	//���� ��������� ���������
	while (true) {

		//�� ��������� ������� ������������� ���������
		EExprKind ek = ek_Normal;

		//��������� ���� ����������� ��������� (���� ����) ��� ����������� ��������� ������������� ���������
		if (PFP) {
			CBaseName* BN = PFP->GetNameByIndex(curr_fp_number);
			if (BN)
				switch (BN->name_id) {
				case id_CCharVar:
					ek = ek_Char;
					break;
				case id_CProcedureVar:
					ek = ek_ProcedureVar;
					break;
				}
		}

		//�������� ���������� ��������� � ������ ��������� �������������
		CExpr* Expr = new CExpr(parent_element, ek, PFP != NULL);

		//������������� ���������� ���������
		int err_num = Expr->Init(lb);
		if (err_num) {
			delete Expr;
			return err_num;
		}

		/**/
		//������ �������� ������������� �� ������������

		//���������� ������������������� ��������� � ������
		ExprVector->push_back(Expr);

		//�������� ������� "," (����� ������ ���� � ��������� ���������)
		DECL_SAVE_POS
		if (!lb->ReadLex(li) || lex_k_comma != li.lex) {
			RESTORE_POS
			return 0;
		}

		//���� ����. ��������� - ����������� ������� �����
		curr_fp_number++;
	}//while

}//Init


//-----------------------------------------------------------------------------
//������ ���� CExprList
void CExprList::WriteCPP(CPP_files& f)
{
	CExprVector::const_iterator ci = ExprVector->begin();
	//������ ���� ������� ��������� (���� ����)
	if (ci != ExprVector->end()) (*ci)->WriteCPP(f);
	//������ ���� ����������� ��������� (���� ����)
	for (++ci; ci != ExprVector->end(); ++ci) {
		fprintf(f.fc, ", ");
		(*ci)->WriteCPP(f);
	}
}//WriteCPP


//-----------------------------------------------------------------------------
//������ ���� ������ ������������ ��������� ��� ������ ���������, ��� �������� �������� ������������ ���. ���������
//FactExpr - ��������� ������������ ��������� , FormalPar - ���������� ��������
void CExprList::WriteCPP_proc_OneParam(CPP_files& f, CExpr* FactExpr, CBaseName* FormalPar)
{
	//�������� ������� ��������� ������� � ���������� ����������
	bool is_open_arr = (id_CArrayVar == FormalPar->name_id && 0 == static_cast<CArrayVar*>(FormalPar)->ArrayType->size);
	//��������� ������������ ���������, ������������� � ��������� - ����� ���� NULL � ������ ���������,
	//����� ���� ����� � ������ ������������� ��-�� �������
	CBaseVar* FactPar = static_cast<CBaseVar*>(FactExpr->FindLastName());

	//��������, ��������� �� ��������� � ��������� ��� VAR RECORD (��� ������ &)
	if (id_CRecordVar == FormalPar->name_id && static_cast<CBaseVar*>(FormalPar)->is_var) {
		//��������� ��������� ��� VAR RECORD, ��������� ��� ������������ ���������
		if (FactPar)
			switch (FactPar->name_id) {
			case id_CRecordVar:
				//������ ������ �� ��������� � ������ ���������-���������� ������
				if (FactPar->is_var) break;
			case id_CPointerVar:
				//��������� � �������� ����. ��������� ����� ���� ������ � '^' (������������� �� ������������)
			case id_CRecordType:
				//����������� ���� ������ ������
				fprintf(f.fc, "&");
			}//switch
	}


	//��������, ��� ����������� ������� ���������� ��� ���������� - ����� �� ������ �����������
	/**/
	/*vvvvvvvvvvvvvv*/
	//������ ���������� ���� ��� ��������� ������� (���������� ��� ������������ �������)
	if (is_open_arr && FactPar && id_CArrayVar == FactPar->name_id) {
		//��������� ��. �� ���
		CBaseType* BT = static_cast<CArrayVar*>(FactPar)->ArrayType->FindLastType();
		CBaseVar* BV = NULL;
		//�������� ��������� ���������� �� ����������� ����
		BT->CreateVar(BV, BT->parent_element);
		fprintf(f.fc, "(");
		if (BV->GetTypeModuleAlias()) fprintf(f.fc, "%s::", BV->GetTypeModuleAlias());
		fprintf(f.fc, "%s*)", BV->GetTypeName());
		delete BV;
	}
	/*^^^^^^^^^^^^^^*/


	//�������� ������ "�������� ������ + ���������� ���������" (�� ��������� ��������� �� ����� �������� ������������� ���������)
	if (!(is_open_arr && id_CCharVar == FactExpr->GetResultId()))
		//������ ���� ���������
		FactExpr->WriteCPP(f);

	//�������� ������������ ������������������ ���������� ���������� � �������� ������ ����������������� ����
	if (FactPar && id_CCommonVar == FactPar->name_id && id_CRecordVar == FormalPar->name_id)
		fprintf(f.fc, "->O2M_SPEC_%s", static_cast<CCommonVar*>(FactPar)->GetCPPCompoundName());

	//��������� ��������� ������� � ���������� ���������� (������ ������ ������������)
	if (is_open_arr) {

		//��������� ���� ������� ��������� ������������ ���������
		const CArrayType* AT;
		if (FactPar && id_CArrayVar == FactPar->name_id)
			AT = static_cast<CArrayVar*>(FactPar)->ArrayType;
		else {

			//��������� ����� (����� ���� ��������� ��� ���������� ���������� ��� ��� ������)
			CBaseName* BN = FactExpr->FindLastName();

			//�������� ��������� ���������� ���������, �������������� �� ��� ������ � 1 ������
			if (id_CCharVar == BN->name_id) {
				fprintf(f.fc, "\"%c\", 2", static_cast<CCharVar*>(BN)->ConstValue);	//1 ������ + '\0'
				return;
			}

			//���� �������� ��� ���������� ��� ��� ������ (������ ���� ��������� ��� �������������)
			if (id_CArrayVar == BN->name_id)
				AT = static_cast<CArrayVar*>(BN)->ArrayType;
			else
				AT = static_cast<CArrayType*>(BN);
		}

		//������ ������ ������������ (���� ���� ? -> ArrOfChar + "String")
		if (AT) {
			int dimension = 0;	//������ ������������ � 0
			while (id_CArrayType == AT->name_id) {
				//����� ����������� (� ���� ����� ��� ����� ����������, ���������� �����������)
				if (AT->size)
					fprintf(f.fc, ", %i", AT->size);
				else {
					fprintf(f.fc, ", O2M_ARR_%i_", dimension);
					FactExpr->WriteCPP(f);	//��� ��������� ������� FactExpr ������ ��������� �������� (?)
				}
				//������� � ��������� �����������
				++dimension;
				AT = static_cast<CArrayType*>(AT->Type);
			}//while
		}

	}//if
}


//-----------------------------------------------------------------------------
//������ ����, ���� CExprList �������� ������ ���������� ��� ������ ���������
void CExprList::WriteCPP_proc(CPP_files& f, CFormalPars* FP)
{
	CExprVector::const_iterator ci;
	int number = 0;
	for (ci = ExprVector->begin(); ci != ExprVector->end(); ++ci, ++number) {
		//����������� ����������� ������ ����� ������� ���������
		fprintf(f.fc, "%s", number ? ", " : "");
		//������ ���������� ���������
		WriteCPP_proc_OneParam(f, *ci, FP->GetNameByIndex(number));
	}
}//WriteCPP_proc


//-----------------------------------------------------------------------------
//������ ����, ���� CExprList �������� ������ ���������� ��� ������ ��������������� ���������
void CExprList::WriteCPP_common_proc(CPP_files& f, CFormalPars* FP)
{
	CExprVector::const_iterator ci;
	int number = 0;
	for (ci = ExprVector->begin(); ci != ExprVector->end(); ++ci, ++number) {
		//����������� ����������� ������ ����� ������� ���������
		fprintf(f.fc, "%s&", number ? ", " : "");
		//������ ���������� ���������
		WriteCPP_proc_OneParam(f, *ci, FP->GetNameByIndex(number));
	}
}


//-----------------------------------------------------------------------------
//������ ���� CExprList ��� �������� �������, IsOpenArray - ������� ��������� �������,
//ArrayName - �������� ������� (���. ��� ��������� ������� ������� ����� ����������)
void CExprList::WriteCPP_index(CPP_files& f, bool IsOpenArray, const char* ArrayName/*CBaseName* BN*/)
{
	fprintf(f.fc, "[");

	//�������� ������� ��������� �������
	if (IsOpenArray) {//������ �������� => �������������� ���������

		//������ ���� ������� ��������� (���� ����)
		CExprVector::const_iterator ci = ExprVector->begin();
		if (ci != ExprVector->end()) {
			fprintf(f.fc, "(");
			(*ci)->WriteCPP(f);
			fprintf(f.fc, ")");
		}
		//������ ���� ����������� ��������� (���� ����)
		int i;
		//��� ��������� �������� �� ������ �����������
		for (i = 1; i < ExprVector->size(); i++) fprintf(f.fc, "*O2M_ARR_%i_%s", i, ArrayName);
		//������� ���������� ������������
		int dimention = 1;	//������� �����������
		for (++ci; ci != ExprVector->end(); ++ci) {
			fprintf(f.fc, "+(");
			(*ci)->WriteCPP(f);
			fprintf(f.fc, ")");
			++dimention;
			//��� ��������� �������� �� ������� �����������
			for (i = dimention; i < ExprVector->size(); i++) fprintf(f.fc, "*O2M_ARR_%i_%s", i, ArrayName);
		}//for

	} else {//������ �� �������� => ������� �������

		//������ ���� ������� ��������� (���� ����)
		CExprVector::const_iterator ci = ExprVector->begin();
        if (ci != ExprVector->end())
            (*ci)->WriteCPP(f);
		//������ ���� ����������� ��������� (���� ����)
		for (++ci; ci != ExprVector->end(); ++ci) {
			fprintf(f.fc, "][");
			(*ci)->WriteCPP(f);
		}//for

	}//else

	fprintf(f.fc, "]");
}//WriteCPP_index


//-----------------------------------------------------------------------------
//����������
CFactor::~CFactor()
{
	delete ConstVar;
	delete StdProcFunc;
	delete Factor;
	delete Expr;
	delete Designator;
	delete Call;
}//~CFactor


//-----------------------------------------------------------------------------
//������������� ������� Factor �� ������ ������
int CFactor::Init(CLexBuf *lb)
{
	DECL_SAVE_POS
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//////////////////////////////////////////
	//������ ������� ��� �������� ���� �������
	if (!lb->ReadLex(li)) return s_e_Factor;

	///////////////////////////////////////////////////////
	//NIL - ������ �������� �� ������� �� �������� ExprKind
	if (lex_k_NIL == li.lex) {
		FactorKind = fk_ConstVar;
		ConstVar = new CPointerVar(parent_element);
		ConstVar->is_const = true;
		return CalcResultId();
	}

	//����� ������ ����� �������������� ������ switch'a
	int err_num;

	///////////////////////////////////////////////////////////////////////////
	//�������� ������������� ������������� ��������� ��� ����������� ����������
	if (ek_ProcedureVar == ExprKind) {
		RESTORE_POS
		//�������� � ������������� Designator
		Designator = new CDesignator(parent_element, in_fact_pars);
		Designator->req_proc_var = true;
		err_num = Designator->Init(lb);
		if (err_num) return err_num;
		//�������� ������������ ���� ���������� � ������ ��������� �������������
		switch (Designator->GetResultId()) {
		case id_CProcedure:
		case id_CProcedureVar:
			FactorKind = fk_Designator;
			return CalcResultId();
		default:
			return s_e_Incompatible;
		}//switch
	}

	//////////////////////////////////////////////////////////////
	//��������� �������������� �������� � ������ ������� ���������

	//��� �������� ����� � ������ ������������� ��� ���������� ���������
	long dec_num = 0;
	//�������� ������������� �������������� �� 16# � 10# ������� ���������
	if (lex_h == li.lex || lex_c == li.lex) {
		//�������������� �������� ������ ������� �� 16# � 10# ������� ���������
		int num = 0;
		int mul = 1;
		int len = strlen(li.st);
		for (int i = 1; i <= len; i++) {
			switch (li.st[len - i]) {
			case '1': num = 1; break;
			case '2': num = 2; break;
			case '3': num = 3; break;
			case '4': num = 4; break;
			case '5': num = 5; break;
			case '6': num = 6; break;
			case '7': num = 7; break;
			case '8': num = 8; break;
			case '9': num = 9; break;
			case 'A': num = 10; break;
			case 'B': num = 11; break;
			case 'C': num = 12; break;
			case 'D': num = 13; break;
			case 'E': num = 14; break;
			case 'F': num = 15; break;
			default: num = 0;
			}
			dec_num += num * mul;
			mul *= 16;
		}
	} else	//�������� ������ � ���������� ������ - ����������� � �����
		if (lex_d == li.lex) dec_num = atoi(li.st);

	//////////////////////////////////
	//�������� ���� ���������� �������
	switch (li.lex) {

	////////////////////////////////////////////////////
	//�������� ������� (��� ��� ���������� ��� Factor'a)

	//'{' ����������� ��������� ��� ����� ���������� ������� ���� {a,b}.ProcName()
	case lex_k_op_brace:
		//������� ��������� ������� ������������ SET
		ConstVar = new CSetVar(parent_element);
		err_num = static_cast<CSetVar*>(ConstVar)->SetInit(lb);
		//�������� ������� �� ������� "}" ����� (������� ������ ���������� ���������)
		if (!err_num) {
			DECL_SAVE_POS
			if (!lb->ReadLex(li) || lex_k_dot != li.lex) {
				//����� �� ����������
				RESTORE_POS
				//������ FactorKind ��������, ��������� ���������
				FactorKind = fk_ConstVar;
				return CalcResultId();
			}//else
		}
		//���� ���������� ������ ��� ����� - ����� ���� � ������� ���������� ���������
		RESTORE_POS
		//���������� ����������
		delete ConstVar;
		ConstVar = NULL;
		//�������� � ������������� CallStatement
		Call = new CCallStatement(parent_element, NULL, false, false);
		err_num = Call->CCallStatement::Init(lb);
		if (err_num) return err_num;
		//��������� ���������� � CallStatement �����������
		Designator = Call->GiveDesAway();
		//������ FactorKind ��������, ��������� ���������
		FactorKind = fk_Designator;
		return CalcResultId();

	//'~' Factor
	case lex_k_negation:
		FactorKind = fk_Negation;
		Factor = new CFactor(parent_element, ExprKind, in_fact_pars);
		err_num = Factor->Init(lb);
		if (err_num) return err_num;
		return CalcResultId();

	//'(' Expr ')'
	case lex_k_op_bracket:
		FactorKind = fk_Expr;
		Expr = new CExpr(parent_element);
		err_num = Expr->Init(lb);
		if (err_num) return err_num;
		if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex)
			return s_e_ClBracketMissing;
		return CalcResultId();

	///////////////////////////////
	//�������� ��������� (��������)

	//Digit (SHORTINT, INTEGER, LONGINT)
	case lex_d:
	case lex_h:
		FactorKind = fk_ConstVar;
		//�������� ������������ ����, ������������ ��� �������� �����
		if (SHRT_MAX >= dec_num) {
			ConstVar = new CShortintVar(parent_element);
			static_cast<CShortintVar*>(ConstVar)->ConstValue = dec_num;
		} else
			if (INT_MAX >= dec_num) {
				ConstVar = new CIntegerVar(parent_element);
				static_cast<CIntegerVar*>(ConstVar)->ConstValue = dec_num;
			} else {
				ConstVar = new CLongintVar(parent_element);
				static_cast<CLongintVar*>(ConstVar)->ConstValue = dec_num;
			}
		//��������� �������� �������������
		ConstVar->is_const = true;
		//����� ��������� ������������� ���������
		return CalcResultId();

	//Digit (REAL)
	case lex_r:
		FactorKind = fk_ConstVar;
		ConstVar = new CRealVar(parent_element);
		static_cast<CRealVar*>(ConstVar)->SetConstValue(li.st);
		return CalcResultId();

	//Digit (LONGREAL)
	case lex_l:
		FactorKind = fk_ConstVar;
		ConstVar = new CLongrealVar(parent_element);
		static_cast<CLongrealVar*>(ConstVar)->SetConstValue(li.st);
		return CalcResultId();

	//Char (CHAR)
	case lex_c:
		FactorKind = fk_ConstVar;
		//�������� ������������ �������
		if ((unsigned char)dec_num != dec_num) return l_e_CharConstTooLong;
		//�������� ����������
		ConstVar = new CCharVar(parent_element);
		static_cast<CCharVar*>(ConstVar)->SetConstValue(dec_num);
		return CalcResultId();

	//String (ARRAY N OF CHAR) ��� Char (CHAR)
	case lex_s:
		FactorKind = fk_ConstVar;
		//�������� ���������� ���� ���������
		if (ek_Char == ExprKind) {
			//�������� ����������� ������� ������ ��������
			if (strlen(li.st) != 1) return s_e_ExprCompatibility;
			//�������� ���������� (�������)
			ConstVar = new CCharVar(parent_element);
			static_cast<CCharVar*>(ConstVar)->SetConstValue(li.st[0]);
			//����� ���������
			return CalcResultId();
		}
		//�������� ����������
		ConstVar = new CArrayVar(parent_element);
		static_cast<CArrayVar*>(ConstVar)->SetConstValue(li.st);
		//�������� ���� "������ ��������" � ������� ��� � ����������
		{
			CArrayType *AT = new CArrayType(parent_element);
			AT->size = strlen(li.st) + 1;
			AT->Type = new CCharType(parent_element);
			static_cast<CArrayVar*>(ConstVar)->ArrayType = AT;
		}
		return CalcResultId();

	//TRUE (BOOLEAN)
	case lex_k_TRUE:
		FactorKind = fk_ConstVar;
		ConstVar = new CBooleanVar(parent_element);
		static_cast<CBooleanVar*>(ConstVar)->SetConstValue(true);
		return CalcResultId();

	//FALSE (BOOLEAN)
	case lex_k_FALSE:
		FactorKind = fk_ConstVar;
		ConstVar = new CBooleanVar(parent_element);
		static_cast<CBooleanVar*>(ConstVar)->SetConstValue(false);
		return CalcResultId();

	////////////////////////////////////////////////
	//�������� ������� ����������� ���������-�������
	case lex_k_ABS:
		FactorKind = fk_StdProcFunc;
		StdProcFunc = new CAbsStdProcFunc(parent_element);
		break;
	case lex_k_ASH:
		FactorKind = fk_StdProcFunc;
		StdProcFunc = new CAshStdProcFunc(parent_element);
		break;
	case lex_k_CAP:
		FactorKind = fk_StdProcFunc;
		StdProcFunc = new CCapStdProcFunc(parent_element);
		break;
	case lex_k_CHR:
		FactorKind = fk_StdProcFunc;
		StdProcFunc = new CChrStdProcFunc(parent_element);
		break;
	case lex_k_ENTIER:
		FactorKind = fk_StdProcFunc;
		StdProcFunc = new CEntierStdProcFunc(parent_element);
		break;
	case lex_k_LEN:
		FactorKind = fk_StdProcFunc;
		StdProcFunc = new CLenStdProcFunc(parent_element);
		break;
	case lex_k_LONG:
		FactorKind = fk_StdProcFunc;
		StdProcFunc = new CLongStdProcFunc(parent_element);
		break;
	case lex_k_MAX:
		FactorKind = fk_StdProcFunc;
		StdProcFunc = new CMaxStdProcFunc(parent_element);
		break;
	case lex_k_MIN:
		FactorKind = fk_StdProcFunc;
		StdProcFunc = new CMinStdProcFunc(parent_element);
		break;
	case lex_k_ODD:
		FactorKind = fk_StdProcFunc;
		StdProcFunc = new COddStdProcFunc(parent_element);
		break;
	case lex_k_ORD:
		FactorKind = fk_StdProcFunc;
		StdProcFunc = new COrdStdProcFunc(parent_element);
		break;
	case lex_k_SHORT:
		FactorKind = fk_StdProcFunc;
		StdProcFunc = new CShortStdProcFunc(parent_element);
		break;
	case lex_k_SIZE:
		FactorKind = fk_StdProcFunc;
		StdProcFunc = new CSizeStdProcFunc(parent_element);
		break;

	}//switch

	/////////////////////////////////////////////////////////////////
	//������������� � ������ ���������� ����������� ���������-�������
	if (FactorKind == fk_StdProcFunc) {
		int err_num = StdProcFunc->Init(lb);
		if (err_num) return err_num;
		return CalcResultId();
	}

	////////////////////////////////////////////
	//Designator (����� ������� ��� ������ ����)
	if (lex_i == li.lex || lex_k_op_brace == li.lex) {
		RESTORE_POS
		//�������������� �����������
		Designator = new CDesignator(parent_element, in_fact_pars);
		int err_num = Designator->Init(lb);
		if (err_num) return err_num;
		//�������� ������� ����������� ��������� (��������� �������� � ������������� CallStatement)
		if (Designator->IsProcName()) {
			Call = new CCallStatement(parent_element, Designator, false, false);
			err_num = Call->Init(lb);
			if (err_num) return err_num;
		}
		//����� ��������� Designatora
		FactorKind = fk_Designator;
		return CalcResultId();
	}//if

	return s_e_Factor;
}//Init


//-----------------------------------------------------------------------------
//���������� ResultId � ��������� ������
int CFactor::CalcResultId()
{
	switch (FactorKind) {
	case fk_Negation:
		ResultId = Factor->GetResultId();
		if (id_CBooleanVar != ResultId) ResultId = id_CBaseName;
		break;
	case fk_Expr:
		ResultId = Expr->GetResultId();
		break;
	case fk_ConstVar:
		ResultId = ConstVar->name_id;
		break;
	case fk_StdProcFunc:
		ResultId = StdProcFunc->GetResultId();
		break;
	case fk_Designator:
		ResultId = Designator->GetResultId();
		break;
	}
	//�������� ������� ����������� ���� ����������
	return (id_CBaseName == ResultId) ? s_e_ExprCompatibility : 0;
}


//-----------------------------------------------------------------------------
//��������, �������� �� ������, ������������ � CFactor, read-only
bool CFactor::IsReadOnly() const
{
	switch (FactorKind) {
	case fk_Expr:
		return Expr->IsReadOnly();
	case fk_Designator:
		return Designator->IsReadOnly();
	default:
		return true;
	}//switch
}


//-----------------------------------------------------------------------------
//������ ���� CFactor
void CFactor::WriteCPP(CPP_files& f)
{
	switch (FactorKind) {
	case fk_Negation:
		fprintf(f.fc, "!");
		Factor->WriteCPP(f);
		break;
	case fk_Expr:
		fprintf(f.fc, "(");
		Expr->WriteCPP(f);
		fprintf(f.fc, ")");
		break;
	case fk_ConstVar:
		ConstVar->WriteCPP_ConstValue(f);
		break;
	case fk_StdProcFunc:
		StdProcFunc->WriteCPP(f);
		break;
	case fk_Designator:
		//�������� ������� ������ ��������� ������� (���� ��� - ������ ���� �����������)
		if (Call)
			Call->WriteCPP(f);
		else
			Designator->WriteCPP(f);
		break;		
	}//switch
}


//-----------------------------------------------------------------------------
//���������� � ��������� ���������� ��������, � ��������� �������������
int CTermPair::ApplyOperation(CBaseVar *&BaseConst) const
{
	//��� �������� �������������� ��������: (BaseConst) = (BaseConst) <MulOp> (fac)
	CBaseVar* fac;

	//��������� ��������� �� ���������
	int err_num = Factor->CreateConst(fac);
	if (err_num) return err_num;

	//���������������, ��� �������� ������������ ����� ��������� � CTermPair::Init
	switch (MulOp) {

	//��������: *
	case mop_M:
		switch (BaseConst->name_id) {
		case id_CSetVar:
			static_cast<CSetVar*>(BaseConst)->ConstValue &= static_cast<CSetVar*>(fac)->ConstValue;
			break;
		case id_CIntegerVar:
			static_cast<CIntegerVar*>(BaseConst)->ConstValue *= fac->GetIntValue();
			break;
		case id_CLongintVar:
			static_cast<CLongintVar*>(BaseConst)->ConstValue *= fac->GetIntValue();
			break;
		case id_CShortintVar:
			static_cast<CShortintVar*>(BaseConst)->ConstValue *= fac->GetIntValue();
			break;
		case id_CRealVar:
			switch (fac->name_id) {
			case id_CRealVar:
				static_cast<CRealVar*>(BaseConst)->ConstValue *= static_cast<CRealVar*>(fac)->ConstValue;
				break;
			case id_CLongrealVar:
				static_cast<CRealVar*>(BaseConst)->ConstValue *= static_cast<CLongrealVar*>(fac)->ConstValue;
				break;
			default:
				return s_m_Error;
			}
			break;
		case id_CLongrealVar:
			switch (fac->name_id) {
			case id_CRealVar:
				static_cast<CLongrealVar*>(BaseConst)->ConstValue *= static_cast<CRealVar*>(fac)->ConstValue;
				break;
			case id_CLongrealVar:
				static_cast<CLongrealVar*>(BaseConst)->ConstValue *= static_cast<CLongrealVar*>(fac)->ConstValue;
				break;
			default:
				return s_m_Error;
			}
			break;
		default:
			//��� �������� ������ ���� ���������� ����
			return s_m_Error;
		}
		break;

	//��������: /
	case mop_D:
		switch (BaseConst->name_id) {
		case id_CSetVar:
			static_cast<CSetVar*>(BaseConst)->ConstValue ^= static_cast<CSetVar*>(fac)->ConstValue;
			break;
		case id_CIntegerVar:
			if (!fac->GetIntValue()) return s_e_DivisionByZero;
			static_cast<CIntegerVar*>(BaseConst)->ConstValue /= fac->GetIntValue();
			break;
		case id_CLongintVar:
			if (!fac->GetIntValue()) return s_e_DivisionByZero;
			static_cast<CLongintVar*>(BaseConst)->ConstValue /= fac->GetIntValue();
			break;
		case id_CShortintVar:
			if (!fac->GetIntValue()) return s_e_DivisionByZero;
			static_cast<CShortintVar*>(BaseConst)->ConstValue /= fac->GetIntValue();
			break;
		case id_CRealVar:
			switch (fac->name_id) {
			case id_CRealVar:
				if (!static_cast<CRealVar*>(fac)->ConstValue) return s_e_DivisionByZero;
				static_cast<CRealVar*>(BaseConst)->ConstValue /= static_cast<CRealVar*>(fac)->ConstValue;
				break;
			case id_CLongrealVar:
				if (!static_cast<CLongrealVar*>(fac)->ConstValue) return s_e_DivisionByZero;
				static_cast<CRealVar*>(BaseConst)->ConstValue /= static_cast<CLongrealVar*>(fac)->ConstValue;
				break;
			default:
				return s_m_Error;
			}
			break;
		case id_CLongrealVar:
			switch (fac->name_id) {
			case id_CRealVar:
				if (!static_cast<CRealVar*>(fac)->ConstValue) return s_e_DivisionByZero;
				static_cast<CLongrealVar*>(BaseConst)->ConstValue /= static_cast<CRealVar*>(fac)->ConstValue;
				break;
			case id_CLongrealVar:
				if (!static_cast<CLongrealVar*>(fac)->ConstValue) return s_e_DivisionByZero;
				static_cast<CLongrealVar*>(BaseConst)->ConstValue /= static_cast<CLongrealVar*>(fac)->ConstValue;
				break;
			default:
				return s_m_Error;
			}
			break;
		default:
			//��� �������� ������ ���� ���������� ����
			return s_m_Error;
		}
		break;

	//��������: DIV
	case mop_DIV:
		if (!fac->GetIntValue()) return s_e_DivisionByZero;
		switch (BaseConst->name_id) {
		case id_CIntegerVar:
			static_cast<CIntegerVar*>(BaseConst)->ConstValue = DIV(static_cast<CIntegerVar*>(BaseConst)->ConstValue, fac->GetIntValue());
			break;
		case id_CLongintVar:
			static_cast<CLongintVar*>(BaseConst)->ConstValue = DIV(static_cast<CLongintVar*>(BaseConst)->ConstValue, fac->GetIntValue());
			break;
		case id_CShortintVar:
			static_cast<CShortintVar*>(BaseConst)->ConstValue = DIV(static_cast<CShortintVar*>(BaseConst)->ConstValue, fac->GetIntValue());
			break;
		default:
			//��� �������� ������ ���� ���������� ����
			return s_m_Error;
		}
		break;

	//��������: MOD
	case mop_MOD:
		if (!fac->GetIntValue()) return s_e_DivisionByZero;
		switch (BaseConst->name_id) {
		case id_CIntegerVar:
			static_cast<CIntegerVar*>(BaseConst)->ConstValue = MOD(static_cast<CIntegerVar*>(BaseConst)->ConstValue, fac->GetIntValue());
			break;
		case id_CLongintVar:
			static_cast<CLongintVar*>(BaseConst)->ConstValue = MOD(static_cast<CLongintVar*>(BaseConst)->ConstValue, fac->GetIntValue());
			break;
		case id_CShortintVar:
			static_cast<CShortintVar*>(BaseConst)->ConstValue = MOD(static_cast<CShortintVar*>(BaseConst)->ConstValue, fac->GetIntValue());
			break;
		default:
			//��� �������� ������ ���� ���������� ����
			return s_m_Error;
		}
		break;

	//��������: &
	case mop_AND:
		switch (BaseConst->name_id) {
		case id_CBooleanVar:
			if (id_CBooleanVar == fac->name_id)
				static_cast<CBooleanVar*>(BaseConst)->ConstValue = static_cast<CBooleanVar*>(BaseConst)->ConstValue && static_cast<CBooleanVar*>(fac)->ConstValue;
			else
				return s_m_Error;
			break;
		default:
			//��� �������� ������ ���� ���������� ����
			return s_m_Error;
		}
		break;

	//��� �������� ������ ���� ���������� ����
	default:
		return s_m_Error;
	}//switch

	//����������� �������������� ��������
	delete fac;

	return 0;
}


//-----------------------------------------------------------------------------
//������������� ������� TermPair �� ������ ������
int CTermPair::Init(CLexBuf *lb)
{
	//��������� �������� ���������
	Factor = new CFactor(parent_element, ek_Normal, false);
	int err_num = Factor->Init(lb);
	if (err_num) return err_num;

	//��������� ���� ���������� ��������� (��� ��������)
	const EName_id res_id = Factor->GetResultId();

	//�������� ���� ����������
	switch (MulOp) {

	//��������: *, /
	case mop_M:
	case mop_D:
		//�������� ��������� �� ���������� � ������
		if (id_CSetVar == res_id || CBaseVar::IsDigitId(res_id)) {
			ResultId = res_id;
			return 0;
		} else return s_e_ExprCompatibility;

	//��������: DIV, MOD
	case mop_DIV:
	case mop_MOD:
		//�������� ��������� � ����� ������
		if (CBaseVar::IsIntId(res_id)) {
			ResultId = res_id;
			return 0;
		} else return s_e_ExprCompatibility;

	//��������: &
	case mop_AND:
		//�������� ��������� � ��������� ����������
		if (id_CBooleanVar == res_id) {
			ResultId = id_CBooleanVar;
			return 0;
		} else return s_e_ExprCompatibility;
	
	//��� ���������� �������� MulOp ������ ���� ��� ����������
	default:
		return s_m_Error;
	}//switch
}//Init


//-----------------------------------------------------------------------------
//������ ���� CTermPair
void CTermPair::WriteCPP(CPP_files& f)
{
	switch (MulOp) {
	case mop_M:
		fprintf(f.fc, " %c ", (Factor->GetResultId() == id_CSetVar) ? '&' : '*');
		break;
	case mop_D:
		if (Factor->GetResultId() == id_CSetVar)
			fprintf(f.fc, " ^ ");
		else
			fprintf(f.fc, " / double(");	//��� ������� ��� ���������� ������ ���� ��������������
		break;
	case mop_DIV:
		fprintf(f.fc, ") / ");
		break;
	case mop_MOD:
		fprintf(f.fc, " , ");
		break;
	case mop_AND:
		fprintf(f.fc, " && ");
	}

	Factor->WriteCPP(f);

	if (mop_D == MulOp && Factor->GetResultId() != id_CSetVar)
		fprintf(f.fc, ")");
}//WriteCPP


//-----------------------------------------------------------------------------
//����������
CTerm::~CTerm()
{
	if (TermPairStore) {
		CBaseVector::iterator i;
		for(i = TermPairStore->begin(); i != TermPairStore->end(); ++i)
			delete *i;
		delete TermPairStore;
	}
	delete Factor;
}//~CTerm


//-----------------------------------------------------------------------------
//����� ��������� ���������� � ������� ��������� (�� ����������� l-value)
CBaseName* CTerm::FindLastName()
{
	if (TermPairStore) return NULL;
	return Factor->FindLastName();
}


//-----------------------------------------------------------------------------
//������������� ������� Term �� ������ ������
int CTerm::Init(CLexBuf *lb)
{
	//�������� ������� ���������
	Factor = new CFactor(parent_element, ExprKind, in_fact_pars);
	int err_num = Factor->Init(lb);
	if (err_num) return err_num;

	//��������� ���������������� ���� ���������� ���������
	ResultId = Factor->GetResultId();

	//�������� ���������� ���� ���������
	if (ek_Char == ExprKind || ek_ProcedureVar == ExprKind) return 0;

	DECL_SAVE_POS
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//��� �������� ��������� MulOp
	EMulOp MulOp = mop_NULL;

	//���� ������ ����������
	while (true) {
		//�������� ������� MulOp
		if (!lb->ReadLex(li) || (lex_k_dot > li.lex)) {
			RESTORE_POS
			return 0;
		}

		//�������� ���� ��������
		switch (li.lex) {
		case lex_k_asterix:
			MulOp = mop_M;
			break;
		case lex_k_slash:
			MulOp = mop_D;
			break;
		case lex_k_DIV:
			MulOp = mop_DIV;
			break;
		case lex_k_MOD:
			MulOp = mop_MOD;
			break;
		case lex_k_and:
			MulOp = mop_AND;
			break;
		default:
			RESTORE_POS
			return 0;
		}

		//�������� ������ ����������
		if (!TermPairStore) TermPairStore = new CBaseVector;
		//�������� ������ ���������
		CTermPair *TP = new CTermPair(parent_element);
		TP->MulOp = MulOp;
		err_num = TP->Init(lb);
		if (err_num) return err_num;
		//��������� ���������� ��������� � ������
		TermPairStore->push_back(TP);

		//���������� �������� ���� ���������� ��������� (� ������ ������ ����������)
		if (id_CBooleanVar == ResultId && TP->GetResultId() != ResultId) return s_e_ExprCompatibility;
		else {
			//��������� ���. ����, ������������ ��� ��������
			ResultId = GetMaxDigitId(ResultId, TP->GetResultId());
			if (id_CBaseName == ResultId) return s_e_ExprCompatibility;
			//���������� ��������� ���� (� ����������� �� ��������)
			switch (MulOp) {
			//��� �������� '/' ���� ������� ���������� ������������ ���, ����������� ��� �������� (���� ��� �� �������� ��� �����������)
			case mop_D:
				if (id_CLongrealVar != ResultId && id_CSetVar != ResultId) ResultId = id_CRealVar;
				break;
			//��� �������� DIV � MOD ���������� ��������� ������� ������ ����
			case mop_DIV:
			case mop_MOD:
				if (!CBaseVar::IsIntId(ResultId)) return s_e_ExprCompatibility;
				break;
			}//switch
		}//else
	
		SAVE_POS
	}//while true

	return 0;
}//Init


//-----------------------------------------------------------------------------
//��������, �������� �� ������, ������������ � CTerm, read-only
bool CTerm::IsReadOnly() const
{
	if (TermPairStore) return true;
	return Factor->IsReadOnly();
}


//-----------------------------------------------------------------------------
//������ ���� CTerm
void CTerm::WriteCPP(CPP_files& f)
{
	//������ ������ ��������� (� �������� �������) ��� �������� DIV, MOD, *
	if (TermPairStore) {
		CBaseVector::reverse_iterator ri;
		for (ri = TermPairStore->rbegin(); ri != TermPairStore->rend(); ri++) {
			switch (static_cast<CTermPair*>(*ri)->MulOp) {
			case mop_DIV:
				fprintf(f.fc, "long(floor(double(");
				break;
			case mop_MOD:
				fprintf(f.fc, "MOD(");
				break;
			case mop_M:
				fprintf(f.fc, "(");
				break;
			}//switch
		}//for
	}

	//������ ������� ���������
	Factor->WriteCPP(f);

	//������ ���������� ���������� (���� ����)
	if (TermPairStore) {
		CBaseVector::const_iterator ci;
		for (ci = TermPairStore->begin(); ci != TermPairStore->end(); ++ci) {
			//������ �������� ����� �������� � ������� �������� ��� ������� ��������
			static_cast<CTermPair*>(*ci)->CTermPair::WriteCPP(f);
			//��� ������������� ���������� ���������� ����������� ��-�� ��� �������� DIV, MOD, *
			switch (static_cast<CTermPair*>(*ci)->MulOp) {
			case mop_DIV:
				fprintf(f.fc, "))");
				break;
			case mop_MOD:
			case mop_M:
				fprintf(f.fc, ")");
				break;
			}//switch
		}//for
	}
}//WriteCPP


//-----------------------------------------------------------------------------
//���������� � ��������� ���������� ��������, � ��������� �������������
int CSimpleExprPair::ApplyOperation(CBaseVar *&BaseConst) const
{
	//��� �������� �������������� ��������: (BaseConst) = (BaseConst) <AddOp> (fac)
	CBaseVar* fac;

	//��������� ��������� �� "����������"
	int err_num = Term->CreateConst(fac);
	if (err_num) return err_num;

	//���������������, ��� �������� ������������ ����� ��������� � CTermPair::Init
	switch (AddOp) {

	//��������: +
	case aop_ADD:
		switch (BaseConst->name_id) {
		case id_CSetVar:
			static_cast<CSetVar*>(BaseConst)->ConstValue |= static_cast<CSetVar*>(fac)->ConstValue;
			break;
		case id_CIntegerVar:
			static_cast<CIntegerVar*>(BaseConst)->ConstValue += fac->GetIntValue();
			break;
		case id_CLongintVar:
			static_cast<CLongintVar*>(BaseConst)->ConstValue += fac->GetIntValue();
			break;
		case id_CShortintVar:
			static_cast<CShortintVar*>(BaseConst)->ConstValue += fac->GetIntValue();
			break;
		case id_CRealVar:
			switch (fac->name_id) {
			case id_CRealVar:
				static_cast<CRealVar*>(BaseConst)->ConstValue += static_cast<CRealVar*>(fac)->ConstValue;
				break;
			case id_CLongrealVar:
				static_cast<CRealVar*>(BaseConst)->ConstValue += static_cast<CLongrealVar*>(fac)->ConstValue;
				break;
			default:
				return s_m_Error;
			}
			break;
		case id_CLongrealVar:
			switch (fac->name_id) {
			case id_CRealVar:
				static_cast<CLongrealVar*>(BaseConst)->ConstValue += static_cast<CRealVar*>(fac)->ConstValue;
				break;
			case id_CLongrealVar:
				static_cast<CLongrealVar*>(BaseConst)->ConstValue += static_cast<CLongrealVar*>(fac)->ConstValue;
				break;
			default:
				return s_m_Error;
			}
			break;
		default:
			//��� �������� ������ ���� ���������� ����
			return s_m_Error;
		}
		break;

	//��������: -
	case aop_SUB:
		switch (BaseConst->name_id) {
		case id_CSetVar:
			static_cast<CSetVar*>(BaseConst)->ConstValue &= ~static_cast<CSetVar*>(fac)->ConstValue;
			break;
		case id_CIntegerVar:
			static_cast<CIntegerVar*>(BaseConst)->ConstValue -= fac->GetIntValue();
			break;
		case id_CLongintVar:
			static_cast<CLongintVar*>(BaseConst)->ConstValue -= fac->GetIntValue();
			break;
		case id_CShortintVar:
			static_cast<CShortintVar*>(BaseConst)->ConstValue -= fac->GetIntValue();
			break;
		case id_CRealVar:
			switch (fac->name_id) {
			case id_CRealVar:
				static_cast<CRealVar*>(BaseConst)->ConstValue -= static_cast<CRealVar*>(fac)->ConstValue;
				break;
			case id_CLongrealVar:
				static_cast<CRealVar*>(BaseConst)->ConstValue -= static_cast<CLongrealVar*>(fac)->ConstValue;
				break;
			default:
				return s_m_Error;
			}
			break;
		case id_CLongrealVar:
			switch (fac->name_id) {
			case id_CRealVar:
				static_cast<CLongrealVar*>(BaseConst)->ConstValue -= static_cast<CRealVar*>(fac)->ConstValue;
				break;
			case id_CLongrealVar:
				static_cast<CLongrealVar*>(BaseConst)->ConstValue -= static_cast<CLongrealVar*>(fac)->ConstValue;
				break;
			default:
				return s_m_Error;
			}
			break;
		default:
			//��� �������� ������ ���� ���������� ����
			return s_m_Error;
		}
		break;

	//��������: OR
	case aop_OR:
		switch (BaseConst->name_id) {
		case id_CBooleanVar:
			if (id_CBooleanVar == fac->name_id)
				static_cast<CBooleanVar*>(BaseConst)->ConstValue = static_cast<CBooleanVar*>(BaseConst)->ConstValue || static_cast<CBooleanVar*>(fac)->ConstValue;
			else
				return s_m_Error;
			break;
		default:
			//��� �������� ������ ���� ���������� ����
			return s_m_Error;
		}
		break;

	//��� �������� ������ ���� ���������� ����
	default:
		return s_m_Error;
	}//switch

	//����������� �������������� ��������
	delete fac;

	return 0;
}


//-----------------------------------------------------------------------------
//������������� ������� SimpleExprPair �� ������ ������
int CSimpleExprPair::Init(CLexBuf *lb)
{
	//��������� �������� "����������"
	Term = new CTerm(parent_element, ek_Normal, false);
	int err_num = Term->Init(lb);
	if (err_num) return err_num;

	//��������� ���� ���������� "����������" (��� ��������)
	const EName_id res_id = Term->GetResultId();

	//�������� ���� ����������
	switch (AddOp) {

	//��������: +, -
	case aop_ADD:
	case aop_SUB:
		//�������� ��������� �� ���������� � ������
		if (id_CSetVar == res_id || CBaseVar::IsDigitId(res_id)) {
			ResultId = res_id;
			return 0;
		} else return s_e_ExprCompatibility;

	//��������: OR
	case aop_OR:
		//�������� ��������� � ��������� ����������
		if (id_CBooleanVar == res_id) {
			ResultId = id_CBooleanVar;
			return 0;
		} else return s_e_ExprCompatibility;

	//��� ���������� �������� AddOp ������ ���� ��� ����������
	default:
		return s_m_Error;
	}//switch
}//Init


//-----------------------------------------------------------------------------
//������ ���� CSimpleExprPair
void CSimpleExprPair::WriteCPP(CPP_files& f)
{
	bool need_bracket = false;

	switch (AddOp) {
	case aop_ADD:
		fprintf(f.fc, " %c ", (Term->GetResultId() == id_CSetVar) ? '|' : '+');
		break;
	case aop_SUB:
		need_bracket = Term->GetResultId() == id_CSetVar;
		fprintf(f.fc, " %s", need_bracket ? "& ~(" : "- ");
		break;
	case aop_OR:
		fprintf(f.fc, " || ");
	}

	Term->WriteCPP(f);

	if (need_bracket) fprintf(f.fc, ")");
}//WriteCPP


//-----------------------------------------------------------------------------
//����������
CSimpleExpr::~CSimpleExpr()
{
	if (SimpleExprPairStore) {
		CBaseVector::iterator i;
		for(i = SimpleExprPairStore->begin(); i != SimpleExprPairStore->end(); ++i)
			delete *i;
		delete SimpleExprPairStore;
	}
	delete Term;
}//~CSimpleExpr


//-----------------------------------------------------------------------------
//����� ��������� ���������� � ������� ��������� (l-value ��� ���������)
CBaseName* CSimpleExpr::FindLastName()
{
	if (SimpleExprPairStore) return NULL;
	return Term->FindLastName();
}


//-----------------------------------------------------------------------------
//������������� ������� SimpleExpr �� ������ ������
int CSimpleExpr::Init(CLexBuf *lb)
{
	DECL_SAVE_POS
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� '+' ��� '-'
	if (!lb->ReadLex(li) || (lex_k_plus != li.lex && lex_k_minus != li.lex)) {
		RESTORE_POS
	} else {
		SAVE_POS
		negative = lex_k_minus == li.lex;
		unary = true;
	}

	//������������� ������� Terma (����������)
	Term = new CTerm(parent_element, ExprKind, in_fact_pars);
	int err_num = Term->Init(lb);
	if (err_num) return err_num;

	//��������� ���������������� ���� ���������� ���������
	ResultId = Term->GetResultId();

	//�������� ������������� ��������� ��� ������� �������� '+' ��� '-'
	if (unary && !(CBaseVar::IsDigitId(ResultId) || (negative && id_CSetVar == ResultId))) {
		RESTORE_POS
		return s_e_ExprCompatibility;
	}

	//�������� ���������� ���� ���������
	if (ek_Char == ExprKind || ek_ProcedureVar == ExprKind) return 0;

	SAVE_POS

	//��� �������� �������� AddOp
	EAddOp AddOp = aop_NULL;

	//���� ������ ���������
	while (true) {
		//�������� ������� AddOp
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) {
			RESTORE_POS
			return 0;
		}

		//�������� ���� ��������
		switch (li.lex) {
		case lex_k_plus:
			AddOp = aop_ADD;
			break;
		case lex_k_minus:
			AddOp = aop_SUB;
			break;
		case lex_k_OR:
			AddOp = aop_OR;
			break;
		default:
			RESTORE_POS
			return 0;
		}

		//�������� ������ ��������� (���� ��� �� ������)
		if (!SimpleExprPairStore) SimpleExprPairStore = new CBaseVector;
		//�������� ������ ����������
		CSimpleExprPair *SEP = new CSimpleExprPair(parent_element);
		SEP->AddOp = AddOp;
		err_num = SEP->Init(lb);
		if (err_num) return err_num;
		//��������� ���������� ���������� � ������
		SimpleExprPairStore->push_back(SEP);

		//���������� �������� ���� ���������� ��������� (� ������ ������ ����������)
		if ((id_CSetVar == ResultId || id_CBooleanVar == ResultId) && SEP->GetResultId() != ResultId)
			return s_e_ExprCompatibility;
		else {
			ResultId = GetMaxDigitId(ResultId, SEP->GetResultId());
			if (id_CBaseName == ResultId) return s_e_ExprCompatibility;
		}//else
	
		SAVE_POS
	}//while true

	return 0;
}//Init


//-----------------------------------------------------------------------------
//��������, �������� �� ������, ������������ � CSimpleExpr, read-only
bool CSimpleExpr::IsReadOnly() const
{
	if (SimpleExprPairStore) return true;
	return Term->IsReadOnly();
}


//-----------------------------------------------------------------------------
//������ ���� CSimpleExpr
void CSimpleExpr::WriteCPP(CPP_files& f)
{
	//�������� ������������� ��������� ��� ���������� (���������)
	if (negative) fprintf(f.fc, "%c", (Term->GetResultId() == id_CSetVar) ? '~' : '-');

	Term->WriteCPP(f);

	if (SimpleExprPairStore) {
		CBaseVector::const_iterator ci;
		for (ci = SimpleExprPairStore->begin(); ci != SimpleExprPairStore->end(); ++ci)
			(*ci)->WriteCPP(f);
	}
}//WriteCPP


//-----------------------------------------------------------------------------
//����������
CExpr::~CExpr()
{
	delete IS_qual;
	delete SimpleExpr1;
	delete SimpleExpr2;
}//~CExpr


//-----------------------------------------------------------------------------
//�������� ���������
int CExpr::CreateConst(CBaseVar* &BaseConst)
{
	//�������� ������� ������������� �������� IS (����������� ������ � ���������� � ����������)
	if (rel_IS == Relation) return s_e_ExprNotConst;

	//���������� ��� �������� ������������� ��������
	CBaseVar* Const1 = NULL;
	CBaseVar* Const2 = NULL;

	//�������� ��������� �� SimpleExpr1
	int err_num = SimpleExpr1->CreateConst(Const1);
	if (err_num) return err_num;

	/**/
	//����� ������������ ����, ������������ ��� �������� ���������� ���������
	//���������� ������
	switch (Const1->name_id) {
	case id_CLongintVar:
		long li = static_cast<CLongintVar*>(Const1)->ConstValue;
		if (SHRT_MAX >= li) {
			delete Const1;
			Const1 = new CShortintVar(parent_element);
			static_cast<CShortintVar*>(Const1)->ConstValue = li;
			Const1->is_const = true;
		}
		break;
	}

	//��������� ���������� ������� ��������� (���������� ������������� ��������)
	if (!SimpleExpr2) {
		BaseConst = Const1;	//���������� ��������� �� ������� ���������
		return 0;
	}

	//�������� ��������� �� SimpleExpr2
	err_num = SimpleExpr2->CreateConst(Const2);
	if (err_num) {
		delete Const1;
		return err_num;
	}

	//�������� ����������� ���������� ��� �������� ���������� ��������
	BaseConst = new CBooleanVar(parent_element);
	BaseConst->is_const = true;

	//�������� ������� �������������� �������� � ���������-��������� 
	//(����� �������� � ����������� ������)
	if (CBaseVar::IsIntId(Const1->name_id) && id_CSetVar == Const2->name_id) {
		const long val = Const1->GetIntValue();
		//�������� ����������� �������� ���������
		if (SET_MAX < val || 0 > val) {
			//��� ��������� ������ ������� ��� ��������� �������
			delete Const1;
			delete Const2;
			delete BaseConst;
			BaseConst = NULL;
			//��������� �� ���� �������
			return s_e_SetElemRange;
		}
		//�������� ����������� ���� �������� (������-�� ������ ���� ��� ���������)
		if (rel_IN == Relation) {
			static_cast<CBooleanVar*>(BaseConst)->ConstValue = (1 << val & static_cast<CSetVar*>(Const2)->ConstValue) != 0;
			return 0;
		} else
			return s_m_Error;
	}

	//���������� �������� ���������
	static_cast<CBooleanVar*>(BaseConst)->ConstValue = ApplyOperation(Const1, Const2);

	return 0;
}


//-----------------------------------------------------------------------------
//������������� ������� Expr �� ������ ������
int CExpr::Init(CLexBuf *lb)
{
	//������������� ������� SimpleExpr ���������
	SimpleExpr1 = new CSimpleExpr(parent_element, ExprKind, in_fact_pars);
	int err_num = SimpleExpr1->Init(lb);
	if (err_num) return err_num;

	//�������� ���������� ���� ���������
	if (ek_Char == ExprKind || ek_ProcedureVar == ExprKind) return CalcResultId();

	DECL_SAVE_POS
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//������ ��������� ������� (��������� �����)
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) {
		RESTORE_POS
		return CalcResultId();
	}

	//�������� ���� ��������
	switch (li.lex) {
	case lex_k_eq:
		Relation = rel_EQ;
		break;
	case lex_k_ne:
		Relation = rel_NE;
		break;
	case lex_k_lt:
		Relation = rel_LT;
		break;
	case lex_k_le:
		Relation = rel_LE;
		break;
	case lex_k_gt:
		Relation = rel_GT;
		break;
	case lex_k_ge:
		Relation = rel_GE;
		break;
	case lex_k_IN:
		Relation = rel_IN;
		break;
	case lex_k_IS: {
			//� ������ �������� IS ������ ��������� SimpleExpr2 �������� ��������������� �����. �����.
			Relation = rel_IS;
			CBaseName* BN = SimpleExpr1->FindLastName();
			if (!BN) return s_e_OperandNotVariable;
			//�������� ���� ���������� ��� ���. ���������
			switch(BN->name_id) {
			case id_CCommonVar:
				//�������� ������������ ���������� ������ ����������� (��������-����������)
				if (!static_cast<CCommonVar*>(BN)->is_var) return s_e_GuardVarNotRecOrP;
				if (!static_cast<CCommonVar*>(BN)->IsPureCommon()) return s_e_GuardVarNotRecOrP;
				//��������� ���� ���������� ����������
				BN = static_cast<CCommonVar*>(BN)->FindType();
				break;
			case id_CPointerVar:
				BN = static_cast<CPointerVar*>(BN)->FindType();
			}
			//���������� � ��������� �����. �����-��
			IS_qual = new CQualident;
			//�������� ������ � ���������� ��� � �������
			const char *m_n;
			const char *n;
			switch (BN->name_id) {
			case id_CCommonType:
				//�������� ������� "<" - ������ �������� �������������
				if (!lb->ReadLex(li) || lex_k_lt != li.lex) return s_e_OpAngleMissing;
				//������������� �������� ��������
				err_num = IS_qual->Init(lb, parent_element);
				if (err_num) return err_num;
				//��������, �������� �� ������� ���������� ��� ������� ���������
				if (!static_cast<CCommonType*>(BN)->FindSpec(IS_qual->pref_ident, IS_qual->ident, IS_qual->ident)) return s_e_SpecTypeTag;
				//�������� ������� ">" - ����� �������� �������������
				if (!lb->ReadLex(li) || lex_k_gt != li.lex) return s_e_ClAngleMissing;
				break;
			case id_CRecordVar:
				//���������� �������� ���� ��� �������� ������� ���������� ����
				m_n = static_cast<CRecordVar*>(BN)->GetTypeModuleName();
				n = static_cast<CRecordVar*>(BN)->GetTypeName();
				goto both_kinds;
			case id_CRecordType: {
				//���������� �������� ���� ��� �������� ������� ���������� ����
				m_n = static_cast<CRecordType*>(BN)->GetModuleName();
				n = static_cast<CRecordType*>(BN)->name;
			both_kinds:
				//������������� �������� ����� ����
				err_num = IS_qual->Init(lb, parent_element);
				if (err_num) return err_num;
				//��������� ������� ���������� �������������� ���� (����� IS �����������)
				if (!n) return s_e_GuardTypeNotExt;
				//��������� ���� (� ��������� ��������� ������ ����)
				BN = parent_element->GetGlobalName(IS_qual->pref_ident, IS_qual->ident);
				if (!BN) return s_e_UndeclaredIdent;
				if (id_CPointerType == BN->name_id) BN = static_cast<CPointerType*>(BN)->FindType();
				if (id_CRecordType != BN->name_id) return s_e_IdentNotRecordType;
				if (!static_cast<CRecordType*>(BN)->IsExtension(m_n, n)) return s_e_GuardTypeNotExt;
				break;
				}
			default:
				//������ ������� ������������� ����
				return s_e_GuardVarNotRecOrP;
			}
			//����� ��������� �������� IS
			ResultId = id_CBooleanVar;
			return 0;
		}//case
	default:
		//����������� ��������, ����������� � CExpr
		RESTORE_POS
		return CalcResultId();
	}

	//���������� ���������� ��������, ������������ �� � ������ ���������
	//������������� ���������

	//�������� ��������� ������������� ���������
	if (id_CCharVar == SimpleExpr1->GetResultId())
		SimpleExpr2 = new CSimpleExpr(parent_element, ek_Char, false);
	else
		SimpleExpr2 = new CSimpleExpr(parent_element, ek_Normal, false);
	//������������� ������� SimpleExpr
	err_num = SimpleExpr2->Init(lb);
	if (err_num) return err_num;
	//� ������, ���� SimpleExpr2 ����������, ��������������� ������� �������������
	//SimpleExpr1 �� ������ � ������ (������ ���� � ��� ������)
	if (id_CCharVar == SimpleExpr2->GetResultId()) {
		//����� ������ SimpleExpr1 :(
		CArrayVar* AV = static_cast<CArrayVar*>(SimpleExpr1->FindLastName());
		if (AV && id_CArrayVar == AV->name_id && AV->ConstString && strlen(AV->ConstString) == 1) {
			//���������� ����������� �������������� ������ � ������, ��� � ������
			AV->ConstStringIsChar = true;
			//������������� ResultId (��� ���� CHAR�� ��������� ������ �������� ���������)
			switch (Relation) {
			case rel_EQ:
			case rel_NE:
			case rel_LT:
			case rel_LE:
			case rel_GT:
			case rel_GE:
				ResultId = id_CBooleanVar;
				return 0;
			default:
				return s_e_ExprCompatibility;
			}//switch
		}
	}

	//�������� ������������� ������� ���������
	return CalcResultId();
}//Init


//-----------------------------------------------------------------------------
//��������, �������� �� ������, ������������ � CExpr, read-only
bool CExpr::IsReadOnly() const
{
	if (SimpleExpr2) return true;
	return SimpleExpr1->IsReadOnly();
}


//-----------------------------------------------------------------------------
//������ ���� CExpr
void CExpr::WriteCPP(CPP_files& f)
{
	//������ �������� (���� ����)
	switch (Relation) {
	case rel_IN:			//�������� "IN"
		fprintf(f.fc, "(1 << (");
		SimpleExpr1->WriteCPP(f);
		fprintf(f.fc, ") & (");
		SimpleExpr2->WriteCPP(f);
		fprintf(f.fc, "))");
		return;
	case rel_IS:			//�������� "IS"
		//��������� ������� ��������
		CBaseName* BN = SimpleExpr1->FindLastName();
		if (id_CPointerVar == BN->name_id) BN = static_cast<CPointerVar*>(BN)->FindType();
		//�������� ������ � ����������
		if (id_CCommonVar == BN->name_id || id_CCommonType == BN->name_id) {
			//��������� ���� � ������ ���������
			SimpleExpr1->WriteCPP(f);
			fprintf(f.fc, "->O2M_SID == ");
			SimpleExpr1->WriteCPP(f);
			fprintf(f.fc, "->O2M_SID_");
			if (id_CCommonVar == BN->name_id) BN = static_cast<CCommonVar*>(BN)->FindType();
			const CCommonType::SSpec* spec = static_cast<CCommonType*>(BN)->FindSpec(IS_qual->pref_ident, IS_qual->ident, IS_qual->ident);
			if (!spec) throw error_Internal("CExpr::WriteCPP");
			if (spec->Tag)
				fprintf(f.fc, "%s", spec->Tag);
			else
				fprintf(f.fc, "%s_%s", spec->QualName ? spec->QualName : parent_element->GetParentModule()->name, spec->Name);
		} else {
			//��������� ���� � ������ ������ (��. �� ������)
			fprintf(f.fc, "!strcmp(");
			SimpleExpr1->WriteCPP(f);
			fprintf(f.fc, "->O2M_SYS_ID(), \"");
			CBaseName* BN = parent_element->GetGlobalName(IS_qual->pref_ident, IS_qual->ident);
			if (id_CPointerType == BN->name_id) BN = static_cast<CPointerType*>(BN)->FindType();
			fprintf(f.fc, "%s\")", static_cast<CRecordType*>(BN)->GetRuntimeId());
		}
		return;
	}//switch

	//������ 1-�� �������� ���������
	SimpleExpr1->WriteCPP(f);

	//������ ����� �������� (���� ����)
	switch (Relation) {
	case rel_EQ: 
		fprintf(f.fc, " == ");
		break;
	case rel_NE: 
		fprintf(f.fc, " != ");
		break;
	case rel_LT: 
		fprintf(f.fc, " < ");
		break;
	case rel_LE: 
		fprintf(f.fc, " <= ");
		break;
	case rel_GT: 
		fprintf(f.fc, " > ");
		break;
	case rel_GE: 
		fprintf(f.fc, " >= ");
		break;
	}//switch

	//������ 2-�� �������� ��������� (���� ����)
	if (SimpleExpr2) SimpleExpr2->WriteCPP(f);
}//WriteCPP


//-----------------------------------------------------------------------------
//����� ��������� ���������� � ������� ��������� (l-value ��� ���������)
CBaseName* CExpr::FindLastName()
{
	if (SimpleExpr2) return NULL;
	return SimpleExpr1->FindLastName();
}


//-----------------------------------------------------------------------------
//����� ��������� ���������� (l-value) � ������� ��������� (��� �������� read-only)
CBaseName* CExpr::FindLValue()
{
	if (SimpleExpr2) return NULL;
	//�������� ������� ��������� ��� l-value
	CBaseName* BN = SimpleExpr1->FindLastName();
	//�������� ������� ������� � ���������� �������� ���������, ���� �� �����������,
	//������ �����������, �.�. ������ ��������� ����� ���� ��������������
	if (!BN || (CBaseVar::IsVarId(BN->name_id) && static_cast<CBaseVar*>(BN)->is_const)) return NULL;
	//������� ���������� l-value
	return BN;
}


//-----------------------------------------------------------------------------
//���������� �������� ������� �������� ��� ��������� ����������� ����������,
//���������������, ��� ������������ �������� � ������������� ���������� ��� ���������
bool CExpr::ApplyOperation(const CBaseVar* const c1, const CBaseVar* const c2) const
{
	//������������� �������� (��� ���������)
	const EName_id id1 = c1->name_id;
	const EName_id id2 = c2->name_id;

	//����������: �������� ������� �������������� �������� � ���������-���������
	//���������� �� ������ ������ �������, �.�. ����� �������� � ����������� ������
	//(������ ������������� ��������� �� ������� ���������� ������ ���������)

	//�������� ������� ���� ������������� ���������
	if (CBaseVar::IsIntId(id1) && CBaseVar::IsIntId(id2))
		switch (Relation) {
		case rel_EQ: return c1->GetIntValue() == c2->GetIntValue();
		case rel_NE: return c1->GetIntValue() != c2->GetIntValue();
		case rel_LT: return c1->GetIntValue() < c2->GetIntValue();
		case rel_LE: return c1->GetIntValue() <= c2->GetIntValue();
		case rel_GT: return c1->GetIntValue() > c2->GetIntValue();
		case rel_GE: return c1->GetIntValue() >= c2->GetIntValue();
		default:;
		}

	//�������� ������� ���� �������� ���������
	if (CBaseVar::IsDigitId(id1) && CBaseVar::IsDigitId(id2)) {
		//��������� ������� ��������� �������� (������� long double ������ �������)
		long double val1;
		switch (id1) {
		case id_CShortintVar:
		case id_CIntegerVar:
		case id_CLongintVar:
			val1 = c1->GetIntValue();
			break;
		case id_CRealVar:
			val1 = static_cast<const CRealVar*>(c1)->ConstValue;
			break;
		case id_CLongrealVar:
			val1 = static_cast<const CLongrealVar*>(c1)->ConstValue;
			break;
		default:
			val1 = 0;
		}
		//��������� ������� ��������� �������� (������� long double ������ �������)
		long double val2;
		switch (id2) {
		case id_CShortintVar:
		case id_CIntegerVar:
		case id_CLongintVar:
			val2 = c2->GetIntValue();
			break;
		case id_CRealVar:
			val2 = static_cast<const CRealVar*>(c2)->ConstValue;
			break;
		case id_CLongrealVar:
			val2 = static_cast<const CLongrealVar*>(c2)->ConstValue;
			break;
		default:
			val2 = 0;
		}
		//���������� �������� ��� ����������� ����������
		switch (Relation) {
		case rel_EQ: return val1 == val2;
		case rel_NE: return val1 != val2;
		case rel_LT: return val1 < val2;
		case rel_LE: return val1 <= val2;
		case rel_GT: return val1 > val2;
		case rel_GE: return val1 >= val2;
		default:;
		}
	}

	//�������� ������� ���� ���������� ���������
	if (id_CCharVar == id1 && id_CCharVar == id2) {
		switch (Relation) {
		case rel_EQ: return static_cast<const CCharVar*>(c1) == static_cast<const CCharVar*>(c2);
		case rel_NE: return static_cast<const CCharVar*>(c1) != static_cast<const CCharVar*>(c2);
		case rel_LT: return static_cast<const CCharVar*>(c1) < static_cast<const CCharVar*>(c2);
		case rel_LE: return static_cast<const CCharVar*>(c1) <= static_cast<const CCharVar*>(c2);
		case rel_GT: return static_cast<const CCharVar*>(c1) > static_cast<const CCharVar*>(c2);
		case rel_GE: return static_cast<const CCharVar*>(c1) >= static_cast<const CCharVar*>(c2);
		default:;
		}
	}

	//�������� ������� ���� ��������� ��������
	if (id_CArrayVar == id1 && id_CArrayVar == id2) {
		//������������������ ��������� ���� �����
		const int cmp = strcmp(static_cast<const CArrayVar*>(c1)->ConstString, static_cast<const CArrayVar*>(c2)->ConstString);
		//������������� ����������� �������� � ����������� �� ��������
		switch (Relation) {
		case rel_EQ: return !cmp;
		case rel_NE: return cmp != 0;
		case rel_LT: return cmp < 0;
		case rel_LE: return cmp <= 0;
		case rel_GT: return cmp > 0;
		case rel_GE: return cmp >= 0;
		default:;
		}
	}

	//�������� ������� ���� ��������� ��������
	if (id_CBooleanVar == id1 && id_CBooleanVar == id2) {
		switch (Relation) {
		case rel_EQ: return static_cast<const CBooleanVar*>(c1)->ConstValue == static_cast<const CBooleanVar*>(c2)->ConstValue;
		case rel_NE: return static_cast<const CBooleanVar*>(c1)->ConstValue != static_cast<const CBooleanVar*>(c2)->ConstValue;
		default:;
		}
	}

	//�������� ������� ���� ��������-��������
	if (id_CSetVar == id1 && id_CSetVar == id2) {
		switch (Relation) {
		case rel_EQ: return static_cast<const CSetVar*>(c1)->ConstValue == static_cast<const CSetVar*>(c2)->ConstValue;
		case rel_NE: return static_cast<const CSetVar*>(c1)->ConstValue != static_cast<const CSetVar*>(c2)->ConstValue;
		default:;
		}
	}

	//�������� ������� ���� NIL
	if (id_CPointerVar == id1 && id_CPointerVar == id2) {
		switch (Relation) {
		case rel_EQ: return true;
		case rel_NE: return false;
		default:;
		}
	}

	//��� ��������� ��������� ��������� � �������� ������ ���� ��� ����������
	throw error_Internal("CExpr::ApplyOperation");
}


//-----------------------------------------------------------------------------
//���������� ResultId � ��������� ������
int CExpr::CalcResultId()
{
	//�������� ��������� ��� ������� �������� (��� ��������)
	const EName_id res_id1 = SimpleExpr1->GetResultId();

	//�������� ������ �������� (����� id ������������� ����� ��������)
	if (rel_NULL == Relation) {
		ResultId = res_id1;
		return 0;
	}

	//�������� ��������� ��� ������� �������� (��� ��������),
	//���������������, ��� ������� SimpleExpr2 ��������� � CExpr::Init
	const EName_id res_id2 = SimpleExpr2->GetResultId();

	//��� ������� ���� ��������� ����������� ��������� ������ ����� BOOLEAN
	ResultId = id_CBooleanVar;

	//�������� ���� ��������� � ���������
	switch (Relation) {

	//��������: =, # (����� �������� ����������� �� ����. case)
	case rel_EQ:
	case rel_NE:
		switch (res_id1) {
		case id_CBooleanVar:
		case id_CSetVar:
			if (res_id2 != res_id1) return s_e_ExprCompatibility;
			return 0;
		case id_CPointerVar:
			/**/ //����� ����� ������� ��������
			return 0;
		/**/ //�� ���� ������ ���� ���������� id_CProcedureType ������ ���� id_CProcedureVar
		case id_CProcedureType:
			if (res_id2 != id_CProcedureType) return s_e_ExprCompatibility;
			return 0;
		}

	//��������: <, <=, >, >= (���� ���������� �������� �������� =, #)
	case rel_LT:
	case rel_LE:
	case rel_GT:
	case rel_GE:
		switch (res_id1) {
		case id_CCharVar:
		case id_CArrayVar:
			if (res_id2 != res_id1) return s_e_ExprCompatibility;
			return 0;
		}
		if (CBaseVar::IsDigitId(res_id1)) {
			if (CBaseVar::IsDigitId(res_id2)) {
				return 0;
			}
			return s_e_ExprCompatibility;
		}
		//�� ���� �� ��������� �� ��������
		return s_e_ExprCompatibility;

	//��������: IN
	case rel_IN:
		if (!CBaseVar::IsIntId(res_id1) || res_id2 != id_CSetVar) return s_e_ExprCompatibility;
		return 0;

	//��� ��������� �������� Relation ������ ���� ��� ����������
	default:
		return s_m_Error;
	}//switch
}


//-----------------------------------------------------------------------------
//����������
CDesignator::~CDesignator()
{
	delete Qualident;
	//������� ������ ��-��� �����������
	SDesElemStore::iterator i;
	for (i = DesElemStore.begin(); i != DesElemStore.end(); i++) {
		delete (*i)->ExprList;
		delete (*i)->Qualident;
		delete (*i)->ident;
		delete (*i)->CPPCompoundName;
		delete *i;
	}//for
}


//-----------------------------------------------------------------------------
//�������� ������ ��-�� ����������� � ������������� ��� ����
CDesignator::SDesElem* CDesignator::CreateSDesElem(const EDesKind DKind)
{
	SDesElem* DesElem = new SDesElem;
	DesElem->DesKind = DKind;
	DesElem->ExprList = NULL;
	DesElem->ident = NULL;
	DesElem->Qualident = NULL;
	DesElem->CPPCompoundName = NULL;
	return DesElem;	
}


//-----------------------------------------------------------------------------
//����� ���������� ����� � ������� ��-��� �����������
//��� ����� ���� ����������, ���������, ��� (� ������ �������)
CBaseName* CDesignator::FindLastName()
{
	//����� ������� ����� � ������� (������ ��������������, ��������� � Init)
	CBaseName* BN = parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident);

	//������� ������ ��-��� ����������� (���� ����) � ��������� �� ��������� �������� ����
	SDesElemStore::const_iterator ci;
	for (ci = DesElemStore.begin(); ci != DesElemStore.end(); ci++)
		switch ((*ci)->DesKind) {
		case dk_Record:
		case dk_Pointer:
			//����� ����� � ��������� ������� ���� (��� ������ ��������������, ��������� � Init)
			BN = BN->FindName((*ci)->ident);
			break;
		case dk_Array:
			{
				//��������� ���������� ������������
				int ExprCount = (*ci)->ExprList->GetExprCount();
				//��������� ���� � ������ ��������� �����������
				if (id_CArrayVar == BN->name_id) {
					CArrayType* AT = static_cast<CArrayVar*>(BN)->ArrayType;
					//��������� ���� ��-�� ������� � ������ ���������� ���������
					BN = AT->GetType(ExprCount);
				} else {
					CArrayType* AT = static_cast<CArrayType*>(static_cast<CPointerVar*>(BN)->FindType());
					//��������� ���� ��-�� ������� � ������ ���������� ���������
					BN = AT->GetType(ExprCount);
				}//else
				//�������� ������� ���� ��������� - � ���� ������ ��������� �������� ���, �� ���. �� ���������
				if (id_CPointerType == BN->name_id) BN = static_cast<CPointerType*>(BN)->FindType();
			}
			break;
		default:
			/**/
			//���� �� ����������� ������ ����
			break;
		}//switch

	//������ ���������� �����
	return BN;
}


//-----------------------------------------------------------------------------
//������������� ������� Designator �� ������ ������
int CDesignator::Init(CLexBuf *lb)
{
	////////////////////////////////////////////////////////////////////
	//1-� ����: ��������� Qualident � ����� ��� �������� � �������� ����
	////////////////////////////////////////////////////////////////////

	//�������� ������� ����� � ������ ������
	Qualident = new CQualident;
	int err_num = Qualident->Init(lb, parent_element);
	if (err_num) return err_num;

	//�������� ������� ����������� ����� � ������� ����
	CBaseName* BN = parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident);

	//���� �� �������� ���������� ���, ����� ��������� �������� ������� ����������
	//���� ������, ��������� ��� ������ � ������� ������ (�.�. ������� '.')
	if (!BN) {
		//������ ident ���������������� �� ��� ��� ������, � ��� ��� ���������� �������
		BN = parent_element->GetGlobalName(Qualident->pref_ident);
		//�������� ���������� ���������� �������
		if (BN) {
			//�������� ������������ ���� ���������� �������
			EDesKind DesKind;
			switch (BN->name_id) {
			case id_CRecordVar:
				//��������, �������� �� ������ VAR-���������� (������� ��������������� ��� ���������)
				DesKind = static_cast<CBaseVar*>(BN)->is_var ? dk_Pointer : dk_Record;
				break;
			case id_CPointerVar:
			case id_CCommonVar:
				DesKind = dk_Pointer;
				break;
			default:
				//������������ ������ ����� '.'
				return s_e_IdentNotRecordType;
			}
			//�������� ��������� ���������� ����� � ������ ���� ���������� �������
			BN = BN->FindName(Qualident->ident);
			if (!BN) return s_e_DesRecordField;
			//�������� ��-�� ����������� ���������� ���� � ��������� ��� � ������
			SDesElem* DesElem = CreateSDesElem(DesKind);
			DesElem->ident = Qualident->ident;
			Qualident->ident = NULL;
			DesElemStore.push_back(DesElem);
			//�.�. ��������� ident ��������� � ��-� �����������, ������������ Qualident
			Qualident->ident = Qualident->pref_ident;
			Qualident->pref_ident = NULL;
		} else	//���������� � ������� ������ �� �������
			return s_e_UndeclaredIdent;
	}//if

	/////////////////////////////////////////////////////////////////////////
	//2-� ����: � BN - ��������� ������ � �������, ���������� ��������� ��-��
	/////////////////////////////////////////////////////////////////////////

	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//������� ������������� ����������� ��������� ��-��� �����������
	bool continue_elems_parsing = true;

	while (continue_elems_parsing) {
		//���������� �������� read-only (���� ������ ������������) ��������� ��������� �������� BN
		if (Qualident->pref_ident && CBaseVar::IsVarId(BN->name_id))
			is_read_only = is_read_only || static_cast<CBaseVar*>(BN)->is_read_only;

		//���������� �������, �.�. ����� ����������� ������ �� �� ������� ��-��� �����������
		DECL_SAVE_POS

		//��������� ��������� ������� (���� ����)
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) {
			RESTORE_POS
			break;
		}

		//�������� ���� ���������� �������
		switch (li.lex) {

		////////////////////////////////////////////////////
		//�������� '.' - ��������� ��� ������, ��. �� ������
		case lex_k_dot: {
			//������ �������� ������� � �������� ���������� ������� '^'
			present_last_up = false;
			//��� �������� ��������� ������ �� CompoundName
			const char* pCompoundName = NULL;
			//������� ���� ���������� ��-�� ����������� (����������� �������)
			EDesKind DesKind;
			//�������� ���� �������
			switch (BN->name_id) {
			case id_CRecordVar:
				DesKind = static_cast<CBaseVar*>(BN)->is_var ? dk_Pointer : dk_Record;
				break;
			case id_CPointerVar:
				if (!static_cast<CPointerVar*>(BN)->IsRecordPointer()) return s_e_IdentNotRecordType;
				DesKind = dk_Pointer;
				break;
			case id_CCommonVar:
				pCompoundName = static_cast<CCommonVar*>(BN)->GetCPPCompoundName();
				DesKind = dk_Common;
				break;
			case id_CRecordType:
				DesKind = dk_Record;
				break;
			case id_CPointerType:
				BN = static_cast<CPointerType*>(BN)->FindType();
				if (id_CRecordType != BN->name_id) return s_e_IdentNotRecordType;
				DesKind = dk_Pointer;
				break;
			default:
				//������������ ������ ����� '.'
				return s_e_IdentNotRecordType;
			}//switch
			//������ ����. ������� (������ ���� �������������)
			if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
			//�������� ��������� ���������� ����� � ������ ���� ���������� �������
			BN = BN->FindName(li.st);
			if (!BN) return s_e_DesRecordField;
			//�������� ��-�� ����������� ���������� ���� � ��������� ��� � ������
			SDesElem* DesElem = CreateSDesElem(DesKind);
			DesElem->ident = str_new_copy(li.st);
			if (pCompoundName) DesElem->CPPCompoundName = str_new_copy(pCompoundName);
			DesElemStore.push_back(DesElem);
			//����������� ��������� ��-��� �����������
			continue;
		}

		///////////////////////////////////////////////
		//�������� '^' - ��������� ������ ��� ���������
		case lex_k_up_arrow:
			if (id_CPointerVar != BN->name_id && id_CPointerType != BN->name_id) return s_e_DesNotPointer;
			//���������� ������� ������� � �������� ���������� ������� '^'
			present_last_up = true;
			//����������� ��������� ��-��� �����������
			continue;

		////////////////////////////////////////////////////////
		//�������� '[' - ��������� ��� ������� ��� ��. �� ������
		case lex_k_op_square: {
			//������ �������� ������� � �������� ���������� ������� '^'
			present_last_up = false;
			//�������� ������� ������� ��� ��. �� ������
			if (id_CArrayVar != BN->name_id && id_CPointerVar != BN->name_id) return s_e_DesNotArray;
			if (id_CPointerVar == BN->name_id && !static_cast<CPointerVar*>(BN)->IsArrayPointer()) return s_e_DesNotArray;
			//�������� ��-�� ����������� ���������� ���� � ��������� ��� � ������ (������� ��������� ������� ����� ��������� ����)
			SDesElem* DesElem = CreateSDesElem(dk_Array);
			DesElem->ExprList = new CExprList(parent_element, NULL);
			DesElemStore.push_back(DesElem);
			//�������� ������� �������� �������� ������� (�������� ExprList ��� ������ ��� ����������)
			err_num = DesElem->ExprList->Init(lb);
			if (err_num) return err_num;
			//�������� ������� ']'
			if (!lb->ReadLex(li) || lex_k_cl_square != li.lex) return s_e_ClSquareMissing;
			//�������� ����������� �������� �������� �������
			SAVE_POS
			while (lb->ReadLex(li) && lex_k_op_square == li.lex) {
				//��������� ������ ��� ��������� ����������� ��������� ������ [...]
				CExprList EL(parent_element, NULL);
				//�������� ���������� ������ ��������� (��� ������ �������� ��������������)
				err_num = EL.Init(lb);
				if (err_num) return err_num;
				//����������� ����������� ��������� � �������� ������
				EL.MoveAllExpr(DesElem->ExprList);
				//�������� ������� ']'
				if (!lb->ReadLex(li) || lex_k_cl_square != li.lex) return s_e_ClSquareMissing;
				//��������� ������ [...] ����������
				SAVE_POS
			}
			RESTORE_POS
			//��������� ������ ���������� ������������ �������� �������
			int ExprCount = DesElem->ExprList->GetExprCount();
			//�������� ���������� ����������� ������������ �������� ��������������� ���������� �������� �������
			//������� ������� ��� ��. �� ������ ����� '[' ��������� ����
			CArrayType* AT;
			if (id_CArrayVar == BN->name_id) {
				AT = static_cast<CArrayVar*>(BN)->ArrayType;
				if (AT->GetDimCount() < ExprCount) return s_e_DesNotArray;
				//��������� ���� ��-�� ������� � ������ ���������� ���������
				BN = AT->GetType(ExprCount);
			} else {
				AT = static_cast<CArrayType*>(static_cast<CPointerVar*>(BN)->FindType());
				if (AT->GetDimCount() < ExprCount) return s_e_DesNotArray;
				//��������� ���� ��-�� ������� � ������ ���������� ���������
				BN = AT->GetType(ExprCount);
			}
			//��������, �������� �� ������ ��������
			if (!AT->size) DesElem->DesKind = dk_OpenArray;
			//����������� ��������� ��-��� �����������
			continue;
		}

		////////////////////////////////////////////////////////////////////////////
		//�������� '(' - ��������� ��� ���������-���������� ������ ��� ��. �� ������
		case lex_k_op_bracket: {
			//������ �������� ������� � �������� ���������� ������� '^'
			present_last_up = false;
			//�������� ���� �������
			switch (BN->name_id) {
			case id_CRecordVar:
				//������ ����������� ������ ���� VAR-����������
				if (!static_cast<CRecordVar*>(BN)->is_var) return s_e_GuardVarNotRecOrP;
			case id_CPointerVar:
				//��. ������ ����������� ��������� �� ������
				if (!static_cast<CPointerVar*>(BN)->IsRecordPointer()) return s_e_GuardVarNotRecOrP;
			default:
				//������ ������ ��������� ��� ������ ����� �� ��������������
				RESTORE_POS
				continue_elems_parsing = false;
				continue;
			}
			//�������� ��-�� ����������� ���������� ����
			SDesElem* DesElem = CreateSDesElem(dk_Guard);
			DesElem->Qualident = new CQualident;
			//�������� ������� �������� ����
			err_num = DesElem->Qualident->InitTypeName(lb, parent_element);
			if (err_num) {
				delete DesElem->Qualident;
				delete DesElem;
				//��������� �������� ��������� ��������� ��-��� �����������
				RESTORE_POS
				continue_elems_parsing = false;
				continue;
			}

			/**/ //��������� ���, ���������� � Qualident

			//��������� ��-�� ����������� � ������
			DesElemStore.push_back(DesElem);
			//�������� ������� ')'
			if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;
			//����������� ��������� ��-��� �����������
			continue;
		}

		//////////////////////////////////////////////////////////////////////////////
		//��������� ������, �� ����������� � ��-�� �����������, ����� ��������� ��-���
		default:
			RESTORE_POS
			continue_elems_parsing = false;

		}//switch

	}//while

	//////////////////////////////////////////////////////////////////////////
	//3-� ����: �������� ���� �������, �� ������� ��������� ������ �����������
	//(��������� � BN), ���������� ���������� ��������� ResultId
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////
	//�������� ������� ����� ���������
	if (CProcedure::IsProcId(BN->name_id)) {
		//�������� ������������� ������������� ��������� �� ��� ������
		if (req_proc_var) {
			ResultId = id_CProcedure;
			return 0;
		}
		//����������� �������� �������
		is_proc_name = true;
		ResultId = BN->GetResultId();
		return 0;
	}

	/////////////////////////////////////////
	//�������� ������� ����������� ����������
	if (id_CProcedureVar == BN->name_id || id_CProcedureType == BN->name_id) {
		//�������� ������������� ������������� ����������� ���������� �� ��� ������ �������
		if (req_proc_var) {
			ResultId = id_CProcedureVar;
			return 0;
		}
		DECL_SAVE_POS
		//��������� ����. ������� ��� ������ ����������� ������ ���������
		if (!lb->ReadLex(li)) return s_e_AssignMissing;
		//�������� ���������� ������� (��� ����� ���� ':=' - �������������; '(',';', <��������> - �����)
		if (lex_k_assign == li.lex) {
			//��������� ������������� (���������� ��������) ����������� ����������
			RESTORE_POS
			ResultId = id_CProcedureVar;
			return 0;
		} else {
			//��������� ������ ��������� ����� ����������� ����������
			RESTORE_POS
			is_proc_name = true;
			ResultId = BN->GetResultId();
			return 0;
		}
	}

	//� ������ ������� ���������� '^' �������� ���, �� ���. ��������� ���������,
	//�.�. ������ �� �������� �� ��� ���������
	if (present_last_up) {
		if (id_CPointerType == BN->name_id)
			BN = static_cast<CPointerType*>(BN)->FindType();
		else
			BN = static_cast<CPointerVar*>(BN)->FindType();
	}

	//���������� ���������� ��������� ��� ����������
	ResultId = BN->GetResultId();

	return 0;
}


//-----------------------------------------------------------------------------
//������ ���� CDesignator
void CDesignator::WriteCPP(CPP_files& f)
{
	//������� ������������� ������ ')' ����� ���������� ����
	bool need_cl_bracket = false;
	//��� ����������� ��-�� ����������� (������������ � �������� ��������)
	const char* PrevName = Qualident->ident;

	//�������� ������������� ������ �������������
	if (present_last_up) fprintf(f.fc, "*(");

	//��������� ����������, � ������� ���������� �����������
	//(��� �������� ������������� ������ ���������� ����)
	CBaseName* BN = parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident);
	if (gen_Guard_code && CBaseVar::IsVarId(BN->name_id)) {
		CBaseVar* BV = static_cast<CBaseVar*>(BN);
		//�������� ������������� ������ ���������� ����
		if (BV->is_guarded) {
			fprintf(f.fc, "static_cast<");
			if (BV->GetTypeModuleAlias()) fprintf(f.fc, "%s::", BV->GetTypeModuleAlias());
			fprintf(f.fc, "%s*>(", BV->GetTypeName());
			need_cl_bracket = true;
		}
	}

	//������ ��������
	if (Qualident->pref_ident) fprintf(f.fc, "%s::", Qualident->pref_ident);
	fprintf(f.fc, "%s", Qualident->ident);

	//������ �������� ��������� � ������ ���������� ���������� (���� ����������)
	if (id_CCommonVar == BN->name_id)
		if (!DesElemStore.empty()) fprintf(f.fc, "%sO2M_SPEC_%s", static_cast<CCommonVar*>(BN)->is_var ? "->" : ".", static_cast<CCommonVar*>(BN)->GetCPPCompoundName());

	//������ �������� ��������� � ������ ��. �� �������������
	if (id_CPointerVar == BN->name_id && id_CSpecType == static_cast<CPointerVar*>(BN)->FindType()->name_id)
		if (!DesElemStore.empty()) {
			CSpecType* ST = static_cast<CSpecType*>(static_cast<CPointerVar*>(BN)->FindType());
			fprintf(f.fc, "->O2M_SPEC_");
			if (ST->GetQualSpecName()) fprintf(f.fc, "%s_", ST->GetQualSpecName());
			fprintf(f.fc, ST->GetSpecName());
		}

	//������ ')' ����� ���������� ���� (���� ����)
	if (need_cl_bracket) fprintf(f.fc, ")");

	//������� � ��������� ���� ��-��� �����������
	SDesElemStore::const_iterator ci;
	for (ci = DesElemStore.begin(); ci != DesElemStore.end(); ci++) {
		switch ((*ci)->DesKind) {
		case dk_Common:
			fprintf(f.fc, ".O2M_SPEC_%s->%s", (*ci)->CPPCompoundName, (*ci)->ident);
			break;
		case dk_Pointer:
		case dk_Record:
			fprintf(f.fc, "%s%s", (dk_Record == (*ci)->DesKind) ? "." : "->", (*ci)->ident);
			break;
		case dk_Array:
			(*ci)->ExprList->WriteCPP_index(f, false, PrevName);
			break;
		case dk_OpenArray:
			(*ci)->ExprList->WriteCPP_index(f, true, PrevName);
			break;
		default:
			/**/
			//������ ���� ���� �� �����������
			break;
		}//switch
		//���������� ������� ��� (���� ����) ��� ������������� �� ��������� ��������
		PrevName = (*ci)->ident;
	}//for

	//�������� ������������� ������ ')' ����� �������������
	if (present_last_up) fprintf(f.fc, ")");

	//�������� ������ � ����������� ��������� ��� ������� ��. �� ������
	if (in_fact_pars) {
		//��������, �������� �� ������� Designator ���������� �� ������
		CBaseName* BV = FindLastName();
		if (BV && id_CPointerVar == BV->name_id) {
			//������� ��. �� �� ������ - ���������� ���������
			if (!static_cast<CPointerVar*>(BV)->IsArrayPointer()) return;
			//������ �������� ������� �� ������������ (������� ������������ ������)
			CArrayType* AT = static_cast<CArrayType*>(static_cast<CPointerVar*>(BV)->FindType());
			while (id_CArrayType == AT->name_id) {
				//������ ������� ������� (����� ����������� � ���� ������� ��� � ���� ����������(� ����������))
				if (AT->size > 0)
					fprintf(f.fc, ",%i", AT->size);
				else
					fprintf(f.fc, ",O2M_ARR_0_%s", BV->name);
				AT = static_cast<CArrayType*>(AT->Type);
			}//while
		}//if
	}//if

}//WriteCPP


//-----------------------------------------------------------------------------
//������ ���� CDesignator ��� ��������� ���� ���������� ���� (��� ����������
//����������) - ���. � ����� ������ ������������
void CDesignator::WriteCPP_Guardless(CPP_files &f)
{
	//��� ������ �� ������������ ������ ��� ������������� �������� ������ ���������
	gen_Guard_code = !DesElemStore.empty();
	WriteCPP(f);
}


//-----------------------------------------------------------------------------
//����������� ���� ��������� � ��������� ������ ���������
void CExprList::MoveAllExpr(CExprList* ExprList)
{
	//����������� ���������� � ��������� ������
	CExprVector::const_iterator ci;
	for (ci = ExprVector->begin(); ci != ExprVector->end(); ++ci)
		ExprList->ExprVector->push_back(*ci);
	//������� ������ ��������� (��� ��������� ����������)
	ExprVector->clear();
}


//-----------------------------------------------------------------------------
//�������� ���������
int CSimpleExpr::CreateConst(CBaseVar *&BaseConst)
{
	//�������� ��������� �� ������� Term
	int err_num = Term->CreateConst(BaseConst);
	if (err_num) return err_num;

	//�������� ������� ����� "-"
	if (negative) {
		switch (BaseConst->name_id) {
		case id_CShortintVar:
			static_cast<CShortintVar*>(BaseConst)->ConstValue = - static_cast<CShortintVar*>(BaseConst)->ConstValue;
			break;
		case id_CIntegerVar:
			static_cast<CIntegerVar*>(BaseConst)->ConstValue = - static_cast<CIntegerVar*>(BaseConst)->ConstValue;
			break;
		case id_CLongintVar:
			static_cast<CLongintVar*>(BaseConst)->ConstValue = - static_cast<CLongintVar*>(BaseConst)->ConstValue;
			break;
		case id_CRealVar:
			static_cast<CRealVar*>(BaseConst)->ConstValue = - static_cast<CRealVar*>(BaseConst)->ConstValue;
			break;
		case id_CLongrealVar:
			static_cast<CLongrealVar*>(BaseConst)->ConstValue = - static_cast<CLongrealVar*>(BaseConst)->ConstValue;
			break;
		case id_CSetVar:
			static_cast<CSetVar*>(BaseConst)->ConstValue = ~static_cast<CSetVar*>(BaseConst)->ConstValue;
			break;
		}//switch
	}

	//�������� ������� ������ ��-���
	if (SimpleExprPairStore) {
		CBaseVector::const_iterator ci;
		for (ci = SimpleExprPairStore->begin(); ci != SimpleExprPairStore->end(); ++ci) {
			err_num = static_cast<CSimpleExprPair*>(*ci)->ApplyOperation(BaseConst);
			if (err_num) return err_num;
		}
	}

	return 0;
}


//-----------------------------------------------------------------------------
//�������� ���������
int CTerm::CreateConst(CBaseVar *&BaseConst)
{
	//�������� ��������� �� ������� Factor
	int err_num = Factor->CreateConst(BaseConst);
	if (err_num) return err_num;

	//�������� ������� ������ ����������
	if (TermPairStore) {
		CBaseVector::const_iterator ci;
		for (ci = TermPairStore->begin(); ci != TermPairStore->end(); ++ci) {
			err_num = static_cast<CTermPair*>(*ci)->ApplyOperation(BaseConst);
			if (err_num) return err_num;
		}
	}

	return 0;
}


//-----------------------------------------------------------------------------
//�������� ���������
int CFactor::CreateConst(CBaseVar *&BaseConst)
{
	switch (FactorKind) {
	case fk_Negation:
		{
			int err_num = Factor->CreateConst(BaseConst);
			if (err_num) return err_num;
			//� CFactor::Init ������ ���� ���������, ��� ResultId == id_CBooleanVar
			if (id_CBooleanVar != BaseConst->name_id) {
				delete BaseConst;
				return s_m_Error;
			}
			//���������� �������� ���������
			static_cast<CBooleanVar*>(BaseConst)->ConstValue = !static_cast<CBooleanVar*>(BaseConst)->ConstValue;
			return 0;
		}
	case fk_Expr:
		return Expr->CreateConst(BaseConst);
	case fk_ConstVar:
		//����� ������� Var->CreateConst ���������� ��������� ������� �������� �������������
		if (!ConstVar->is_const) return s_e_ExprNotConst;
		//��� is_const == true ������� CreateConst �� ������ ���������� NULL
		BaseConst = ConstVar->CreateConst(parent_element);
		return 0;
	case fk_StdProcFunc:
		return StdProcFunc->CreateConst(BaseConst);
	case fk_Designator:
		{
			//��������� ��������� �� ����� (���� ����)
			CBaseName* BN = parent_element->GetGlobalName(Designator->Qualident->pref_ident, Designator->Qualident->ident);
			//�������� ������� ����������
			if (!BN || !CBaseVar::IsVarId(BN->name_id)) return s_e_ExprNotConst;
			//����� ������� Var->CreateConst ���������� ��������� ������� �������� �������������
			if (!static_cast<CBaseVar*>(BN)->is_const) return s_e_ExprNotConst;
			//��� is_const == true ������� CreateConst �� ������ ���������� NULL
			BaseConst = static_cast<CBaseVar*>(BN)->CreateConst(parent_element);
			return 0;
		}
	default:
		//��� ��������� �������� FactorKind ������ ���� ���������� ����
		return s_m_Error;
	}//switch
}


//-----------------------------------------------------------------------------
//����� ���������� ������� � ������� ��������� (l-value ��� ���������)
CBaseName* CFactor::FindLastName()
{
	switch (FactorKind) {
	case fk_Expr:
		return Expr->FindLastName();
	case fk_Designator:
		{
			//�������� ���� ���� RecordName.FieldName
			CBaseName *BN = Designator->FindLastName();
			//�������� ���������� ���������� (����� ������ l-value ��� ���������)
			if (!BN || (!CBaseVar::IsVarId(BN->name_id) && !CBaseType::IsTypeId(BN->name_id))) return NULL;
			return static_cast<CBaseVar*>(BN);
		}
	case fk_ConstVar:
		return ConstVar;
	default:
		return NULL;
	}//switch
}


//---------------------------------------------------------------
//������������� ������� ������� Call (�������� ������ ���������) �� ������ ������
int CCallStatement::Init(CLexBuf *lb)
{
	//��� ���������� ����������� ��������� ���������������� ��������������� �����
	//���� {a}.Proc(b)
	if (!Designator) {
		//����� ��������� ������������� ���������� ����������� ����������
		int err_num = InitProcCommonPars(lb);
		if (err_num) return err_num;
		//����� ��� ������ ���������� � ������� �������
		CLexInfo li;
		//�������� ������� '.'
		if (!lb->ReadLex(li) || lex_k_dot != li.lex) return s_e_DotMissing;
		//�������������� �����������
		is_des_owner = true;
		Designator = new CDesignator(parent_element, false);
		err_num = Designator->Init(lb);
		if (err_num) return err_num;
	}

	//���� Designator ��� ������� ����� ����������� => �������� ��������� ������ �������
	//����������� ���������� � ������� ���������������� ������ ���� ProcName{a}(b)

	//��������, �������� �� ����������� ����������
	if (!Designator->IsProcName()) return s_e_CallNotProc;
	//�������� ������ ������� ��� ���������
	if (is_proc_call && id_CBaseName != Designator->GetResultId())
		return s_e_CallFuncAsProc;

	//��������� ��. �� CProcedure ��� CProcedureVar
	CBaseName* BN = Designator->FindLastName();
	//��������� ��. �� FormalPars, CCommonPars � ������ CProcedure ��� CProcedureVar
	CFormalPars* FP;
	CCommonPars* CP = NULL;
	if (CProcedure::IsProcId(BN->name_id)) {
		FP = static_cast<CProcedure*>(BN)->FormalPars;
		//��������� ��. �� ���������� ��������� (���� ����)
		if (id_CCommonProc == BN->name_id || id_CDfnCommonProc == BN->name_id)
			CP = static_cast<CCommonProc*>(BN)->CommonPars;
	} else
		if (id_CProcedureVar == BN->name_id)
			FP = &static_cast<CProcedureVar*>(BN)->FormalPars;
		else
			if (id_CProcedureType == BN->name_id)
				FP = &static_cast<CProcedureType*>(BN)->FormalPars;
			else
				return s_m_Error;

	//����� ��������� ������������� ����������� ����������
	return InitProcPars(lb, CP, FP);
}//Init


//-----------------------------------------------------------------------------
//�������� ������� � ���������� ���������� ���������� ��� ������ ��������� (�������)
int CCallStatement::InitProcCommonPars(CLexBuf *lb)
{
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	DECL_SAVE_POS
	//�������� ������� ��. ����� "{" (���� ���, ���������� ��������� �����������)
	if (!lb->ReadLex(li) || lex_k_op_brace != li.lex) {
		RESTORE_POS
		return 0;
	}

	//���� "{", �������� ������� ���������� CommonList (�� �� ������ ���� ������ �����)
	if (CommonList) return s_e_CommonProcCallMixed;
	//�������� ������� ��. ����� "}" (���������� �������� CommonList)
	SAVE_POS
	if (!lb->ReadLex(li) || lex_k_cl_brace != li.lex) {
		//��. ����� "}" �����������, ��������� ������� CommonList
		RESTORE_POS
		//�������� CExprList
		CommonList = new CExprList(parent_element, NULL);
		//������������� CommonList
		int err_num = CommonList->Init(lb);
		if (err_num) return err_num;
		//�������� ������� ��. ����� "}"
		if (!lb->ReadLex(li) || lex_k_cl_brace != li.lex) return s_e_ClBraceMissing;
	}

	return 0;
}


//-----------------------------------------------------------------------------
//���������� ������� � ���������� ����������� ���������� ��� ������� �������� (�������)
int CCallStatement::InitProcPars(CLexBuf *lb, CCommonPars* CP, CFormalPars* FP)
{
	//�������� ������� ������ ���������� ��������� ���� CommonProcName{a,b}()
	int err_num = InitProcCommonPars(lb);
	if (err_num) return err_num;

	DECL_SAVE_POS
	//����� ��� ������ ���������� � ������� �������
	CLexInfo li;

	//�������� ������� ��. ����� "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) {
		//� ������ ������� '(' ������ ����������� ��������������
		if (!is_proc_call) return s_e_OpBracketMissing;
		//� ������ ��������� ������� '(' �������������
		RESTORE_POS
	} else {
		//���� "(", ��������� ������� ")" ��� ExprList
		//�������� ������� ��. ����� ")" (���������� ExprList)
		SAVE_POS
		if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) {
			//��. ����� ")" �����������, ��������� ������� ExprList
			RESTORE_POS
			//�������� � ������������� ExprList
			ExprList = new CExprList(parent_element, FP);
			err_num = ExprList->Init(lb);
			if (err_num) return err_num;
			//�������� ������� ��. ����� ")"
			if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;
		}
	}

	//��������� ��������� ��������� ����������� � ���������� ����������� ����������
	int CommonParsCount = CommonList ? CommonList->GetExprCount() : 0;
	int ActualParsCount = ExprList ? ExprList->GetExprCount() : 0;

	//�������� ������������ ���������� ���������� ����������� ����������
	if (CP) {
		//�������� ������� ������ ���������� ���������� (������ ��������������)
		if (!CommonParsCount) return s_e_CommonParsExpected;
		//�������� ���������� ��������� ���������� � ����������� ���������� ����������
		if (CP->FPStore.size() > CommonParsCount)
			return s_e_CommonParsFewer;
		else
			if (CP->FPStore.size() < CommonParsCount) return s_e_CommonParsMore;
	} else
		if (CommonParsCount) return s_e_CommonParsMore;
	//�������� ������������ ���������� ����������� ����������
	if (FP->FPStore.size() > ActualParsCount)
		return s_e_ParsFewer;
	else
		if (FP->FPStore.size() < ActualParsCount) return s_e_ParsMore;

	return 0;
}


//---------------------------------------------------------------
//������ ���� CCallStatement
void CCallStatement::WriteCPP(CPP_files& f)
{
	//���� CallStatement - ����� ���������, ��������� ��������� ��� ��������������
	if (is_proc_call) f.tab_fc();

	//������ ���� ������ �����������
	Designator->WriteCPP(f);

	//������ ������ ������ ����������
	fprintf(f.fc, "(");

	//��������� ��. �� CProcedure ��� CProcedureVar
	CBaseName *BN = Designator->FindLastName();

	//�������� ������� ����������� ����������
	if (ExprList) {
		//����� ����� ����������� CProcedure � CProcedureVar
		if (CProcedure::IsProcId(BN->name_id))
			ExprList->WriteCPP_proc(f, static_cast<CProcedure*>(BN)->FormalPars);
		else
			if (id_CProcedureVar == BN->name_id)
				ExprList->WriteCPP_proc(f, &static_cast<CProcedureVar*>(BN)->FormalPars);
			else
				ExprList->WriteCPP_proc(f, &static_cast<CProcedureType*>(BN)->FormalPars);
	}
	//� ������ ���������� ��������� ����������� ���������� ��������� (���� ��� ��������)
	if ((id_CCommonProc == BN->name_id || id_CDfnCommonProc == BN->name_id || id_CHandlerProc == BN->name_id) && CommonList) {
		if (ExprList && (ExprList->GetExprCount() > 0)) fprintf(f.fc, ",");
		//����� ����� ����������� CProcedure � CProcedureVar
		if (id_CProcedureVar == BN->name_id)
			CommonList->WriteCPP_common_proc(f, &static_cast<CProcedureVar*>(BN)->FormalPars);
		else
			if (id_CCommonProc == BN->name_id || id_CDfnCommonProc == BN->name_id)
				CommonList->WriteCPP_common_proc(f, static_cast<CCommonProc*>(BN)->CommonPars);
			else
				CommonList->WriteCPP_common_proc(f, static_cast<CProcedure*>(BN)->FormalPars);
	}

	//���������� ������ ����������
	fprintf(f.fc, ")");
}
