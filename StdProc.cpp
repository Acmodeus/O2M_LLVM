//=============================================================================
// Описание объектов - стандартных процедур
//=============================================================================

#include "StdProc.h"
#include "Type.h"
#include "Var.h"

#include <math.h>
#include <float.h>


//-----------------------------------------------------------------------------
//создание константы
int CAbsStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	//получение константы из выражения в формальных параметрах
	int err_num = Expr.CreateConst(BaseConst);
	if (err_num) return err_num;
	//вычисление значения функции из известного значения формального параметра
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
//инициализация процедуры-функции ABS
int CAbsStdProcFunc::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex)
		return s_e_OpBracketMissing;

	//получение выражения
	int err_num = Expr.Init(lb);
	if (err_num) return err_num;

	//проверка типа выражения
	ResultId = Expr.GetResultId();
	if (!CBaseVar::IsDigitId(ResultId)) return s_e_Incompatible;

	//проверка наличия ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex)
		return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//запись кода процедуры-функции ABS
void CAbsStdProcFunc::WriteCPP(CPP_files& f)
{
	if (id_CLongintVar == ResultId) fprintf(f.fc, "l"); else
		if (CBaseVar::IsRealId(ResultId)) fprintf(f.fc, "f");
	fprintf(f.fc, "abs(");
	Expr.WriteCPP(f);
	fprintf(f.fc, ")");
}//WriteCPP


//-----------------------------------------------------------------------------
//создание константы
int CAshStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	//получение константы из x-выражения в формальных параметрах
	CBaseVar* vX;
	int err_num = ExprX.CreateConst(vX);
	if (err_num) return err_num;
	//получение константы из n-выражения в формальных параметрах
	CBaseVar* vN;
	err_num = ExprN.CreateConst(vN);
	if (err_num) {
		delete vX;
		return err_num;
	}
	//создание целочисленной константы, расчет и занесение в нее значения функции
	BaseConst = new CLongintVar(parent_element);
	static_cast<CLongintVar*>(BaseConst)->ConstValue = vX->GetIntValue() << vN->GetIntValue();
	//уничтожение промежуточных констант
	delete vX;
	delete vN;

	return 0;
}


//-----------------------------------------------------------------------------
//инициализация процедуры-функции ASH
int CAshStdProcFunc::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex)
		return s_e_OpBracketMissing;

	//получение первого выражения
	int err_num = ExprX.Init(lb);
	if (err_num) return err_num;

	//проверка наличия ","
	if (!lb->ReadLex(li) || lex_k_comma != li.lex) return s_e_CommaMissing;

	//получение второго выражения
	err_num = ExprN.Init(lb);
	if (err_num) return err_num;

	//проверка типов выражений
	if (!CBaseVar::IsIntId(ExprX.GetResultId())) return s_e_Incompatible;
	if (!CBaseVar::IsIntId(ExprN.GetResultId())) return s_e_Incompatible;
	ResultId = id_CLongintVar;

	//проверка наличия ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex)
		return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//запись кода процедуры-функции ASH
void CAshStdProcFunc::WriteCPP(CPP_files& f)
{
	fprintf(f.fc, "((");
	ExprX.WriteCPP(f);
	fprintf(f.fc, ") << (");
	ExprN.WriteCPP(f);
	fprintf(f.fc, "))");
}//WriteCPP


//-----------------------------------------------------------------------------
//создание константы
int CCapStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	//получение константы из выражения в формальных параметрах
	int err_num = Expr.CreateConst(BaseConst);
	if (err_num) return err_num;
	//вычисление значения функции из известного значения формального параметра
	static_cast<CCharVar*>(BaseConst)->ConstValue = toupper(static_cast<CCharVar*>(BaseConst)->ConstValue);
	return 0;
}


//-----------------------------------------------------------------------------
//инициализация процедуры CAP
int CCapStdProcFunc::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//получение выражения
	int err_num = Expr.Init(lb);
	if (err_num) return err_num;

	//проверка типа выражения
	if (Expr.GetResultId() != id_CCharVar) return s_e_Incompatible;

	//проверка наличия ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	//CAP всегда возвращает значение типа CHAR
	ResultId = id_CCharVar;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//Запись кода процедуры-функции CAP
void CCapStdProcFunc::WriteCPP(CPP_files& f)
{
	//проверка наличия символьной переменной
	CArrayVar* AV = static_cast<CArrayVar*>(Expr.FindLastName());
	if (AV && (id_CArrayVar == AV->name_id) && AV->ConstString) {
		//имеем строку из 1 символа (проверено в Init), выводим 1 символ
		fprintf(f.fc, "toupper('%c')", AV->ConstString[0]);
		return;
	}

	//запись выражения целиком
	fprintf(f.fc, "toupper(");
	Expr.WriteCPP(f);
	fprintf(f.fc, ")");
}//WriteCPP


