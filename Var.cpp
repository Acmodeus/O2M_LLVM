//=============================================================================
// Описание классов переменных (Var)
//=============================================================================

#include "Var.h"


//-----------------------------------------------------------------------------
//деструктор объекта CArrayVar
CArrayVar::~CArrayVar()
{
	delete ArrayType;
	delete[] ConstString;
}//~CArrayVar


//-----------------------------------------------------------------------------
//проверка завершенности типа эл-тов массива
int CArrayVar::CheckComplete(CLexBuf *lb)
{
	return ArrayType->CheckComplete(lb);
}


//-----------------------------------------------------------------------------
//создание константы, подразумевается, что is_const == true
CBaseVar* CArrayVar::CreateConst(const CBaseName* parent) const
{
	//константное значение массива возможно только для строк
	if (!ConstString) return CBaseVar::CreateConst(parent);

	//создание константной переменной
	CArrayVar* AV = static_cast<CArrayVar*>(CArrayVar::CreateVar(parent));

	//копирование данных
	AV->ConstString = str_new_copy(ConstString);
	AV->ConstStringIsChar = ConstStringIsChar;

	return AV;
}


//-----------------------------------------------------------------------------
//создание копии переменной (без копирования данных в случае констант)
CBaseVar* CArrayVar::CreateVar(const CBaseName* parent) const
{
	CArrayVar* AV = new CArrayVar(parent);
	Assign(AV);

	//копирование типа в переменную
	CBaseType *BT = NULL;
	ArrayType->CreateType(BT);
	AV->ArrayType = static_cast<CArrayType*>(BT);

	return AV;
}


//-----------------------------------------------------------------------------
//инициализация массива строкой символов
int CArrayVar::SetConstValue(const char *st)
{
	is_const = true;
	ConstString = str_new_copy(st);
	return 0;
}


//-----------------------------------------------------------------------------
//Запись кода CArrayVar при объявлении переменной
void CArrayVar::WriteCPP(CPP_files& f)
{
	//поиск описания типа эл-тов массива
	CArrayType *AT = static_cast<CArrayType*>(ArrayType->FindLastType());
	
	//объявление массива отличается наличием "[" размера "]" от объявления переменной => пишем объявление переменной
	CBaseVar* BV;
	static_cast<CBaseType*>(AT)->CreateVar(BV, parent_element);
	BV->external = external;				//для записи в файлы .h/.cpp (при необходимости)
	BV->name = name;						//используется название массива для записи первой части объявления массива
	BV->WriteCPP(f);
	BV->name = NULL;						//использовано название от массива => его нельзя удалять с временной переменной
	delete BV;

	//объявление переменной уже записано => пишем размеры массива по каждому измерению
	AT = ArrayType;
	//перебор вложенных типов-массивов (если есть)
	while (AT->name_id == id_CArrayType) {
		//запись текущего размера (0 если открытый массив)
		if (external) fprintf(f.fh, "[%i]", AT->size);
		fprintf(f.fc, "[%i]", AT->size);
		//переход к след. размеру (точнее к след. типу, если есть)
		AT = static_cast<CArrayType*>(AT->Type);
	}
}//WriteCPP


//-----------------------------------------------------------------------------
//запись кода объявления массива в параметрах процедуры (добавляется объявление переменных для передачи действительных размеров открытых массивов)
void CArrayVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	TFileType *file = to_h ? f.fh : f.fc;
	CArrayType *AT = static_cast<CArrayType*>(ArrayType->FindLastType());

	CBaseVar* BV;
	static_cast<CBaseType*>(AT)->CreateVar(BV, parent_element);

	//генерация имени
	char *formal_name;
	if (!is_var) {//генерация имени для переданного массива-значения
		formal_name = new char[strlen("O2M_ARR_#") + strlen(name)];
		strcpy(formal_name, "O2M_ARR_");
		strcat(formal_name, name);
	} else formal_name = name;

	//проверка типа массива (открытый / не открытый)
	if (ArrayType->size == 0) {
		if (BV->GetTypeModuleName()) fprintf(file, "%s::", to_h ? BV->GetTypeModuleName() : BV->GetTypeModuleAlias());
		fprintf(file, "%s *%s", BV->GetTypeName(), formal_name);
	} else {
		BV->name = formal_name;
		BV->WriteCPP_fp(f, to_h);
		BV->name = NULL;
	}

	//уничтожение сгенерированного имени
	if (!is_var) delete[] formal_name;
	delete BV;

	//запись размеров массива, если не открытый массив
	AT = ArrayType;
	if (AT->size != 0)
		while (AT->name_id == id_CArrayType) {
			fprintf(file, "[%i]", AT->size);
			AT = static_cast<CArrayType*>(AT->Type);
		}

	//запись размерностей массива в виде переменных для открытого массива
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
//запись объявления поля-массива в записи (в отличие от остальных переменных здесь нельзя использовать WriteCPP_fp)
void CArrayVar::WriteCPP_rec(CPP_files& f, const bool to_h)
{
	TFileType *file = to_h ? f.fh : f.fc;
	CArrayType *AT = static_cast<CArrayType*>(ArrayType->FindLastType());

	CBaseVar* BV;
	static_cast<CBaseType*>(AT)->CreateVar(BV, parent_element);

	//проверка типа массива (открытый / не открытый)
	if (ArrayType->size == 0) {
		if (BV->GetTypeModuleName()) fprintf(file, "%s::", to_h ? BV->GetTypeModuleName() : BV->GetTypeModuleAlias());
		fprintf(file, "%s *%s", BV->GetTypeName(), name);
	} else {
		BV->name = name;
		BV->WriteCPP_fp(f, to_h);
		BV->name = NULL;
	}

	delete BV;

	//запись размеров массива, если не открытый массив
	AT = ArrayType;
	if (AT->size != 0) {
		while (AT->name_id == id_CArrayType) {
			fprintf(file, "[%i]", AT->size);
			AT = static_cast<CArrayType*>(AT->Type);
		}
	} else throw error_Internal("CArrayVar::WriteCPP_rec");

}//WriteCPP_rec


//-----------------------------------------------------------------------------
//запись кода при использовании константного значения, подразумевается, что is_const == true
void CArrayVar::WriteCPP_ConstValue(CPP_files& f)
{
	//константное значение массива возможно только для строк
	if (!ConstString) throw error_Internal("CArrayVar::WriteCPP_ConstValue");

	//проверка строки на наличие недопустимых символов
	char *new_st = new char[strlen(ConstString)*2 + 1];

	int i = 0;
	int j = 0;
	for (i = 0; i < strlen(ConstString); i++, j++) {
		//проверка наличия '\'
		if (ConstString[i] == '\\') {
			new_st[j] = '\\';
			new_st[++j] = '\\';
			continue;
		}
		//проверка наличия '"'
		if (ConstString[i] == '"') {
			new_st[j] = '\\';
			new_st[++j] = '"';
			continue;
		}
		//запрещенного символа не найдено
		new_st[j] = ConstString[i];
	}
	new_st[j] = '\0';

	//собственно вывод строки (или символа)
	if (strlen(ConstString) == 1 && ConstStringIsChar)
		fprintf(f.fc, "'%s'", new_st);
	else
		fprintf(f.fc, "\"%s\"", new_st);

	delete[] new_st;
}


