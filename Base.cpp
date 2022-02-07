//=============================================================================
// Описание базовых и вспомогательных классов
//=============================================================================

#include "Base.h"
#include "Var.h"
#include "Type.h"


//-----------------------------------------------------------------------------
//деструктор, закрываюший открытые файлы
CPP_files::~CPP_files()
{
	//закрытие открытых файлов
	if (f2ml) {
		fprintf(f2ml, "</Module>\n");
		fclose(f2ml);
	}
	if (fh) {
		fprintf(fh, "}\n#endif\n");
		fclose(fh);
	}
	if (fc) fclose(fc);
	//очистка полного имени файла
	delete[] f_name;
}


//-----------------------------------------------------------------------------
//открытие файлов для записи с выдачей сообщения при возникновении ошибки
int CPP_files::Init(const char *name)
{
	//подготовка строки для формирования полного пути к файлу
    ext_offs = /*strlen("CPP/.") +*/ strlen(name);
	f_name = new char[ext_offs + strlen("cpp_")];
	//формирование полного пути к файлу без расширения
    //strcpy(f_name, "CPP/");
    strcpy(f_name, name);

	//добавление расширения ".2ml"
	strcat(f_name, ".2ml");
	//открытие 2ml файла для записи
	f2ml = fopen(f_name, "w");
	if (!f2ml) {
		fprintf(output, textCheckFolder, "CPP");
		goto fault_exit;
	}

	//смена расширения на "cpp"
	f_name[ext_offs] = 0;
	strcat(f_name, "cpp");
	//открытие cpp файла для записи
	fc = fopen(f_name, "w");
	if (!fc) goto fault_exit;

	//смена расширения на "h"
	f_name[ext_offs] = 'h';
	f_name[ext_offs + 1] = 0;
	//открытие h файла для записи
	fh = fopen(f_name, "w");
	if (!fh) goto fault_exit;

	//запись комментария в начало файла
	fprintf(fc, comment_format, comment_line_cpp, comment_title, comment_line_cpp);
	fprintf(fh, comment_format, comment_line_cpp, comment_title, comment_line_cpp);

	//запись заголовка в 2ml файл
	fprintf(f2ml, "<?xml version=\"1.0\" ?>\n<!-- %s -->\n<Module Name=\"%s\">\n", comment_title, name);

	//предотвращение повторного объявления
	fprintf(fh, "#ifndef O2M_H_FILE_%s\n", name);
	fprintf(fh, "#define O2M_H_FILE_%s\n", name);
	//вставка файла _O2M_sys.h
	fprintf(fh, "#include \"_O2M_sys.h\"\n");

	//открытие файлов завершено удачно
	return 0;

fault_exit:
	//вывод сообщения об ошибке открытия файла для записи
	fprintf(output, textCannotOpenW, f_name);
	return s_m_Error;
}


//-----------------------------------------------------------------------------
//вставка табуляций в файл описаний
void CPP_files::tab_fc()
{
	for(int i = tab_level_c; i > 0; i--) fprintf(fc, "\t");
}


//-----------------------------------------------------------------------------
//деструктор, закрываюший открытый файл
DFN_file::~DFN_file()
{
	//закрытие открытого файла
	if (f) fclose(f);
	//очистка полного имени файла
	delete[] f_name;
}


//-----------------------------------------------------------------------------
//открытие файла для записи с выдачей сообщения при возникновении ошибки
int DFN_file::Init(const char *name)
{
	//подготовка строки для формирования полного пути к файлу
    f_name = new char[strlen("DFN") +strlen(name) + strlen(".dfn_")];
	//формирование полного пути к файлу
    strcpy(f_name, "DFN/");
    strcat(f_name, name);
	strcat(f_name, ".dfn");

	//открытие файла
	f = fopen(f_name, "w");
	if (!f) {
        fprintf(output, textCheckFolder, "DFN");
		fprintf(output, textCannotOpenW, f_name);
		return s_m_Error;
	}

	//запись комментария в начало файла
	fprintf(f, "%s(* %s *)\n%s\n", comment_line_dfn, comment_title, comment_line_dfn);

	return 0;
}