//-----------------------------------------------------------------------------
//создание константы
int CChrStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	//получение константы из выражения в формальных параметрах
	int err_num = Expr.CreateConst(BaseConst);
	if (err_num) return err_num;
	//получение целочисленного значения
	long int_data = BaseConst->GetIntValue();
	//замена целочисленной константы на символьную
	delete BaseConst;
	BaseConst = new CCharVar(parent_element);
	//вычисление значения функции из известного значения формального параметра
	static_cast<CCharVar*>(BaseConst)->ConstValue = int_data;
	return 0;
}


//-----------------------------------------------------------------------------
//инициализация процедуры-функции CHR
int CChrStdProcFunc::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//получение выражения
	int err_num = Expr.Init(lb);
	if (err_num) return err_num;

	//проверка типа выражения
	if (!CBaseVar::IsIntId(Expr.GetResultId())) return s_e_Incompatible;
	ResultId = id_CCharVar;

	//проверка наличия ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//запись кода процедуры-функции CHR
void CChrStdProcFunc::WriteCPP(CPP_files& f)
{
	Expr.WriteCPP(f);
}//WriteCPP


//-----------------------------------------------------------------------------
//создание константы
int CEntierStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	//получение константы из выражения в формальных параметрах
	int err_num = Expr.CreateConst(BaseConst);
	if (err_num) return err_num;
	//получение вещественного значения
	double real_data;
	switch (BaseConst->GetResultId()) {
	case id_CRealVar:
		real_data = static_cast<CRealVar*>(BaseConst)->ConstValue;
		break;
	case id_CLongrealVar:
		real_data = static_cast<CLongrealVar*>(BaseConst)->ConstValue;
	default:
		//такого быть не должно, см. CEntierStdProcFunc::Init
		real_data = 0;
	}
	//замена вещественной константы на целочисленную (LONGINT)
	delete BaseConst;
	BaseConst = new CLongintVar(parent_element);
	//вычисление значения функции из известного значения формального параметра
	static_cast<CLongintVar*>(BaseConst)->ConstValue = static_cast<long>(floor(real_data));
	return 0;
}


//-----------------------------------------------------------------------------
//инициализация процедуры-функции ENTIER
int CEntierStdProcFunc::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//получение первого выражения
	int err_num = Expr.Init(lb);
	if (err_num) return err_num;

	//проверка типа выражения
	if (!CBaseVar::IsRealId(Expr.GetResultId())) return s_e_Incompatible;
	ResultId = id_CLongintVar;

	//проверка наличия ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//запись кода процедуры-функции ENTIER
void CEntierStdProcFunc::WriteCPP(CPP_files& f)
{
	fprintf(f.fc, "long(floor(");
	Expr.WriteCPP(f);
	fprintf(f.fc, "))");
}//WriteCPP


//-----------------------------------------------------------------------------
//конструктор
CLenStdProcFunc::CLenStdProcFunc(const CBaseName *parent) : CStdProcFunc(parent),
	array_name(NULL),
	dimension(0),
	array_size(0)
{
}


//-----------------------------------------------------------------------------
//деструктор
CLenStdProcFunc::~CLenStdProcFunc()
{
	delete[] array_name;
}


//-----------------------------------------------------------------------------
//создание константы
int CLenStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	//проверка типа массива (для открытого массива нельзя создать константу)
	if (!array_size) return s_e_ExprNotConst;
	//создание константы (тип LONGINT)
	BaseConst = new CLongintVar(parent_element);
	static_cast<CLongintVar*>(BaseConst)->ConstValue = array_size;
	return 0;
}