//-----------------------------------------------------------------------------
//Запись кода CArrayVar в файл .dfn
void CArrayVar::WriteDFN(DFN_file& f)
{
	//в случае строковой константы пишем только данные (строку символов)
	if (is_const) {
		fprintf(f.f, "%s = \"%s\"", name, ConstString);
		return;
	}

	//запись названия переменной
	fprintf(f.f, "%s%s : ", name, is_read_only ? "-" : "");
	//перечисление вложенных массивов
	CArrayType *AT = ArrayType;
	while (AT->name_id == id_CArrayType) {
		fprintf(f.f, "ARRAY ");
		if (AT->size != 0) fprintf(f.f, "%i ", AT->size);
		fprintf(f.f, "OF ");
		AT = static_cast<CArrayType*>(AT->Type);
	}
	//запись описания типа
	AT->WriteDFN(f);

}//WriteDFN


//-----------------------------------------------------------------------------
//создание константы, подразумевается, что is_const == true
CBaseVar* CBooleanVar::CreateConst(const CBaseName *parent) const
{
	CBooleanVar* BV = static_cast<CBooleanVar*>(CBooleanVar::CreateVar(parent));

	//копирование данных
	BV->ConstValue = ConstValue;

	return BV;
}


//-----------------------------------------------------------------------------
//создание копии переменной (без копирования данных в случае констант)
CBaseVar* CBooleanVar::CreateVar(const CBaseName* parent) const
{
	CBaseVar* BV = new CBooleanVar(parent);
	Assign(BV);
	return BV;
}


//-----------------------------------------------------------------------------
//Запись кода CBooleanVar при объявлении переменной
void CBooleanVar::WriteCPP(CPP_files& f)
{
	//запись декларации переменной
	if (external) {
		fprintf(f.fh, "extern %s %s", CBooleanType::GetCPPTypeName(), name);
		fprintf(f.fc, "%s %s::%s%s", CBooleanType::GetCPPTypeName(), parent_element->name, is_var ? "&" : "", name);
	} else
		fprintf(f.fc, "%s %s%s", CBooleanType::GetCPPTypeName(), is_var ? "&" : "", name);

	//запись инициализации константы (если надо)
	if (is_const) fprintf(f.fc, " = %s", ConstValue ? "true" : "false");
}//WriteCPP


//-----------------------------------------------------------------------------
//запись кода переменной в параметрах процедуры
void CBooleanVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	//проверка наличия именованного типа
	if (type_name)
		WriteCPP_fp_named_type(f, to_h);
	else
		fprintf(to_h ? f.fh : f.fc, "%s %s%s", CBooleanType::GetCPPTypeName(), is_var ? "&" : "", name);
}


//-----------------------------------------------------------------------------
//запись кода при использовании константного значения, подразумевается, что is_const == true
void CBooleanVar::WriteCPP_ConstValue(CPP_files& f)
{
	fprintf(f.fc, "%s", ConstValue ? "true" : "false");
}


//-----------------------------------------------------------------------------
//Запись кода CBooleanVar в файл .dfn
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
//создание константы, подразумевается, что is_const == true
CBaseVar* CCharVar::CreateConst(const CBaseName *parent) const
{
	CCharVar* CV = static_cast<CCharVar*>(CCharVar::CreateVar(parent));

	//копирование данных
	CV->ConstValue = ConstValue;

	return CV;
}


//-----------------------------------------------------------------------------
//создание копии переменной (без копирования данных в случае констант)
CBaseVar* CCharVar::CreateVar(const CBaseName* parent) const
{
	CBaseVar* BV = new CCharVar(parent);
	Assign(BV);
	return BV;
}


//-----------------------------------------------------------------------------
//преобразование указанной строки в значение переменной
void CCharVar::SetConstValue(const char ch)
{
	is_const = true;
	ConstValue = ch;
}


//-----------------------------------------------------------------------------
//Запись кода CCharVar при объявлении переменной
void CCharVar::WriteCPP(CPP_files& f)
{
	//запись декларации переменной
	if (external) {
		fprintf(f.fh, "extern %s %s", CCharType::GetCPPTypeName(), name);
		fprintf(f.fc, "%s %s::%s%s", CCharType::GetCPPTypeName(), parent_element->name, is_var ? "&" : "", name);
	} else
		fprintf(f.fc, "%s %s%s", CCharType::GetCPPTypeName(), is_var ? "&" : "", name);

	//запись инициализации константы (если надо)
	if (is_const) fprintf(f.fc, " = %i", ConstValue);
}//WriteCPP


//-----------------------------------------------------------------------------
//запись кода переменной в параметрах процедуры
void CCharVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	//проверка наличия именованного типа
	if (type_name)
		WriteCPP_fp_named_type(f, to_h);
	else
		fprintf(to_h ? f.fh : f.fc, "%s %s%s", CCharType::GetCPPTypeName(), is_var ? "&" : "", name);
}


//-----------------------------------------------------------------------------
//запись кода при использовании константного значения, подразумевается, что is_const == true
void CCharVar::WriteCPP_ConstValue(CPP_files& f)
{
	fprintf(f.fc, "%i", ConstValue);
}


//-----------------------------------------------------------------------------
//Запись кода CCharVar в файл .dfn
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
//создание копии переменной (без копирования данных в случае констант)
CBaseVar* CCommonVar::CreateVar(const CBaseName* parent) const
{
	CCommonVar* CV = new CCommonVar(parent);
	Assign(CV);

	//копирование параметров признака в переменную
	if (QualSpecName) CV->QualSpecName = str_new_copy(QualSpecName);
	if (SpecName) CV->SpecName = str_new_copy(SpecName);
	if (Tag) CV->Tag = str_new_copy(Tag);
	if (CPPCompoundName) CV->CPPCompoundName = str_new_copy(CPPCompoundName);

	return CV;
}


//-----------------------------------------------------------------------------
//поиск имени в таблице имен
CBaseName* CCommonVar::FindName(const char *search_name) const
{
	//получение типа специализации
	CBaseType* BT = FindType();
	if (!BT) return NULL;
	//поиск имени в таблице имен типа специализации
	return BT->FindName(search_name);
}


//-----------------------------------------------------------------------------
//поиск типа специализации (если есть)
CBaseType* CCommonVar::FindType() const
{
	//получение ук. на обобщенный тип
	CCommonType* CT = static_cast<CCommonType*>(parent_element->GetGlobalName(type_module_name, type_name));
	if (!CT || id_CCommonType != CT->name_id) return NULL;

	//получение описания типа специализации по обобщенному типу и признаку
	const CCommonType::SSpec* spec = CT->FindSpec(QualSpecName, SpecName, Tag);
	if (!spec) return NULL;

	//получение названия модуля, содержащего тип специализации
	const char* ModuleName = spec->QualName ? spec->QualName : (spec->IsExtended ? NULL : CT->GetModuleAlias());

	//получение типа специализации из текущей области видимости по описанию типа специализации
	CBaseName* BN = parent_element->GetGlobalName(ModuleName, spec->Name);
	if (!BN || !CBaseType::IsTypeId(BN->name_id)) return NULL;

	return static_cast<CBaseType*>(BN);
}