//-----------------------------------------------------------------------------
//вставка табуляций в DFN файл
void DFN_file::tab()
{
	for(int i = tab_level; i > 0; i--) fprintf(f, "\t");
}


//-----------------------------------------------------------------------------
//копирование имени и атрибутов в указанный объект (BaseName)
void CBaseName::Assign(CBaseName* BaseName) const
{
	//копирование имени (если есть)
	if (name) BaseName->SetName(name);
	//копирование атрибутов
	BaseName->external = external;
}//Assign


//-----------------------------------------------------------------------------
//копирование в объект указанного имени
void CBaseName::SetName(const char *NewName)
{
	name = str_new_copy(NewName);
}


//-----------------------------------------------------------------------------
//получение по названию объекта из глобальной области видимости
CBaseName* CBaseName::GetGlobalName(const char* global_name) const
{
	//проверка наличия названия объекта
	if (!global_name) return NULL;
	//поиск указанного имени, начиная с текущей таблицы имен
	const CBaseName* Scope = this;
	while (true) {
		//проверка наличия имени в текущей таблице имен
		if (CBaseName* GlobalName = Scope->FindName(global_name)) return GlobalName;
		//переход к более глобальной области видимости
		Scope = Scope->parent_element;
		//достигли самой глобальной области видимости
		if (!Scope) return NULL;
	}//while
}//GetGlobalName


//-----------------------------------------------------------------------------
//получение по уточненному названию объекта из глобальной области видимости
CBaseName* CBaseName::GetGlobalName(const char* module_name, const char* global_name) const
{
	//проверка наличия названия модуля, если его нет - ищем локальное имя
	if (!module_name) return GetGlobalName(global_name);

	//поиск объекта, способного хранить объекты с составным именем (импортированные)
	const CBaseName* BaseName = this;
	while (BaseName)
		switch (BaseName->name_id) {
		//вызов процедуры дальнейшего поиска имени в зависимости от типа BaseName,
		//или переход к более высокому объекту в списке родительских объектов
		case id_CModule:
			return static_cast<const CModule*>(BaseName)->FindImportedName(module_name, global_name);
		case id_CDfnModule:
			return static_cast<const CDfnModule*>(BaseName)->FindImportedName(module_name, global_name);
		case id_CWithLoopLink:
			return static_cast<const CWithLoopLink*>(BaseName)->FindImportedName(module_name, global_name);
		default:
			//проверка наличия процедуры
			if (CProcedure::IsProcId(BaseName->name_id))
				return static_cast<const CProcedure*>(BaseName)->FindImportedName(module_name, global_name);
			//переход к более высокому объекту в списке родительских объектов
			BaseName = BaseName->parent_element;
		}//switch

	//среди цепочки parent_element CModule или CDfnModule должны быть всегда
	throw error_Internal("::GetGlobalName");
}//GetGlobalName


//-----------------------------------------------------------------------------
//получение модуля (обычного или DFN) из эл-тов parent_element
const CBaseName* CBaseName::GetParentModule() const
{
	//перебор parent_element пока не будет найден модуль какого-либо типа
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
	//при вызове данного метода у модуля parent_element отсутствует
	return this;
}


//-----------------------------------------------------------------------------
//копирование имени и атрибутов в указанный объект (BaseVar)
int CIdentDef::AssignVar(CBaseVar *BaseVar) const
{
	//проверка наличия обобщенной переменной (если есть <>)
	if (is_common && id_CCommonVar != BaseVar->name_id) return s_e_CommonTypeExpected;
	//копирование атрибутов, унаследованных от CBaseName
	CBaseName::Assign(BaseVar);
	//копирование дополнительных атрибутов
	BaseVar->is_read_only = is_read_only;
	//установка признака в случае специализированной обобщенной переменной
	if (is_common && TagName)
		return static_cast<CCommonVar*>(BaseVar)->SetTagName(QualTagName, TagName);
	else
		return 0;
}