//-----------------------------------------------------------------------------
//инициализация процедуры LEN
int CLenStdProcFunc::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//результат выражения в любом случае id_CLongintVar
	ResultId = id_CLongintVar;

	//проверка наличия "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//проверка наличия названия массива или текстовой строки
	if (!lb->ReadLex(li) || (lex_i != li.lex && lex_s != li.lex)) return s_e_LEN_NotArray;

	//обработка текстовой строки
	if (lex_s == li.lex) {
		//определение длины строки
		array_size = strlen(li.st);
		if (array_size <= 1) return s_e_LEN_NotArray;
		//проверка наличия ")"
		if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;
		//обработка текстовой строки закончена
		return 0;
	}

	//запоминаем название массива
	array_name = str_new_copy(li.st);

	//проверяем наличие массива с указанным именем
	CBaseName *BN = parent_element->GetGlobalName(array_name);
	if (!BN || (id_CArrayVar != BN->name_id)) return s_e_LEN_NotArray;

	//проверка наличия "," или ")"
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_ClBracketMissing;
	if (lex_k_comma == li.lex) {
		//создание константы
		CBaseVar* BV;
		int err_num = ConstSelector(lb, BV, parent_element);
		if (err_num) return err_num;
		//получение номера требуемой размерности массива
		if (CBaseVar::IsIntId(BV->name_id))
			dimension = static_cast<CBaseVar*>(BV)->GetIntValue();
		else {
			delete BV;
			return s_e_ParamNotIntConst;
		}
		//уничтожение константы (больше не нужна)
		delete BV;
		//проверка наличия след. лексемы
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_ClBracketMissing;
	}
	//проверка наличия ")"
	if (lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	//поверка допустимости значения константы (превышения допустимого количества измерений)
	if (dimension < 0 ||
		dimension >= static_cast<CArrayVar*>(BN)->ArrayType->GetDimCount())
		return s_e_LEN_Dimension;

	//получение описания требуемой размерности массива
	BN = static_cast<CArrayVar*>(BN)->ArrayType->GetType(dimension);
	if (!BN || (id_CArrayType != BN->name_id)) return s_m_Error;	//?!

	//сохранение размера массива в требуемой размерности
	array_size = static_cast<CArrayType*>(BN)->size;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//Запись кода процедуры LEN
void CLenStdProcFunc::WriteCPP(CPP_files& f)
{
	//проверка типа массива (открытый / обычный)
	if (array_size)
		fprintf(f.fc, "%i", array_size);
	else
		fprintf(f.fc, "O2M_ARR_%i_%s", dimension, array_name);
}//WriteCPP


//-----------------------------------------------------------------------------
//создание константы
int CLongStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	//получение константы из выражения в формальных параметрах
	int err_num = Expr.CreateConst(BaseConst);
	if (err_num) return err_num;
	//переменные для временного хранения значения констант
	int int_data;
	double real_data;
	//проверка типа аргумента и замена константы на константу нужного типа
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
//инициализация процедуры-функции LONG
int CLongStdProcFunc::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//получение первого выражения
	int err_num = Expr.Init(lb);
	if (err_num) return err_num;

	//проверка типа аргумента
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

	//проверка наличия ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//запись кода процедуры-функции LONG
void CLongStdProcFunc::WriteCPP(CPP_files& f)
{
	Expr.WriteCPP(f);
}//WriteCPP


//-----------------------------------------------------------------------------
//создание константы
int CMaxStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	//проверка типа, для которого вычисляется максимальное значение
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
		//выбор между SET и INTEGER
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
//инициализация процедуры-функции MAX
int CMaxStdProcFunc::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//инициализация аргумента - основного типа
	CQualident TmpQual;
	int err_num = TmpQual.InitTypeName(lb, parent_element);
	if (err_num) return err_num;

	//допускается указание только базового типа
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

	//проверка наличия ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//запись кода процедуры-функции MAX
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
		//выбор между SET и INTEGER
		fprintf(f.fc, "%i", AppliedToSET ? SET_MAX : INT_MAX);
		break;
	case id_CLongintVar:
		fprintf(f.fc, "%li", LONG_MAX);
		break;
	case id_CRealVar:
	case id_CLongrealVar:
		//double по размеру совпадает с long double
		fprintf(f.fc, "%g", DBL_MAX);
		break;
	}//switch
}


//-----------------------------------------------------------------------------
//создание константы
int CMinStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	//проверка типа, для которого вычисляется минимальное значение
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
		//выбор между SET и INTEGER
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
//инициализация процедуры-функции MIN
int CMinStdProcFunc::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//инициализация аргумента - основного типа
	CQualident TmpQual;
	int err_num = TmpQual.InitTypeName(lb, parent_element);
	if (err_num) return err_num;

	//допускается указание только базового типа
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

	//проверка наличия ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//запись кода процедуры-функции MIN
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
		//выбор между SET и INTEGER
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
		//double по размеру совпадает с long double
		fprintf(f.fc, "%g", DBL_MIN);
		break;
	}//switch
}


//-----------------------------------------------------------------------------
//создание константы
int COddStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	//получение константы из выражения в формальных параметрах
	int err_num = Expr.CreateConst(BaseConst);
	if (err_num) return err_num;
	//получение целочисленного значения
	long int_data = BaseConst->GetIntValue();
	//замена целочисленной константы на булевскую
	delete BaseConst;
	BaseConst = new CBooleanVar(parent_element);
	//вычисление значения функции из известного значения формального параметра
	static_cast<CBooleanVar*>(BaseConst)->ConstValue = int_data % 2 == 1;
	return 0;
}