//-----------------------------------------------------------------------------
//получение имени признака
void CCommonVar::GetTagName(const char* &QualName, const char* &Name, const char* &TagName)
{
	QualName = QualSpecName;
	Name = SpecName;
	TagName = Tag;
}


//-----------------------------------------------------------------------------
//проверка отсутствия параметризации переменной конкретным типом
bool CCommonVar::IsPureCommon()
{
	return (NULL == Tag) && (NULL == SpecName);
}


//-----------------------------------------------------------------------------
//установка имени признака
int CCommonVar::SetTagName(const char *QualName, const char *Name)
{
	//зарещен повторный вызов данной процедуры
	if (QualSpecName || Tag) return s_e_CommonTypeExpected;

	//получение ук. на обобщенный тип
	CCommonType* CT = static_cast<CCommonType*>(parent_element->GetGlobalName(type_module_name, type_name));
	if (!CT || id_CCommonType != CT->name_id) return s_m_Error;

	//получение описания типа специализации по обобщенному типу и признаку
	const CCommonType::SSpec* spec = CT->FindSpec(QualName, Name, Name);
	if (!spec) return s_e_SpecTypeTag;

	//копирование параметров признака
	if (spec->Tag) Tag = str_new_copy(Name);
	if (spec->QualName) QualSpecName = str_new_copy(spec->QualName);
	if (spec->Name) SpecName = str_new_copy(spec->Name);	//NULL == spec->Name при типе NIL

	//установка составного имени, используемого при генерации кода
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
//Запись кода CCommonVar при объявлении переменной
void CCommonVar::WriteCPP(CPP_files& f)
{
	//проверка наличия имени типа
	if (type_name) {
		//запись имени модуля перед типом переменной		
		if (type_module_name) fprintf(f.fc, "%s::", type_module_name);

		//проверка, экспортируется ли переменная
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

		//генерация кода вызова конструктора, специализирующего обобщение
		if (type_module_name) fprintf(f.fc, "%s::", type_module_name);
		fprintf(f.fc, "%s::O2M_INIT_%s)", type_name, CPPCompoundName);
	}
}//WriteCPP


//-----------------------------------------------------------------------------
//запись кода переменной в параметрах процедуры
void CCommonVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	if (to_h) {//запись кода в .h файл
		if (type_module_name) {
			//поиск модуля по его псевдониму
			CBaseName* BN = parent_element->GetGlobalName(type_module_name);
			//запись префикса по настоящему имени модуля
			if ( BN && (BN->name_id == id_CImportModule) )
				fprintf(f.fh, "%s::", static_cast<CImportModule*>(BN)->real_name);
		}
		//запись типа и названия переменной
		fprintf(f.fh, "%s* %s", type_name, name);
	} else {//запись кода в .cpp файл
		if (type_module_name) fprintf(f.fc, "%s::", type_module_name);
		fprintf(f.fc, "%s* %s", type_name, name);
	}//else
}


//-----------------------------------------------------------------------------
//Запись кода CCommonVar в файл .dfn
void CCommonVar::WriteDFN(DFN_file& f)
{
	//обобщенная переменная не может быть константой и всегда именованного типа
	WriteDFN_named_type(f);
}//WriteDFN


//-----------------------------------------------------------------------------
//создание константы, подразумевается, что is_const == true
CBaseVar* CIntegerVar::CreateConst(const CBaseName *parent) const
{
	CIntegerVar* IV = static_cast<CIntegerVar*>(CIntegerVar::CreateVar(parent));

	//копирование данных
	IV->ConstValue = ConstValue;
	
	return IV;
}


//-----------------------------------------------------------------------------
//создание копии переменной (без копирования данных в случае констант)
CBaseVar* CIntegerVar::CreateVar(const CBaseName* parent) const
{
	CBaseVar* BV = new CIntegerVar(parent);
	Assign(BV);
	return BV;
}


//-----------------------------------------------------------------------------
//преобразование указанной строки в значение переменной
void CIntegerVar::SetConstValue(const char *st)
{
	ConstValue = atoi(st);
}


//-----------------------------------------------------------------------------
//Запись кода CIntegerVar при объявлении переменной
void CIntegerVar::WriteCPP(CPP_files& f)
{
	//запись декларации переменной
	if (external) {
		fprintf(f.fh, "extern %s %s", CIntegerType::GetCPPTypeName(), name);
		fprintf(f.fc, "%s %s::%s%s", CIntegerType::GetCPPTypeName(), parent_element->name, is_var ? "&" : "", name);
	} else
		fprintf(f.fc, "%s %s%s", CIntegerType::GetCPPTypeName(), is_var ? "&" : "", name);

	//запись инициализации константы (если надо)
	if (is_const) fprintf(f.fc, " = %i", ConstValue);
}//WriteCPP


//-----------------------------------------------------------------------------
//запись кода переменной в параметрах процедуры
void CIntegerVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	//проверка наличия именованного типа
	if (type_name)
		WriteCPP_fp_named_type(f, to_h);
	else
		fprintf(to_h ? f.fh : f.fc, "%s %s%s", CIntegerType::GetCPPTypeName(), is_var ? "&" : "", name);
}


//-----------------------------------------------------------------------------
//запись кода при использовании константного значения, подразумевается, что is_const == true
void CIntegerVar::WriteCPP_ConstValue(CPP_files& f)
{
	if (INT_MIN == ConstValue)
		fprintf(f.fc, "(%i - 1)", ConstValue + 1);
	else
		fprintf(f.fc, "%i", ConstValue);
}


//-----------------------------------------------------------------------------
//Запись кода CIntegerVar в файл .dfn
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
//создание константы, подразумевается, что is_const == true
CBaseVar* CLongintVar::CreateConst(const CBaseName *parent) const
{
	CLongintVar* LV = static_cast<CLongintVar*>(CLongintVar::CreateVar(parent));

	//копирование данных
	LV->ConstValue = ConstValue;

	return LV;
}


//-----------------------------------------------------------------------------
//создание копии переменной (без копирования данных в случае констант)
CBaseVar* CLongintVar::CreateVar(const CBaseName* parent) const
{
	CBaseVar* BV = new CLongintVar(parent);
	Assign(BV);
	return BV;
}


//-----------------------------------------------------------------------------
//Запись кода CLongintVar при объявлении переменной
void CLongintVar::WriteCPP(CPP_files& f)
{
	//запись декларации переменной
	if (external) {
		fprintf(f.fh, "extern %s %s", CLongintType::GetCPPTypeName(), name);
		fprintf(f.fc, "%s %s::%s%s", CLongintType::GetCPPTypeName(), parent_element->name, is_var ? "&" : "", name);
	} else
		fprintf(f.fc, "%s %s%s", CLongintType::GetCPPTypeName(), is_var ? "&" : "", name);

	//запись инициализации константы (если надо)
	if (is_const) fprintf(f.fc, " = %i", ConstValue);
}//WriteCPP