//-----------------------------------------------------------------------------
//инициализация объекта CIdentDef
int CIdentDef::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия ид.
	if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;

	//запоминаем полученное имя
	SetName(li.st);

	DECL_SAVE_POS

	//получение след. лексемы ("*", "-", "<")
	if (!lb->ReadLex(li)) {
		RESTORE_POS
		return 0;
	}

	//проверка полученной лексемы
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

	//проверка наличия "<"
	if (!lb->ReadLex(li) || lex_k_lt != li.lex) {
		RESTORE_POS
		return 0;
	}

lt_received:

	//установка признака наличия обобщения
	is_common = true;

	//проверка наличия ">" (используется специализация по умолчанию)
	SAVE_POS
	if (!lb->ReadLex(li) || lex_k_gt != li.lex) {
		RESTORE_POS
	} else
		return 0;

	//получение признака
	CQualident qual;
	int err_num = qual.Init(lb, parent_element);
	//проверка отсутствия идентификатора (это не ошибка)
	if (err_num && s_e_IdentExpected != err_num) return err_num;

	//запоминаем название признака (если есть)
	if (qual.pref_ident) QualTagName = str_new_copy(qual.pref_ident);
	if (qual.ident) TagName = str_new_copy(qual.ident);

	//проверка наличия ">"
	if (!lb->ReadLex(li) || lex_k_gt != li.lex) return s_e_ClAngleMissing;

	return 0;
}//Init CIdentDef


//-----------------------------------------------------------------------------
//деструктор объекта CIdentList
CIdentList::~CIdentList()
{
	delete BaseType;
	CBaseNameVector::iterator i;
	for(i = BNVector.begin(); i != BNVector.end(); ++i)
		delete *i;
}//~CIdentList


//-----------------------------------------------------------------------------
//загрузка списка идентификаторов без создания новых объектов
int CIdentList::LoadIdentList(CLexBuf *lb)
{
	//создание имени IdentDef
	CIdentDef* IdentDef = new CIdentDef(parent_element, true);
	//чтение первого имени (может отсутствовать)
	int err_num = IdentDef->Init(lb);

	//проверка наличия ошибок или признака отсутствия идентификатора
	if (err_num)
		if (s_e_IdentExpected == err_num) {
			delete IdentDef;			//уничтожаем временный эл-т
			return s_m_IdentDefAbsent;	//нет ид-а (это НЕ ошибка)
		} else return err_num;

	//проверка отсутствия полученного названия в таблице имен
	if (parent_element->FindName(IdentDef->name)) {
		delete IdentDef;
		return s_e_Redefinition;
	}

	//проверяем допустимость экспорта имени (если экспортируется)
	if (is_local && IdentDef->external) {
		delete IdentDef;
		return s_e_IdentWrongMarked;
	}

	//запоминаем считанное название в списке названий
	BNVector.push_back(IdentDef);

	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//цикл чтения оставшихся элементов списка
	while(true) {
		DECL_SAVE_POS

		//проверка наличия ","
		if (!lb->ReadLex(li) || lex_k_comma != li.lex) {
			RESTORE_POS
			break;
		}

		//чтение следующего имени (должно присутствовать)
		IdentDef = new CIdentDef(parent_element, true);
		if (err_num = IdentDef->Init(lb)) {
			delete IdentDef;
			return err_num;
		}

		//проверка отсутствия полученного названия в таблице имен
		if (parent_element->FindName(IdentDef->name)) {
			delete IdentDef;
			return s_e_Redefinition;
		}

		//проверка отсутствия полученного названия в списке названий
		CBaseNameVector::const_iterator ci;
		for (ci = BNVector.begin(); ci != BNVector.end(); ci++)
			if (!strcmp( (*ci)->name, IdentDef->name)) {
				delete IdentDef;
				return s_e_Redefinition;
			}

		//проверяем допустимость экспорта имени (если экспортируется)
		if (is_local && IdentDef->external) {
			delete IdentDef;
			return s_e_IdentWrongMarked;
		}

		//запоминаем считанное название в списке названий
		BNVector.push_back(IdentDef);

	}//while

	//проверка наличия кл. слова ":"
	if (!lb->ReadLex(li) || lex_k_colon != li.lex) return s_e_ColonMissing;

	//создание объекта типа из потока лексем
	err_num = TypeSelector(lb, BaseType, parent_element);
	if (err_num) return err_num;

	//получение типа (для проверки открытости типа массив)
	CBaseType* BT;
	if (id_CQualidentType == BaseType->name_id)
		static_cast<CQualidentType*>(BaseType)->GetNamedType(BT, false);
	else
		BT = BaseType;

	//в случае массива проверка, является ли массив открытым (это недопустимо)
	if (BT && id_CArrayType == BT->name_id && 0 == static_cast<CArrayType*>(BT)->size)
		return s_e_OpenArrayNotAllowed;

	return 0;
}//CIdentList::LoadIdentList


