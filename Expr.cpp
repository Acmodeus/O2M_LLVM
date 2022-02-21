//=============================================================================
// Описание деклараций для построения выражений
//=============================================================================

#include "Expr.h"
#include "Type.h"
#include "Var.h"
#include "StdProc.h"


//-----------------------------------------------------------------------------
//деструктор
CExprList::~CExprList()
{
	CExprVector::iterator i;
	for(i = ExprVector->begin(); i != ExprVector->end(); ++i)
		delete *i;
	delete ExprVector;
}//~CExprList


//-----------------------------------------------------------------------------
//инициализация объекта ExprList из потока лексем
int CExprList::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//текущий номер выражения в списке фактических параметров
	int curr_fp_number = 0;

	//цикл обработки выражений
	while (true) {

		//по умолчанию обычная интерпретация выражения
		EExprKind ek = ek_Normal;

		//получение типа формального параметра (если есть) для определения требуемой интерпретации выражения
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

		//создание очередного выражения с учетом требуемой интерпретации
		CExpr* Expr = new CExpr(parent_element, ek, PFP != NULL);

		//инициализация очередного выражения
		int err_num = Expr->Init(lb);
		if (err_num) {
			delete Expr;
			return err_num;
		}

		/**/
		//ввести проверку совместимости по присваиванию

		//сохранение инициализированного выражения в списке
		ExprVector->push_back(Expr);

		//проверка наличия "," (тогда должно быть и следующее выражение)
		DECL_SAVE_POS
		if (!lb->ReadLex(li) || lex_k_comma != li.lex) {
			RESTORE_POS
			return 0;
		}

		//есть след. выражение - увеличиваем текущий номер
		curr_fp_number++;
	}//while

}//Init


//-----------------------------------------------------------------------------
//запись кода CExprList
void CExprList::WriteCPP(CPP_files& f)
{
	CExprVector::const_iterator ci = ExprVector->begin();
	//запись кода первого выражения (если есть)
	if (ci != ExprVector->end()) (*ci)->WriteCPP(f);
	//запись кода последующих выражений (если есть)
	for (++ci; ci != ExprVector->end(); ++ci) {
		fprintf(f.fc, ", ");
		(*ci)->WriteCPP(f);
	}
}//WriteCPP


//-----------------------------------------------------------------------------
//запись кода одного фактического параметра при вызове процедуры, для открытых массивов записываются доп. параметры
//FactExpr - выражение фактического параметра , FormalPar - формальный параметр
void CExprList::WriteCPP_proc_OneParam(CPP_files& f, CExpr* FactExpr, CBaseName* FormalPar)
{
	//проверка наличия открытого массива в формальных параметрах
	bool is_open_arr = (id_CArrayVar == FormalPar->name_id && 0 == static_cast<CArrayVar*>(FormalPar)->ArrayType->size);
	//получение фактического параметра, передаваемого в процедуру - может быть NULL в случае выражения,
	//может быть типом в случае использования эл-та массива
	CBaseVar* FactPar = static_cast<CBaseVar*>(FactExpr->FindLastName());

	//проверка, объявлено ли выражение в процедуре как VAR RECORD (для записи &)
	if (id_CRecordVar == FormalPar->name_id && static_cast<CBaseVar*>(FormalPar)->is_var) {
		//выражение объявлено как VAR RECORD, проверяем тип фактического параметра
		if (FactPar)
			switch (FactPar->name_id) {
			case id_CRecordVar:
				//взятие адреса не требуется в случае параметра-переменной записи
				if (FactPar->is_var) break;
			case id_CPointerVar:
				//указатель в качестве факт. параметра может быть только с '^' (совместимость по присваиванию)
			case id_CRecordType:
				//подстановка кода взятия адреса
				fprintf(f.fc, "&");
			}//switch
	}


	//учитывая, что многомерные массивы передаются как одномерные - этого не должно требоваться
	/**/
	/*vvvvvvvvvvvvvv*/
	//запись приведения типа для открытого массива (необходимо для многомерного массива)
	if (is_open_arr && FactPar && id_CArrayVar == FactPar->name_id) {
		//получение ук. на тип
		CBaseType* BT = static_cast<CArrayVar*>(FactPar)->ArrayType->FindLastType();
		CBaseVar* BV = NULL;
		//создание временной переменной по полученному типу
		BT->CreateVar(BV, BT->parent_element);
		fprintf(f.fc, "(");
		if (BV->GetTypeModuleAlias()) fprintf(f.fc, "%s::", BV->GetTypeModuleAlias());
		fprintf(f.fc, "%s*)", BV->GetTypeName());
		delete BV;
	}
	/*^^^^^^^^^^^^^^*/


	//проверка случая "открытый массив + символьная константа" (не константа отсечется на этапе проверки совместимости выражений)
	if (!(is_open_arr && id_CCharVar == FactExpr->GetResultId()))
		//запись кода выражения
		FactExpr->WriteCPP(f);

	//проверка подставления специализированной обобщенной переменной в качестве записи специализирующего типа
	if (FactPar && id_CCommonVar == FactPar->name_id && id_CRecordVar == FormalPar->name_id)
		fprintf(f.fc, "->O2M_SPEC_%s", static_cast<CCommonVar*>(FactPar)->GetCPPCompoundName());

	//обработка открытого массива в формальных параметрах (запись списка размерностей)
	if (is_open_arr) {

		//получение типа первого измерения фактического параметра
		const CArrayType* AT;
		if (FactPar && id_CArrayVar == FactPar->name_id)
			AT = static_cast<CArrayVar*>(FactPar)->ArrayType;
		else {

			//получение имени (могут быть строковая или символьная переменная или тип массив)
			CBaseName* BN = FactExpr->FindLastName();

			//проверка получения символьной константы, интерпретируем ее как массив в 1 символ
			if (id_CCharVar == BN->name_id) {
				fprintf(f.fc, "\"%c\", 2", static_cast<CCharVar*>(BN)->ConstValue);	//1 символ + '\0'
				return;
			}

			//была получена или переменная или тип массив (должно быть проверено при инициализации)
			if (id_CArrayVar == BN->name_id)
				AT = static_cast<CArrayVar*>(BN)->ArrayType;
			else
				AT = static_cast<CArrayType*>(BN);
		}

		//запись списка размерностей (если есть ? -> ArrOfChar + "String")
		if (AT) {
			int dimension = 0;	//отсчет размерностей с 0
			while (id_CArrayType == AT->name_id) {
				//вывод размерности (в виде числа или имени переменной, содержащей размерность)
				if (AT->size)
					fprintf(f.fc, ", %i", AT->size);
				else {
					fprintf(f.fc, ", O2M_ARR_%i_", dimension);
					FactExpr->WriteCPP(f);	//для открытого массива FactExpr должно содержать название (?)
				}
				//переход к следующей размерности
				++dimension;
				AT = static_cast<CArrayType*>(AT->Type);
			}//while
		}

	}//if
}


//-----------------------------------------------------------------------------
//запись кода, если CExprList содержит список параметров при вызове процедуры
void CExprList::WriteCPP_proc(CPP_files& f, CFormalPars* FP)
{
	CExprVector::const_iterator ci;
	int number = 0;
	for (ci = ExprVector->begin(); ci != ExprVector->end(); ++ci, ++number) {
		//разделитель вставляется только после первого выражения
		fprintf(f.fc, "%s", number ? ", " : "");
		//запись очередного выражения
		WriteCPP_proc_OneParam(f, *ci, FP->GetNameByIndex(number));
	}
}//WriteCPP_proc


//-----------------------------------------------------------------------------
//запись кода, если CExprList содержит список параметров при вызове параметрической процедуры
void CExprList::WriteCPP_common_proc(CPP_files& f, CFormalPars* FP)
{
	CExprVector::const_iterator ci;
	int number = 0;
	for (ci = ExprVector->begin(); ci != ExprVector->end(); ++ci, ++number) {
		//разделитель вставляется только после первого выражения
		fprintf(f.fc, "%s&", number ? ", " : "");
		//запись очередного выражения
		WriteCPP_proc_OneParam(f, *ci, FP->GetNameByIndex(number));
	}
}