//-----------------------------------------------------------------------------
//запись кода переменной в параметрах процедуры
void CLongintVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	//проверка наличия именованного типа
	if (type_name)
		WriteCPP_fp_named_type(f, to_h);
	else
		fprintf(to_h ? f.fh : f.fc, "%s %s%s", CLongintType::GetCPPTypeName(), is_var ? "&" : "", name);
}


//-----------------------------------------------------------------------------
//запись кода при использовании константного значения, подразумевается, что is_const == true
void CLongintVar::WriteCPP_ConstValue(CPP_files& f)
{
	if (LONG_MIN == ConstValue)
		fprintf(f.fc, "(%li - 1)", ConstValue + 1);
	else
		fprintf(f.fc, "%li", ConstValue);
}


//-----------------------------------------------------------------------------
//Запись кода CLongintVar в файл .dfn
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
//создание константы, подразумевается, что is_const == true
CBaseVar* CLongrealVar::CreateConst(const CBaseName *parent) const
{
	CLongrealVar* LV = static_cast<CLongrealVar*>(CLongrealVar::CreateVar(parent));

	//копирование данных
	LV->ConstValue = ConstValue;

	return LV;
}


//-----------------------------------------------------------------------------
//создание копии переменной (без копирования данных в случае констант)
CBaseVar* CLongrealVar::CreateVar(const CBaseName* parent) const
{
	CBaseVar* BV = new CLongrealVar(parent);
	Assign(BV);
	return BV;
}


//-----------------------------------------------------------------------------
//преобразование указанной строки в значение переменной
void CLongrealVar::SetConstValue(const char *st)
{
	is_const = true;
	ConstValue = atof(st);
}//StoreData


//-----------------------------------------------------------------------------
//Запись кода CLongrealVar при объявлении переменной
void CLongrealVar::WriteCPP(CPP_files& f)
{
	//запись декларации переменной
	if (external) {
		fprintf(f.fh, "extern %s %s", CLongrealType::GetCPPTypeName(), name);
		fprintf(f.fc, "%s %s::%s%s", CLongrealType::GetCPPTypeName(), parent_element->name, is_var ? "&" : "", name);
	} else
		fprintf(f.fc, "%s %s%s", CLongrealType::GetCPPTypeName(), is_var ? "&" : "", name);

	//запись инициализации константы (если надо)
	if (is_const) fprintf(f.fc, " = %g", ConstValue);
}//WriteCPP


//-----------------------------------------------------------------------------
//запись кода переменной в параметрах процедуры
void CLongrealVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	//проверка наличия именованного типа
	if (type_name)
		WriteCPP_fp_named_type(f, to_h);
	else
		fprintf(to_h ? f.fh : f.fc, "%s %s%s", CLongrealType::GetCPPTypeName(), is_var ? "&" : "", name);
}


//-----------------------------------------------------------------------------
//запись кода при использовании константного значения, подразумевается, что is_const == true
void CLongrealVar::WriteCPP_ConstValue(CPP_files& f)
{
	fprintf(f.fc, "%g", ConstValue);
}


//-----------------------------------------------------------------------------
//Запись кода CLongrealVar в файл .dfn
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
//проверка завершенности типа данной переменной
int CPointerVar::CheckComplete(CLexBuf *lb)
{
	//поиск типа
	CBaseName* BN = FindType();
	//проверка наличия объявленного типа
	if (!BN) {
		lb->SetCurrPos(TypePos);
		return s_e_UndeclaredIdent;
	}
	//проверка допустимости типа (допускаются только типы запись, массив и обобщение)
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
//создание константы, подразумевается, что is_const == true
CBaseVar* CPointerVar::CreateConst(const CBaseName *parent) const
{
	//константным значением указателя может быть только NIL => не требуется копирование данных
	return CPointerVar::CreateVar(parent);
}


//-----------------------------------------------------------------------------
//создание копии переменной (без копирования данных в случае констант)
CBaseVar* CPointerVar::CreateVar(const CBaseName* parent) const
{
	//создание переменной и копирование основных атрибутов
	CPointerVar* PV = new CPointerVar(parent);
	Assign(PV);
	//копирование типа (в случае неименованного типа)
	if (Type) Type->CreateType(PV->Type);
	//копирование доп. атрибутов
	PV->IsArray = IsArray;
	PV->IsRecord = IsRecord;
	PV->qualident_type = qualident_type;
	PV->TypePos = TypePos;
	return PV;
}


//-----------------------------------------------------------------------------
//поиск имени в таблице имен указываемого типа, имеет смысл только при ук. на запись
CBaseName* CPointerVar::FindName(const char *search_name) const
{
	//проверка наличия типа запись (вообще-то в данном случае функция не должна вызываться)
	if (!IsRecord) return NULL;
	//получение типа
	CBaseType* BT = FindType();
	//поиск имени в списке имен типа
	if (id_CSpecType == BT->name_id)
		return static_cast<CSpecType*>(BT)->FindName(search_name);
	else
		return static_cast<CRecordType*>(FindType())->FindName(search_name);
}


//-----------------------------------------------------------------------------
//поиск типа (не QualidentType, не PointerType), на кот. указывает указатель
CBaseType* CPointerVar::FindType() const
{
	//проверка наличия неименованного типа
	if (Type) return Type;

	//поиск по цепочке именованных типов
	CBaseName* BN = parent_element->GetGlobalName(type_module_name, type_name);
	while (BN && id_CQualidentType == BN->name_id)
		BN = BN->parent_element->GetGlobalName(static_cast<CQualidentType*>(BN)->Qualident->pref_ident, static_cast<CQualidentType*>(BN)->Qualident->ident);

	//при обнаружении типа ук. используется его метод поиска типа
	//(только в случае, если переменная была объявлена через QualidentType)
	if (qualident_type && BN && id_CPointerType == BN->name_id)
		return static_cast<CPointerType*>(BN)->FindType();

	//возврат найденного типа (или NULL)
	return static_cast<CBaseType*>(BN);
}


//-----------------------------------------------------------------------------
//запись кода доп. переменных, содержащих размерности в случае открытого массива
void CPointerVar::WriteCPP_array(CPP_files &f)
{
	//проверка отсутствия массива
	if (!IsArray) return;
	//получение типа, на кот. указывает POINTER
	CArrayType* AT = static_cast<CArrayType*>(FindType());
	//генерация переменной для каждой открытой размерности
	int dimension = 0;	//отсчет размерностей с 0
	while (id_CArrayType == AT->name_id) {
		//при отсутствии открытой размерности прекращаем генерацию
		if (AT->size) break;
		//генерация переменной для хранения текущей размерности
		if (external) {
			fprintf(f.fh, ";\nextern int O2M_ARR_%i_%s", dimension, name);
			fprintf(f.fc, ";\nint %s::O2M_ARR_%i_%s", parent_element->name, dimension, name);
		} else
			fprintf(f.fc, ";\nint O2M_ARR_%i_%s", dimension, name);
		//переход к следующей размерности
		++dimension;
		AT = static_cast<CArrayType*>(AT->Type);
	}//while
}


//-----------------------------------------------------------------------------
//Запись кода CPointerVar при объявлении переменной
void CPointerVar::WriteCPP(CPP_files& f)
{
	//проверка именованного типа (есть имя типа)
	if (type_name) {
		//запись кода объявления переменной
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
		//в случае открытого массива требуется генерация доп. переменных
		WriteCPP_array(f);
		return;
	}

	//неименованный тип (записывается с генерацией искусственного имени вида O2M_UNNM_<имя переменной>)
	if (Type) {
		//генерация кода типа, на кот. указывает указатель
		Type->name = new char[strlen("O2M_UNNM_") + strlen(name) + 1];
		strcpy(Type->name, "O2M_UNNM_");
		strcat(Type->name, name);
		//для типа RECORD генерируем RuntimeId
		if (id_CRecordType == Type->name_id) static_cast<CRecordType*>(Type)->InitRuntimeId();
		//запись кода типа
		Type->WriteCPP(f, external);
		//запись кода объявления переменной
		if (external) {
			fprintf(f.fh, "extern O2M_UNNM_%s* %s", name, name);
			fprintf(f.fc, "O2M_UNNM_%s* %s::%s", name, parent_element->name, name);
		} else {
			//собственно запись кода объявления переменной
			fprintf(f.fc, "O2M_UNNM_%s* %s", name, name);
		}
		//в случае открытого массива требуется генерация доп. переменных
		WriteCPP_array(f);
		return;
	}

	//объявление константы
	if (is_const) {
		fprintf(f.fc, "void %s = 0", name);
		return;
	}

}//WriteCPP


//-----------------------------------------------------------------------------
//запись кода переменной в параметрах процедуры
void CPointerVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	//для упрощения вывода информации
	TFileType* ff = to_h ? f.fh : f.fc;

	//проверка именованного типа (есть имя типа)
	if (type_name) {
		if (type_module_name) fprintf(ff, "%s::", type_module_name);
		fprintf(ff, "%s* %s%s", type_name, is_var ? "&" : "", name);
		return;
	} else {

		//неименованный тип (записывается с генерацией искусственного имени вида O2M_UNNM_<имя переменной>)
		if (Type) {
			//генерация кода типа, на кот. указывает указатель
			Type->name = new char[strlen("O2M_UNNM_") + strlen(name) + 1];
			strcpy(Type->name, "O2M_UNNM_");
			strcat(Type->name, name);
			//для типа RECORD генерируем RuntimeId
			if (id_CRecordType == Type->name_id) static_cast<CRecordType*>(Type)->InitRuntimeId();
			//запись кода типа
			Type->WriteCPP(f, to_h);
			//запись кода объявления переменной
			fprintf(ff, "O2M_UNNM_%s* %s", name, name);
			return;
		}

	}//else

	/**/
	//в дальнейшем, для ук. на неименованный тип, объявленный в параметрах процедуры,
	//объявлять тип с искусственным именем в доп. пространстве имен процедуры
}