//-----------------------------------------------------------------------------
//деструктор объекта Qualident
CQualident::~CQualident()
{
	delete[] pref_ident;
	delete[] ident;
}//~CQualident


//-----------------------------------------------------------------------------
//инициализация объекта Qualident (не имени типа) из потока лексем с проверкой
//наличия объявленного первого идентификатора (только в случае наличия уточнения)
int CQualident::Init(CLexBuf *lb, const CBaseName *parent_element)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//получение и проверка наличия известного ид-а
	if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	ident = str_new_copy(li.st);

	DECL_SAVE_POS

	//проверка наличия кл. слова "."
	if (!lb->ReadLex(li) || lex_k_dot != li.lex) {
		RESTORE_POS
	} else {
		//проверка наличия первого идентификатора
		if (!parent_element->GetGlobalName(ident)) {
			RESTORE_POS
			return s_e_UndeclaredIdent;
		}
		//получение второго ид-а (ключевого слова)
		if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
		//ident на самом деле был pref_ident-ом, заносим его туда 
		pref_ident = ident;
		//запоминаем новый ид-р
		ident = str_new_copy(li.st);
	}

	return 0;
}//Init


//-----------------------------------------------------------------------------
//инициализация объекта Qualident (в случае имени типа) из потока лексем
//с определением типа результата выражения
int CQualident::InitTypeName(CLexBuf *lb, const CBaseName *parent_element)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//получение ид-а или ключевого слова
	if (!lb->ReadLex(li) || (lex_i != li.lex && lex_k_dot > li.lex))
		return s_e_IdentExpected;

	//проверка использования простого типа
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
		//простой тип распознан, запоминать ident не требуется
		return 0;
	}

	ident = str_new_copy(li.st);

	DECL_SAVE_POS

	//проверка наличия кл. слова "." (тогда считанный идентификатор - имя модуля)
	if (!lb->ReadLex(li) || lex_k_dot != li.lex) {
		RESTORE_POS
	} else {
		//проверка наличия первого идентификатора - имени модуля
		CBaseName* BN = parent_element->GetGlobalName(ident);
		if (!BN) {
			RESTORE_POS
			return s_e_UndeclaredIdent;
		}
		if (id_CImportModule != BN->name_id) {
			RESTORE_POS
			return s_e_TypeDefinition;
		}
		//получение второго ид-а (ключевого слова)
		if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
		//ident на самом деле был pref_ident-ом, заносим его туда 
		pref_ident = ident;
		//запоминаем новый ид-р
		ident = str_new_copy(li.st);
	}

	//поиск типа в таблице имен
	CBaseName* BN = parent_element->GetGlobalName(pref_ident, ident);
	if (!BN) return s_e_UndeclaredIdent;
	if (!CBaseType::IsTypeId(BN->name_id)) return s_e_IdentNotType;

	//получение типа результата по типу
	TypeResultId = BN->GetResultId();

	return 0;
}


//-----------------------------------------------------------------------------
//копирование атрибутов текущего Qualident'а в указанный объект
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
//очистка содержимого CQualident для повторного вызова Init
void CQualident::Clear()
{
	delete[] pref_ident;
	pref_ident = NULL;
	delete[] ident;
	ident = NULL;
	TypeResultId = id_CBaseName;
}