//-----------------------------------------------------------------------------
//запись кода CExprList для индексов массива, IsOpenArray - признак открытого массива,
//ArrayName - название массива (исп. для получения размера массива через переменную)
void CExprList::WriteCPP_index(CPP_files& f, bool IsOpenArray, const char* ArrayName/*CBaseName* BN*/)
{
	fprintf(f.fc, "[");

	//проверка наличия открытого массива
	if (IsOpenArray) {//массив открытый => преобразование координат

		//запись кода первого выражения (если есть)
		CExprVector::const_iterator ci = ExprVector->begin();
		if (ci != ExprVector->end()) {
			fprintf(f.fc, "(");
			(*ci)->WriteCPP(f);
			fprintf(f.fc, ")");
		}
		//запись кода последующих выражений (если есть)
		int i;
		//для получения смещения по первой размерности
		for (i = 1; i < ExprVector->size(); i++) fprintf(f.fc, "*O2M_ARR_%i_%s", i, ArrayName);
		//перебор оставшихся размерностей
		int dimention = 1;	//текущая размерность
		for (++ci; ci != ExprVector->end(); ++ci) {
			fprintf(f.fc, "+(");
			(*ci)->WriteCPP(f);
			fprintf(f.fc, ")");
			++dimention;
			//для получения смещения по текущей размерности
			for (i = dimention; i < ExprVector->size(); i++) fprintf(f.fc, "*O2M_ARR_%i_%s", i, ArrayName);
		}//for

	} else {//массив не открытый => обычные индексы

		//запись кода первого выражения (если есть)
		CExprVector::const_iterator ci = ExprVector->begin();
        if (ci != ExprVector->end())
            (*ci)->WriteCPP(f);
		//запись кода последующих выражений (если есть)
		for (++ci; ci != ExprVector->end(); ++ci) {
			fprintf(f.fc, "][");
			(*ci)->WriteCPP(f);
		}//for

	}//else

	fprintf(f.fc, "]");
}//WriteCPP_index


//-----------------------------------------------------------------------------
//деструктор
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
//инициализация объекта Factor из потока лексем
int CFactor::Init(CLexBuf *lb)
{
	DECL_SAVE_POS
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//////////////////////////////////////////
	//чтение лексемы для проверки типа фактора
	if (!lb->ReadLex(li)) return s_e_Factor;

	///////////////////////////////////////////////////////
	//NIL - данная проверка не зависит от значения ExprKind
	if (lex_k_NIL == li.lex) {
		FactorKind = fk_ConstVar;
		ConstVar = new CPointerVar(parent_element);
		ConstVar->is_const = true;
		return CalcResultId();
	}

	//номер ошибки будет использоваться внутри switch'a
	int err_num;

	///////////////////////////////////////////////////////////////////////////
	//проверка необходимости интерпретации выражения как процедурной переменной
	if (ek_ProcedureVar == ExprKind) {
		RESTORE_POS
		//создание и инициализация Designator
		Designator = new CDesignator(parent_element, in_fact_pars);
		Designator->req_proc_var = true;
		err_num = Designator->Init(lb);
		if (err_num) return err_num;
		//проверка допустимости типа результата с учетом требуемой интерпретации
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
	//получение целочисленного значения с учетом системы счисления

	//для хранения числа в случае целочисленной или символьной константы
	long dec_num = 0;
	//проверка необходимости преобразования из 16# в 10# систему счисления
	if (lex_h == li.lex || lex_c == li.lex) {
		//преобразование цифровой записи символа из 16# в 10# систему счисления
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
	} else	//получена строка с десятичным числом - преобразуем в число
		if (lex_d == li.lex) dec_num = atoi(li.st);

	//////////////////////////////////
	//проверка типа полученной лексемы
	switch (li.lex) {

	////////////////////////////////////////////////////
	//проверка символа (его вид определяет тип Factor'a)

	//'{' конструктор множества или вызов обобщенной функции вида {a,b}.ProcName()
	case lex_k_op_brace:
		//вначале проверяем наличие конструктора SET
		ConstVar = new CSetVar(parent_element);
		err_num = static_cast<CSetVar*>(ConstVar)->SetInit(lb);
		//проверка наличия за скобкой "}" точки (признак вызова обобщенной процедуры)
		if (!err_num) {
			DECL_SAVE_POS
			if (!lb->ReadLex(li) || lex_k_dot != li.lex) {
				//точки не обнаружено
				RESTORE_POS
				//теперь FactorKind известен, завершаем обработку
				FactorKind = fk_ConstVar;
				return CalcResultId();
			}//else
		}
		//была обнаружена ошибка или точка - имеем дело с вызовом обобщенной процедуры
		RESTORE_POS
		//уничтожаем переменную
		delete ConstVar;
		ConstVar = NULL;
		//создание и инициализация CallStatement
		Call = new CCallStatement(parent_element, NULL, false, false);
		err_num = Call->CCallStatement::Init(lb);
		if (err_num) return err_num;
		//получение созданного в CallStatement обозначения
		Designator = Call->GiveDesAway();
		//теперь FactorKind известен, завершаем обработку
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
	//проверка константы (литерала)

	//Digit (SHORTINT, INTEGER, LONGINT)
	case lex_d:
	case lex_h:
		FactorKind = fk_ConstVar;
		//проверка минимального типа, достаточного для хранения числа
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
		//установка признака константности
		ConstVar->is_const = true;
		//конец обработки целочисленной константы
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
		//проверка переполнения символа
		if ((unsigned char)dec_num != dec_num) return l_e_CharConstTooLong;
		//создание переменной
		ConstVar = new CCharVar(parent_element);
		static_cast<CCharVar*>(ConstVar)->SetConstValue(dec_num);
		return CalcResultId();

	//String (ARRAY N OF CHAR) или Char (CHAR)
	case lex_s:
		FactorKind = fk_ConstVar;
		//проверка требуемого типа выражения
		if (ek_Char == ExprKind) {
			//проверка допустимого размера строки символов
			if (strlen(li.st) != 1) return s_e_ExprCompatibility;
			//создание переменной (символа)
			ConstVar = new CCharVar(parent_element);
			static_cast<CCharVar*>(ConstVar)->SetConstValue(li.st[0]);
			//конец обработки
			return CalcResultId();
		}
		//создание переменной
		ConstVar = new CArrayVar(parent_element);
		static_cast<CArrayVar*>(ConstVar)->SetConstValue(li.st);
		//создание типа "массив символов" и вставка его в переменную
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
	//проверка наличия стандартной процедуры-функции
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
	//инициализация в случае нахождения стандартной процедуры-функции
	if (FactorKind == fk_StdProcFunc) {
		int err_num = StdProcFunc->Init(lb);
		if (err_num) return err_num;
		return CalcResultId();
	}

	////////////////////////////////////////////
	//Designator (вызов функции или охрана типа)
	if (lex_i == li.lex || lex_k_op_brace == li.lex) {
		RESTORE_POS
		//инициализируем обозначение
		Designator = new CDesignator(parent_element, in_fact_pars);
		int err_num = Designator->Init(lb);
		if (err_num) return err_num;
		//проверка наличия обозначения процедуры (требуется создание и инициализация CallStatement)
		if (Designator->IsProcName()) {
			Call = new CCallStatement(parent_element, Designator, false, false);
			err_num = Call->Init(lb);
			if (err_num) return err_num;
		}
		//конец обработки Designatora
		FactorKind = fk_Designator;
		return CalcResultId();
	}//if

	return s_e_Factor;
}//Init


//-----------------------------------------------------------------------------
//вычисление ResultId с проверкой ошибок
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
	//проверка наличия допустимого типа результата
	return (id_CBaseName == ResultId) ? s_e_ExprCompatibility : 0;
}


//-----------------------------------------------------------------------------
//проверка, является ли объект, содержащийся в CFactor, read-only
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
//Запись кода CFactor
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
		//проверка наличия вызова процедуры функции (если нет - запись кода обозначения)
		if (Call)
			Call->WriteCPP(f);
		else
			Designator->WriteCPP(f);
		break;		
	}//switch
}


//-----------------------------------------------------------------------------
//применение к указанной переменной операции, с проверкой константности
int CTermPair::ApplyOperation(CBaseVar *&BaseConst) const
{
	//для хранения промежуточного значения: (BaseConst) = (BaseConst) <MulOp> (fac)
	CBaseVar* fac;

	//получение константы из множителя
	int err_num = Factor->CreateConst(fac);
	if (err_num) return err_num;

	//подразумевается, что проверка допустимости типов выполнена в CTermPair::Init
	switch (MulOp) {

	//операция: *
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
			//все варианты должны быть обработаны выше
			return s_m_Error;
		}
		break;

	//операция: /
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
			//все варианты должны быть обработаны выше
			return s_m_Error;
		}
		break;

	//операция: DIV
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
			//все варианты должны быть обработаны выше
			return s_m_Error;
		}
		break;

	//операция: MOD
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
			//все варианты должны быть обработаны выше
			return s_m_Error;
		}
		break;

	//операция: &
	case mop_AND:
		switch (BaseConst->name_id) {
		case id_CBooleanVar:
			if (id_CBooleanVar == fac->name_id)
				static_cast<CBooleanVar*>(BaseConst)->ConstValue = static_cast<CBooleanVar*>(BaseConst)->ConstValue && static_cast<CBooleanVar*>(fac)->ConstValue;
			else
				return s_m_Error;
			break;
		default:
			//все варианты должны быть обработаны выше
			return s_m_Error;
		}
		break;

	//все варианты должны быть обработаны выше
	default:
		return s_m_Error;
	}//switch

	//уничтожение промежуточного значения
	delete fac;

	return 0;
}