//-----------------------------------------------------------------------------
//запись кода при использовании константного значения, подразумевается, что is_const == true
void CPointerVar::WriteCPP_ConstValue(CPP_files& f)
{
	//константным значением указателя может быть только NIL
	fprintf(f.fc, "0");
}


//-----------------------------------------------------------------------------
//Запись кода CPointerVar в файл .dfn
void CPointerVar::WriteDFN(DFN_file& f)
{
	//запись в случае именованного типа
	if (type_name) {
		fprintf(f.f, "%s : ", name);
		//в случае ук. на именованный тип нужно вставлять "POINTER TO"
		if (!qualident_type) fprintf(f.f, "POINTER TO ");
		if (type_module_name) fprintf(f.f, "%s.", type_module_name);
		fprintf(f.f, "%s", type_name);
		return;
	}//if

	//запись начала объявления неименованного типа указатель
	fprintf(f.f, "%s%s : POINTER TO ", name, is_read_only ? "-" : "");

	//получение и запись объявления неименованного типа
	CBaseType* BT = FindType();
	BT->WriteDFN(f);

}//WriteDFN


//-----------------------------------------------------------------------------
//создание копии переменной (без копирования данных в случае констант)
CBaseVar* CProcedureVar::CreateVar(const CBaseName* parent) const
{
	//создание переменной
	CBaseVar* BV = new CProcedureVar(parent);
	Assign(BV);

	//копирование формальных параметров в созданную переменную
	FormalPars.Assign(static_cast<CProcedureVar*>(BV)->FormalPars, parent);

	return BV;
}


//-----------------------------------------------------------------------------
//получение типа результата выражения (для процедуры - функции) или id_CBaseName
EName_id CProcedureVar::GetResultId() const
{
	//проверка переменной-процедуры (не имеет типа результата)
	if (!FormalPars.Qualident) return id_CBaseName;
	//возврат типа результата выражения
	return FormalPars.Qualident->TypeResultId;
}


//-----------------------------------------------------------------------------
//Запись кода CProcedureVar при объявлении переменной
void CProcedureVar::WriteCPP(CPP_files& f)
{
	//проверка наличия имени типа
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

	//для упрощения выбора целевого файла (.h или .cpp)
	TFileType* const ff = external ? f.fh : f.fc;

	//запись типа для функции (если есть) или void
	if (FormalPars.Qualident) FormalPars.Qualident->WriteCPP_type(f, external, parent_element);
	else fprintf(ff, "void");
	fprintf(ff, " (*%s) (", name);
	FormalPars.WriteCPP_pars(f, external);
	fprintf(ff, ")");

}//WriteCPP


//-----------------------------------------------------------------------------
//запись кода переменной в параметрах процедуры
void CProcedureVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	//проверка наличия имени типа
	if (type_name)
		WriteCPP_fp_named_type(f, to_h);
	else {
		//для упрощения выбора целевого файла (.h или .cpp)
		TFileType* const ff = external ? f.fh : f.fc;

		//запись типа для функции (если есть) или void
		if (FormalPars.Qualident) FormalPars.Qualident->WriteCPP_type(f, external, parent_element);
		else fprintf(ff, "void");
		fprintf(ff, " (*%s) (", name);
		FormalPars.WriteCPP_pars(f, external);
		fprintf(ff, ")");
	}//else
}


//-----------------------------------------------------------------------------
//Запись кода CProcedureVar в файл .dfn
void CProcedureVar::WriteDFN(DFN_file& f)
{
	if (is_const) throw error_Internal("CProcedureVar::WriteDFN");

	if (!type_name)
		fprintf(f.f, "%s%s : PROCEDURE", name, is_read_only ? "-" : "");
	else
		WriteDFN_named_type(f);
}//WriteDFN


//-----------------------------------------------------------------------------
//создание копии переменной (без копирования данных в случае констант)
CBaseVar* CPtrVar::CreateVar(const CBaseName* parent) const
{
	CBaseVar* BV = new CPtrVar(parent);
	Assign(BV);
	return BV;
}