//-----------------------------------------------------------------------------
//запись кода для CQualident, содержащего имя типа
void CQualident::WriteCPP_type(CPP_files& f, const bool to_h, const CBaseName* parent_element)
{
	//выбор файла для записи информации
	TFileType* ff = to_h ? f.fh : f.fc;
	//получение названия типа (если есть)
	const char* TypeName = CBaseType::GetCPPTypeName(TypeResultId);
	if (TypeName) fprintf(ff, TypeName);
	else {
		//получение типа по его имени (в случае наличия псевдонима он может потребоваться)
		CBaseType* BT = static_cast<CBaseType*>(parent_element->GetGlobalName(pref_ident, ident));
		//запись уточнения имени (пространства имен)
		if (pref_ident) fprintf(ff, "%s::", to_h ? BT->GetModuleName() : BT->GetModuleAlias());
		//проверка, содержит ли Qualident имя типа или является описанием типа
		switch (TypeResultId) {
		case id_CPointerVar:
			fprintf(ff, "%s*", ident);
			break;
		default:	//id_CQualidentType, id_CRecordType, и т.д.
			fprintf(ff, "%s", ident);
		}//switch
	}//else
}


//-----------------------------------------------------------------------------
//Запись кода CQualident в .dfn файл
void CQualident::WriteDFN(DFN_file& f)
{
	//проверка, содержит ли Qualident имя типа или является описанием типа
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
	default:	//id_CQualidentType, id_CRecordType, и т.д.
		if (pref_ident)	fprintf(f.f, "%s.", pref_ident);
		fprintf(f.f, "%s", ident);
	}//switch
}//WriteDFN


//-----------------------------------------------------------------------------
//получение кода названия данного типа в C++
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
//получение псевдонима DFN модуля, экспортирующего данный тип (если есть)
const char* CBaseType::GetModuleAlias() const
{
	//получение модуля
	const CBaseName* BN = GetParentModule();
	//проверка, является ли модуль DFN модулем
	if (id_CDfnModule == BN->name_id)
		return static_cast<const CDfnModule*>(BN)->alias_name;
	else
		return NULL;
}


//-----------------------------------------------------------------------------
//получение имени DFN модуля, экспортирующего данный тип (если есть)
const char* CBaseType::GetModuleName() const
{
	//получение модуля
	const CBaseName* BN = GetParentModule();
	//проверка, является ли модуль DFN модулем
	if (id_CDfnModule == BN->name_id)
		return static_cast<const CDfnModule*>(BN)->name;
	else
		return NULL;
}


//-----------------------------------------------------------------------------
//получение id результата (преобразование id типа в id соответствующей переменной)
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
//проверка, являются ли данный тип и тип имя которого указано. одинаковыми
bool CBaseType::IsSame(const char *module_name, const char *type_name) const
{
	//получение названия модуля, экспортирующего данный тип (если есть)
	const char* m_n = GetModuleAlias();
	//сравнение уточненного имени типа с указанным именем
	if (m_n && module_name) {
		if (!strcmp(m_n, module_name) && !strcmp(name, type_name)) return true;
	} else
		if (!m_n && !module_name && !strcmp(name, type_name)) return true;
	//типы не равны
	return false;
}


//-----------------------------------------------------------------------------
//проверка принадлежности указанного ид. к ид-у типа
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
//копирование имени типа в переменную
void CBaseVar::SetTypeName(const char *TypeModuleName, const char *TypeName)
{
	//т.к. в некоторых случаях имя типа может быть уже установлено, его надо убрать
	if (type_module_name) {
		delete[] type_module_name;
		type_module_name = NULL;
	}
	if (type_name) {
		delete[] type_name;
		type_name = NULL;
	}

	//собственно установка имени типа
	if (TypeModuleName) type_module_name = str_new_copy(TypeModuleName);
	if (TypeName) type_name = str_new_copy(TypeName);
}//SetTypeName


//-----------------------------------------------------------------------------
//копирование атрибутов текущего объекта в указанную переменную
void CBaseVar::Assign(CBaseVar *BaseVar) const
{
	//копирование атрибутов, унаследованных от CBaseName
	CBaseName::Assign(BaseVar);
	//копирование собственных атрибутов
	BaseVar->is_const = is_const;
	BaseVar->is_guarded = is_guarded;
	BaseVar->is_read_only = is_read_only;
	BaseVar->is_var = is_var;
	//копирование названия типа
	BaseVar->SetTypeName(type_module_name, type_name);
}