//-----------------------------------------------------------------------------
//инициализация объекта TermPair из потока лексем
int CTermPair::Init(CLexBuf *lb)
{
	//получение текущего множителя
	Factor = new CFactor(parent_element, ek_Normal, false);
	int err_num = Factor->Init(lb);
	if (err_num) return err_num;

	//получение типа результата множителя (для скорости)
	const EName_id res_id = Factor->GetResultId();

	//проверка типа результата
	switch (MulOp) {

	//операции: *, /
	case mop_M:
	case mop_D:
		//операции применимы ко множествам и числам
		if (id_CSetVar == res_id || CBaseVar::IsDigitId(res_id)) {
			ResultId = res_id;
			return 0;
		} else return s_e_ExprCompatibility;

	//операции: DIV, MOD
	case mop_DIV:
	case mop_MOD:
		//операции применимы к целым числам
		if (CBaseVar::IsIntId(res_id)) {
			ResultId = res_id;
			return 0;
		} else return s_e_ExprCompatibility;

	//операция: &
	case mop_AND:
		//операция применима к булевским переменным
		if (id_CBooleanVar == res_id) {
			ResultId = id_CBooleanVar;
			return 0;
		} else return s_e_ExprCompatibility;
	
	//все допустимые варианты MulOp должны быть уже обработаны
	default:
		return s_m_Error;
	}//switch
}//Init


//-----------------------------------------------------------------------------
//Запись кода CTermPair
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
			fprintf(f.fc, " / double(");	//при делении тип результата должен быть действительным
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
//деструктор
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
//поиск последней переменной в цепочке выражений (не обязательно l-value)
CBaseName* CTerm::FindLastName()
{
	if (TermPairStore) return NULL;
	return Factor->FindLastName();
}