//-----------------------------------------------------------------------------
//Запись кода CPtrVar при объявлении переменной
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
//запись кода переменной в параметрах процедуры
void CPtrVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	//проверка наличия имени типа
	if (type_name) {
		WriteCPP_fp_named_type(f, to_h);
		return;
	}
	//запись информации в файл
	fprintf(to_h ? f.fh : f.fc, "bool %s%s", is_var ? "&" : "", name);
}


//-----------------------------------------------------------------------------
//Запись кода CPtrVar в файл .dfn
void CPtrVar::WriteDFN(DFN_file& f)
{
	if (is_const) throw error_Internal("CPtrVar::WriteDFN");

	if (!type_name)
		fprintf(f.f, "%s%s : BOOLEAN", name, is_read_only ? "-" : "");
	else
		WriteDFN_named_type(f);
}//WriteDFN


//-----------------------------------------------------------------------------
//создание константы, подразумевается, что is_const == true
CBaseVar* CRealVar::CreateConst(const CBaseName *parent) const
{
	CRealVar* RV = static_cast<CRealVar*>(CRealVar::CreateVar(parent));

	//копирование данных
	RV->ConstValue = ConstValue;

	return RV;
}


//-----------------------------------------------------------------------------
//создание копии переменной (без копирования данных в случае констант)
CBaseVar* CRealVar::CreateVar(const CBaseName* parent) const
{
	CBaseVar* BV = new CRealVar(parent);
	Assign(BV);
	return BV;
}


//-----------------------------------------------------------------------------
//преобразование указанной строки в значение переменной
void CRealVar::SetConstValue(const char *st)
{
	is_const = true;
	ConstValue = atof(st);
}//StoreData


//-----------------------------------------------------------------------------
//Запись кода CRealVar при объявлении переменной
void CRealVar::WriteCPP(CPP_files& f)
{
	//запись декларации переменной
	if (external) {
		fprintf(f.fh, "extern %s %s", CRealType::GetCPPTypeName(), name);
		fprintf(f.fc, "%s %s::%s%s", CRealType::GetCPPTypeName(), parent_element->name, is_var ? "&" : "", name);
	} else
		fprintf(f.fc, "%s %s%s", CRealType::GetCPPTypeName(), is_var ? "&" : "", name);

	//запись инициализации константы (если надо)
	if (is_const) fprintf(f.fc, " = %g", ConstValue);
}//WriteCPP


//-----------------------------------------------------------------------------
//запись кода переменной в параметрах процедуры
void CRealVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	//проверка наличия именованного типа
	if (type_name)
		WriteCPP_fp_named_type(f, to_h);
	else
		fprintf(to_h ? f.fh : f.fc, "%s %s%s", CRealType::GetCPPTypeName(), is_var ? "&" : "", name);
}


//-----------------------------------------------------------------------------
//запись кода при использовании константного значения, подразумевается, что is_const == true
void CRealVar::WriteCPP_ConstValue(CPP_files& f)
{
	fprintf(f.fc, "%g", ConstValue);
}


//-----------------------------------------------------------------------------
//Запись кода CRealVar в файл .dfn
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
//добавление указанного эл-та в список полей
void CRecordVar::AddName(CBaseName* BN) const
{
	if (!CBaseVar::IsVarId(BN->name_id)) throw error_Internal("CRecordVar::AddName");
	FieldStore.push_back(static_cast<CBaseVar*>(BN));
}


//-----------------------------------------------------------------------------
//проверка завершенности типов у всех полей записи
int CRecordVar::CheckComplete(CLexBuf *lb)
{
	//проверка импортированности типа переменной (тогда он уже проверен при инициализации DFN модуля)
	if (GetTypeModuleAlias()) return 0;
	//проверка завершенности полей записи
	TFieldStore::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci) {
		int err_num = (*ci)->CheckComplete(lb);
		if (err_num) return err_num;
	}
	return 0;
}


//-----------------------------------------------------------------------------
//создание копии переменной (без копирования данных в случае констант)
CBaseVar* CRecordVar::CreateVar(const CBaseName* parent) const
{
	CBaseVar* BV = new CRecordVar(parent);
	Assign(BV);
	//копирование полей записи
	TFieldStore::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci)
		BV->AddName((*ci)->CreateVar(BV));
	return BV;
}


//-----------------------------------------------------------------------------
//деструктор объекта CRecordVar
CRecordVar::~CRecordVar()
{
	//очистка списка полей записи
	TFieldStore::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci)
		delete *ci;
	//очистка имени базового типа
	delete Qualident;
}//~CRecordVar


//-----------------------------------------------------------------------------
//поиск имени в списке полей, NULL - если имя не найдено
CBaseName* CRecordVar::FindName(const char* search_name) const
{
	//проверка наличия именованного типа
	if (type_name) {
		//используется поиск среди полей типа на случай наличия связанной с типом процедуры
		CBaseName* BN = parent_element->GetGlobalName(type_module_name, type_name);
		return static_cast<CRecordType*>(BN)->FindName(search_name);
	}

	//поиск имени в собственном списке полей записи
	//(используется для переменных неименованного типа)
	TFieldStore::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci)
		if (!strcmp( (*ci)->name, search_name ))
			return *ci;

	//проверка наличия базового типа
	if (Qualident) {
		//получение ук. на объект базового типа (должен присутствовать)
		CBaseName* BN = parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident);
		return static_cast<CRecordType*>(BN)->FindName(search_name);
	}

	//имя не найдено
	return NULL;
}//FindName


//-----------------------------------------------------------------------------
//генерация кода при объявлении переменной
void CRecordVar::WriteCPP(CPP_files& f)
{
	//проверка наличия имени типа
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

	//генерация объявления переменной неименованного типа

	//для объявления переменной как extern требуется указать имя типа
	if (external)
		fprintf(f.fh, "struct O2M_UNNM_%s {\n", name);
	else
		fprintf(f.fc, "struct {\n");
	//генерация кода полей записи
	TFieldStore::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci) {
		fprintf(external ? f.fh : f.fc, "\t");
		(*ci)->WriteCPP_fp(f, external);
		fprintf(external ? f.fh : f.fc, ";\n");
	}
	//генерация объявления переменной с учетом экспорта
	if (external) {
		fprintf(f.fh, "};\nextern O2M_UNNM_%s %s", name, name);
		fprintf(f.fc, "O2M_UNNM_%s %s::%s", name, parent_element->name, name);
	} else
		fprintf(f.fc, "} %s", name);
}


