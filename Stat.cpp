//=============================================================================
// Описание классов инструкций (Statements)
//=============================================================================

#include "Stat.h"
#include "StdProc.h"
#include "Type.h"
#include "Var.h"


//-----------------------------------------------------------------------------
//деструктор объекта ELSIF
CElsifPair::~CElsifPair()
{
	delete StatementSeq;
	delete Expr;
}//~CElsifPair


//-----------------------------------------------------------------------------
//инициализация объекта ELSIF из потока лексем
int CElsifPair::Init(CLexBuf *lb)
{
	//проверка наличия выражения
	Expr = new CExpr(parent_element);
	int err_num = Expr->Init(lb);
	if (err_num) return err_num;

	//проверка типа выражения
	if (Expr->GetResultId() != id_CBooleanVar) return s_e_IF_ExprType;

	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия кл. слова THEN
	if (!lb->ReadLex(li) || lex_k_THEN != li.lex) return s_e_THEN;

	//проверка наличия послед. операторов
	StatementSeq = new CStatementSeq(parent_element);
	err_num = StatementSeq->Init(lb);
	if (err_num) return err_num;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//Запись кода CElsifPair
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
//деструктор объекта IF
CIfStatement::~CIfStatement()
{
	ElsifPairList_type::const_iterator ci;
	for (ci = ElsifPairList.begin(); ci != ElsifPairList.end(); ++ci)
		delete *ci;
	delete ElseStatementSeq;
}//~CIfStatement


//-----------------------------------------------------------------------------
//проверка наличия оператора RETURN
EHaveRet CIfStatement::HaveRet() const
{
	//признаки наличия веток операторов без и с RETURN
	bool HaveNo = false;
	bool HaveYes = false;
	//проверка наличия RETURN среди содержимого секций ELSIF
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
	//проверка наличия RETURN в секции ELSE (если есть)
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
	//т.к. все случаи NotAll проверены выше, проверяем наличие No и Yes
	return HaveYes ? HaveNo ? hr_NotAll : hr_Yes : hr_No;
}


//-----------------------------------------------------------------------------
//инициализация объекта IF из потока лексем
int CIfStatement::Init(CLexBuf *lb)
{
	//создание списка ELSIF с первым элементом (для IF ... THEN)
	CElsifPair* EPair = new CElsifPair(parent_element);
	int err_num = EPair->Init(lb);
	if (err_num) {
		delete EPair;
		return err_num;
	}
	ElsifPairList.push_back(EPair);
	
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//чтение очередной лексемы (ключевого слова)
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;

	//проверка наличия ELSIF (список уже создан)
	while (lex_k_ELSIF == li.lex) {
		//проверка очередной пары Expr THEN StatementSeq
		EPair = new CElsifPair(parent_element);
		err_num = EPair->Init(lb);
		if (err_num) {
			delete EPair;
			return err_num;
		}
		ElsifPairList.push_back(EPair);

		//чтение очередной лексемы (ключевого слова)
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;
	};

	//проверка наличия ELSE
	if (lex_k_ELSE == li.lex) {
		ElseStatementSeq = new CStatementSeq(parent_element);
		err_num = ElseStatementSeq->Init(lb);
		if (err_num) return err_num;
		//чтение очередной лексемы (ключевого слова)
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;
	};

	//проверка наличия конца (остальные варианты проверены)
	if (lex_k_END != li.lex) return s_e_END;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//Запись кода CIfStatement
void CIfStatement::WriteCPP(CPP_files& f)
{
	ElsifPairList_type::iterator i = ElsifPairList.begin();
	//запись первого условия (должно быть всегда)
	if (!ElsifPairList.empty()) (*i)->WriteCPP(f);
	//запись последующих условий ELSIF (могут отсутствовать)
	for(++i; i != ElsifPairList.end(); ++i) {
		fprintf(f.fc, " else\n");
		(*i)->WriteCPP(f);
	}
	//запись последнего чсловия ELSE
	if (ElseStatementSeq) {
		fprintf(f.fc, " else {\n");
		ElseStatementSeq->WriteCPP(f);
		f.tab_fc();
		fprintf(f.fc, "}");
	}
}//WriteCPP


//-----------------------------------------------------------------------------
//деструктор
CCaseLabelsSeq::~CCaseLabelsSeq()
{
	CCaseLabelsSeq::CaseLabelsList_type::iterator i;
	for (i = CaseLabelsList.begin(); i != CaseLabelsList.end(); ++i)
		delete *i;
}//CCaseLabelsSeq


//-----------------------------------------------------------------------------
//инициализация объекта CaseLabelsSeq из потока лексем
int CCaseLabelsSeq::Init(CLexBuf *lb, CCaseStatement* const CaseStatement)
{
	DECL_SAVE_POS
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия первой метки (может не быть, тогда есть "|", "ELSE", или "END")
	if (!lb->ReadLex(li) || lex_k_vertical == li.lex || lex_k_ELSE == li.lex || lex_k_END == li.lex) {
		RESTORE_POS
		return s_m_CaseAbsent;
	}

	RESTORE_POS

	//первая метка присутствует => есть весь Case, инициализируем его
	while (true) {
		//получение очередной метки (интервала)
		CCaseLabels* CaseLabels = new CCaseLabels(parent_element);
		int err_num = CaseLabels->Init(lb, CaseStatement, this);
		if (err_num) {
			delete CaseLabels;	//очистка временной переменной
			return err_num;
		}
		//занесение очередной метки в список
		CaseLabelsList.push_back(CaseLabels);
		//получение след. лексемы (должно быть "," или ":")
		if (!lb->ReadLex(li)) return s_e_ColonMissing;
		//проверка полученной лексемы
		switch (li.lex) {
		case lex_k_comma:
			continue;
		case lex_k_colon:
			break;
		default:
			return s_e_ColonMissing;
		}
		//получен ":", конец обработки
		break;
	}

	return StatementSeq.Init(lb);
}//Init


//-----------------------------------------------------------------------------
//Запись кода CCaseLabelsSeq
void CCaseLabelsSeq::WriteCPP(CPP_files& f, CExpr* Expr)
{
	CCaseLabelsSeq::CaseLabelsList_type::const_iterator i;

	//проверка первой метки
	if (CaseLabelsList.empty()) throw error_Internal("CCaseLabelsSeq::WriteCPP");
	i = CaseLabelsList.begin();
	(*i)->WriteCPP(f, Expr);
	
	//цикл перебора оставшихся меток
	for (++i; i != CaseLabelsList.end(); ++i)
	{
		fprintf(f.fc, ")||(");
		(*i)->WriteCPP(f, Expr);
	}

	//генерация кода послед-ти операторов
	fprintf(f.fc, ")) {\n");
	StatementSeq.WriteCPP(f);
	f.tab_fc();
	fprintf(f.fc, "}");
}//WriteCPP


//-----------------------------------------------------------------------------
//проверка наличия указанного значения в уже созданных метках
bool CCaseLabelsSeq::ValueExists(const long Value, const bool IsRng, const long HighValue)
{
	CCaseLabelsSeq::CaseLabelsList_type::const_iterator ci;
	for (ci = CaseLabelsList.begin(); ci != CaseLabelsList.end(); ci++)
		if ((*ci)->ValueExists(Value, IsRng, HighValue)) return true;
	return false;
}


//-----------------------------------------------------------------------------
//инициализация объекта CaseLabels из потока лексем
int CCaseLabels::Init(CLexBuf *lb, CCaseStatement* const CaseStatement, CCaseLabelsSeq* const CaseLabelsSeq)
{
	//создание константы
	CBaseVar* BV;
	int err_num = ConstSelector(lb, BV, parent_element);
	if (err_num) return err_num;

	//запоминаем тип полученной константы
	EName_id ExprResultId = BV->GetResultId();
	//получаем тип выражения в CASE
	EName_id CaseExprResultId = CaseStatement->GetExprResultId();

	//запоминаем значение константы (одновременно проверяя тип)
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

	//уничтожение константы (значение уже сохранено)
	delete BV;

	//проверка допустимости сочетания выражений в CASE и метке
	if (CBaseVar::IsIntId(CaseExprResultId)) {
		//проверка поглощения полученной константы типом выражения в CASE
		if (!CBaseVar::IsIntId(ExprResultId) || !IsId1IncloseId2(CaseExprResultId, ExprResultId))
			return s_e_CASE_WrongLabelType;
	} else
		if (id_CCharVar != ExprResultId && id_CArrayVar != ExprResultId) return s_e_CASE_WrongLabelType;

	DECL_SAVE_POS
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия кл. слова ".."
	if (!lb->ReadLex(li) || lex_k_dots != li.lex) {
		RESTORE_POS
		//проверка отсутствия уже загруженной метки с аналогичными параметрами
		if (CaseLabelsSeq->ValueExists(ConstValue, false, 0) || CaseStatement->ValueExists(ConstValue, false, 0)) return s_e_CASE_LabelExists;
		//метка не содержит диапазона, конец обработки
		return 0;
	}

	//есть второе выражение
	IsRange = true;

	//создание второй константы
	err_num = ConstSelector(lb, BV, parent_element);
	if (err_num) return err_num;

	//запоминаем тип полученной константы
	ExprResultId = BV->GetResultId();

	//запоминаем значение константы (одновременно проверяя тип)
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

	//уничтожение константы (значение уже сохранено)
	delete BV;

	//проверка допустимости сочетания выражений в CASE и метке
	if (CBaseVar::IsIntId(CaseExprResultId)) {
		//проверка поглощения полученной константы типом выражения в CASE
		if (!CBaseVar::IsIntId(ExprResultId) || !IsId1IncloseId2(CaseExprResultId, ExprResultId))
			return s_e_CASE_WrongLabelType;
	} else
		if (id_CCharVar != ExprResultId && id_CArrayVar != ExprResultId) return s_e_CASE_WrongLabelType;

	//проверка допустимости интервала
	if (ConstValue > ConstHighValue) return s_e_CASE_LabelType;

	//проверка отсутствия уже загруженной метки с аналогичными параметрами
	if (CaseLabelsSeq->ValueExists(ConstValue, true, ConstHighValue) || CaseStatement->ValueExists(ConstValue, true, ConstHighValue)) return s_e_CASE_LabelExists;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//Запись кода CCaseLabels
void CCaseLabels::WriteCPP(CPP_files& f, CExpr* Expr)
{
	Expr->WriteCPP(f);
	//проверка наличия диапазона
	if (IsRange) {
		fprintf(f.fc, " >= %li && ", ConstValue);
		Expr->WriteCPP(f);
		fprintf(f.fc, " <= %li", ConstHighValue);
	} else
		fprintf(f.fc, " == %li", ConstValue);
}//WriteCPP


//-----------------------------------------------------------------------------
//проверка наличия указанного значения в текущей метке
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
//деструктор объекта CASE
CCaseStatement::~CCaseStatement()
{
	delete ElseStatementSeq;
	delete Expr;
	//очистка списка наборов меток
	CaseLabelsSeqList_type::const_iterator ci;
	for (ci = CaseLabelsSeqList.begin(); ci != CaseLabelsSeqList.end(); ++ci)
		delete *ci;
}//~CCaseStatement


//-----------------------------------------------------------------------------
//проверка наличия оператора RETURN
EHaveRet CCaseStatement::HaveRet() const
{
	//признаки наличия веток операторов без и с RETURN
	bool HaveNo = false;
	bool HaveYes = false;
	//проверка наличия RETURN среди содержимого секций ELSIF
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
	//проверка наличия RETURN в секции ELSE (если есть)
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
	//т.к. все случаи NotAll проверены выше, проверяем наличие No и Yes
	return HaveYes ? HaveNo ? hr_NotAll : hr_Yes : hr_No;
}


//-----------------------------------------------------------------------------
//инициализация объекта CASE из потока лексем
int CCaseStatement::Init(CLexBuf *lb)
{
	//проверка наличия выражения
	Expr = new CExpr(parent_element);
	int err_num = Expr->Init(lb);
	if (err_num) return err_num;

	//проверка типа выражения
	EName_id ExprResultId = Expr->GetResultId();
	if (!CBaseVar::IsIntId(ExprResultId) && (id_CCharVar != ExprResultId)) return s_e_CASE_Expr;

	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия кл. слова OF
	if (!lb->ReadLex(li) || lex_k_OF != li.lex) return s_e_OF;

	//цикл инициализации эл-тов Case
	while (true) {
		CCaseLabelsSeq* CLSeq = new CCaseLabelsSeq(parent_element);
		err_num = CLSeq->Init(lb, this);

		//при наличии Case заносим его в список
		switch (err_num) {
		case 0:
			//ошибок не обнаружено - заносим Case в список
			CaseLabelsSeqList.push_back(CLSeq);
			break;
		case s_m_CaseAbsent:
			//признак отсутствия Case - ничего не заносим в список
			delete CLSeq;
			break;
		default:
			//ошибка при инициализации Case
			delete CLSeq;
			return err_num;
		}

		//получение след. лексемы (должно быть ключевое слово)
		if (!lb->ReadLex(li)) return s_e_END;

		//проверка полученного ключевого слова
		switch (li.lex) {
		case lex_k_vertical:
			continue;
		case lex_k_ELSE:
		case lex_k_END:
			break;
		default:
			return s_e_END;
		}

		//получен ELSE или END - конец поиска Caseов
		break;
	}//while

	//проверка наличия секции ELSE
	if (lex_k_ELSE == li.lex) {
		ElseStatementSeq = new CStatementSeq(parent_element);
		err_num = ElseStatementSeq->Init(lb);
		if (err_num) return err_num;
		//проверяем след. лексему
		if (!lb->ReadLex(li) || lex_k_END != li.lex) return s_e_END;
	}

	return 0;
}//Init


//-----------------------------------------------------------------------------
//Запись кода CCaseStatement
void CCaseStatement::WriteCPP(CPP_files& f)
{
	//цикл перебора последовательностей меток
	CCaseStatement::CaseLabelsSeqList_type::const_iterator i;
	for (i = CaseLabelsSeqList.begin(); i != CaseLabelsSeqList.end(); ++i)
	{
		f.tab_fc();
		fprintf(f.fc, "if ((");
		(*i)->WriteCPP(f, Expr);
		fprintf(f.fc, " else\n");
	}//for

	f.tab_fc();

	//проверка наличия условия ELSE
	if (ElseStatementSeq) {
		fprintf(f.fc, "{\n");
		ElseStatementSeq->WriteCPP(f);
		f.tab_fc();
		fprintf(f.fc, "}");
	} else
		fprintf(f.fc, "\texit(0)");
}//WriteCPP


//-----------------------------------------------------------------------------
//проверка наличия указанного значения в уже созданных наборах меток
bool CCaseStatement::ValueExists(const long Value, const bool IsRng, const long HighValue)
{
	CaseLabelsSeqList_type::const_iterator ci;
	for (ci = CaseLabelsSeqList.begin(); ci != CaseLabelsSeqList.end(); ci++)
		if ((*ci)->ValueExists(Value, IsRng, HighValue)) return true;
	return false;
}


//-----------------------------------------------------------------------------
//создание охраняемой переменной нужного типа (для помещения в таблицу имен WithLoopLink)
CBaseVar* CGuard::CreateGuardVar()
{
	//поиск типа и создание переменной
	CBaseName* BN = parent_element->GetGlobalName(TypeName.pref_ident, TypeName.ident);
	CBaseVar* BV = NULL;
	static_cast<CBaseType*>(BN)->CreateVar(BV, parent_element);
	//установка требуемых атрибутов переменной
	BV->SetName(VarName.ident);
	BV->SetTypeName(TypeName.pref_ident, TypeName.ident);
	//поскольку запись может присутствовать только как VAR параметр процедуры, требуется
	//принудительная установка признака переменной для записи
	//(для указателя значение данного параметра не играет роли)
	BV->is_var = true;
	//проверка типа переменной для доп. настройки
	switch (BV->name_id) {
	case id_CPointerVar:
		//установка признака объявления переменной через QualidentType (необходимо для поиска в списке полей записи)
		static_cast<CPointerVar*>(BV)->qualident_type = true;
		//для указателя установка признака ук. на запись (проверено в Init)
		static_cast<CPointerVar*>(BV)->SetIsRecord();
		//установка признака нахождения под охраной
		BV->is_guarded = true;
		break;
	case id_CCommonVar:
		//установка признака переменной
		static_cast<CCommonVar*>(BV)->SetTagName(spec_name.pref_ident, spec_name.ident);
		break;
	default:
		//установка признака нахождения под охраной
		BV->is_guarded = true;
	}//switch
	return BV;
}


//-----------------------------------------------------------------------------
//получение названия модуля (только для импортированной переменной)
const char* CGuard::GetVarModuleName()
{
	return VarName.pref_ident;
}


//-----------------------------------------------------------------------------
//инициализация объекта Guard из потока лексем
int CGuard::Init(CLexBuf *lb)
{
	//инициализация имени тестируемой переменной
	int err_num = VarName.Init(lb, parent_element);
	if (err_num) return err_num;

	//проверка наличия тестируемой переменной
	CBaseName* BN = parent_element->GetGlobalName(VarName.pref_ident, VarName.ident);
	if (!BN) {
		//проверка случая <запись>.<поле> - тогда выдается ": missing" вместо "undeclared"
		if (parent_element->GetGlobalName(VarName.pref_ident)) return s_e_ColonMissing;
		else return s_e_UndeclaredIdent;
	}

	//имя типа запись (для проверки наследования в случае записи или ук. на запись)
	const char* module_name;
	const char* type_name;

	//проверка типа тестируемой переменной
	switch (BN->name_id) {
	case id_CRecordVar:
		//получена переменная типа запись
		if (!static_cast<CRecordVar*>(BN)->is_var) return s_e_GuardVarNotRecOrP;
		type_id = id_CRecordType;
		//запоминание названия типа переменной
		module_name = static_cast<CRecordVar*>(BN)->GetTypeModuleAlias();
		type_name = static_cast<CRecordVar*>(BN)->GetTypeName();
		break;
	case id_CCommonVar:
		//получена переменная типа обобщение
		if (!static_cast<CCommonVar*>(BN)->is_var) return s_e_GuardVarNotRecOrP;
		if (!static_cast<CCommonVar*>(BN)->IsPureCommon()) return s_e_GuardVarNotRecOrP;
		type_id = id_CCommonType;
		break;
	case id_CPointerVar:
		//получение типа, на который указывает данный указатель
		BN = static_cast<CPointerVar*>(BN)->FindType();
		//проверка наличия ук. на обобщение
		if (id_CCommonType == BN->name_id) {
			type_id = id_CCommonType;
			break;
		}
		//получена переменная типа указатель на запись
		type_id = id_CPointerType;
		//запоминание названия типа переменной
		module_name = static_cast<CBaseType*>(BN)->GetModuleAlias();
		type_name = BN->name;
		break;
	default:
		//получена переменная недопустимого типа
		return s_e_GuardVarNotRecOrP;
	}//switch

	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия символа ":"
	if (!lb->ReadLex(li) || lex_k_colon != li.lex) return s_e_ColonMissing;

	//получение типа в охране с проверкой необходимости считывать "<" параметр ">"
	if (id_CCommonType == type_id) {

		//обработка обобщенной переменой или ук. на обобщение

		//проверка наличия "<" - начало признака специализации
		if (!lb->ReadLex(li) || lex_k_lt != li.lex) return s_e_OpAngleMissing;
		//инициализация описания признака
		err_num = spec_name.Init(lb, parent_element);
		if (err_num) return err_num;
		//получение типа в случае наличия обобщенной переменной
		if (id_CCommonVar == BN->name_id) BN = static_cast<CCommonVar*>(BN)->FindType();
		//получение имени типа тестируемой обобщенной переменной
		const char* m_a = static_cast<CBaseType*>(BN)->GetModuleAlias();
		if (m_a) TypeName.pref_ident = str_new_copy(m_a);
		TypeName.ident = str_new_copy(BN->name);
		//проверка, является ли признак допустимым для данного обобщения
		if (id_CCommonType != BN->name_id) return s_m_Error;
		if (!static_cast<CCommonType*>(BN)->FindSpec(spec_name.pref_ident, spec_name.ident, spec_name.ident)) return s_e_SpecTypeTag;
		//проверка наличия ">" - конец признака специализации
		if (!lb->ReadLex(li) || lex_k_gt != li.lex) return s_e_ClAngleMissing;
		//завершение обработки обобщения
		return 0;

	} else {

		//обработка записи или ук. на запись

		//инициализация имени типа тестируемой переменной
		err_num = TypeName.Init(lb, parent_element);
		if (err_num) return err_num;
		//проверка наличия известного имени типа
		BN = parent_element->GetGlobalName(TypeName.pref_ident, TypeName.ident);
		if (!BN) return s_e_UndeclaredIdent;
		//проверка, был ли указан идентификатор типа
		if (!CBaseType::IsTypeId(BN->name_id)) return s_e_IdentNotType;
		//проверка, соответствует ли тип в охране охраняемой переменной
		if (id_CRecordType == type_id) {
			//проверка наличия типа запись
			if (id_CRecordType != BN->name_id) return s_e_GuardTypeNotRec;
		} else {
			//проверка наличия типа указатель
			if (id_CPointerType != BN->name_id) return s_e_GuardTypeNotP;
			//получение типа, на кот. указывает указатель
			BN = static_cast<CPointerType*>(BN)->FindType();
			//проверка, указывает ли ук. на запись
			if (id_CRecordType != BN->name_id) return s_e_GuardTypeNotExt;
		}
		//проверка наличия имени типа (для переменной неименованного типа нет смысла проверять охрану)
		if (!type_name) return s_e_GuardTypeNotExt;
		//проверка, является ли охрана расширением типа охраняемой переменной
		if (!static_cast<CRecordType*>(BN)->IsExtension(module_name, type_name))
			return s_e_GuardTypeNotExt;
		//завершение обработки записи или ук. на запись
		return 0;

	}//else

}//Init


//-----------------------------------------------------------------------------
//Запись кода CGuard
void CGuard::WriteCPP(CPP_files& f)
{
	//начало формирование секции Guard, включая проверку отсутствия NIL
	f.tab_fc();
	fprintf(f.fc, "if (");
	if (VarName.pref_ident) fprintf(f.fc, "%s::", VarName.pref_ident);
	fprintf(f.fc, "%s && ", VarName.ident);

	//проверка наличия обобщенного типа
	if (id_CCommonType == type_id) {
		//WITH с обобщением
		fprintf(f.fc, "(");
		if (VarName.pref_ident) fprintf(f.fc, "%s::", VarName.pref_ident);
		fprintf(f.fc, "%s->O2M_SID == ", VarName.ident);
		if (VarName.pref_ident) fprintf(f.fc, "%s::", VarName.pref_ident);
		fprintf(f.fc, "%s->O2M_SID_", VarName.ident);
	} else {
		//обычный необобщенный WITH
		fprintf(f.fc, "!strcmp(");
		if (VarName.pref_ident) fprintf(f.fc, "%s::", VarName.pref_ident);
		fprintf(f.fc, "%s->O2M_SYS_ID(), ", VarName.ident);
	}

	//получение типа-охраны
	CBaseName* BN = parent_element->GetGlobalName(TypeName.pref_ident, TypeName.ident);

	//генерация кода типа-охраны
	switch (type_id) {
	case id_CCommonType:
		//обработка обобщения
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
		//обработка ук. на запись
		fprintf(f.fc, "\"%s\"", static_cast<CRecordType*>(static_cast<CPointerType*>(BN)->FindType())->GetRuntimeId());
		break;
	default:
		//обработка записи
		fprintf(f.fc, "\"%s\"", static_cast<CRecordType*>(BN)->GetRuntimeId());
	}

	//формирование секции Guard с последовательностью операторов
	fprintf(f.fc, ")) {\n");
}//WriteCPP


//-----------------------------------------------------------------------------
//деструктор объекта |
CGuardPair::~CGuardPair()
{
	delete StatementSeq;
	delete Guard;
}//~CGuardPair


//-----------------------------------------------------------------------------
//инициализация объекта | из потока лексем
int CGuardPair::Init(CLexBuf *lb)
{
	//переменная для получения номера ошибки
	int err_num = 0;

	//проверка наличия выражения
	Guard = new CGuard(parent_element);
	err_num = Guard->Init(lb);
	if (err_num) return err_num;

	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия кл. слова DO
	if (!lb->ReadLex(li) || lex_k_DO != li.lex) return s_e_DO;

	//занесение в WithLoopLink охраняемой переменной
	WithLoopLink.AddName(Guard->GetVarModuleName(), Guard->CreateGuardVar());

	//проверка наличия послед. операторов
	StatementSeq = new CStatementSeq(&WithLoopLink);
	err_num = StatementSeq->Init(lb);
	if (err_num) return err_num;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//деструктор объекта WITH
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
//проверка наличия оператора RETURN
EHaveRet CWithStatement::HaveRet() const
{
	//признаки наличия веток операторов без и с RETURN
	bool HaveNo = false;
	bool HaveYes = false;
	//проверка наличия RETURN среди содержимого секций ELSIF
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
	//проверка наличия RETURN в секции ELSE (если есть)
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
	//т.к. все случаи NotAll проверены выше, проверяем наличие No и Yes
	return HaveYes ? HaveNo ? hr_NotAll : hr_Yes : hr_No;
}


//-----------------------------------------------------------------------------
//инициализация объекта WITH из потока лексем
int CWithStatement::Init(CLexBuf *lb)
{
	//создание списка | с первым элементом (для WITH ... DO)
	CGuardPair* GPair = new CGuardPair(parent_element);
	int err_num = GPair->Init(lb);
	if (err_num) {
		delete GPair;
		return err_num;
	}

	GuardPairStore = new CBaseVector;
	GuardPairStore->push_back(GPair);

	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//чтение очередной лексемы (ключевого слова)
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;

	//проверка наличия | (список уже создан)
	while (lex_k_vertical == li.lex) {
		//проверка очередной пары Guard DO StatementSeq
		GPair = new CGuardPair(parent_element);
		err_num = GPair->Init(lb);
		if (err_num) {
			delete GPair;
			return err_num;
		}
		GuardPairStore->push_back(GPair);

		//чтение очередной лексемы (ключевого слова)
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;
	}

	if (lex_k_ELSE == li.lex) {
		ElseStatementSeq = new CStatementSeq(parent_element);
		err_num = ElseStatementSeq->Init(lb);
		if (err_num) return err_num;
		//чтение очередной лексемы (ключевого слова)
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;
	}

	//проверка наличия конца (остальные варианты проверены)
	if (lex_k_END != li.lex) return s_e_END;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//Запись кода CWithStatement
void CWithStatement::WriteCPP(CPP_files& f)
{
	//запись кода первой охраны (всегда должна присутствовать)
	CBaseVector::const_iterator ci = GuardPairStore->begin();
	(*ci)->WriteCPP(f);
	//запись кода оставшихся охран
	for (++ci; ci != GuardPairStore->end(); ++ci) {
		fprintf(f.fc, " else\n");
		(*ci)->WriteCPP(f);
	}

	//запись последовательности операторов из раздела ELSE (если есть)
	fprintf(f.fc, " else\n");
	if (ElseStatementSeq) {
		f.tab_fc();
		fprintf(f.fc, "{\n");

		ElseStatementSeq->WriteCPP(f);

		f.tab_fc();
		fprintf(f.fc, "}");
	} else {
		//запись кода выхода из программы при отсутствии блока ELSE
		f.tab_fc();
		fprintf(f.fc, "\texit(0)");
	}
}//WriteCPP


//-----------------------------------------------------------------------------
//деструктор объекта StatementSeq
CStatementSeq::~CStatementSeq()
{
	CBaseVector::const_iterator ci;
	for(ci = StatStore.begin(); ci != StatStore.end(); ++ci)
		delete *ci;
}//~CStatementSeq


//-----------------------------------------------------------------------------
//проверка наличия оператора RETURN
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
//инициализация объекта из потока лексем
int CStatementSeq::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	while(true){
		//инициализация одного оператора
		int err_num	= StatementInit(lb);
		if (err_num) return err_num;
		DECL_SAVE_POS
		//чтение кл. слова ";" (если есть)
		if (!lb->ReadLex(li) || lex_k_semicolon != li.lex) {
			RESTORE_POS
			return 0;
		}
	}

	return 0;
}//CStatementSeq::Init


//-----------------------------------------------------------------------------
//инициализация объекта Statement из потока лексем
int CStatementSeq::StatementInit(CLexBuf *lb)
{
	DECL_SAVE_POS
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//получение ключевого слова или ид.
	if (!lb->ReadLex(li) || (lex_k_dot > li.lex && lex_i != li.lex)) return s_e_Statement;

	//пока никакой оператор не найден
	CBase *Base = NULL;

	//проверка типа полученной лексемы
	switch (li.lex) {

	//проверка наличия вызова или присваивания
	case lex_i:
	case lex_k_op_brace:
		RESTORE_POS
		//проверка наличия вызова обобщенной процедуры
		if (lex_k_op_brace == li.lex)
			Base = new CCallStatement(parent_element, NULL, false, true);
		else {
			//получение Designator, по которому определяется вызов процедуры или присваивание
			CDesignator* Des = new CDesignator(parent_element, false);
			int err_num = Des->Init(lb);
			if (err_num) {
				delete Des;
				return err_num;
			}
			//проверка наличия оператора вызова или оператора присваивания
			if (Des->IsProcName())
				Base = new CCallStatement(parent_element, Des, true, true);
			else
				Base = new CAssignStatement(parent_element, Des);
		}//else
		break;

	//проверка наличия стандартной процедуры (StdProc)
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

	//проверка наличия оператора (Statement)
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

	//проверка наличия вызова процедуры-функции (ошибка)
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

	//инициализация созданного объекта (если он был создан)
	if (Base) {
		int err_num = Base->Init(lb);
		if (err_num) {
			delete Base;
			return err_num;
		}
		//занесение считанного объекта в список
		StatStore.push_back(Base);
	} else {
		//найден пустой оператор (ошибки нет)
		RESTORE_POS
	}

	return 0;
}//CStatementSeq::StatementInit


//-----------------------------------------------------------------------------
//Запись кода CStatementSeq
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
//инициализация объекта EXIT из потока лексем
int CExitStatement::Init(CLexBuf *lb)
{
	//проверка нахождения EXIT в StatementSeq от оператора LOOP
	const CBaseName* BN = this->parent_element;
	while (BN) {
		//проверка наличия среди parent_element объекта, использующегося для связи с оператором LOOP
		if (id_CWithLoopLink == BN->name_id && static_cast<const CWithLoopLink*>(BN)->UnderLoop()) {
			//сохранение UID оператора LOOP (необходимо для генерации кода C++)
			UID = static_cast<const CWithLoopLink*>(BN)->LoopUID;
			return 0;
		}
		BN = BN->parent_element;
	}
	return s_e_EXIT_NotInLoop;
}//Init


//-----------------------------------------------------------------------------
//Запись кода CExitStatement
void CExitStatement::WriteCPP(CPP_files& f)
{
	f.tab_fc();
	fprintf(f.fc, "goto O2M_EXIT_%i", UID);
}//WriteCPP


//-----------------------------------------------------------------------------
//деструктор объекта FOR
CForStatement::~CForStatement()
{
	delete[] var_name;
	delete StatementSeq;
	delete ForExpr;
	delete ToExpr;
}//~CForStatement


//-----------------------------------------------------------------------------
//инициализация объекта FOR из потока лексем
int CForStatement::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//получение ид-ра
	if (!lb->ReadLex(li) || (lex_i != li.lex)) return s_e_IdentExpected;
	var_name = str_new_copy(li.st);

	//поиск ид-ра в таблице имен и проверка его типа
	CBaseName* BN = parent_element->GetGlobalName(var_name);
	if (!BN) return s_e_UndeclaredIdent;
	if (!CBaseVar::IsIntId(BN->name_id)) return s_e_FOR_VarNotInt;

	//проверка наличия кл. слова :=
	if (!lb->ReadLex(li) || lex_k_assign != li.lex) return s_e_AssignMissing;

	//проверка наличия выражения For
	ForExpr = new CExpr(parent_element);
	int err_num = ForExpr->Init(lb);
	if (err_num) return err_num;

	//проверка наличия кл. слова TO
	if (!lb->ReadLex(li) || lex_k_TO != li.lex) return s_e_TO;

	//проверка наличия выражения To
	ToExpr = new CExpr(parent_element);
	err_num = ToExpr->Init(lb);
	if (err_num) return err_num;

	//чтение очередной лексемы (ключевого слова)
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_DO;

	//проверка наличия ConstExpr, если получен BY
	if (lex_k_BY == li.lex) {
		//создание константного выражения для BY
		CBaseVar* ByVar;
		err_num = ConstSelector(lb, ByVar, parent_element);
		if (err_num) return err_num;
		//проверка получения целочисленной константы
		if (!CBaseVar::IsIntId(ByVar->name_id)) {
			delete ByVar;
			return s_e_ExprNotIntConst;
		}
		step = ByVar->GetIntValue();
		delete ByVar;
		//проверка шага, равного 0
		if (!step) return s_e_FOR_BY_Zero;
		//чтение очередной лексемы (ключевого слова)
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_DO;
	} else
		step = 1;	//шаг по умолчанию

	//проверка наличия кл. слова DO
	if (lex_k_DO != li.lex) return s_e_DO;

	//проверка наличия послед. операторов
	StatementSeq = new CStatementSeq(parent_element);
	err_num = StatementSeq->Init(lb);
	if (err_num) return err_num;

	//проверка наличия кл. слова END
	if (!lb->ReadLex(li) || lex_k_END != li.lex) return s_e_END;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//Запись кода CForStatement
void CForStatement::WriteCPP(CPP_files& f)
{
	//временная переменная (условие TO не должно меняться внутри цикла)
	f.tab_fc();
	fprintf(f.fc, "{long O2M_FOR=");
	ToExpr->WriteCPP(f);
	fprintf(f.fc, ";\n");

	//генерация оператора for
	f.tab_fc();
	fprintf(f.fc, "for(%s=", var_name);
	ForExpr->WriteCPP(f);

	//обработка числового значения шага
	fprintf(f.fc, "; %s%c=O2M_FOR; %s+=%li) {\n", var_name, (step > 0) ? '<' : '>', var_name, step);

	//запись последовательности операторов
	StatementSeq->WriteCPP(f);

	f.tab_fc();
	fprintf(f.fc, "}}");
}//WriteCPP


//-----------------------------------------------------------------------------
//статическая переменная для создания уникального ID для каждого оператора LOOP
int CLoopStatement::CurrentUID = 0;


//-----------------------------------------------------------------------------
//деструктор объекта LOOP
CLoopStatement::~CLoopStatement()
{
	delete StatementSeq;
}//~CLoopStatement


//-----------------------------------------------------------------------------
//инициализация объекта LOOP из потока лексем
int CLoopStatement::Init(CLexBuf *lb)
{
	//установка UID для передачи в StatementSeq (в нем скорее всего есть операторы EXIT)
	WithLoopLink.LoopUID = CurrentUID++;

	//проверка наличия послед. операторов
	StatementSeq = new CStatementSeq(&WithLoopLink);
	int err_num = StatementSeq->Init(lb);
	if (err_num) return err_num;

	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия кл. слова END
	if (!lb->ReadLex(li) || lex_k_END != li.lex) return s_e_END;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//Запись кода CLoopStatement
void CLoopStatement::WriteCPP(CPP_files& f)
{
	f.tab_fc();
	fprintf(f.fc, "while(true) {\n");

	StatementSeq->WriteCPP(f);

	f.tab_fc();
	fprintf(f.fc, "} O2M_EXIT_%i:", WithLoopLink.LoopUID);
}//WriteCPP


//-----------------------------------------------------------------------------
//деструктор объекта REPEAT
CRepeatStatement::~CRepeatStatement()
{
	delete StatementSeq;
	delete Expr;
}//~CRepeatStatement


//-----------------------------------------------------------------------------
//инициализация объекта REPEAT из потока лексем
int CRepeatStatement::Init(CLexBuf *lb)
{
	//проверка наличия послед. операторов
	StatementSeq = new CStatementSeq(parent_element);
	int err_num = StatementSeq->Init(lb);
	if (err_num) return err_num;

	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия кл. слова UNTIL
	if (!lb->ReadLex(li) || lex_k_UNTIL != li.lex) return s_e_UNTIL;

	//проверка наличия выражения
	Expr = new CExpr(parent_element);
	err_num = Expr->Init(lb);
	if (err_num) return err_num;

	//проверка типа выражения
	return (Expr->GetResultId() == id_CBooleanVar) ? 0 : s_e_UNTIL_ExprType;
}//Init


//-----------------------------------------------------------------------------
//Запись кода CRepeatStatement
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
//деструктор объекта RETURN
CReturnStatement::~CReturnStatement()
{
	delete Expr;
}//~CReturnStatement


//-----------------------------------------------------------------------------
//инициализация объекта RETURN из потока лексем
int CReturnStatement::Init(CLexBuf *lb)
{
	//получение ук. на родительский объект
	const CBaseName *BN = parent_element;
	//поиск процедуры (или модуля) среди иерархии родительских объектов
	while (BN->parent_element && !CProcedure::IsProcId(BN->name_id))
		BN = BN->parent_element;

	//проверка включенности оператора в процедуру (функцию)
	if (CProcedure::IsProcId(BN->name_id)) {
		//получение результата выражения и проверка включенности в функцию
		const EName_id ResultId = static_cast<const CProcedure*>(BN)->GetResultId();
		if (id_CBaseName != ResultId) {
			//проверка необходимости интерпретировать выражение как символьное
			if (id_CCharVar == ResultId)
				Expr = new CExpr(parent_element, ek_Char, false);
			else
				Expr = new CExpr(parent_element);
			//инициализация выражения
			int err_num = Expr->Init(lb);
			if (err_num) return err_num;
			//можно в случае id_CCharVar == ResultId выводить не err_num,
			//а более подходящее сообщение о несовместимости по присваиванию
			/**/
			//здесь необходима проверка совместимости по присваиванию Expr->GetResultId с ResultId
			/**/
			//завершение обработки выражения
			return 0;
		}
	}

	//данный оператор находится в процедуре или модуле

	DECL_SAVE_POS

	//проверяем отсутствие корректного выражения
	Expr = new CExpr(parent_element);
	if (!Expr->Init(lb)) {
		//выражение корректно - выводим сообщение об ошибке (с проверкой нахождения в модуле)
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
//Запись кода CReturnStatement
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
//деструктор объекта WHILE
CWhileStatement::~CWhileStatement()
{
	delete StatementSeq;
	delete Expr;
}//~CWhileStatement


//-----------------------------------------------------------------------------
//инициализация объекта WHILE из потока лексем
int CWhileStatement::Init(CLexBuf *lb)
{
	//проверка наличия выражения
	Expr = new CExpr(parent_element);
	int err_num = Expr->Init(lb);
	if (err_num) return err_num;

	//проверка типа выражения
	if (Expr->GetResultId() != id_CBooleanVar) return s_e_WHILE_ExprType;

	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия кл. слова DO
	if (!lb->ReadLex(li) || lex_k_DO != li.lex) return s_e_DO;

	//проверка наличия послед. операторов
	StatementSeq = new CStatementSeq(parent_element);
	err_num = StatementSeq->Init(lb);
	if (err_num) return err_num;

	//проверка наличия кл. слова END
	if (!lb->ReadLex(li) || lex_k_END != li.lex) return s_e_END;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//Запись кода CWhileStatement
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
//инициализация объекта Assign (оператор присваивания) из потока лексем
int CAssignStatement::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия кл. слова ":="
	if (!lb->ReadLex(li) || lex_k_assign != li.lex) return s_e_AssignMissing;

	//проверка отсутствия эл-тов только для чтения в левой части присваивания
	if (Designator->IsReadOnly()) return s_e_AssignReadonly;

	//получение объекта, на который ссылается обозначение
	CBaseName *BN = Designator->FindLastName();
	//проверка наличия объекта и отсутствия признака константы, если он отсутствует,
	//объект именованный, т.к. только константы могут быть неименованными
	if (!BN || (CBaseVar::IsVarId(BN->name_id) && static_cast<CBaseVar*>(BN)->is_const)) return s_e_AssignConst;

	//проверка присвоения обобщенной переменной через ук. на обобщенную переменную
	if (id_CCommonVar == BN->name_id) {
		cv_compound_name = static_cast<CCommonVar*>(BN)->GetCPPCompoundName();
		if (!cv_compound_name) return s_e_SpecTypeExpected;
	}

	//проверка присвоения текстовой строки массиву
	if (id_CArrayVar == Designator->GetResultId())
		str_to_array = true;
	if (id_CPointerVar == BN->name_id && static_cast<CPointerVar*>(BN)->IsArrayPointer())
		str_to_array = true;

	//проверка требуемой интерпретации выражения 
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

	//инициализация выражения
	int err_num = Expr->Init(lb);
	if (err_num) return err_num;
	EName_id ERI = Expr->GetResultId();

	//проверка отсутствия указателя в случае присвоения обобщенной переменной
	if (cv_compound_name && id_CRecordVar != ERI) return s_e_Incompatible;


	////////////////////////////////////////
	//проверка совместимости по присваиванию
	/**/

	//пока при присвоении указателей не проверяется условие расширения типов
	if (id_CPointerVar == Designator->GetResultId() && id_CPointerVar != ERI && id_CArrayVar != ERI) return s_e_Incompatible;

	//проверка присвоения текстовой строки массиву
	if (id_CArrayVar == BN->name_id && id_CArrayVar != ERI && id_CCharVar != ERI) return s_e_Incompatible;

	//проверка совместимости по присваиванию для числовых типов
	/**/	//временно, пока не реализована полная проверка
	if (CBaseVar::IsDigitId(BN->name_id))
		if (!IsId1IncloseId2(BN->name_id, ERI))
			return s_e_Incompatible;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//Запись кода CAssignStatement
void CAssignStatement::WriteCPP(CPP_files& f)
{
	////////////////////////////////////////////////////
	//генерация кода присвоения текстовой строки массиву
	if (str_to_array) {
		WriteCPP_array(f);
		return;
	}

	//////////////////////////////////////
	//генерация кода обычного присваивания
	f.tab_fc();
	//в случае присвоения обобщенной переменной генерируем исп. значения переменной
	if (cv_compound_name) fprintf(f.fc, "*");
	//запись кода обозначения без генерации кода охраны (если есть охрана)
	Designator->WriteCPP_Guardless(f);
	//в случае присвоения обобщенной переменной генерируем обращение к соотв. члену
	if (cv_compound_name) fprintf(f.fc, "->O2M_SPEC_%s", cv_compound_name);
	//запись операции присвоения
	fprintf(f.fc, " = ");
	//запись выражения
	Expr->WriteCPP(f);
}//WriteCPP


//-----------------------------------------------------------------------------
//запись кода в случае присвоения текстовой строки массиву
void CAssignStatement::WriteCPP_array(CPP_files &f)
{
	f.tab_fc();
	fprintf(f.fc, "COPY(");

	//запись значения источника
	Expr->WriteCPP(f);
	fprintf(f.fc, ", ");
	//получение источника (переменной-массива или указателя)
	CBaseName* BN = Expr->FindLastName();
	WriteCPP_COPY_Par(f, BN);
	fprintf(f.fc, ", ");

	//запись значения приемника
	Designator->WriteCPP(f);
	fprintf(f.fc, ", ");
	//получение приемника (переменной-массива или указателя)
	BN = Designator->FindLastName();
	WriteCPP_COPY_Par(f, BN);
	fprintf(f.fc, ")");

}


//-----------------------------------------------------------------------------
//Запись кода CGuardPair
void CGuardPair::WriteCPP(CPP_files &f)
{
	//запись кода проверки динамического типа переменной
	Guard->WriteCPP(f);

	//запись последовательности операторов
	if (StatementSeq) StatementSeq->WriteCPP(f);

	//запись закрывающей скобки блока операторов (открывающая скобка записана в Guard->WriteCPP)
	f.tab_fc();
	fprintf(f.fc, "}");
}