//-----------------------------------------------------------------------------
//инициализация процедуры-функции ODD
int COddStdProcFunc::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//получение первого выражения
	int err_num = Expr.Init(lb);
	if (err_num) return err_num;

	//проверка типа выражения
	if (!CBaseVar::IsIntId(Expr.GetResultId())) return s_e_Incompatible;
	ResultId = id_CBooleanVar;

	//проверка наличия ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//запись кода процедуры-функции ODD
void COddStdProcFunc::WriteCPP(CPP_files& f)
{
	fprintf(f.fc, "((");
	Expr.WriteCPP(f);
	fprintf(f.fc, ")%%2 == 1)");
}//WriteCPP


//-----------------------------------------------------------------------------
//создание константы
int COrdStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	//получение константы из выражения в формальных параметрах
	int err_num = Expr.CreateConst(BaseConst);
	if (err_num) return err_num;
	//получение символьного значения
	char ch = static_cast<CCharVar*>(BaseConst)->ConstValue;
	//замена символьной константы на целочисленную
	delete BaseConst;
	BaseConst = new CIntegerVar(parent_element);
	//вычисление значения функции из известного значения формального параметра
	static_cast<CIntegerVar*>(BaseConst)->ConstValue = (ch < 0) ? (ch + 256) : ch;
	return 0;
}


//-----------------------------------------------------------------------------
//инициализация процедуры ORD
int COrdStdProcFunc::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//получение выражения
	int err_num = Expr.Init(lb);
	if (err_num) return err_num;

	//проверка типа выражения
	if (Expr.GetResultId() != id_CCharVar) return s_e_Incompatible;

	//проверка наличия ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	ResultId = id_CIntegerVar;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//запись кода процедуры ORD
void COrdStdProcFunc::WriteCPP(CPP_files& f)
{
	//проверка наличия символьной переменной
	CArrayVar* AV = static_cast<CArrayVar*>(Expr.FindLastName());
	if (AV && (id_CArrayVar == AV->name_id) && AV->ConstString) {
		//имеем строку из 1 символа (проверено в Init), выводим 1 символ
		fprintf(f.fc, "ORD('%c')", AV->ConstString[0]);
		return;
	}

	//запись выражения целиком
	fprintf(f.fc, "ORD(");
	Expr.WriteCPP(f);
	fprintf(f.fc, ")");
}//WriteCPP


//-----------------------------------------------------------------------------
//создание константы
int CShortStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	//получение константы из выражения в формальных параметрах
	int err_num = Expr.CreateConst(BaseConst);
	if (err_num) return err_num;
	//переменные для временного хранения значения констант
	int int_data;
	double real_data;
	//проверка типа аргумента и замена константы на константу нужного типа
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
//инициализация процедуры-функции SHORT
int CShortStdProcFunc::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//получение первого выражения
	int err_num = Expr.Init(lb);
	if (err_num) return err_num;

	//проверка типа аргумента
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

	//проверка наличия ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//запись кода процедуры-функции SHORT
void CShortStdProcFunc::WriteCPP(CPP_files& f)
{
	Expr.WriteCPP(f);
}//WriteCPP