//-----------------------------------------------------------------------------
//запись кода переменной в параметрах процедуры
void CRecordVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	//проверка наличия имени типа и отсутствия признака параметра-переменной
	if (type_name && !is_var) {
		WriteCPP_fp_named_type(f, to_h);
		return;
	}

	//проверка признака параметра-переменной
	if (is_var) {
		if (!to_h) {//запись кода в .cpp файл
			if (type_module_name) fprintf(f.fc, "%s::", type_module_name);
			fprintf(f.fc, "%s* %s", type_name, name);
		}
		else {//запись кода в .h файл
			if (type_module_name) {
				//поиск модуля по его псевдониму
				CBaseName* BN = parent_element->GetGlobalName(type_module_name);
				//запись префикса по настоящему имени модуля
				if ( BN && (BN->name_id == id_CImportModule) )
					fprintf(f.fh, "%s::", static_cast<CImportModule*>(BN)->real_name);
			}
			//запись типа и названия переменной
			fprintf(f.fh, "%s* %s", type_name, name);
		}//else

		return;
	}//if


	//неименованный тип запись (например, внутри другой записи)
	//генерация описания типа (переменная неименованного типа)
	fprintf(to_h ? f.fh : f.fc, "struct {\n\t");
	//запись кода полей записи
	TFieldStore::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci) {
		(*ci)->WriteCPP_fp(f, to_h);
		fprintf(to_h ? f.fh : f.fc, ";\n\t");
	}
	fprintf(to_h ? f.fh : f.fc, "} %s", name);

}


//-----------------------------------------------------------------------------
//запись кода при использования CRecordVar для описания типа указателя
void CRecordVar::WriteCPP_pointer(CPP_files& f)
{
	//запись в .h файл
	if (external) {
		fprintf(f.fh, "struct %s", name);
		
		//запись базового типа (если есть)
		if (Qualident) {
			fprintf(f.fh, " : ");
			CBaseName* BN = parent_element->GetGlobalName(Qualident->pref_ident);
			if (BN) fprintf(f.fh, "%s::", static_cast<CImportModule*>(BN)->real_name);
			fprintf(f.fh, "%s", Qualident->ident);
		}
		fprintf(f.fh, " {\n");

		//запись кода метода для получения ид. типа времени исполнения во время исполнения
		fprintf(f.fh, "\t%sconst char* O2M_SYS_ID() {return \"%s\";};\n", Qualident ? "" : "virtual ", name);

		//запись кода полей записи
		TFieldStore::const_iterator ci;
		for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci) {
			(*ci)->WriteCPP_fp(f, true);
			fprintf(f.fh, ";\n");
		}
		//завершение записи кода записи
		fprintf(f.fh, "};\n");

		return;
	}


	//запись в .cpp файл
	fprintf(f.fc, "struct %s", name);

	//запись базового типа (если есть)
	if (Qualident) {
		fprintf(f.fc, " : ");
		Qualident->WriteCPP_type(f, false, parent_element);
	}
	fprintf(f.fc, " {\n");

	//запись кода метода для получения ид. типа времени исполнения во время исполнения
	fprintf(f.fc, "\t%sconst char* O2M_SYS_ID() {return \"%s\";};\n", Qualident ? "" : "virtual ", name);

	//запись кода полей записи
	TFieldStore::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci) {
		(*ci)->WriteCPP_fp(f, false);
		fprintf(f.fc, ";\n");
	}
	fprintf(f.fc, "};\n");

}//WriteCPP_pointer


//-----------------------------------------------------------------------------
//Запись кода CRecordVar в файл .dfn
void CRecordVar::WriteDFN(DFN_file& f)
{
	if (is_const) throw error_Internal("CRecordVar::WriteDFN");

	if (!type_name) {

		fprintf(f.f, "%s%s : RECORD", name, is_read_only ? "-" : "");
		//генерация кода базового типа
		if (Qualident) {
			fprintf(f.f, " (");
			Qualident->WriteDFN(f);
			fprintf(f.f, ")");
		}

		//увеличение отступа в DFN файле
		f.tab_level++;

		fprintf(f.f, "\n");
		f.tab();

		//генерация кода полей записи
		TFieldStore::const_iterator ci;
		for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci)
			if ((*ci)->external)
			{
				(*ci)->WriteDFN(f);
				fprintf(f.f, ";\n");
				f.tab();
			}
		//конец генерации кода записи
		fprintf(f.f, "END");

		//уменьшение отступа в DFN файле
		f.tab_level--;

	} else
		WriteDFN_named_type(f);
}//WriteDFN


//-----------------------------------------------------------------------------
//создание константы, подразумевается, что is_const == true
CBaseVar* CSetVar::CreateConst(const CBaseName *parent) const
{
	//создание переменной
	CSetVar* SV = static_cast<CSetVar*>(CSetVar::CreateVar(parent));
	//копирование константного значения
	SV->ConstValue = ConstValue;
	//константа создана
	return SV;
}


//-----------------------------------------------------------------------------
//создание копии переменной (без копирования данных в случае констант)
CBaseVar* CSetVar::CreateVar(const CBaseName* parent) const
{
	//создание копии переменной с атрибутами
	CSetVar* SV = new CSetVar(parent);
	Assign(SV);
	return SV;
}


//-----------------------------------------------------------------------------
//деструктор объекта CSetVar
CSetVar::~CSetVar()
{
	CBaseVector::iterator vi;
	for (vi = SetElems.begin(); vi != SetElems.end(); vi++)
		delete *vi;
}//~CSetVar


//-----------------------------------------------------------------------------
//обработка конструктора множества, если есть (может не быть), при возникновении ошибок
//должно обеспечиваться уничтожение всех временных объектов
int CSetVar::SetInit(CLexBuf *lb)
{
	DECL_SAVE_POS
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//для проверки константности конструктора множества
	is_const = true;
	
	//проверка наличия "}" (отсутствие элементов - константный конструктор пустого множества)
	if (lb->ReadLex(li) && lex_k_cl_brace == li.lex) return 0;

	RESTORE_POS

	while (true) {
		//создание эл-та множества
		CSetVarElem *SE = new CSetVarElem(parent_element);
		int err_num = SE->Init(lb);
		if (err_num) {
			delete SE;
			return err_num;
		}
		//занесение эл-та множества в список
		SetElems.push_back(SE);

		//проверка, является ли данный конструктор множества константным (все эл-ты должны быть константами)
		is_const = is_const && SE->IsConst();

		//проверка наличия ","
		if (!lb->ReadLex(li) || lex_k_comma != li.lex) break;
	}

	//проверка наличия "}" (конец конструктора множества)
	if (lex_k_cl_brace != li.lex) return s_e_ClBraceMissing;

	//в случае, если все эл-ты множества константны, вычисляем ConstValue
	if (is_const) {
		CBaseVector::iterator vi;
		for (vi = SetElems.begin(); vi != SetElems.end(); vi++) {
			//собственно вычисление ConstValue
			ConstValue |= static_cast<CSetVarElem*>(*vi)->GetConstValue();
			//эл-т SetVarElem уже не нужен
			delete *vi;
		}
		//очищаем список (он уже не нужен)
		SetElems.clear();
	}

	return 0;
}//Init CSetVar


//-----------------------------------------------------------------------------
//Запись кода CSetVar при объявлении переменной
void CSetVar::WriteCPP(CPP_files& f)
{
	//запись декларации переменной
	if (external) {
		fprintf(f.fh, "extern %s %s", CSetType::GetCPPTypeName(), name);
		fprintf(f.fc, "%s %s::%s%s", CSetType::GetCPPTypeName(), parent_element->name, is_var ? "&" : "", name);
	} else
		fprintf(f.fc, "%s %s%s", CSetType::GetCPPTypeName(), is_var ? "&" : "", name);

	//запись инициализации константы (если надо)
	if (is_const) {
		fprintf(f.fc, " = ");
		CSetVar::WriteCPP_ConstValue(f);
	}
}//WriteCPP