//-----------------------------------------------------------------------------
//выдача псевдонима модуля, экспортирующего тип переменной (работает и для неименованных типов)
const char* CBaseVar::GetTypeModuleAlias()
{
	//у импортированной переменной отсутствует имя модуля с типом, если тип взят из того же модуля, что и переменная
	if (!type_module_name) {
		//в случае поля записи необходимо проверить parent_element у записи
		const CBaseName* BN = GetParentModule();
		//проверка, была ли импортирована переменная
		if (id_CDfnModule == BN->name_id) return static_cast<const CDfnModule*>(BN)->alias_name;
	}
	//выдача названия модуля
	return type_module_name;
}


//-----------------------------------------------------------------------------
//выдача имени модуля, экспортирующего тип переменной (работает и для неименованных типов)
const char* CBaseVar::GetTypeModuleName()
{
	//у импортированной переменной отсутствует имя модуля с типом, если тип взят из того же модуля, что и переменная
	if (!type_module_name) {
		//в случае поля записи необходимо проверить parent_element у записи
		const CBaseName* BN = GetParentModule();
		//проверка, была ли импортирована переменная
		if (id_CDfnModule == BN->name_id) return static_cast<const CDfnModule*>(BN)->name;
	}
	//выдача названия модуля
	return type_module_name;
}


//-----------------------------------------------------------------------------
//выдача имени типа переменной (именованный тип или тип CPP)
const char* CBaseVar::GetTypeName()
{
	//проверка именованного типа
	if (type_name) return type_name;
	//для базовых типов
	return CBaseType::GetCPPTypeName(name_id);
}


//-----------------------------------------------------------------------------
//проверка принадлежности указанного ид. к ид-у переменной
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
//проверка принадлежности указанного ид. к ид-у переменной числового типа
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
//проверка принадлежности указанного ид. к ид-у переменной целого типа
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
//проверка принадлежности указанного ид. к ид-у переменной действительного (вещественного) типа
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
//деструктор объекта CBaseVar
CBaseVar::~CBaseVar()
{
	delete[] type_module_name;
	delete[] type_name;
}


//-----------------------------------------------------------------------------
//создание копии CQualident
void CQualident::CreateCopy(CQualident *&qual) const
{
	qual = new CQualident;
	//копирование типа результата
	qual->TypeResultId = TypeResultId;
	//копирование идентификатора (если есть)
	if (ident) qual->ident = str_new_copy(ident);
	//копирование префикса (если есть)
	if (pref_ident) qual->pref_ident = str_new_copy(pref_ident);
}


//-----------------------------------------------------------------------------
//запись кода в dfn, если переменная именованного типа
void CBaseVar::WriteDFN_named_type(DFN_file &f)
{
	fprintf(f.f, "%s : ", name);
	if (type_module_name) fprintf(f.f, "%s.", type_module_name);
	fprintf(f.f, "%s", type_name);
}


//-----------------------------------------------------------------------------
//добавление имени в таблицу имен
void CWithLoopLink::AddName(const char* ModuleName, CBaseName *GuardVar) const
{
	//запоминание названия модуля (если есть)
	if (ModuleName) VarModuleName = str_new_copy(ModuleName);
	//запоминание переменной
	Var = GuardVar;
}


//-----------------------------------------------------------------------------
//поиск имени вида имя_модуля.имя в таблице имен
CBaseName* CWithLoopLink::FindImportedName(const char *module_name, const char *search_name) const
{
	if (Var && VarModuleName && !strcmp(search_name, Var->name) && !strcmp(module_name, VarModuleName)) return Var;
	return parent_element->GetGlobalName(module_name, search_name);
}


//-----------------------------------------------------------------------------
//поиск имени в таблице имен
CBaseName* CWithLoopLink::FindName(const char* search_name) const
{
	if (Var && !VarModuleName && !strcmp(search_name, Var->name)) return Var;
	return parent_element->FindName(search_name);
}