//-----------------------------------------------------------------------------
//инициализация объекта Term из потока лексем
int CTerm::Init(CLexBuf *lb)
{
	//создание первого множителя
	Factor = new CFactor(parent_element, ExprKind, in_fact_pars);
	int err_num = Factor->Init(lb);
	if (err_num) return err_num;

	//получение предварительного типа результата выражения
	ResultId = Factor->GetResultId();

	//проверка требуемого типа выражерия
	if (ek_Char == ExprKind || ek_ProcedureVar == ExprKind) return 0;

	DECL_SAVE_POS
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//тип операции умножения MulOp
	EMulOp MulOp = mop_NULL;

	//цикл поиска множителей
	while (true) {
		//проверка наличия MulOp
		if (!lb->ReadLex(li) || (lex_k_dot > li.lex)) {
			RESTORE_POS
			return 0;
		}

		//проверка типа операции
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

		//создание списка множителей
		if (!TermPairStore) TermPairStore = new CBaseVector;
		//создание нового множителя
		CTermPair *TP = new CTermPair(parent_element);
		TP->MulOp = MulOp;
		err_num = TP->Init(lb);
		if (err_num) return err_num;
		//занесение созданного множителя в список
		TermPairStore->push_back(TP);

		//вычисление текущего типа результата выражения (с учетом нового слагаемого)
		if (id_CBooleanVar == ResultId && TP->GetResultId() != ResultId) return s_e_ExprCompatibility;
		else {
			//получение мин. типа, поглощающего оба операнда
			ResultId = GetMaxDigitId(ResultId, TP->GetResultId());
			if (id_CBaseName == ResultId) return s_e_ExprCompatibility;
			//дальнейшая обработка типа (в зависимости от операции)
			switch (MulOp) {
			//для операции '/' надо выбрать наименьший вещественный тип, поглощающий оба операнда (если это не операция над множествами)
			case mop_D:
				if (id_CLongrealVar != ResultId && id_CSetVar != ResultId) ResultId = id_CRealVar;
				break;
			//для операций DIV и MOD достаточно проверить наличие целого типа
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
//проверка, является ли объект, содержащийся в CTerm, read-only
bool CTerm::IsReadOnly() const
{
	if (TermPairStore) return true;
	return Factor->IsReadOnly();
}


//-----------------------------------------------------------------------------
//Запись кода CTerm
void CTerm::WriteCPP(CPP_files& f)
{
	//запись набора префиксов (в обратном порядке) для операций DIV, MOD, *
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

	//запись первого множителя
	Factor->WriteCPP(f);

	//запись оставшихся множителей (если есть)
	if (TermPairStore) {
		CBaseVector::const_iterator ci;
		for (ci = TermPairStore->begin(); ci != TermPairStore->end(); ++ci) {
			//запись текущего знака операции и второго оперенда для текущей операции
			static_cast<CTermPair*>(*ci)->CTermPair::WriteCPP(f);
			//при необходимости необходимо записывать закрывающие эл-ты для операций DIV, MOD, *
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
//применение к указанной переменной операции, с проверкой константности
int CSimpleExprPair::ApplyOperation(CBaseVar *&BaseConst) const
{
	//для хранения промежуточного значения: (BaseConst) = (BaseConst) <AddOp> (fac)
	CBaseVar* fac;

	//получение константы из "слагаемого"
	int err_num = Term->CreateConst(fac);
	if (err_num) return err_num;

	//подразумевается, что проверка допустимости типов выполнена в CTermPair::Init
	switch (AddOp) {

	//операция: +
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
			//все варианты должны быть обработаны выше
			return s_m_Error;
		}
		break;

	//операция: -
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
			//все варианты должны быть обработаны выше
			return s_m_Error;
		}
		break;

	//операция: OR
	case aop_OR:
		switch (BaseConst->name_id) {
		case id_CBooleanVar:
			if (id_CBooleanVar == fac->name_id)
				static_cast<CBooleanVar*>(BaseConst)->ConstValue = static_cast<CBooleanVar*>(BaseConst)->ConstValue || static_cast<CBooleanVar*>(fac)->ConstValue;
			else
				return s_m_Error;
			break;
		default:
			//все варианты должны быть обработаны выше
			return s_m_Error;
		}
		break;

	//все варианты должны быть обработаны выше
	default:
		return s_m_Error;
	}//switch

	//уничтожение промежуточного значения
	delete fac;

	return 0;
}


//-----------------------------------------------------------------------------
//инициализация объекта SimpleExprPair из потока лексем
int CSimpleExprPair::Init(CLexBuf *lb)
{
	//получение текущего "слагаемого"
	Term = new CTerm(parent_element, ek_Normal, false);
	int err_num = Term->Init(lb);
	if (err_num) return err_num;

	//получение типа результата "слагаемого" (для скорости)
	const EName_id res_id = Term->GetResultId();

	//проверка типа результата
	switch (AddOp) {

	//операции: +, -
	case aop_ADD:
	case aop_SUB:
		//операции применимы ко множествам и числам
		if (id_CSetVar == res_id || CBaseVar::IsDigitId(res_id)) {
			ResultId = res_id;
			return 0;
		} else return s_e_ExprCompatibility;

	//операция: OR
	case aop_OR:
		//операция применима к булевским переменным
		if (id_CBooleanVar == res_id) {
			ResultId = id_CBooleanVar;
			return 0;
		} else return s_e_ExprCompatibility;

	//все допустимые варианты AddOp должны быть уже обработаны
	default:
		return s_m_Error;
	}//switch
}//Init


//-----------------------------------------------------------------------------
//Запись кода CSimpleExprPair
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
//деструктор
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
//поиск последней переменной в цепочке выражений (l-value или константа)
CBaseName* CSimpleExpr::FindLastName()
{
	if (SimpleExprPairStore) return NULL;
	return Term->FindLastName();
}


//-----------------------------------------------------------------------------
//инициализация объекта SimpleExpr из потока лексем
int CSimpleExpr::Init(CLexBuf *lb)
{
	DECL_SAVE_POS
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия '+' или '-'
	if (!lb->ReadLex(li) || (lex_k_plus != li.lex && lex_k_minus != li.lex)) {
		RESTORE_POS
	} else {
		SAVE_POS
		negative = lex_k_minus == li.lex;
		unary = true;
	}

	//инициализация первого Terma (слагаемого)
	Term = new CTerm(parent_element, ExprKind, in_fact_pars);
	int err_num = Term->Init(lb);
	if (err_num) return err_num;

	//получение предварительного типа результата выражения
	ResultId = Term->GetResultId();

	//проверка совместимости выражения при наличии унарного '+' или '-'
	if (unary && !(CBaseVar::IsDigitId(ResultId) || (negative && id_CSetVar == ResultId))) {
		RESTORE_POS
		return s_e_ExprCompatibility;
	}

	//проверка требуемого типа выражения
	if (ek_Char == ExprKind || ek_ProcedureVar == ExprKind) return 0;

	SAVE_POS

	//тип операции сложения AddOp
	EAddOp AddOp = aop_NULL;

	//цикл поиска слагаемых
	while (true) {
		//проверка наличия AddOp
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) {
			RESTORE_POS
			return 0;
		}

		//проверка типа операции
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

		//создание списка слагаемых (если еще не создан)
		if (!SimpleExprPairStore) SimpleExprPairStore = new CBaseVector;
		//создание нового слагаемого
		CSimpleExprPair *SEP = new CSimpleExprPair(parent_element);
		SEP->AddOp = AddOp;
		err_num = SEP->Init(lb);
		if (err_num) return err_num;
		//занесение созданного слагаемого в список
		SimpleExprPairStore->push_back(SEP);

		//вычисление текущего типа результата выражения (с учетом нового слагаемого)
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
//проверка, является ли объект, содержащийся в CSimpleExpr, read-only
bool CSimpleExpr::IsReadOnly() const
{
	if (SimpleExprPairStore) return true;
	return Term->IsReadOnly();
}


//-----------------------------------------------------------------------------
//Запись кода CSimpleExpr
void CSimpleExpr::WriteCPP(CPP_files& f)
{
	//проверка использования отрицания или дополнения (множества)
	if (negative) fprintf(f.fc, "%c", (Term->GetResultId() == id_CSetVar) ? '~' : '-');

	Term->WriteCPP(f);

	if (SimpleExprPairStore) {
		CBaseVector::const_iterator ci;
		for (ci = SimpleExprPairStore->begin(); ci != SimpleExprPairStore->end(); ++ci)
			(*ci)->WriteCPP(f);
	}
}//WriteCPP


//-----------------------------------------------------------------------------
//деструктор
CExpr::~CExpr()
{
	delete IS_qual;
	delete SimpleExpr1;
	delete SimpleExpr2;
}//~CExpr


//-----------------------------------------------------------------------------
//создание константы
int CExpr::CreateConst(CBaseVar* &BaseConst)
{
	//проверка наличия неконстантной операции IS (применяется только к переменным и указателям)
	if (rel_IS == Relation) return s_e_ExprNotConst;

	//переменные для хранения промежуточных значений
	CBaseVar* Const1 = NULL;
	CBaseVar* Const2 = NULL;

	//создание константы по SimpleExpr1
	int err_num = SimpleExpr1->CreateConst(Const1);
	if (err_num) return err_num;

	/**/
	//выбор минимального типа, достаточного для хранения результата выражения
	//отладочная версия
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

	//проверяем отсутствие второго выражения (отсутствие необходимости проверок)
	if (!SimpleExpr2) {
		BaseConst = Const1;	//возвращаем константу от первого выражения
		return 0;
	}

	//создание константы по SimpleExpr2
	err_num = SimpleExpr2->CreateConst(Const2);
	if (err_num) {
		delete Const1;
		return err_num;
	}

	//создание константной переменной для хранения булевского значения
	BaseConst = new CBooleanVar(parent_element);
	BaseConst->is_const = true;

	//проверка наличия целочисленного операнда и константы-множества 
	//(может привести к обнаружению ошибки)
	if (CBaseVar::IsIntId(Const1->name_id) && id_CSetVar == Const2->name_id) {
		const long val = Const1->GetIntValue();
		//проверка допустимого значения константы
		if (SET_MAX < val || 0 > val) {
			//при получении ошибки удаляем все созданные объекты
			delete Const1;
			delete Const2;
			delete BaseConst;
			BaseConst = NULL;
			//константа не была создана
			return s_e_SetElemRange;
		}
		//проверка допустимого типа операции (вообще-то должно быть уже проверено)
		if (rel_IN == Relation) {
			static_cast<CBooleanVar*>(BaseConst)->ConstValue = (1 << val & static_cast<CSetVar*>(Const2)->ConstValue) != 0;
			return 0;
		} else
			return s_m_Error;
	}

	//вычисление значения константы
	static_cast<CBooleanVar*>(BaseConst)->ConstValue = ApplyOperation(Const1, Const2);

	return 0;
}


//-----------------------------------------------------------------------------
//инициализация объекта Expr из потока лексем
int CExpr::Init(CLexBuf *lb)
{
	//инициализация первого SimpleExpr выражения
	SimpleExpr1 = new CSimpleExpr(parent_element, ExprKind, in_fact_pars);
	int err_num = SimpleExpr1->Init(lb);
	if (err_num) return err_num;

	//проверка требуемого типа выражения
	if (ek_Char == ExprKind || ek_ProcedureVar == ExprKind) return CalcResultId();

	DECL_SAVE_POS
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//чтение следующей лексемы (ключевого слова)
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) {
		RESTORE_POS
		return CalcResultId();
	}

	//проверка типа операции
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
			//в случае операции IS вместо получения SimpleExpr2 получаем непосредственно уточн. идент.
			Relation = rel_IS;
			CBaseName* BN = SimpleExpr1->FindLastName();
			if (!BN) return s_e_OperandNotVariable;
			//проверка типа переменной для доп. обработки
			switch(BN->name_id) {
			case id_CCommonVar:
				//проверка соответствия переменной нужным требованиям (параметр-переменная)
				if (!static_cast<CCommonVar*>(BN)->is_var) return s_e_GuardVarNotRecOrP;
				if (!static_cast<CCommonVar*>(BN)->IsPureCommon()) return s_e_GuardVarNotRecOrP;
				//получение типа обобщенной переменной
				BN = static_cast<CCommonVar*>(BN)->FindType();
				break;
			case id_CPointerVar:
				BN = static_cast<CPointerVar*>(BN)->FindType();
			}
			//подготовка к получению уточн. идент-ра
			IS_qual = new CQualident;
			//проверка работы с обобщением или с записью
			const char *m_n;
			const char *n;
			switch (BN->name_id) {
			case id_CCommonType:
				//проверка наличия "<" - начало признака специализации
				if (!lb->ReadLex(li) || lex_k_lt != li.lex) return s_e_OpAngleMissing;
				//инициализация описания признака
				err_num = IS_qual->Init(lb, parent_element);
				if (err_num) return err_num;
				//проверка, является ли признак допустимым для данного обобщения
				if (!static_cast<CCommonType*>(BN)->FindSpec(IS_qual->pref_ident, IS_qual->ident, IS_qual->ident)) return s_e_SpecTypeTag;
				//проверка наличия ">" - конец признака специализации
				if (!lb->ReadLex(li) || lex_k_gt != li.lex) return s_e_ClAngleMissing;
				break;
			case id_CRecordVar:
				//запоминаем название типа для проверки наличия расширения типа
				m_n = static_cast<CRecordVar*>(BN)->GetTypeModuleName();
				n = static_cast<CRecordVar*>(BN)->GetTypeName();
				goto both_kinds;
			case id_CRecordType: {
				//запоминаем название типа для проверки наличия расширения типа
				m_n = static_cast<CRecordType*>(BN)->GetModuleName();
				n = static_cast<CRecordType*>(BN)->name;
			both_kinds:
				//инициализация описания имени типа
				err_num = IS_qual->Init(lb, parent_element);
				if (err_num) return err_num;
				//проверяем наличие переменной неименованного типа (тогда IS неприменимо)
				if (!n) return s_e_GuardTypeNotExt;
				//получение типа (с проверкой получения именно типа)
				BN = parent_element->GetGlobalName(IS_qual->pref_ident, IS_qual->ident);
				if (!BN) return s_e_UndeclaredIdent;
				if (id_CPointerType == BN->name_id) BN = static_cast<CPointerType*>(BN)->FindType();
				if (id_CRecordType != BN->name_id) return s_e_IdentNotRecordType;
				if (!static_cast<CRecordType*>(BN)->IsExtension(m_n, n)) return s_e_GuardTypeNotExt;
				break;
				}
			default:
				//первый операнд недопустимого типа
				return s_e_GuardVarNotRecOrP;
			}
			//конец обработки операции IS
			ResultId = id_CBooleanVar;
			return 0;
		}//case
	default:
		//отсутствует операция, разбираемая в CExpr
		RESTORE_POS
		return CalcResultId();
	}

	//обнаружена допустимая операция, обрабатываем ее с учетом требуемой
	//интерпретации выражения

	//проверка требуемой интерпретации выражения
	if (id_CCharVar == SimpleExpr1->GetResultId())
		SimpleExpr2 = new CSimpleExpr(parent_element, ek_Char, false);
	else
		SimpleExpr2 = new CSimpleExpr(parent_element, ek_Normal, false);
	//инициализация второго SimpleExpr
	err_num = SimpleExpr2->Init(lb);
	if (err_num) return err_num;
	//в случае, если SimpleExpr2 символьный, предпринимается попытка преобразовать
	//SimpleExpr1 из строки в символ (только если в нем строка)
	if (id_CCharVar == SimpleExpr2->GetResultId()) {
		//лезем внутрь SimpleExpr1 :(
		CArrayVar* AV = static_cast<CArrayVar*>(SimpleExpr1->FindLastName());
		if (AV && id_CArrayVar == AV->name_id && AV->ConstString && strlen(AV->ConstString) == 1) {
			//обнаружена возможность преобразования строки в символ, что и делаем
			AV->ConstStringIsChar = true;
			//устанавливаем ResultId (для двух CHARов допустимы только операции сравнения)
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

	//проверка совместимости данного выражения
	return CalcResultId();
}//Init


//-----------------------------------------------------------------------------
//проверка, является ли объект, содержащийся в CExpr, read-only
bool CExpr::IsReadOnly() const
{
	if (SimpleExpr2) return true;
	return SimpleExpr1->IsReadOnly();
}


//-----------------------------------------------------------------------------
//Запись кода CExpr
void CExpr::WriteCPP(CPP_files& f)
{
	//запись префикса (если есть)
	switch (Relation) {
	case rel_IN:			//операция "IN"
		fprintf(f.fc, "(1 << (");
		SimpleExpr1->WriteCPP(f);
		fprintf(f.fc, ") & (");
		SimpleExpr2->WriteCPP(f);
		fprintf(f.fc, "))");
		return;
	case rel_IS:			//операция "IS"
		//получение первого операнда
		CBaseName* BN = SimpleExpr1->FindLastName();
		if (id_CPointerVar == BN->name_id) BN = static_cast<CPointerVar*>(BN)->FindType();
		//проверка работы с обобщением
		if (id_CCommonVar == BN->name_id || id_CCommonType == BN->name_id) {
			//генерация кода в случае обобщения
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
			//генерация кода в случае записи (ук. на запись)
			fprintf(f.fc, "!strcmp(");
			SimpleExpr1->WriteCPP(f);
			fprintf(f.fc, "->O2M_SYS_ID(), \"");
			CBaseName* BN = parent_element->GetGlobalName(IS_qual->pref_ident, IS_qual->ident);
			if (id_CPointerType == BN->name_id) BN = static_cast<CPointerType*>(BN)->FindType();
			fprintf(f.fc, "%s\")", static_cast<CRecordType*>(BN)->GetRuntimeId());
		}
		return;
	}//switch

	//запись 1-го простого выражения
	SimpleExpr1->WriteCPP(f);

	//запись знака операции (если есть)
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

	//запись 2-го простого выражения (если есть)
	if (SimpleExpr2) SimpleExpr2->WriteCPP(f);
}//WriteCPP


//-----------------------------------------------------------------------------
//поиск последней переменной в цепочке выражений (l-value или константа)
CBaseName* CExpr::FindLastName()
{
	if (SimpleExpr2) return NULL;
	return SimpleExpr1->FindLastName();
}


//-----------------------------------------------------------------------------
//поиск последней переменной (l-value) в цепочке выражений (без проверки read-only)
CBaseName* CExpr::FindLValue()
{
	if (SimpleExpr2) return NULL;
	//проверка наличия константы или l-value
	CBaseName* BN = SimpleExpr1->FindLastName();
	//проверка наличия объекта и отсутствия признака константы, если он отсутствует,
	//объект именованный, т.к. только константы могут быть неименованными
	if (!BN || (CBaseVar::IsVarId(BN->name_id) && static_cast<CBaseVar*>(BN)->is_const)) return NULL;
	//возврат найденного l-value
	return BN;
}


//-----------------------------------------------------------------------------
//вычисление значения текущей операции для указанных константных переменных,
//подразумевается, что допустимость операции и константность переменных уже проверены
bool CExpr::ApplyOperation(const CBaseVar* const c1, const CBaseVar* const c2) const
{
	//использование констант (для краткости)
	const EName_id id1 = c1->name_id;
	const EName_id id2 = c2->name_id;

	//примечание: проверка наличия целочисленного операнда и константы-множества
	//происходит до вызова данной функции, т.к. может привести к обнаружению ошибки
	//(выхода целочисленной константы за пределы допустимых границ множества)

	//проверка наличия двух целочисленных операндов
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

	//проверка наличия двух числовых операндов
	if (CBaseVar::IsDigitId(id1) && CBaseVar::IsDigitId(id2)) {
		//получение первого числового значения (емкости long double должно хватить)
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
		//получение второго числового значения (емкости long double должно хватить)
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
		//выполнение операции над полученными значениями
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

	//проверка наличия двух символьных операндов
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

	//проверка наличия двух строковых констант
	if (id_CArrayVar == id1 && id_CArrayVar == id2) {
		//лексикографическое сравнение двух строк
		const int cmp = strcmp(static_cast<const CArrayVar*>(c1)->ConstString, static_cast<const CArrayVar*>(c2)->ConstString);
		//интерпретация полученного значения в зависимости от операции
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

	//проверка наличия двух булевских констант
	if (id_CBooleanVar == id1 && id_CBooleanVar == id2) {
		switch (Relation) {
		case rel_EQ: return static_cast<const CBooleanVar*>(c1)->ConstValue == static_cast<const CBooleanVar*>(c2)->ConstValue;
		case rel_NE: return static_cast<const CBooleanVar*>(c1)->ConstValue != static_cast<const CBooleanVar*>(c2)->ConstValue;
		default:;
		}
	}

	//проверка наличия двух констант-множеств
	if (id_CSetVar == id1 && id_CSetVar == id2) {
		switch (Relation) {
		case rel_EQ: return static_cast<const CSetVar*>(c1)->ConstValue == static_cast<const CSetVar*>(c2)->ConstValue;
		case rel_NE: return static_cast<const CSetVar*>(c1)->ConstValue != static_cast<const CSetVar*>(c2)->ConstValue;
		default:;
		}
	}

	//проверка наличия двух NIL
	if (id_CPointerVar == id1 && id_CPointerVar == id2) {
		switch (Relation) {
		case rel_EQ: return true;
		case rel_NE: return false;
		default:;
		}
	}

	//все возможные сочетания операндов и операций должны быть уже обработаны
	throw error_Internal("CExpr::ApplyOperation");
}


//-----------------------------------------------------------------------------
//вычисление ResultId с проверкой ошибок
int CExpr::CalcResultId()
{
	//создание константы для первого операнда (для простоты)
	const EName_id res_id1 = SimpleExpr1->GetResultId();

	//проверка одного операнда (тогда id соответствует этому операнду)
	if (rel_NULL == Relation) {
		ResultId = res_id1;
		return 0;
	}

	//создание константы для второго операнда (для простоты),
	//подразумевается, что наличие SimpleExpr2 проверено в CExpr::Init
	const EName_id res_id2 = SimpleExpr2->GetResultId();

	//при наличии двух операндов результатом выражения всегда будет BOOLEAN
	ResultId = id_CBooleanVar;

	//проверка типа отношения в выражении
	switch (Relation) {

	//операции: =, # (часть проверок переносятся на след. case)
	case rel_EQ:
	case rel_NE:
		switch (res_id1) {
		case id_CBooleanVar:
		case id_CSetVar:
			if (res_id2 != res_id1) return s_e_ExprCompatibility;
			return 0;
		case id_CPointerVar:
			/**/ //нужна более сложная проверка
			return 0;
		/**/ //по идее вместо типа результата id_CProcedureType должен быть id_CProcedureVar
		case id_CProcedureType:
			if (res_id2 != id_CProcedureType) return s_e_ExprCompatibility;
			return 0;
		}

	//операции: <, <=, >, >= (плюс дальнейшая проверка операций =, #)
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
		//ни один из вариантов не сработал
		return s_e_ExprCompatibility;

	//операция: IN
	case rel_IN:
		if (!CBaseVar::IsIntId(res_id1) || res_id2 != id_CSetVar) return s_e_ExprCompatibility;
		return 0;

	//все возможные варианты Relation должны быть уже обработаны
	default:
		return s_m_Error;
	}//switch
}


//-----------------------------------------------------------------------------
//деструктор
CDesignator::~CDesignator()
{
	delete Qualident;
	//очистка списка эл-тов обозначения
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
//создание нового эл-та обозначения и инициализация его типа
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
//поиск последнего имени в цепочке эл-тов обозначения
//это может быть переменная, процедура, тип (в случае массива)
CBaseName* CDesignator::FindLastName()
{
	//поиск первого имени в цепочке (должно присутствовать, проверено в Init)
	CBaseName* BN = parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident);

	//перебор списка эл-тов обозначения (если есть) с переходом по локальным таблицам имен
	SDesElemStore::const_iterator ci;
	for (ci = DesElemStore.begin(); ci != DesElemStore.end(); ci++)
		switch ((*ci)->DesKind) {
		case dk_Record:
		case dk_Pointer:
			//поиск имени в локальной таблице имен (имя должно присутствовать, проверено в Init)
			BN = BN->FindName((*ci)->ident);
			break;
		case dk_Array:
			{
				//получение количества размерностей
				int ExprCount = (*ci)->ExprList->GetExprCount();
				//получение типа с учетом требуемой размерности
				if (id_CArrayVar == BN->name_id) {
					CArrayType* AT = static_cast<CArrayVar*>(BN)->ArrayType;
					//получение типа эл-та массива с учетом количества измерений
					BN = AT->GetType(ExprCount);
				} else {
					CArrayType* AT = static_cast<CArrayType*>(static_cast<CPointerVar*>(BN)->FindType());
					//получение типа эл-та массива с учетом количества измерений
					BN = AT->GetType(ExprCount);
				}//else
				//проуерка наличия типа указатель - в этом случае требуется получить тип, на кот. он указывает
				if (id_CPointerType == BN->name_id) BN = static_cast<CPointerType*>(BN)->FindType();
			}
			break;
		default:
			/**/
			//пока не реализована охрана типа
			break;
		}//switch

	//выдача найденного имени
	return BN;
}


//-----------------------------------------------------------------------------
//инициализация объекта Designator из потока лексем
int CDesignator::Init(CLexBuf *lb)
{
	////////////////////////////////////////////////////////////////////
	//1-й этап: получение Qualident и поиск его описания в таблицах имен
	////////////////////////////////////////////////////////////////////

	//проверка наличия имени в потоке лексем
	Qualident = new CQualident;
	int err_num = Qualident->Init(lb, parent_element);
	if (err_num) return err_num;

	//проверка наличия уточненного имени в таблице имен
	CBaseName* BN = parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident);

	//если не найденно уточненное имя, тогда требуется проверка наличия переменной
	//типа запись, указатель или скаляр в текущем модуле (т.к. найдена '.')
	if (!BN) {
		//первый ident интерпретируется не как имя модуля, а как имя локального объекта
		BN = parent_element->GetGlobalName(Qualident->pref_ident);
		//проверка нахождения локального объекта
		if (BN) {
			//проверка допустимости типа найденного объекта
			EDesKind DesKind;
			switch (BN->name_id) {
			case id_CRecordVar:
				//проверка, является ли запись VAR-параметром (который рассматривается как указатель)
				DesKind = static_cast<CBaseVar*>(BN)->is_var ? dk_Pointer : dk_Record;
				break;
			case id_CPointerVar:
			case id_CCommonVar:
				DesKind = dk_Pointer;
				break;
			default:
				//недопустимый объект перед '.'
				return s_e_IdentNotRecordType;
			}
			//проверка вхождения считанного имени в список имен найденного объекта
			BN = BN->FindName(Qualident->ident);
			if (!BN) return s_e_DesRecordField;
			//создание эл-та обозначения требуемого типа и занесение его в список
			SDesElem* DesElem = CreateSDesElem(DesKind);
			DesElem->ident = Qualident->ident;
			Qualident->ident = NULL;
			DesElemStore.push_back(DesElem);
			//т.к. последний ident перемещен в эл-т обозначения, корректируем Qualident
			Qualident->ident = Qualident->pref_ident;
			Qualident->pref_ident = NULL;
		} else	//переменной в текущем модуле не найдено
			return s_e_UndeclaredIdent;
	}//if

	/////////////////////////////////////////////////////////////////////////
	//2-й этап: в BN - последний объект в цепочке, распознаем остальные эл-ты
	/////////////////////////////////////////////////////////////////////////

	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//признак необходимости продолжения выделения эл-тов обозначения
	bool continue_elems_parsing = true;

	while (continue_elems_parsing) {
		//вычисление признака read-only (если объект импортирован) используя известное значение BN
		if (Qualident->pref_ident && CBaseVar::IsVarId(BN->name_id))
			is_read_only = is_read_only || static_cast<CBaseVar*>(BN)->is_read_only;

		//запоминаем позицию, т.к. может встретиться символ не из цепочки эл-тов обозначения
		DECL_SAVE_POS

		//получение следующей лексемы (если есть)
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) {
			RESTORE_POS
			break;
		}

		//проверка типа полученной лексемы
		switch (li.lex) {

		////////////////////////////////////////////////////
		//проверка '.' - допустимо для записи, ук. на запись
		case lex_k_dot: {
			//снятие признака наличия в описании последнего символа '^'
			present_last_up = false;
			//для хранения временной ссылки на CompoundName
			const char* pCompoundName = NULL;
			//признак типа следующего эл-та обозначения (вычисляется заранее)
			EDesKind DesKind;
			//проверка типа объекта
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
				//недопустимый объект перед '.'
				return s_e_IdentNotRecordType;
			}//switch
			//чтение след. лексемы (должен быть идентификатор)
			if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
			//проверка вхождения считанного имени в список имен найденного объекта
			BN = BN->FindName(li.st);
			if (!BN) return s_e_DesRecordField;
			//создание эл-та обозначения требуемого типа и занесение его в список
			SDesElem* DesElem = CreateSDesElem(DesKind);
			DesElem->ident = str_new_copy(li.st);
			if (pCompoundName) DesElem->CPPCompoundName = str_new_copy(pCompoundName);
			DesElemStore.push_back(DesElem);
			//продолжение обработки эл-тов обозначения
			continue;
		}

		///////////////////////////////////////////////
		//проверка '^' - допустимо только для указателя
		case lex_k_up_arrow:
			if (id_CPointerVar != BN->name_id && id_CPointerType != BN->name_id) return s_e_DesNotPointer;
			//запоминаем признак наличия в описании последнего символа '^'
			present_last_up = true;
			//продолжение обработки эл-тов обозначения
			continue;

		////////////////////////////////////////////////////////
		//проверка '[' - допустимо для массива или ук. на массив
		case lex_k_op_square: {
			//снятие признака наличия в описании последнего символа '^'
			present_last_up = false;
			//проверка наличия массива или ук. на массив
			if (id_CArrayVar != BN->name_id && id_CPointerVar != BN->name_id) return s_e_DesNotArray;
			if (id_CPointerVar == BN->name_id && !static_cast<CPointerVar*>(BN)->IsArrayPointer()) return s_e_DesNotArray;
			//создание эл-та обозначения требуемого типа и занесение его в список (наличие открытого массива будет проверено ниже)
			SDesElem* DesElem = CreateSDesElem(dk_Array);
			DesElem->ExprList = new CExprList(parent_element, NULL);
			DesElemStore.push_back(DesElem);
			//проверка наличия описания индексов массива (удаление ExprList при ошибке уже обеспечено)
			err_num = DesElem->ExprList->Init(lb);
			if (err_num) return err_num;
			//проверка наличия ']'
			if (!lb->ReadLex(li) || lex_k_cl_square != li.lex) return s_e_ClSquareMissing;
			//проверка дальнейшего описания индексов массива
			SAVE_POS
			while (lb->ReadLex(li) && lex_k_op_square == li.lex) {
				//временный список для получения содержимого очередных скобок [...]
				CExprList EL(parent_element, NULL);
				//загрузка очередного списка выражений (при ошибке удаление автоматическое)
				err_num = EL.Init(lb);
				if (err_num) return err_num;
				//перемещение загруженных выражений в основной список
				EL.MoveAllExpr(DesElem->ExprList);
				//проверка наличия ']'
				if (!lb->ReadLex(li) || lex_k_cl_square != li.lex) return s_e_ClSquareMissing;
				//очередные скобки [...] обработаны
				SAVE_POS
			}
			RESTORE_POS
			//получение общего количества обнаруженных индексов массива
			int ExprCount = DesElem->ExprList->GetExprCount();
			//проверка превышения количеством обнаруженных индексов действительного количества индексов массива
			//наличие массива или ук. на массив перед '[' проверено выше
			CArrayType* AT;
			if (id_CArrayVar == BN->name_id) {
				AT = static_cast<CArrayVar*>(BN)->ArrayType;
				if (AT->GetDimCount() < ExprCount) return s_e_DesNotArray;
				//получение типа эл-та массива с учетом количества измерений
				BN = AT->GetType(ExprCount);
			} else {
				AT = static_cast<CArrayType*>(static_cast<CPointerVar*>(BN)->FindType());
				if (AT->GetDimCount() < ExprCount) return s_e_DesNotArray;
				//получение типа эл-та массива с учетом количества измерений
				BN = AT->GetType(ExprCount);
			}
			//проверка, является ли массив открытым
			if (!AT->size) DesElem->DesKind = dk_OpenArray;
			//продолжение обработки эл-тов обозначения
			continue;
		}

		////////////////////////////////////////////////////////////////////////////
		//проверка '(' - допустимо для параметра-переменной записи или ук. на запись
		case lex_k_op_bracket: {
			//снятие признака наличия в описании последнего символа '^'
			present_last_up = false;
			//проверка типа объекта
			switch (BN->name_id) {
			case id_CRecordVar:
				//запись обязательно должна быть VAR-параметром
				if (!static_cast<CRecordVar*>(BN)->is_var) return s_e_GuardVarNotRecOrP;
			case id_CPointerVar:
				//ук. должен обязательно указывать на запись
				if (!static_cast<CPointerVar*>(BN)->IsRecordPointer()) return s_e_GuardVarNotRecOrP;
			default:
				//случай вызова процедуры или ошибки далее не обрабатывается
				RESTORE_POS
				continue_elems_parsing = false;
				continue;
			}
			//создание эл-та обозначения требуемого типа
			SDesElem* DesElem = CreateSDesElem(dk_Guard);
			DesElem->Qualident = new CQualident;
			//проверка наличия описания типа
			err_num = DesElem->Qualident->InitTypeName(lb, parent_element);
			if (err_num) {
				delete DesElem->Qualident;
				delete DesElem;
				//установка признака окончания обработки эл-тов обозначения
				RESTORE_POS
				continue_elems_parsing = false;
				continue;
			}

			/**/ //проверить тип, занесенный в Qualident

			//занесение эл-та обозначения в список
			DesElemStore.push_back(DesElem);
			//проверка наличия ')'
			if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;
			//продолжение обработки эл-тов обозначения
			continue;
		}

		//////////////////////////////////////////////////////////////////////////////
		//обнаружен символ, не относящийся к эл-ту определения, конец выделения эл-тов
		default:
			RESTORE_POS
			continue_elems_parsing = false;

		}//switch

	}//while

	//////////////////////////////////////////////////////////////////////////
	//3-й этап: проверка типа объекта, на который ссылается данное обозначение
	//(находится в BN), вычисление результата выражения ResultId
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////
	//проверка наличия имени процедуры
	if (CProcedure::IsProcId(BN->name_id)) {
		//проверка необходимости интерпретации процедуры не как вызова
		if (req_proc_var) {
			ResultId = id_CProcedure;
			return 0;
		}
		//обозначение является вызовом
		is_proc_name = true;
		ResultId = BN->GetResultId();
		return 0;
	}

	/////////////////////////////////////////
	//проверка наличия процедурной переменной
	if (id_CProcedureVar == BN->name_id || id_CProcedureType == BN->name_id) {
		//проверка необходимости интерпретации процедурной переменной не как вызова функции
		if (req_proc_var) {
			ResultId = id_CProcedureVar;
			return 0;
		}
		DECL_SAVE_POS
		//получение след. лексемы для выбора дальнейшего метода обработки
		if (!lb->ReadLex(li)) return s_e_AssignMissing;
		//проверка полученной лексемы (это может быть ':=' - инициализация; '(',';', <операция> - вызов)
		if (lex_k_assign == li.lex) {
			//обработка инициализации (присвоения значения) процедурной переменной
			RESTORE_POS
			ResultId = id_CProcedureVar;
			return 0;
		} else {
			//обработка вызова процедуры через процедурную переменную
			RESTORE_POS
			is_proc_name = true;
			ResultId = BN->GetResultId();
			return 0;
		}
	}

	//в случае наличия последнего '^' получаем тип, на кот. ссылается указатель,
	//т.к. именно он отвечает за тип выражения
	if (present_last_up) {
		if (id_CPointerType == BN->name_id)
			BN = static_cast<CPointerType*>(BN)->FindType();
		else
			BN = static_cast<CPointerVar*>(BN)->FindType();
	}

	//вычисление результата выражения для переменной
	ResultId = BN->GetResultId();

	return 0;
}


//-----------------------------------------------------------------------------
//запись кода CDesignator
void CDesignator::WriteCPP(CPP_files& f)
{
	//признак необходимости записи ')' после приведения типа
	bool need_cl_bracket = false;
	//имя предыдущего эл-та обозначения (используется в открытых массивах)
	const char* PrevName = Qualident->ident;

	//проверка необходимости записи разыменования
	if (present_last_up) fprintf(f.fc, "*(");

	//получение переменной, с которой начинается обозначение
	//(для проверки необходимости записи приведения типа)
	CBaseName* BN = parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident);
	if (gen_Guard_code && CBaseVar::IsVarId(BN->name_id)) {
		CBaseVar* BV = static_cast<CBaseVar*>(BN);
		//проверка необходимости записи приведения типа
		if (BV->is_guarded) {
			fprintf(f.fc, "static_cast<");
			if (BV->GetTypeModuleAlias()) fprintf(f.fc, "%s::", BV->GetTypeModuleAlias());
			fprintf(f.fc, "%s*>(", BV->GetTypeName());
			need_cl_bracket = true;
		}
	}

	//запись названия
	if (Qualident->pref_ident) fprintf(f.fc, "%s::", Qualident->pref_ident);
	fprintf(f.fc, "%s", Qualident->ident);

	//запись названия параметра в случае обобщенной переменной (если необходимо)
	if (id_CCommonVar == BN->name_id)
		if (!DesElemStore.empty()) fprintf(f.fc, "%sO2M_SPEC_%s", static_cast<CCommonVar*>(BN)->is_var ? "->" : ".", static_cast<CCommonVar*>(BN)->GetCPPCompoundName());

	//запись названия параметра в случае ук. на специализацию
	if (id_CPointerVar == BN->name_id && id_CSpecType == static_cast<CPointerVar*>(BN)->FindType()->name_id)
		if (!DesElemStore.empty()) {
			CSpecType* ST = static_cast<CSpecType*>(static_cast<CPointerVar*>(BN)->FindType());
			fprintf(f.fc, "->O2M_SPEC_");
			if (ST->GetQualSpecName()) fprintf(f.fc, "%s_", ST->GetQualSpecName());
			fprintf(f.fc, ST->GetSpecName());
		}

	//запись ')' после приведения типа (если надо)
	if (need_cl_bracket) fprintf(f.fc, ")");

	//перебор и генерация кода эл-тов обозначения
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
			//охрана типа пока не реализована
			break;
		}//switch
		//запоминаем текущее имя (если есть) для использования на следующей итерации
		PrevName = (*ci)->ident;
	}//for

	//проверка необходимости записи ')' после разыменования
	if (present_last_up) fprintf(f.fc, ")");

	//проверка записи в фактические параметры при наличии ук. на массив
	if (in_fact_pars) {
		//проверка, является ли текущий Designator указателем на массив
		CBaseName* BV = FindLastName();
		if (BV && id_CPointerVar == BV->name_id) {
			//получен ук. не на массив - прекращаем обработку
			if (!static_cast<CPointerVar*>(BV)->IsArrayPointer()) return;
			//запись размеров массива по размерностям (перебор размерностей масива)
			CArrayType* AT = static_cast<CArrayType*>(static_cast<CPointerVar*>(BV)->FindType());
			while (id_CArrayType == AT->name_id) {
				//запись размера массива (может содержаться в виде размера или в виде переменной(в процедурах))
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
//запись кода CDesignator без генерации кода приведения типа (для охраняемых
//переменных) - исп. в левых частях присваиваний
void CDesignator::WriteCPP_Guardless(CPP_files &f)
{
	//код охраны не генерируется только при использовании значения самого указателя
	gen_Guard_code = !DesElemStore.empty();
	WriteCPP(f);
}


//-----------------------------------------------------------------------------
//перемещение всех элементов в указанный список выражений
void CExprList::MoveAllExpr(CExprList* ExprList)
{
	//копирование указателей в указанный список
	CExprVector::const_iterator ci;
	for (ci = ExprVector->begin(); ci != ExprVector->end(); ++ci)
		ExprList->ExprVector->push_back(*ci);
	//очистка списка выражений (все выражения перемещены)
	ExprVector->clear();
}


//-----------------------------------------------------------------------------
//создание константы
int CSimpleExpr::CreateConst(CBaseVar *&BaseConst)
{
	//создание константы по первому Term
	int err_num = Term->CreateConst(BaseConst);
	if (err_num) return err_num;

	//проверка наличия знака "-"
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

	//проверка наличия других эл-тов
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
//создание константы
int CTerm::CreateConst(CBaseVar *&BaseConst)
{
	//создание константы по первому Factor
	int err_num = Factor->CreateConst(BaseConst);
	if (err_num) return err_num;

	//проверка наличия других множителей
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
//создание константы
int CFactor::CreateConst(CBaseVar *&BaseConst)
{
	switch (FactorKind) {
	case fk_Negation:
		{
			int err_num = Factor->CreateConst(BaseConst);
			if (err_num) return err_num;
			//в CFactor::Init должно быть проверено, что ResultId == id_CBooleanVar
			if (id_CBooleanVar != BaseConst->name_id) {
				delete BaseConst;
				return s_m_Error;
			}
			//вычисление значения выражения
			static_cast<CBooleanVar*>(BaseConst)->ConstValue = !static_cast<CBooleanVar*>(BaseConst)->ConstValue;
			return 0;
		}
	case fk_Expr:
		return Expr->CreateConst(BaseConst);
	case fk_ConstVar:
		//перед вызовом Var->CreateConst необходимо проверить наличие признака константности
		if (!ConstVar->is_const) return s_e_ExprNotConst;
		//при is_const == true функция CreateConst не должна возвращать NULL
		BaseConst = ConstVar->CreateConst(parent_element);
		return 0;
	case fk_StdProcFunc:
		return StdProcFunc->CreateConst(BaseConst);
	case fk_Designator:
		{
			//получение константы по имени (если есть)
			CBaseName* BN = parent_element->GetGlobalName(Designator->Qualident->pref_ident, Designator->Qualident->ident);
			//проверка наличия переменной
			if (!BN || !CBaseVar::IsVarId(BN->name_id)) return s_e_ExprNotConst;
			//перед вызовом Var->CreateConst необходимо проверить наличие признака константности
			if (!static_cast<CBaseVar*>(BN)->is_const) return s_e_ExprNotConst;
			//при is_const == true функция CreateConst не должна возвращать NULL
			BaseConst = static_cast<CBaseVar*>(BN)->CreateConst(parent_element);
			return 0;
		}
	default:
		//все возможные значения FactorKind должны быть обработаны выше
		return s_m_Error;
	}//switch
}


//-----------------------------------------------------------------------------
//поиск последнего объекта в цепочке выражений (l-value или константа)
CBaseName* CFactor::FindLastName()
{
	switch (FactorKind) {
	case fk_Expr:
		return Expr->FindLastName();
	case fk_Designator:
		{
			//проверка имен вида RecordName.FieldName
			CBaseName *BN = Designator->FindLastName();
			//проверка нахождения переменной (нужны только l-value или константы)
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
//инициализация объекта объекта Call (оператор вызова процедуры) из потока лексем
int CCallStatement::Init(CLexBuf *lb)
{
	//при отсутствии обозначения требуется инициализировать параметрический вызов
	//вида {a}.Proc(b)
	if (!Designator) {
		//вызов процедуры инициализации обобщенных фактических параметров
		int err_num = InitProcCommonPars(lb);
		if (err_num) return err_num;
		//буфер для чтения информации о текущей лексеме
		CLexInfo li;
		//проверка наличия '.'
		if (!lb->ReadLex(li) || lex_k_dot != li.lex) return s_e_DotMissing;
		//инициализируем обозначение
		is_des_owner = true;
		Designator = new CDesignator(parent_element, false);
		err_num = Designator->Init(lb);
		if (err_num) return err_num;
	}

	//если Designator был передан через конструктор => осталось проверить только наличие
	//фактических параметров и вариант параметрического вызова вида ProcName{a}(b)

	//проверка, является ли обозначение процедурой
	if (!Designator->IsProcName()) return s_e_CallNotProc;
	//проверка вызова функции как процедуры
	if (is_proc_call && id_CBaseName != Designator->GetResultId())
		return s_e_CallFuncAsProc;

	//получение ук. на CProcedure или CProcedureVar
	CBaseName* BN = Designator->FindLastName();
	//получение ук. на FormalPars, CCommonPars с учетом CProcedure или CProcedureVar
	CFormalPars* FP;
	CCommonPars* CP = NULL;
	if (CProcedure::IsProcId(BN->name_id)) {
		FP = static_cast<CProcedure*>(BN)->FormalPars;
		//получение ук. на обобщенные параметры (если есть)
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

	//вызов процедуры инициализации фактических параметров
	return InitProcPars(lb, CP, FP);
}//Init


//-----------------------------------------------------------------------------
//проверка наличия и считывание обобщенных параметров при вызове процедуры (функции)
int CCallStatement::InitProcCommonPars(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	DECL_SAVE_POS
	//проверка наличия кл. слова "{" (если нет, обобщенные параметры отсутствуют)
	if (!lb->ReadLex(li) || lex_k_op_brace != li.lex) {
		RESTORE_POS
		return 0;
	}

	//есть "{", проверка наличия созданного CommonList (он не должен быть создан ранее)
	if (CommonList) return s_e_CommonProcCallMixed;
	//проверка наличия кл. слова "}" (отсутствие описания CommonList)
	SAVE_POS
	if (!lb->ReadLex(li) || lex_k_cl_brace != li.lex) {
		//кл. слово "}" отсутствует, проверяем наличие CommonList
		RESTORE_POS
		//создание CExprList
		CommonList = new CExprList(parent_element, NULL);
		//инициализация CommonList
		int err_num = CommonList->Init(lb);
		if (err_num) return err_num;
		//проверка наличия кл. слова "}"
		if (!lb->ReadLex(li) || lex_k_cl_brace != li.lex) return s_e_ClBraceMissing;
	}

	return 0;
}


//-----------------------------------------------------------------------------
//считывание обычных и обобщенных фактических параметров для вызовов процедур (функций)
int CCallStatement::InitProcPars(CLexBuf *lb, CCommonPars* CP, CFormalPars* FP)
{
	//проверка наличия вызова обобщенной процедуры вида CommonProcName{a,b}()
	int err_num = InitProcCommonPars(lb);
	if (err_num) return err_num;

	DECL_SAVE_POS
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия кл. слова "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) {
		//в вызове функции '(' должна обязательно присутствовать
		if (!is_proc_call) return s_e_OpBracketMissing;
		//в вызове процедуры наличие '(' необязательно
		RESTORE_POS
	} else {
		//есть "(", проверяем наличие ")" или ExprList
		//проверка наличия кл. слова ")" (отсутствие ExprList)
		SAVE_POS
		if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) {
			//кл. слово ")" отсутствует, проверяем наличие ExprList
			RESTORE_POS
			//создание и инициализация ExprList
			ExprList = new CExprList(parent_element, FP);
			err_num = ExprList->Init(lb);
			if (err_num) return err_num;
			//проверка наличия кл. слова ")"
			if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;
		}
	}

	//получение количеств найденных фактических и обобщенных фактических параметров
	int CommonParsCount = CommonList ? CommonList->GetExprCount() : 0;
	int ActualParsCount = ExprList ? ExprList->GetExprCount() : 0;

	//проверка корректности количества обобщенных фактических параметров
	if (CP) {
		//проверка наличия списка обобщенных параметров (должен присутствовать)
		if (!CommonParsCount) return s_e_CommonParsExpected;
		//проверка совпадения количеств формальных и фактических обобщенных параметров
		if (CP->FPStore.size() > CommonParsCount)
			return s_e_CommonParsFewer;
		else
			if (CP->FPStore.size() < CommonParsCount) return s_e_CommonParsMore;
	} else
		if (CommonParsCount) return s_e_CommonParsMore;
	//проверка корректности количества фактических параметров
	if (FP->FPStore.size() > ActualParsCount)
		return s_e_ParsFewer;
	else
		if (FP->FPStore.size() < ActualParsCount) return s_e_ParsMore;

	return 0;
}


//---------------------------------------------------------------
//Запись кода CCallStatement
void CCallStatement::WriteCPP(CPP_files& f)
{
	//если CallStatement - вызов процедуры, вставляем табуляции для форматирования
	if (is_proc_call) f.tab_fc();

	//запись кода самого обозначения
	Designator->WriteCPP(f);

	//запись начала списка параметров
	fprintf(f.fc, "(");

	//получение ук. на CProcedure или CProcedureVar
	CBaseName *BN = Designator->FindLastName();

	//проверка наличия фактических параметров
	if (ExprList) {
		//выбор между наследником CProcedure и CProcedureVar
		if (CProcedure::IsProcId(BN->name_id))
			ExprList->WriteCPP_proc(f, static_cast<CProcedure*>(BN)->FormalPars);
		else
			if (id_CProcedureVar == BN->name_id)
				ExprList->WriteCPP_proc(f, &static_cast<CProcedureVar*>(BN)->FormalPars);
			else
				ExprList->WriteCPP_proc(f, &static_cast<CProcedureType*>(BN)->FormalPars);
	}
	//в случае обобщающей процедуры добавляются обобщающие параметры (пока без проверки)
	if ((id_CCommonProc == BN->name_id || id_CDfnCommonProc == BN->name_id || id_CHandlerProc == BN->name_id) && CommonList) {
		if (ExprList && (ExprList->GetExprCount() > 0)) fprintf(f.fc, ",");
		//выбор между наследником CProcedure и CProcedureVar
		if (id_CProcedureVar == BN->name_id)
			CommonList->WriteCPP_common_proc(f, &static_cast<CProcedureVar*>(BN)->FormalPars);
		else
			if (id_CCommonProc == BN->name_id || id_CDfnCommonProc == BN->name_id)
				CommonList->WriteCPP_common_proc(f, static_cast<CCommonProc*>(BN)->CommonPars);
			else
				CommonList->WriteCPP_common_proc(f, static_cast<CProcedure*>(BN)->FormalPars);
	}

	//завершение списка параметров
	fprintf(f.fc, ")");
}