//-----------------------------------------------------------------------------
//запись кода переменной в параметрах процедуры
void CSetVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	//проверка наличия именованного типа
	if (type_name)
		WriteCPP_fp_named_type(f, to_h);
	else
		fprintf(to_h ? f.fh : f.fc, "%s %s%s", CSetType::GetCPPTypeName(), is_var ? "&" : "", name);
}


//-----------------------------------------------------------------------------
//запись кода при использовании константного значения, подразумевается, что is_const == true
void CSetVar::WriteCPP_ConstValue(CPP_files& f)
{
	//проверка наличия рассчитанного константного значения
	if (is_const)
		fprintf(f.fc, "%i", ConstValue);
	else {
		//проверка наличия эл-тов в множестве
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
//Запись кода CSetVar в файл .dfn
void CSetVar::WriteDFN(DFN_file& f)
{
	if (is_const) throw error_Internal("CSetVar::WriteDFN");

	if (!type_name)
		fprintf(f.f, "%s%s : SET", name, is_read_only ? "-" : "");
	else
		WriteDFN_named_type(f);
}//WriteDFN


//-----------------------------------------------------------------------------
//деструктор объекта CSetVarElem
CSetVarElem::~CSetVarElem()
{
	delete LowExpr;
	delete HighExpr;
}//~CSetVarElem


//-----------------------------------------------------------------------------
//инициализация объекта CSetVarElem
int CSetVarElem::Init(CLexBuf *lb)
{
	//инициализация первого выражения
	LowExpr = new CExpr(parent_element);
	int err_num = LowExpr->Init(lb);
	if (err_num) return err_num;

	//проверка типа выражения
	if (!CBaseVar::IsIntId(LowExpr->GetResultId())) return s_e_SetElemType;

	//попытка вычисления константы
	CBaseVar* BV;
	err_num = LowExpr->CreateConst(BV);
	if (!err_num) {
		LowBoundValue = BV->GetIntValue();
		delete BV;
		delete LowExpr;
		LowExpr = NULL;
		//проверка допустимого значения константы
		if (SET_MAX < LowBoundValue || 0 > LowBoundValue) return s_e_SetElemRange;
		//преобразование числового значения в множество
		SetValue = 1 << LowBoundValue;
	}

	DECL_SAVE_POS
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия ".." (второго выражения)
	if (!lb->ReadLex(li) || lex_k_dots != li.lex) {
		RESTORE_POS
		return 0;
	}

	IsRange = true;

	//инициализация второго выражения
	HighExpr = new CExpr(parent_element);
	err_num = HighExpr->Init(lb);
	if (err_num) return err_num;

	//проверка типа второго выражения
	if (!CBaseVar::IsIntId(HighExpr->GetResultId())) return s_e_SetElemType;

	//попытка вычисления константы для второго выражения
	err_num = HighExpr->CreateConst(BV);
	if (!err_num) {
		HighBoundValue = BV->GetIntValue();
		delete BV;
		delete HighExpr;
		HighExpr = NULL;
		//проверка допустимого значения константы
		if (SET_MAX < HighBoundValue || 0 > HighBoundValue) return s_e_SetElemRange;
		//проверка возможности получения константного множества
		if (!LowExpr) {
			//вычисление множества - диапазона с учетом предыдущего значения диапазона
			if (LowBoundValue > HighBoundValue) return s_e_SetRange;
			for (int i = LowBoundValue + 1; i <= HighBoundValue; i++) SetValue |= 1 << i;
		}
	}//if

	return 0;
}//Init CSetVarElem


//-----------------------------------------------------------------------------
//Запись кода CSetVarElem
void CSetVarElem::WriteCPP(CPP_files &f)
{
	//проверка возможности использования рассчитанных констант
	if (IsRange && (LowExpr || HighExpr)) {
		fprintf(f.fc, "O2M_SET_RANGE(");	//аргументы данной функции - целочисленные границы диапазона (не множества)
		if (LowExpr) LowExpr->WriteCPP(f); else fprintf(f.fc, "%i", LowBoundValue);	//можно исп. рассчитанную нижнюю границу диапазона
		fprintf(f.fc, ",");
		if (HighExpr) HighExpr->WriteCPP(f); else fprintf(f.fc, "%i", HighBoundValue);	//можно исп. рассчитанную верхнюю границу диапазона
		fprintf(f.fc, ")");
	} else
		if (LowExpr) {
			//нет диапазона или константного множества, требуется преобразование целочисленного выражения в множество
			fprintf(f.fc, "1 << (");
			LowExpr->WriteCPP(f);
			fprintf(f.fc, ")");
		} else
			fprintf(f.fc, "%i", SetValue);	//можно использовать полностью рассчитанное множество
}


//-----------------------------------------------------------------------------
//создание константы, подразумевается, что is_const == true
CBaseVar* CShortintVar::CreateConst(const CBaseName *parent) const
{
	CShortintVar* SV = static_cast<CShortintVar*>(CShortintVar::CreateVar(parent));

	//копирование данных в созданную переменную
	SV->ConstValue = ConstValue;

	return SV;
}


//-----------------------------------------------------------------------------
//создание копии переменной (без копирования данных в случае констант)
CBaseVar* CShortintVar::CreateVar(const CBaseName* parent) const
{
	CBaseVar* BV = new CShortintVar(parent);
	Assign(BV);
	return BV;
}


//-----------------------------------------------------------------------------
//Запись кода CShortintVar при объявлении переменной
void CShortintVar::WriteCPP(CPP_files& f)
{
	//запись декларации переменной
	if (external) {
		fprintf(f.fh, "extern %s %s", CShortintType::GetCPPTypeName(), name);
		fprintf(f.fc, "%s %s::%s%s", CShortintType::GetCPPTypeName(), parent_element->name, is_var ? "&" : "", name);
	} else
		fprintf(f.fc, "%s %s%s", CShortintType::GetCPPTypeName(), is_var ? "&" : "", name);

	//запись инициализации константы (если надо)
	if (is_const) fprintf(f.fc, " = %i", ConstValue);
}//WriteCPP


//-----------------------------------------------------------------------------
//запись кода переменной в параметрах процедуры
void CShortintVar::WriteCPP_fp(CPP_files& f, const bool to_h)
{
	//проверка наличия именованного типа
	if (type_name)
		WriteCPP_fp_named_type(f, to_h);
	else
		fprintf(to_h ? f.fh : f.fc, "%s %s%s", CShortintType::GetCPPTypeName(), is_var ? "&" : "", name);
}


//-----------------------------------------------------------------------------
//запись кода при использовании константного значения, подразумевается, что is_const == true
void CShortintVar::WriteCPP_ConstValue(CPP_files& f)
{
	fprintf(f.fc, "%hi", ConstValue);
}


//-----------------------------------------------------------------------------
//Запись кода CShortintVar в файл .dfn
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