//-----------------------------------------------------------------------------
//создание константы
int CSizeStdProcFunc::CreateConst(CBaseVar *&BaseConst)
{
	/**/
	//не до конца корректная реализация
	//реализовать в дальнейшем

	//для предварительного вычисления размера типа
	int sz;

	switch (Qualident.TypeResultId) {
	case id_CArrayType:
		/**/
		//заменить на вызов ф-ции
		sz = 10000;
		break;
	case id_CRecordType:
		/**/
		//заменить на вызов ф-ции
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

	//выбор минимального целого типа, достаточного для хранения значения
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
//инициализация процедуры-функции SIZE
int CSizeStdProcFunc::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//проверка наличия описания любого типа
	int err_num = Qualident.InitTypeName(lb, parent_element);
	if (err_num) return err_num;

	//проверка наличия ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	//на самом деле здесь может быть любой целый тип
	ResultId = id_CIntegerVar;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//запись кода процедуры-функции SIZE
void CSizeStdProcFunc::WriteCPP(CPP_files& f)
{
	fprintf(f.fc, "sizeof(");
	Qualident.WriteCPP_type(f, false, parent_element);
	fprintf(f.fc, ")");
}//WriteCPP


//-----------------------------------------------------------------------------
//инициализация процедуры NEW
int CNewStdProc::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//получение названия указателя
	int err_num = Des.Init(lb);
	if (err_num) return err_num;

	//получение переменной-указателя по обозначению
	if (Des.IsReadOnly()) return s_e_OperandReadOnly;
	CPointerVar* pVar = static_cast<CPointerVar*>(Des.FindLastName());
	if (!pVar) return s_e_UndeclaredIdent;
	if (id_CPointerVar != pVar->name_id) return s_e_OperandInappl;


	//////////////////////////////
	//проверка получения обобщения

	CBaseType* BT = pVar->FindType();
	if (id_CCommonType == BT->name_id) {

		//проверка наличия "<"
		if (!lb->ReadLex(li) || lex_k_lt != li.lex) return s_e_OpAngleMissing;

		//проверка наличия ">" (используется специализация по умолчанию)
		DECL_SAVE_POS
		if (!lb->ReadLex(li) || lex_k_gt != li.lex) {
			RESTORE_POS
		} else {
			//получение параметров специализации по умолчанию
			const CCommonType::SSpec* spec = static_cast<CCommonType*>(BT)->GetDefaultSpec();
			if (!spec) return s_e_NoDefaultSpec;
			//сохранение параметров специализации по умолчанию
			if (spec->Tag)
				qual.ident = str_new_copy(spec->Tag);
			else {
				qual.ident = str_new_copy(spec->Name);
				if (spec->QualName) qual.pref_ident = str_new_copy(spec->QualName);
			}
			//проверка наличия ")"
			if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;
			return 0;
		}

		//инициализация описания признака
		int err_num = qual.Init(lb, parent_element);
		if (err_num) return err_num;
		//проверка допустимого признака
		if (!static_cast<CCommonType*>(BT)->FindSpec(qual.pref_ident, qual.ident, qual.ident)) return s_e_SpecTypeTag;

		//проверка наличия ">"
		if (!lb->ReadLex(li) || lex_k_gt != li.lex) return s_e_ClAngleMissing;

		//проверка наличия ")"
		if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

		return 0;
	}


	///////////////////////////////////
	//проверка наличия списка выражений

	//проверка наличия "," или ")"
	if (!lb->ReadLex(li) || lex_k_comma != li.lex)
		if (lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;
		else {
			//NEW содержит только 1 параметр => он должен быть записью или ук. на массив фиксированной длины
			if (pVar->IsArrayPointer()) {
				CArrayType *AT = static_cast<CArrayType*>(pVar->FindType());
				while (id_CArrayType == AT->name_id) {
					if (!AT->size) return s_e_ParsFewer;
					//переход к след. размерности (если есть)
					AT = static_cast<CArrayType*>(AT->Type);
				}//while
			}//if
			return 0;
		}

	while (true) {
		//проверка наличия Expr
		CExpr* Ex = new CExpr(parent_element);
		int err_num = Ex->Init(lb);
		if (err_num) {
			delete Ex;
			return err_num;
		}

		//занесение Expr в список выражений
		ExprStore.push_back(Ex);

		//проверка наличия ","
		if (!lb->ReadLex(li) || lex_k_comma != li.lex) break;
	}

	//проверка наличия ")"
	if (lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	//проверка типа, на кот. указывает переменная-указатель (должен быть массив)
	if (!pVar->IsArrayPointer()) return s_e_OperandInappl;
	//проверка соответствия количества размерностей количеству выражений в списке
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
//Запись кода процедуры NEW
void CNewStdProc::WriteCPP(CPP_files& f)
{
	//получение переменной по обозначению
	CPointerVar* pVar = static_cast<CPointerVar*>(Des.FindLastName());
	//получения типа по переменной
	CBaseType* BT = pVar->FindType();

	//генерация отступа
	f.tab_fc();


	////////////////////////////////////////////////////
	//сохранение размерностей в случае открытого массива
	if (!ExprStore.empty()) {
		//отсчет размерностей с 0
		int dimension = 0;
		//генерация кода сохранения размерностей в переменные
		TExprStore::const_iterator ci;
		for (ci = ExprStore.begin(); ci != ExprStore.end(); ++ci) {
			//генерация имени переменной
			fprintf(f.fc, "O2M_ARR_%i_%s = ", dimension, pVar->name);
			//генерация значения размерности
			(*ci)->WriteCPP(f);
			//генерация перевода строки и отступа
			fprintf(f.fc, ";\n");
			f.tab_fc();
			//переход к следующей размерности
			++dimension;
		}//for
	}//if


	//запись имени переменной (первого выражения)
	Des.WriteCPP(f);


	////////////////////////////
	//проверка наличия обобщения
	if (id_CCommonType == BT->name_id) {
		//генерация кода вызова ф-ции new
		fprintf(f.fc, " = new ");
		//генерация имени типа
		const char* ma = BT->GetModuleAlias();
		if (ma) fprintf(f.fc, "%s::", ma);
		fprintf(f.fc, "%s(", BT->name);
		//генерация параметра конструктора типа
		if (ma) fprintf(f.fc, "%s::", ma);
		fprintf(f.fc, "%s::O2M_INIT_", BT->name);
		//создание переменной для получения CompoundName
		CBaseVar* BV;
		static_cast<CCommonType*>(BT)->CreateVar(BV, parent_element);
		static_cast<CCommonVar*>(BV)->SetTagName(qual.pref_ident, qual.ident);
		fprintf(f.fc, "%s)", static_cast<CCommonVar*>(BV)->GetCPPCompoundName());
		delete BV;
		return;
	}

	///////////////////////////////////////////////////////////////
	//проверка наличия специализации (параметризованного обобщения)
	if (id_CSpecType == BT->name_id) {
		//генерация кода вызова ф-ции new
		fprintf(f.fc, " = new ");
		//генерация имени типа
		const char* ma = BT->GetModuleAlias();
		if (ma) fprintf(f.fc, "%s::", ma);
		fprintf(f.fc, "%s(", BT->name);
		//генерация параметра конструктора типа
		if (ma) fprintf(f.fc, "%s::", ma);
		fprintf(f.fc, "%s::O2M_INIT_", BT->name);
		//создание переменной для получения CompoundName
		CBaseVar* BV;
		static_cast<CSpecType*>(BT)->CreateVar(BV, parent_element);
		fprintf(f.fc, "%s)", static_cast<CCommonVar*>(BV)->GetCPPCompoundName());
		delete BV;
		return;
	}


	////////////////////////////////////////////////////
	//обобщение отсутствует, генерация кода обычного new
	fprintf(f.fc, " = new(");

	//запись названия модуля, если есть (может присутствовать и в случае неименованного типа)
	if (pVar->GetTypeModuleAlias()) fprintf(f.fc, "%s::", pVar->GetTypeModuleAlias());

	//проверка наличия именованного типа
	if (pVar->GetTypeName())
		fprintf(f.fc, "%s", pVar->GetTypeName());
	else //неименованный тип => используем искусственное имя (O2M_UNNM_<имя переменной>)
		fprintf(f.fc, "O2M_UNNM_%s", pVar->name);

	//проверка наличия списка выражений с размерностями
	if (ExprStore.empty()) {
		//запись размерностей в случае ук. на массив фиксированной длины
		if (pVar->IsArrayPointer()) {
			CArrayType *AT = static_cast<CArrayType*>(pVar->FindType());
			while (AT->name_id == id_CArrayType) {
				fprintf(f.fc, "[%i]", AT->size);
				//переход к след. размерности (если есть)
				AT = static_cast<CArrayType*>(AT->Type);
			}//while
		}//if
	} else {
		//запись размерностей в случае ук. на открытый массив
		fprintf(f.fc, "[");
		//содержимое переменных, установленных выше, перемножается
		for (int dim = 0; dim < ExprStore.size(); dim++)
			fprintf(f.fc, "%sO2M_ARR_%i_%s", dim ? " * " : "", dim, pVar->name);
		fprintf(f.fc, "]");
	}//else

	fprintf(f.fc, ")");

}//WriteCPP


//-----------------------------------------------------------------------------
//инициализация процедуры COPY
int CCopyStdProc::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//создание первого выражения
	int err_num = ExprSource.Init(lb);
	if (err_num) return err_num;

	//проверка наличия символьного массива (строки)
	if (ExprSource.GetResultId() != id_CArrayVar) return s_e_ExprCompatibility;

	//проверка наличия ","
	if (!lb->ReadLex(li) || lex_k_comma != li.lex) return s_e_CommaMissing;

	//создание второго выражения
	err_num = ExprDest.Init(lb);
	if (err_num) return err_num;

	//проверка наличия l-value во втором выражении
	CBaseName* BN = ExprDest.FindLValue();
	if (!BN) return s_e_OperandNotVariable;
	if (ExprDest.IsReadOnly()) return s_e_OperandReadOnly;
	//проверка допустимости второго выражения
	switch (BN->GetResultId()) {
	case id_CArrayVar:
		/**/
		//ввести проверку типа эл-тов массива
		break;
	case id_CPointerVar:
		/**/
		//ввести проверку типа указателя
		break;
	default:
		return s_e_OperandInappl;
	}//switch

	//проверка наличия ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//Запись кода процедуры COPY
void CCopyStdProc::WriteCPP(CPP_files& f)
{
	f.tab_fc();
	fprintf(f.fc, "COPY(");

	//запись значения первого параметра
	ExprSource.WriteCPP(f);
	fprintf(f.fc, ", ");
	//получение первого параметра процедуры (переменной-массива или указателя)
	CBaseName* BN = ExprSource.FindLastName();
	WriteCPP_COPY_Par(f, BN);
	fprintf(f.fc, ", ");

	//запись значения второго параметра
	ExprDest.WriteCPP(f);
	fprintf(f.fc, ", ");
	//получение второго параметра процедуры (переменной-массива или указателя)
	BN = ExprDest.FindLastName();
	WriteCPP_COPY_Par(f, BN);
	fprintf(f.fc, ")");

}//WriteCPP


//-----------------------------------------------------------------------------
//инициализация процедуры ASSERT
int CAssertStdProc::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//проверка наличия логич. выражения
	int err_num = Expr.Init(lb);
	if (err_num) return err_num;

	//проверка типа логического выражения
	if (Expr.GetResultId() != id_CBooleanVar) return s_e_ASSERT_ExprType;

	//проверка наличия "," или ")"
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_ClBracketMissing;
	if (lex_k_comma == li.lex) {
		//создание константы (второго параметра)
		CBaseVar* BV;
		err_num = ConstSelector(lb, BV, parent_element);
		if (err_num) return err_num;
		//проверка типа второго параметра
		if (!CBaseVar::IsIntId(BV->GetResultId())) {
			delete BV;
			return s_e_ParamNotIntConst;
		}
		//получение значения целочисленной константы и ее уничтожение
		AssertVal = BV->GetIntValue();
		delete BV;
		//проверка наличия след. лексемы
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_ClBracketMissing;
	}
	//проверка наличия ")"
	if (lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//Запись кода процедуры ASSERT
void CAssertStdProc::WriteCPP(CPP_files& f)
{
	f.tab_fc();
	fprintf(f.fc, "if (!(");
	Expr.WriteCPP(f);
	fprintf(f.fc, ")) exit(%li)", AssertVal);
}//WriteCPP


//-----------------------------------------------------------------------------
//инициализация процедуры HALT
int CHaltStdProc::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//проверка наличия целой константы
	CBaseVar* BV;
	int err_num = ConstSelector(lb, BV, parent_element);
	if (err_num) return err_num;

	//проверка типа второго параметра
	if (!CBaseVar::IsIntId(BV->GetResultId())) {
		delete BV;
		return s_e_ParamNotIntConst;
	}
	//получение значения целочисленной константы и ее уничтожение
	HaltVal = BV->GetIntValue();
	delete BV;

	//проверка наличия ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//Запись кода процедуры HALT
void CHaltStdProc::WriteCPP(CPP_files& f)
{
	f.tab_fc();
	fprintf(f.fc, "exit(%li)", HaltVal);
}//WriteCPP


//-----------------------------------------------------------------------------
//инициализация процедуры DEC
int CDecStdProc::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//получение первого выражения
	int err_num = Expr1.Init(lb);
	if (err_num) return err_num;

	//проверка наличия l-value в первом выражении
	if (!Expr1.FindLValue()) return s_e_OperandNotVariable;
	if (Expr1.IsReadOnly()) return s_e_OperandReadOnly;
	//проверка допустимости первого выражения
	if (!CBaseVar::IsIntId(Expr1.GetResultId())) return s_e_OperandInappl;

	//проверка наличия запятой или ")"
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_ClBracketMissing;
	if (lex_k_comma == li.lex) {
		//создание выражения для второго аргумента
		Expr2 = new CExpr(parent_element);
		err_num = Expr2->Init(lb);
		if (err_num) return err_num;
		if (!CBaseVar::IsIntId(Expr2->GetResultId())) return s_e_OperandInappl;
		//проверка след. лексемы
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_ClBracketMissing;
	}
	//проверка наличия ")"
	if (lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//Запись кода процедуры DEC
void CDecStdProc::WriteCPP(CPP_files& f)
{
	//запись имени переменной (первого выражения)
	f.tab_fc();
	Expr1.WriteCPP(f);
	fprintf(f.fc, " -= ");

	//запись второго выражения (если есть)
	if (Expr2) Expr2->WriteCPP(f); else fprintf(f.fc, "1");
}//WriteCPP


//-----------------------------------------------------------------------------
//инициализация процедуры EXCL
int CExclStdProc::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//получение первого выражения
	int err_num = Expr1.Init(lb);
	if (err_num) return err_num;

	//проверка наличия l-value в первом выражении
	if (!Expr1.FindLValue()) return s_e_OperandNotVariable;
	if (Expr1.IsReadOnly()) return s_e_OperandReadOnly;
	//проверка допустимости первого выражения
	if (id_CSetVar != Expr1.GetResultId()) return s_e_OperandInappl;

	//проверка наличия ","
	if (!lb->ReadLex(li) || lex_k_comma != li.lex) return s_e_CommaMissing;

	//создание выражения для второго аргумента
	err_num = Expr2.Init(lb);
	if (err_num) return err_num;
	if (!CBaseVar::IsIntId(Expr2.GetResultId())) return s_e_Incompatible;

	//проверка наличия ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//Запись кода процедуры EXCL
void CExclStdProc::WriteCPP(CPP_files& f)
{
	f.tab_fc();
	Expr1.WriteCPP(f);
	fprintf(f.fc, " &= ~(1 << (");
	Expr2.WriteCPP(f);
	fprintf(f.fc, "))");
}//WriteCPP


//-----------------------------------------------------------------------------
//инициализация процедуры INC
int CIncStdProc::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//получение первого выражения
	int err_num = Expr1.Init(lb);
	if (err_num) return err_num;

	//проверка наличия l-value в первом выражении
	if (!Expr1.FindLValue()) return s_e_OperandNotVariable;
	if (Expr1.IsReadOnly()) return s_e_OperandReadOnly;
	//проверка допустимости первого выражения
	if (!CBaseVar::IsIntId(Expr1.GetResultId())) return s_e_OperandInappl;

	//проверка наличия запятой или ")"
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_ClBracketMissing;
	if (lex_k_comma == li.lex) {
		//создание выражения для второго аргумента
		Expr2 = new CExpr(parent_element);
		err_num = Expr2->Init(lb);
		if (err_num) return err_num;
		if (!CBaseVar::IsIntId(Expr2->GetResultId())) return s_e_Incompatible;
		//проверка след. лексемы
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_ClBracketMissing;
	}

	//проверка наличия ")"
	if (lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//Запись кода процедуры INC
void CIncStdProc::WriteCPP(CPP_files& f)
{
	//запись имени переменной (первого выражения)
	f.tab_fc();
	Expr1.WriteCPP(f);
	fprintf(f.fc, " += ");

	//запись второго выражения (если есть)
	if (Expr2) Expr2->WriteCPP(f); else fprintf(f.fc, "1");
}//WriteCPP


//-----------------------------------------------------------------------------
//инициализация процедуры INCL
int CInclStdProc::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия "("
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) return s_e_OpBracketMissing;

	//получение первого выражения
	int err_num = Expr1.Init(lb);
	if (err_num) return err_num;

	//проверка наличия l-value в первом выражении
	if (!Expr1.FindLValue()) return s_e_OperandNotVariable;
	if (Expr1.IsReadOnly()) return s_e_OperandReadOnly;
	//проверка допустимости первого выражения
	if (id_CSetVar != Expr1.GetResultId()) return s_e_OperandInappl;

	//проверка наличия ","
	if (!lb->ReadLex(li) || lex_k_comma != li.lex) return s_e_ColonMissing;

	//создание выражения для второго аргумента
	err_num = Expr2.Init(lb);
	if (err_num) return err_num;
	if (!CBaseVar::IsIntId(Expr2.GetResultId())) return s_e_Incompatible;

	//проверка наличия ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init


//-----------------------------------------------------------------------------
//Запись кода процедуры INCL
void CInclStdProc::WriteCPP(CPP_files& f)
{
	f.tab_fc();
	Expr1.WriteCPP(f);
	fprintf(f.fc, " |= 1 << (");
	Expr2.WriteCPP(f);
	fprintf(f.fc, ")");
}//WriteCPP


//-----------------------------------------------------------------------------
//деструктор
CNewStdProc::~CNewStdProc()
{
	//очистка списка выражений (размерностей)
	TExprStore::iterator i;
	for (i = ExprStore.begin(); i != ExprStore.end(); ++i)
		delete *i;
}


//-----------------------------------------------------------------------------
//запись кода размерности массива в качестве параметра при вызове COPY
//предполагается, что BN является массивом символов или ук. на массив символов
void WriteCPP_COPY_Par(CPP_files &f, CBaseName* BN)
{
	const char *arr_name = BN->name;
	//в случае указаметя получаем тип указываемого объекта
	if (id_CPointerVar == BN->name_id) BN = static_cast<CPointerVar*>(BN)->FindType();
	//получение размера для переменной или типа
	int arr_size;
	if (id_CArrayVar == BN->name_id)
		arr_size = static_cast<CArrayVar*>(BN)->ArrayType->size;
	else
		arr_size = static_cast<CArrayType*>(BN)->size;
	//запись размера (если массив открытый (размер 0) - используется переменная)
	if (arr_size)
		fprintf(f.fc, "%i", arr_size);
	else
		fprintf(f.fc, "O2M_ARR_0_%s", arr_name);
}

