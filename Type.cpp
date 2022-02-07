//=============================================================================
// Описание классов типов (Type)
//=============================================================================

#include "Stat.h"
#include "Type.h"
#include "Var.h"


//-----------------------------------------------------------------------------
//инициализация списка родительского объекта объектами-переменными
int CIdentList::AssignVars(CLexBuf *lb)
{
	//загрузка списка идентификаторов
	int err_num = LoadIdentList(lb);
	if (err_num) return err_num;

	//в случае именованного типа необходимо проверять его экспорт при экспорте переменной данного типа
	//CBaseType* BT = NULL;	//ук. на тип можно не получать пока он реально не потребуется

	//создание объявленных переменных по указанному типу
	CBaseNameVector::iterator vi;
	for (vi = BNVector.begin(); vi != BNVector.end(); ++vi) {
		CIdentDef* IdentDef = static_cast<CIdentDef*>(*vi);
		//создание переменной требуемого типа
		CBaseVar* BaseVar;
		err_num = BaseType->CreateVar(BaseVar, parent_element);
		if (err_num) return err_num;
		//установка параметров переменной по ее описанию в списке идентификаторов
		err_num = IdentDef->AssignVar(BaseVar);
		if (err_num) {
			delete BaseVar;
			return err_num;
		}

		//проверка получения неспециализированной обобщенной переменной
		if (id_CCommonVar == BaseVar->name_id && static_cast<CCommonVar*>(BaseVar)->IsPureCommon()) {
			//попытка получить имя специализации по умолчанию
			//получение типа обобщение
			CBaseType* CT;
			if (id_CQualidentType == BaseType->name_id) {
				err_num = static_cast<CQualidentType*>(BaseType)->GetNamedType(CT, false);
				if (err_num) return err_num;
			} else
				CT = BaseType;
			if (id_CCommonType != CT->name_id) return s_e_CommonTypeExpected;
			const CCommonType::SSpec* spec = static_cast<CCommonType*>(CT)->GetDefaultSpec();
			if (!spec) {
				//специализация по умолчанию не найдена, выдача сообщения об ошибке
				delete BaseVar;
				return s_e_SpecTypeExpected;
			}
			//установка имени специализации по умолчанию
			err_num = static_cast<CCommonVar*>(BaseVar)->SetTagName(spec->QualName, spec->Name);
			if (err_num) return err_num;
		}

		//занесение полученной переменной в список
		parent_element->AddName(BaseVar);
	}

	return 0;
}//CIdentList::AssignVars


//-----------------------------------------------------------------------------
//деструктор объекта CDeclSeq
CDeclSeq::~CDeclSeq()
{
	CBaseNameVector::const_iterator ci;
	for(ci = BaseNameStore.begin(); ci != BaseNameStore.end(); ++ci)
		delete *ci;
	delete DfnModuleSeq;
}//~CDeclSeq


/**/
/*
//-----------------------------------------------------------------------------
//распечатка DeclSeq (таблицы имен)
void CDeclSeq::debug_PutsDeclSeq()
{
	char *st = new char[MaxStLeng];
	strcpy(st, "===== Name table (");
	strcat(st, parent_element->name);
	strcat(st, ") =====");
	puts(st);
	BaseNameList_type::iterator i;
	for(i = BaseNameList.begin(); i != BaseNameList.end(); ++i)
		switch ((*i)->name_id) {
		case id_CQualidentType:
		case id_CArrayType:
		case id_CRecordType:
		case id_CPointerType:
		case id_CSetType:
		case id_CLongrealType:
		case id_CRealType:
		case id_CLongintType:
		case id_CIntegerType:
		case id_CShortintType:
		case id_CCharType:
		case id_CBooleanType:
		case id_CProcedureType:
			strcpy(st, (*i)->name);
			strcat( st, " = TYPE");
			puts(st);
			break;
		case id_CForwardDecl:
			strcpy(st, (*i)->name);
			strcat( st, " = PROCEDURE^ (Forward)");
			puts(st);
			break;
		case id_CProcedure:
			strcpy(st, (*i)->name);
			strcat( st, " = PROCEDURE");
			puts(st);
			break;
		case id_CDfnProcedure:
			strcpy(st, (*i)->name);
			strcat( st, " = PROCEDURE (DFN)");
			puts(st);
			break;
		case id_CArrayVar:
		case id_CRecordVar:
		case id_CPointerVar:
		case id_CProcedureVar:
		case id_CLongrealVar:
		case id_CRealVar:
		case id_CLongintVar:
		case id_CIntegerVar:
		case id_CShortintVar:
		case id_CCharVar:
		case id_CBooleanVar:
		case id_CSetVar:
			strcpy(st, (*i)->name);
			strcat( st, " = VAR");
			puts(st);
			break;
		case id_CImportModule:
			//Imported table
			const char* r_n = ((CImportModule*)(*i))->real_name;
			CDfnModule* DM = NULL;
			if (DfnModuleSeq) DM = DfnModuleSeq->FindName(r_n);
			if (DM) DM->DfnDeclSeq->debug_PutsDeclSeq();
			break;
		default:
			int num = (*i)->name_id;
			puts((*i)->name);
		};//switch
	strcpy(st, "----- end table (");
	strcat(st, parent_element->name);
	strcat(st, ") ------");
	puts(st);
}//PutsDeclSeq
*/


//-----------------------------------------------------------------------------
//инициализация объекта CDeclSeq
int CDeclSeq::Init(CLexBuf *lb)
{
	DECL_SAVE_POS
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;
	//переменная для получения номера ошибки
	int err_num;

	//проверка наличия ключевого слова (отсутствие ошибок чтения)
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;

	//проверка наличия послед-сти деклараций CONST, TYPE, VAR
	while (true) {

		//тип блока деклараций (константы, типы, переменные)
		enum {dkCONST, dkTYPE, dkVAR} DeclKind;
		//проверка полученной лексемы
		if (lex_k_CONST == li.lex) DeclKind = dkCONST;
		else
			if (lex_k_TYPE == li.lex) DeclKind = dkTYPE;
			else
				if (lex_k_VAR == li.lex) DeclKind = dkVAR;
				else
					break;

		//обработка послед-ти деклараций (или CONST, или TYPE, или VAR)
		while(true) {

			//перед поиском необязательного эл-та запоминаем позицию
			SAVE_POS

			//поиск необязательного эл-та (в завис-ти от вида декларации)
			switch (DeclKind) {
			case dkCONST:
				err_num	= ConstInit(lb);
				break;
			case dkTYPE:
				err_num	= TypeInit(lb);
				break;
			case dkVAR:
				err_num	= VarInit(lb);
				break;
			}

			//проверка инициализации одиночной декларации
			if (err_num == s_m_IdentDefAbsent) {
				//выход, если не нашли декларацию (восст. позицию)
				RESTORE_POS
				break;
			}

			//проверка отсутствия ошибок инициализации декларации
			if (err_num) return err_num;

			//чтение кл. слова ";"
			if (!lb->ReadLex(li) || lex_k_semicolon != li.lex)
				return s_e_SemicolonMissing;

		}//while

		SAVE_POS

		//проверка наличия ключевого слова (отсутствие ошибок чтения)
		if (!lb->ReadLex(li) || (lex_k_dot > li.lex))
			return s_e_BEGIN;
	}//while

	//выполнение доп. проверок корректности таблицы имен
	err_num = CheckCompleteRoot(lb);
	if (err_num) return err_num;

	//проверка наличия послед-сти деклараций PROCEDURE, PROCEDURE ^
	while (lex_k_PROCEDURE == li.lex) {
		err_num = ProcInit(lb);
		if (err_num) return err_num;

		//чтение кл. слова ";"
		if (!lb->ReadLex(li) || lex_k_semicolon != li.lex) return s_e_SemicolonMissing;

		SAVE_POS

		//проверка наличия ключевого слова (отсутствие ошибок чтения)
		if (!lb->ReadLex(li) || (lex_k_dot > li.lex))
			return s_e_BEGIN;
	}

	//последняя считанная лексема не из DeclSeq
	RESTORE_POS

	return 0;
}//CDeclSeq::Init


//-----------------------------------------------------------------------------
//добавление импортируемых модулей в список деклараций
int CDeclSeq::ImportListInit(const CProject *project, CLexBuf *lb)
{
	DECL_SAVE_POS
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия кл. слова IMPORT
	if (!lb->ReadLex(li) || lex_k_IMPORT != li.lex) {
		//нет списка импорта - восстанавливаем позицию
		RESTORE_POS
		return 0;
	}
	else {
		while (true) {
			//есть список импорта - загружаем его
			CImportModule *ImpMod = new CImportModule(parent_element);
			int err_num = ImpMod->Init(lb);
			if (err_num) {
				delete ImpMod;
				return err_num;
			}
			//проверка отсутствия имени в таблице имен (DeclSeq)
			if (FindName(ImpMod->name)) {
				delete ImpMod;
				return s_e_Redefinition;
			}

			//занесение считанного объекта в список
			AddName(ImpMod);

			//проверка отсутствия импортирования самого себя
			if (!strcmp(parent_element->name, ImpMod->real_name))
				return s_e_RecursiveImport;

			//загрузка dfn импортированного модуля
			err_num = LoadDfnModule(project, ImpMod->real_name, ImpMod->name);
			if (err_num) return err_num;

			//проверка наличия ","
			if (!lb->ReadLex(li) || lex_k_comma != li.lex) break;
		}
		//проверка наличия ";"
		if (lex_k_semicolon != li.lex) return s_e_SemicolonMissing;
	}

	return 0;
}//CDeclSeq::ImportListInit


//-----------------------------------------------------------------------------
//Запись кода CDeclSeq для типов
int CDeclSeq::WriteCPP_type(CPP_files& f)
{
	if (id_CModule == parent_element->name_id) fprintf(f.fc, "\n//IMPORT\n");
	CBaseNameVector::const_iterator ci;

	//запись кода импортируемых модулей
	for (ci = BaseNameStore.begin(); ci != BaseNameStore.end(); ++ci) {
		if (id_CImportModule == (*ci)->name_id) {
			(*ci)->WriteCPP(f);
			//отделение namespace'ов друг от друга
			fprintf(f.fc, "\n");
		}
	}

	//запись кода обычных типов (все глобальные типы пишутся в заголовочный файл, т.к.
	//они могут использоваться при экспорте переменной)
	for (ci = BaseNameStore.begin(); ci != BaseNameStore.end(); ++ci)
		if (CBaseType::IsTypeId((*ci)->name_id)) {
			//проверка, является ли тип глобальным, если нет - проверка признака наличия '*'
			bool to_h = (id_CModule == (*ci)->parent_element->name_id) || (*ci)->external;
			//проверка, является ли тип записью (для записей вначале пишется только объявление)
			if (id_CRecordType == (*ci)->name_id)
				fprintf(to_h ? f.fh : f.fc, "struct %s;\n", (*ci)->name);
			else	//обычная запись кода объявления типа
				static_cast<CBaseType*>(*ci)->WriteCPP(f, to_h);
		}

	//генерация кода описаний типа запись (до этого были сгенерированы только объявления,
	//это необходимо, т.к. в методе могут использоваться еще не объявленные на момент объявления
	//записи типы, а объявление метода помещается в код записи)
	for (ci = BaseNameStore.begin(); ci != BaseNameStore.end(); ++ci)
		if (id_CRecordType == (*ci)->name_id) {
			//проверка, является ли тип глобальным, если нет - проверка признака наличия '*'
			bool to_h = (id_CModule == (*ci)->parent_element->name_id) || (*ci)->external;
			//описание типа запись
			static_cast<CRecordType*>(*ci)->WriteCPP(f, to_h);
		}

	return 0;
}//WriteCPP_type


//-----------------------------------------------------------------------------
//Запись кода CDeclSeq для процедур
int CDeclSeq::WriteCPP_proc(CPP_files& f)
{
	CBaseNameVector::const_iterator ci;
	for (ci = BaseNameStore.begin(); ci != BaseNameStore.end(); ++ci)
		if (CProcedure::IsProcId((*ci)->name_id)) {
			fprintf(f.fc, "\n%s", comment_line_cpp);
			(*ci)->WriteCPP(f);
		}
	return 0;
}//WriteCPP_proc


//-----------------------------------------------------------------------------
//Запись кода CDeclSeq для переменных
int CDeclSeq::WriteCPP_var(CPP_files& f)
{
	//признак глобальности всех деклараций в данной таблице имен
	bool is_global = id_CModule == parent_element->name_id;
	//запись комментария начала описания переменных
	fprintf(f.fc, "%s//VAR\n", id_CModule == parent_element->name_id ? "\n" : "");
	//цикл перебора переменных
	CBaseNameVector::const_iterator ci;
	for(ci = BaseNameStore.begin(); ci != BaseNameStore.end(); ++ci)
		if (CBaseVar::IsVarId((*ci)->name_id)) {
			//проверка необходимости включения переменной в безымянное пространство имен
			//(необходимо для глобальных неэкспортированных переменных)
			bool req_unnm_space = is_global && !(*ci)->external;
			if (req_unnm_space) fprintf(f.fc, "namespace {");
			//генерация кода переменной
			(*ci)->WriteCPP(f);
			//отделение переменных друг от друга с записью '}' (если необходимо закрыть пространство имен)
			fprintf(f.fc, ";%s\n", req_unnm_space ? "}" : "");
			//отделение переменных в .h файле
			if ((*ci)->external) fprintf(f.fh, ";\n");
		}
	return 0;
}//WriteCPP


//-----------------------------------------------------------------------------
//Запись в .dfn файл
void CDeclSeq::WriteDFN(DFN_file& f)
{
	//увеличение отступа в DFN файле
	f.tab_level++;

	CBaseNameVector::const_iterator i;

	//признак отсутствия эл-тов в категории, имеющей заголовок (IMPORT, CONST, TYPE, VAR)
	bool no_elems(true);

	//запись кода импортируемых модулей
	for (i = BaseNameStore.begin(); i != BaseNameStore.end(); ++i)
		if ( (*i)->name_id == id_CImportModule )
		{
			//проверка первого импортированного модуля
			if (no_elems) {
				no_elems = false;
				fprintf(f.f, "\nIMPORT\n\t");
			} else
				fprintf(f.f, ", ");
			//запись кода текущего модуля
			(*i)->WriteDFN(f);
		}
	if (!no_elems) fprintf(f.f, ";\n");

	//запись кода экспортируемых констант
	no_elems = true;
	for (i = BaseNameStore.begin(); i != BaseNameStore.end(); ++i)
		if ( (*i)->external && CBaseVar::IsVarId((*i)->name_id) && static_cast<CBaseVar*>(*i)->is_const ) {
			//проверка 1-й экспортируемой константы
			if (no_elems) {
				no_elems = false;
				fprintf(f.f, "\nCONST\n\t");
			} else
				fprintf(f.f, "\t");
			//запись кода текущей константы
			(*i)->WriteDFN(f);
			fprintf(f.f, ";\n");
		}

	//запись кода экспортируемых типов
	no_elems = true;
	for (i = BaseNameStore.begin(); i != BaseNameStore.end(); ++i)
		if ( (*i)->external && CBaseType::IsTypeId((*i)->name_id) )
		{
			//проверка первого экспортируемого типа
			if (no_elems) {
				no_elems = false;
				fprintf(f.f, "\nTYPE\n\t");
			} else
				fprintf(f.f, "\t");
			//запись кода текущего типа
			fprintf(f.f, "%s = ", (*i)->name);
			(*i)->WriteDFN(f);
			fprintf(f.f, ";\n");
		}

	//запись кода экспортируемых переменных
	no_elems = true;
	for(i = BaseNameStore.begin(); i != BaseNameStore.end(); ++i)
		if ( (*i)->external &&
			((*i)->name_id != id_CImportModule) &&
			!CProcedure::IsProcId((*i)->name_id) &&
			!CBaseType::IsTypeId((*i)->name_id) &&
			!(CBaseVar::IsVarId((*i)->name_id) && static_cast<CBaseVar*>(*i)->is_const) )
		{
			//проверка первой экспортируемой переменной
			if (no_elems) {
				no_elems = false;
				fprintf(f.f, "\nVAR\n\t");
			} else
				fprintf(f.f, "\t");
			//запись кода текущей переменной
			(*i)->WriteDFN(f);
			fprintf(f.f, ";\n");
		}

	//запись кода экспортируемых процедур
	for(i = BaseNameStore.begin(); i != BaseNameStore.end(); ++i)
		if ((*i)->external && (id_CProcedure == (*i)->name_id || id_CCommonProc == (*i)->name_id))
		{
			fprintf(f.f, "\n");
			(*i)->WriteDFN(f);
			fprintf(f.f, ";");
		}

	//уменьшение отступа в DFN файле
	f.tab_level--;

}//WriteDFN


//-----------------------------------------------------------------------------
//поиск имени в таблице имен, NULL - если имя не найдено
CBaseName* CDeclSeq::FindName(const char* search_name) const
{
	CBaseNameVector::const_iterator ci;
	for (ci = BaseNameStore.begin(); ci != BaseNameStore.end(); ci++)
		if (!strcmp((*ci)->name, search_name))
			return *ci;
	return NULL;
}//FindName


//-----------------------------------------------------------------------------
//добавление указанного эл-та в таблицу имен
void CDeclSeq::AddName(CBaseName* BN) const
{
	BaseNameStore.push_back(BN);
}


//-----------------------------------------------------------------------------
//инициализация объекта TYPE из потока лексем
int CDeclSeq::TypeInit(CLexBuf *lb)
{
	//создание имени IdentDef
	CIdentDef IdentDef(parent_element, false);
	int err_num = IdentDef.Init(lb);
	//проверка наличия имени (может не быть) и отсутствия ошибок (могут быть)
	if (err_num) {
		if (s_e_IdentExpected == err_num)
			return s_m_IdentDefAbsent;	//нет идентификатора - это не ошибка
		else
			return err_num;				//есть некорректно помеченный идент. - это ошибка
	}

	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	DECL_SAVE_POS

	//проверка наличия кл. слова "=" или "+="
	if (!lb->ReadLex(li)) return s_e_EqualSignExpected;

	//ук. на объект (создаваемый или расширяемый тип)
	CBaseType *BaseType = NULL;

	//проверка типа полученной лексемы: "=", "+=", "." (часть описания расширения)
	switch (li.lex) {

	//обычное описание типа
	case lex_k_eq:
		//создание объекта типа из потока лексем
		err_num = TypeSelector(lb, BaseType, parent_element);
		if (err_num) return err_num;
		//занесение имени объекта в созданный объект
		IdentDef.Assign(BaseType);
		//проверка отсутствия имени в таблице имен (DeclSeq)
		if (FindName(BaseType->name)) return s_e_Redefinition;
		//проверка наличия объекта тип RECORD и создание RuntimeId, если надо
		if (id_CRecordType == BaseType->name_id) {
			err_num = static_cast<CRecordType*>(BaseType)->InitRuntimeId();
			if (err_num) return err_num;
		}
		//занесение считанного объекта в список
		AddName(BaseType);
		return 0;

	//начало описания расширения импортированного обобщения
	case lex_k_dot:
		//получение названия импортированного обобщения
		if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
		//поиск импортированного обобщения в таблицах имен
		BaseType = static_cast<CBaseType*>(parent_element->GetGlobalName(IdentDef.name, li.st));
		if (!BaseType) return s_e_UndeclaredIdent;
		if (id_CCommonType != BaseType->name_id) return s_e_CommonTypeExpected;
		//проверка наличия "+="
		if (!lb->ReadLex(li) || lex_k_add_assign != li.lex) return s_e_AddAssignMissing;
		//проверка отсутствия признака локальности у расширяемого обобщения
		if (static_cast<CCommonType*>(BaseType)->IsLocal) return s_e_CommonTypeLocalExt;
		//инициализация расширения импортированного обобщения
		return static_cast<CCommonType*>(BaseType)->InitExtension(lb, this);

	//описание расширения обобщения
	case lex_k_add_assign:
		//проверка отсутствия "*" (экспорт расширения не допускается)
		if (IdentDef.external) {
			RESTORE_POS
			return s_e_IdentWrongMarked;
		}
		//проверка наличия расширения неимпортированного обобщения
		if (!BaseType) {
			BaseType = static_cast<CBaseType*>(parent_element->GetGlobalName(IdentDef.name));
			if (!BaseType) {
				RESTORE_POS
				return s_e_UndeclaredIdent;
			}
			if (id_CCommonType != BaseType->name_id) {
				RESTORE_POS
				return s_e_CommonTypeExpected;
			}
		}
		//инициализация расширения локального обобщения
		return static_cast<CCommonType*>(BaseType)->InitExtension(lb, this);

	//недопустимый символ
	default:
		return s_e_EqualSignExpected;

	}//switch
}//CDeclSeq::TypeInit


//-----------------------------------------------------------------------------
//проверка завершенности всех объектов, входящих в пространство имен
int CDeclSeq::CheckCompleteRoot(CLexBuf *lb)
{
	//для получения номера ошибки
	int err_num;
	//цикл перебора всех объявлений, входящих в пространство имен
	CBaseNameVector::const_iterator ci;
	for(ci = BaseNameStore.begin(); ci != BaseNameStore.end(); ++ci)
		switch ((*ci)->name_id) {
		//проверка отсутствия в таблице имен опережающего описания без соответствующей процедуры
		case id_CForwardDecl:
			if (!static_cast<CForwardDecl*>(*ci)->CheckSatisfied(lb)) return s_e_ForwardDeclUnsat;
			continue;
		//проверка завершенности типа указатель на именованный тип
		case id_CPointerType:
			err_num = static_cast<CPointerType*>(*ci)->CheckComplete(lb);
			if (err_num) return err_num;
			continue;
		//проверка завершенности типа запись
		case id_CRecordType:
			err_num = static_cast<CRecordType*>(*ci)->CheckComplete(lb);
			if (err_num) return err_num;
			continue;
		//проверка завершенности типа массив
		case id_CArrayType:
			err_num = static_cast<CArrayType*>(*ci)->CheckComplete(lb);
			if (err_num) return err_num;
			continue;
		default:
			//проверка завершенности текущей переменной
			if (CBaseVar::IsVarId((*ci)->name_id)) {
				err_num = static_cast<CBaseVar*>(*ci)->CheckComplete(lb);
				if (err_num) return err_num;
			}
		}//switch
	//проверка завершена
	return 0;
}


//-----------------------------------------------------------------------------
//инициализация объекта CONST из потока лексем
int CDeclSeq::ConstInit(CLexBuf *lb)
{
	//создание имени IdentDef
	CIdentDef IdentDef(parent_element, false);
	if (IdentDef.Init(lb)) return s_m_IdentDefAbsent;	//нет ид-а (это НЕ ошибка)

	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия кл. слова =
	if (!lb->ReadLex(li) || lex_k_eq != li.lex) return s_e_EqualSignExpected;

	//создание константы
	CBaseVar* ConstVar;
	int err_num = ConstSelector(lb, ConstVar, parent_element);
	if (err_num) return err_num;

	//инициализация параметров константы согласно IdentDef
	err_num = IdentDef.AssignVar(ConstVar);
	if (err_num) return err_num;

	//установка признака константы
	ConstVar->is_const = true;

	//занесение полученной константы в таблицу имен
	parent_element->AddName(ConstVar);

	return 0;
}//CDeclSeq::ConstInit


//-----------------------------------------------------------------------------
//инициализация объекта PROCEDURE ([^]) из потока лексем
int CDeclSeq::ProcInit(CLexBuf *lb)
{
	DECL_SAVE_POS
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;
	//для временного хранения создаваемого объекта
	CProcedure *Proc;
	//для перебора контейнера объявлений
	CBaseNameVector::const_iterator ci;

	//проверка отсутствия кл. слова ^
	if (!lb->ReadLex(li) || lex_k_up_arrow != li.lex) {
		//инициализация процедуры
		RESTORE_POS
		Proc = new CProcedure(parent_element);
	} else	//инициализация опережающего описания процедуры
		Proc = new CForwardDecl(parent_element);

	//инициализация процедуры и проверка отсутствия обобщений в ее параметрах
	int err_num = Proc->Init(lb);
	if (err_num && (s_m_CommonProcFound != err_num)) goto fault_exit;

	//проверка обнаружения обобщенной процедуры
	if (s_m_CommonProcFound == err_num) {
		//найдена обобщающая процедура или обработчик
		delete Proc;
		//попытка инициализации обобщающей параметрической процедуры
		RESTORE_POS
		Proc = new CCommonProc(parent_element);
		err_num = Proc->Init(lb);
		if (err_num && (s_m_HProcFound != err_num)) goto fault_exit;

		//проверка наличия обработчика парам. специализации
		if (s_m_HProcFound == err_num) {
			//найден обработчик парам. специализации - уничтожаем CCommonProc
			delete Proc;
			//инициализация обработчика парам. специализации
			RESTORE_POS
			Proc = new CHandlerProc(parent_element);
			err_num = Proc->Init(lb);
			if (err_num) goto fault_exit;
		}

		//проверка наличия другого обработчика с совпадающим списком параметров выполняется линковщиком
		AddName(Proc);
		return 0;
	}

	//проверка наличия в данной области видимости других объектов с совпадающим именем
	for (ci = BaseNameStore.begin(); ci != BaseNameStore.end(); ci++)
		if (CProcedure::IsProcId((*ci)->name_id)) {
			if (Proc->CompareProcNames(static_cast<CProcedure*>(*ci))) {
				//найдены два объекта с совпадающими именами (включая приемники),
				//допустимо наличие объявленного опережающего описания (иначе - ошибка)
				if (id_CForwardDecl != (*ci)->name_id) {
					RESTORE_POS
					err_num = s_e_Redefinition;
					goto fault_exit;
				}
				//в случае описания процедуры - проверка соответствия опережающему описанию
				if (id_CForwardDecl != Proc->name_id) {
					//проверка соответствия объявления процедуры ее опережающему описанию
					err_num = static_cast<CForwardDecl*>(*ci)->CompareDeclarations(Proc);
					if (err_num) {
						RESTORE_POS
						goto fault_exit;
					}
				}
			}
		} else {
			//имена процедур, связанных с типом, не могут конфликтовать с глобальными именами
			if (Proc->Receiver) continue;
			//сравнение с именем объекта, не являющимся процедурой
			if (!strcmp(Proc->name, (*ci)->name)) {
				RESTORE_POS
				err_num = s_e_Redefinition;
				goto fault_exit;
			}
		}

	//занесение считанного объекта в список
	AddName(Proc);
	return 0;

fault_exit:
	//уничтожение временного объекта и возврат сообщения об ошибке
	delete Proc;
	return err_num;

}//CDeclSeq::ProcInit


//-----------------------------------------------------------------------------
//инициализация объекта VAR из потока лексем
int CDeclSeq::VarInit(CLexBuf *lb)
{
	//создание списка имен переменных (с проверкой, является ли список списком локальных переменных)
	CIdentList IdentList(parent_element, (parent_element->name_id != id_CModule) && (parent_element->name_id != id_CDfnModule));
	//создание переменных по созданному списку имен
	return IdentList.AssignVars(lb);
}//CDeclSeq::VarInit


//-----------------------------------------------------------------------------
//конструктор объекта CDfnModuleSeq
CDfnModuleSeq::CDfnModuleSeq(const CBaseName* parent) : parent_element(parent)
{
}


//-----------------------------------------------------------------------------
//деструктор объекта CDfnModuleSeq
CDfnModuleSeq::~CDfnModuleSeq()
{
	DfnModuleList_type::iterator i;
	for(i = DfnModuleList.begin(); i != DfnModuleList.end(); ++i)
		delete (*i);
}//~CDfnModuleSeq


//-----------------------------------------------------------------------------
//запись в project названий всех файлов .dfn модуля (кроме себя)
void CDfnModuleSeq::EnumDfnModules(CProject &project)
{
	if (DfnModuleList.empty()) return;

	DfnModuleList_type::const_iterator ci;
	for (ci = DfnModuleList.begin(); ci != DfnModuleList.end(); ++ci) {
		//добавление очередного файла в список файлов (если его там нет)
		project.AddMakeFile( (*ci)->full_path, (*ci)->name);
		//запись файлов импортированных модулей
		if ((*ci)->DfnDeclSeq->DfnModuleSeq)
			(*ci)->DfnDeclSeq->DfnModuleSeq->EnumDfnModules(project);
	}
}


//-----------------------------------------------------------------------------
//добавление эл-та в список модулей
void CDfnModuleSeq::AddModule(CDfnModule *DM)
{
	DfnModuleList.push_back(DM);
}


//-----------------------------------------------------------------------------
//поиск имени в таблице имен, NULL - если имя не найдено
CDfnModule* CDfnModuleSeq::FindName(const char* search_name) const
{
	DfnModuleList_type::const_iterator i;
	for (i = DfnModuleList.begin(); i != DfnModuleList.end(); ++i)
		if (!strcmp( (*i)->name, search_name ))
			return *i;
	return NULL;
}//FindName


//-----------------------------------------------------------------------------
//Запись кода CDfnModuleSeq
void CDfnModuleSeq::WriteCPP(CPP_files& f)
{
	DfnModuleList_type::const_iterator i;
	for (i = DfnModuleList.begin(); i != DfnModuleList.end(); ++i)
		(*i)->WriteCPP(f);
}


//-----------------------------------------------------------------------------
//деструктор объекта MODULE
CModule::~CModule()
{
	delete StatementSeq;
	delete DeclSeq;
}//~CModule


//-----------------------------------------------------------------------------
//инициализация объекта MODULE
int CModule::Init(const CProject *project, CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия кл. слова MODULE
	if (!lb->ReadLex(li) || lex_k_MODULE != li.lex) return s_e_MODULE;

	//проверка наличия названия модуля
	if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	if (!strcmp(li.st, "SYSTEM")) return s_e_MODULE_SYSTEM;
	SetName(li.st);

	//проверка наличия ";"
	if (!lb->ReadLex(li) || lex_k_semicolon != li.lex) return s_e_SemicolonMissing;

	//создание DeclSeq (для хранения импортируемых модулей)
	DeclSeq = new CDeclSeq(this);
	
	//загрузка списка импортируемых модулей (если есть)
	int err_num = DeclSeq->ImportListInit(project, lb);
	if (err_num) return err_num;

	//проверка наличия DeclSeq
	err_num = DeclSeq->Init(lb);
	if (err_num) return err_num;

	//чтение очередной лексемы (ключевого слова)
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;

	//проверка наличия операторов, если получен BEGIN
	if (lex_k_BEGIN == li.lex) {
		//проверка наличия послед. операторов
		StatementSeq = new CStatementSeq(this);
		err_num = StatementSeq->Init(lb);
		if (err_num) return err_num;
		//чтение очередной лексемы (ключевого слова)
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;
	}

	//проверка наличия кл. слова MODULE END
	if (lex_k_END != li.lex) return s_e_END;

	//проверка наличия названия модуля (в конце)
	if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	if (strcmp(li.st, name)) return s_e_ModuleEndName;

	//проверка наличия "."
	if (!lb->ReadLex(li) || lex_k_dot != li.lex) return s_e_DotMissing;

	return 0;
}//CModule::Init


//-----------------------------------------------------------------------------
//Запись кода CModule
void CModule::WriteCPP(CPP_files& f)
{
	fprintf(f.fc, "//MODULE %s;\n", name);
	fprintf(f.fc, "#include \"%s.h\"\n", name);
	fprintf(f.fc, "using namespace %s;\n", name);

	//генерация кода импортируемых модулей
	if (DeclSeq->DfnModuleSeq) DeclSeq->DfnModuleSeq->WriteCPP(f);

	//занесение деклараций в общее пространство имен
	fprintf(f.fh, "namespace %s {\n", name);

	//генерация кода деклараций типов
	DeclSeq->WriteCPP_type(f);
	//генерация кода переменных
	DeclSeq->WriteCPP_var(f);
	//генерация кода деклараций процедур
	DeclSeq->WriteCPP_proc(f);

	fprintf(f.fh, "//MODULE INITIALIZATION\nvoid %s%s();\n", O2M_SYS_, name);

	fprintf(f.fc, "\n%s//MODULE INITIALIZATION\n", comment_line_cpp);
	fprintf(f.fc, "void %s::%s%s() {\n", name, O2M_SYS_, name);

	//инициализация импортируемых модулей
	DeclSeq->WriteCPP_mod_init(f);

	//генерация кода для операторов (если есть)
	EHaveRet have_return = hr_No;
	if (StatementSeq) {
		have_return = StatementSeq->HaveRet();
		StatementSeq->WriteCPP(f);
	}

	//код для реализации оператора RETURN (если есть RETURNы)
	if (hr_No != have_return) fprintf(f.fc, "\tO2M_RETURN:;\n");

	fprintf(f.fc, "}\n//END %s.\n", name);
}//WriteCPP


//-----------------------------------------------------------------------------
//поиск имени в таблице имен, NULL - если имя не найдено
CBaseName* CModule::FindName(const char* search_name) const
{
	if (DeclSeq) return DeclSeq->FindName(search_name);
	return NULL;
}


//-----------------------------------------------------------------------------
//добавление указанного эл-та в таблицу имен
void CModule::AddName(CBaseName* BN) const
{
	DeclSeq->AddName(BN);
}


//-----------------------------------------------------------------------------
//инициализация объекта ImportModule
int CImportModule::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//получение первого ид-а (имени модуля для использования)
	if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	SetName(li.st);

	DECL_SAVE_POS

	//проверка наличия кл. слова ":="
	if (!lb->ReadLex(li) || lex_k_assign != li.lex) {
		RESTORE_POS
		real_name = str_new_copy(name);
		return 0;
	}

	//получение второго ид-а (исходного имени модуля)
	if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	real_name = str_new_copy(li.st);

	return 0;
}//CImportModule::Init


//-----------------------------------------------------------------------------
//Запись кода CImportModule
void CImportModule::WriteCPP(CPP_files& f)
{
	//создание пространства имен при переименовании модуля
	if (strcmp(name, real_name)) {
		fprintf(f.fc, "//%s := %s\n", name, real_name);
		fprintf(f.fc, "namespace %s {using namespace %s;}", name, real_name);
	} else
		fprintf(f.fc, "//%s", name);
}//WriteCPP


//-----------------------------------------------------------------------------
//конструктор объекта CForwardDecl
CForwardDecl::CForwardDecl(const CBaseName* parent) : CProcedure(parent),
	Satisfied(false)
{
	name_id = id_CForwardDecl;
}


//-----------------------------------------------------------------------------
//проверка состояния флага Satisfied и установка позиции в потоке лексем в
//случае ошибки (отсутствия процедуры)
bool CForwardDecl::CheckSatisfied(CLexBuf *lb) const
{
	//если флаг возведен - все нормально
	if (Satisfied) return true;
	//установка позиции в потоке лексем по LexBufPos в случае ошибки
	lb->SetCurrPos(FDeclPos);
	//возврат признака ошибки
	return false;
}


//-----------------------------------------------------------------------------
//сравнение опережающего объявления с объявлением указанной процедуры и
//установка флага Satisfied, возврат - номер ошибки (с установкой позиции
//в потоке лексем) или 0
int CForwardDecl::CompareDeclarations(CProcedure *P)
{
	//проверка соответствия названий параметров в приемниках (если есть)
	if (Receiver && strcmp(Receiver->name, P->Receiver->name)) return s_e_ParamNotMatch;
	//проверка соответствия количеств параметров
	if (FormalPars->FPStore.size() != P->FormalPars->FPStore.size()) return s_e_ParamCountNotMatch;

	/**/
	//доделать

	Satisfied = true;
	return 0;
}


//-----------------------------------------------------------------------------
//инициализация объекта CForwardDecl
int CForwardDecl::Init(CLexBuf *lb)
{
	DECL_SAVE_POS
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;
	//переменная для получения номера ошибки
	int err_num;

	//проверка наличия приемника
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) {
		RESTORE_POS
	} else {
		Receiver = new CReceiver(this);
		err_num = Receiver->Init(lb, this);
		if (err_num) return err_num;
	}

	//проверка наличия имени и занесение его в текущий объект
	CIdentDef IdentDef(parent_element, false);
	if (err_num = IdentDef.Init(lb)) return err_num;
	IdentDef.Assign(this);

	//сохранение позиции в потоке лексем на случай возникновения ошибки
	FDeclPos = lb->GetCurrPos();

	//создание списка формальных параметров
	FormalPars = new CFormalPars;
	err_num = FormalPars->Init(lb, this);
	if (err_num) return err_num;

	//в случае наличия приемника необходимо занести ссылку на процедуру в таблицу имен
	//типа, с которым она связана
	if (Receiver) {
		//поиск именованного типа запись (должен быть - проверено в приемнике)
		CBaseName* BN = parent_element->GetGlobalName(Receiver->type_name);
		if (id_CPointerType == BN->name_id) BN = static_cast<CPointerType*>(BN)->FindType();
		//занесение в объект с проверкой отсутствия одноименных полей в объекте
		err_num = static_cast<CRecordType*>(BN)->AddProcReference(this);
		if (err_num) return err_num;
	}

	return 0;
}//CForwardDecl::Init


//-----------------------------------------------------------------------------
//Запись кода CForwardDecl
void CForwardDecl::WriteCPP(CPP_files& f)
{
	//запись комментариев
	fprintf(f.fc, "//PROCEDURE^ %s%s%s\n", Receiver ? Receiver->type_name : "", Receiver ? "." : "", name);

	//генерация неименованного namespace для локальной не связанной с типом процедуры
	bool req_unnm_nmsp = !external && !Receiver;
	if (req_unnm_nmsp) fprintf(f.fc, "namespace {\n");

	//генерация кода типа процедуры в описании
	FormalPars->WriteCPP_type(f, false, parent_element);

	//генерация объявления процедуры в заголовочном файле в случае ее экспорта
	//(для процедур, связанных с типом, это не требуется)
	if (external && !Receiver) {
		fprintf(f.fh, "//PROCEDURE %s%s%s\n", Receiver ? Receiver->type_name : "", Receiver ? "." : "", name);
		//генерация кода типа экспортируемой процедуры в объявлении
		FormalPars->WriteCPP_type(f, true, parent_element);
		//генерация названия процедуры в объявлении
		fprintf(f.fh, "%s(", name);
		//генерация namespace для экспорт-й не связанной с типом процедуры в описании
		fprintf(f.fc, "::%s::", parent_element->name);
	}

	//генерация имени типа для процедуры, связанной с типом
	if (Receiver) fprintf(f.fc, "%s::", Receiver->type_name);

	//генерация названия процедуры в описании
	fprintf(f.fc, "%s(", name);

	//генерация кода формальных параметров в коде и в заголовке (если надо)
	FormalPars->WriteCPP_pars(f, false);
	fprintf(f.fc, ");\n");
	if (external && !Receiver) {
		FormalPars->WriteCPP_pars(f, true);
		fprintf(f.fh, ");\n");
	}

	//в случае включения процедуры в неименованный namespace требуется скобка '}'
	if (req_unnm_nmsp) fprintf(f.fc, "}\n");

}//WriteCPP


//-----------------------------------------------------------------------------
//деструктор объекта PROCEDURE
CProcedure::~CProcedure()
{
	delete Receiver;
	delete FormalPars;
	delete DeclSeq;
	delete StatementSeq;
}//~CProcedure


//-----------------------------------------------------------------------------
//инициализация объекта PROCEDURE
int CProcedure::Init(CLexBuf *lb)
{
	DECL_SAVE_POS
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;
	//переменная для получения номера ошибки
	int err_num;

	//проверка наличия приемника
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) {
		RESTORE_POS
	} else {
		Receiver = new CReceiver(this);
		err_num = Receiver->Init(lb, this);
		if (err_num) return err_num;
	}

	//проверка наличия имени и занесение его в текущий объект
	CIdentDef IdentDef(parent_element, false);
	if (err_num = IdentDef.Init(lb)) return err_num;
	IdentDef.Assign(this);

	//проверка наличия списка обобщенных параметров (тогда это обобщающая прцедура или
	//обработчик), ошибка наличия приемника у обобщающей процедуры проверяется в CCommonProc
	SAVE_POS
	if (!lb->ReadLex(li) || lex_k_op_brace == li.lex || lex_k_dot == li.lex)
		return s_m_CommonProcFound;
	else {
		RESTORE_POS
	}

	//создание списка формальных параметров
	FormalPars = new CFormalPars;
	err_num = FormalPars->Init(lb, this);
	if (err_num) return err_num;

	//проверка наличия ";"
	if (!lb->ReadLex(li) || lex_k_semicolon != li.lex) return s_e_SemicolonMissing;

	//в случае наличия приемника необходимо занести ссылку на процедуру в таблицу имен
	//типа, с которым она связана
	if (Receiver) {
		//поиск именованного типа запись (должен быть - проверено в приемнике)
		CBaseName* BN = parent_element->GetGlobalName(Receiver->type_name);
		if (id_CPointerType == BN->name_id) BN = static_cast<CPointerType*>(BN)->FindType();
		//занесение в объект с проверкой отсутствия одноименных полей в объекте
		err_num = static_cast<CRecordType*>(BN)->AddProcReference(this);
		if (err_num) {
			RESTORE_POS
			return err_num;
		}
	}

	//проверка наличия DeclSeq
	DeclSeq = new CDeclSeq(this);
	err_num = DeclSeq->Init(lb);
	if (err_num) return err_num;

	//чтение очередной лексемы (ключевого слова)
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;

	//проверка наличия операторов, если получен BEGIN
	if (lex_k_BEGIN == li.lex) {
		//проверка наличия послед. операторов
		StatementSeq = new CStatementSeq(this);
		err_num = StatementSeq->Init(lb);
		if (err_num) return err_num;
		//чтение очередной лексемы (ключевого слова)
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;
	}

	//проверка наличия RETURN в послед-ти операторов
	have_return = StatementSeq ? StatementSeq->HaveRet() : hr_No;
	//проверка присутствия RETURN в случае процедуры-функции
	if (FormalPars->Qualident) {
		if (hr_No == have_return)
			return s_e_FuncWithoutRETURN;
		else
			if (hr_NotAll == have_return) WriteWarn(s_w_NotAllPathsRETURN, lb);
	}

	//проверка наличия кл. слова END
	if (lex_k_END != li.lex) return s_e_END;

	//проверка наличия названия процедуры (в конце)
	if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	if (strcmp(li.st, name)) return s_e_ProcedureEndName;

	return 0;
}//CProcedure::Init


//-----------------------------------------------------------------------------
//Запись кода CProcedure
void CProcedure::WriteCPP(CPP_files& f)
{
	//запись комментариев
	fprintf(f.fc, "//PROCEDURE %s%s%s\n", Receiver ? Receiver->type_name : "", Receiver ? "." : "", name);

	//генерация неименованного namespace для локальной не связанной с типом процедуры
	bool req_unnm_nmsp = !external && !Receiver;
	if (req_unnm_nmsp) fprintf(f.fc, "namespace {\n");

	//генерация кода типа процедуры в описании
	FormalPars->WriteCPP_type(f, false, parent_element);

	//генерация объявления процедуры в заголовочном файле в случае ее экспорта
	//(для процедур, связанных с типом, это не требуется)
	if (external && !Receiver) {
		fprintf(f.fh, "//PROCEDURE %s\n", name);
		//генерация кода типа экспортируемой процедуры в объявлении
		FormalPars->WriteCPP_type(f, true, parent_element);
		//генерация названия процедуры в объявлении
		fprintf(f.fh, "%s(", name);
		//генерация namespace для экспорт-й не связанной с типом процедуры в описании
		fprintf(f.fc, "%s::", parent_element->name);
	}

	//генерация имени типа для процедуры, связанной с типом
	if (Receiver) fprintf(f.fc, "%s::", Receiver->type_name);

	//генерация названия процедуры в описании
	fprintf(f.fc, "%s(", name);

	//генерация кода формальных параметров в описании и в объявлении (если надо)
	FormalPars->WriteCPP_pars(f, false);
	fprintf(f.fc, ") {\n");
	if (external && !Receiver) {
		FormalPars->WriteCPP_pars(f, true);
		fprintf(f.fh, ");\n");
	}

	//генерация кода приемника (если есть)
	if (Receiver) Receiver->WriteCPP(f);

	//генерация переменной для хранения результатов процедуры-функции
	if (FormalPars->Qualident) {
		fprintf(f.fc, "\t");
		FormalPars->Qualident->WriteCPP_type(f, false, parent_element);
		fprintf(f.fc, " O2M_RESULT;\n");
	}
	//инициализация массивов-значений
	FormalPars->WriteCPP_begin(f);

	//генерация кода деклараций типов
	DeclSeq->WriteCPP_type(f);
	//генерация кода переменных
	DeclSeq->WriteCPP_var(f);

	//генерация кода для операторов
	if (StatementSeq) {
		fprintf(f.fc, "//BEGIN\n");
		StatementSeq->WriteCPP(f);
	}
	//деинициализация открытых массивов-значений
	FormalPars->WriteCPP_end(f, hr_No != have_return);
	//выдача результатов процедуры-функции
	if (FormalPars->Qualident) fprintf(f.fc, "\treturn O2M_RESULT;\n");
	//в случае включения процедуры в неименованный namespace требуется доп. скобка '}'
	if (req_unnm_nmsp) fprintf(f.fc, "}");
	//завершение описания операторов процедуры
	fprintf(f.fc, "}\n//END %s;\n", name);
}//WriteCPP


//-----------------------------------------------------------------------------
//запись описания процедуры (связанной с типом) в объявлении типа запись,
//RT - тип, с которым связана данная процедура
void CProcedure::WriteCPP_RECORD(CPP_files &f, const CRecordType* RT, const bool to_h)
{
	//для упрощения выбора записи в заголовочный файл или код
	TFileType* ff = to_h ? f.fh : f.fc;
	//генерация признака виртуализации
	fprintf(ff, "virtual ");
	//генерация кода типа процедуры
	FormalPars->WriteCPP_type(f, to_h, parent_element);
	//генерация названия процедуры
	fprintf(ff, "%s(", name);
	//генерация кода формальных параметров
	FormalPars->WriteCPP_pars(f, to_h);
	fprintf(ff, ")");
}


//-----------------------------------------------------------------------------
//Запись в .dfn файл
void CProcedure::WriteDFN(DFN_file& f)
{
	fprintf(f.f, "PROCEDURE ");
	//запись кода для приемника (если есть)
	if (Receiver) {
		fprintf(f.f, "(");
		if (Receiver->is_var)
			fprintf(f.f, "VAR ");
		fprintf(f.f, "%s : %s) ", Receiver->name, Receiver->type_name);
	}
	//запись названия процедуры
	fprintf(f.f, "%s (\n", name);
	//запись объявления формальных параметров
	FormalPars->WriteDFN(f);
	fprintf(f.f, "\n)");
	//запись типа процедуры (если есть)
	FormalPars->WriteDFN_type(f);
}//WriteDFN


//-----------------------------------------------------------------------------
//поиск имени вида имя.имя в таблице имен
CBaseName* CProcedure::FindImportedName(const char *module_name, const char *search_name) const
{
	//проверка факта рекурсивного вызова самой процедуры в случае процедуры, связанной с типом
	if (Receiver && !strcmp(Receiver->name, module_name) && !strcmp(name, search_name))
		return const_cast<CProcedure*>(this);
	else
		return parent_element->GetGlobalName(module_name, search_name);
}


//-----------------------------------------------------------------------------
//поиск имени в таблице имен, NULL - если имя не найдено
CBaseName* CProcedure::FindName(const char* search_name) const
{
	//проверка факта рекурсивного вызова самой процедуры
	if (name && !strcmp(name, search_name)) return const_cast<CProcedure*>(this);
	//проверка остальных выриантов
	CBaseName* BN = NULL;
	//поиск имени в декларациях
	if (DeclSeq) BN = DeclSeq->FindName(search_name);
	//поиск имени в списке формальных параметров
	if (!BN && FormalPars) BN = FormalPars->FindName(search_name);
	//поиск имени в приемнике
	if (!BN && Receiver) BN = Receiver->FindName(search_name);
	//возврат найденного имени (или NULL)
	return BN;
}//FindName


//-----------------------------------------------------------------------------
//добавление указанного эл-та в таблицу имен
void CProcedure::AddName(CBaseName* BN) const
{
	if (DeclSeq) DeclSeq->AddName(BN);
}//AddName


//-----------------------------------------------------------------------------
//сравнение имени процедуры с именем указанной процедуры с учетом приемника,
//возврат - признак совпадения имен
bool CProcedure::CompareProcNames(const CProcedure *P) const
{
	if (strcmp(name, P->name) || (Receiver && !P->Receiver) || (!Receiver && P->Receiver)) return false;
	if (Receiver && strcmp(Receiver->type_name, P->Receiver->type_name)) return false;
	return true;
}


//-----------------------------------------------------------------------------
//проверка принадлежности указанного ид. к ид-у процедуры
bool CProcedure::IsProcId(const EName_id id)
{
	switch (id) {
	case id_CProcedure:
	case id_CDfnProcedure:
	case id_CDfnCommonProc:
	case id_CForwardDecl:
	case id_CCommonProc:
	case id_CHandlerProc:
		return true;
	default:
		return false;
	}//switch
}


//-----------------------------------------------------------------------------
//получения типа выражения (для процедуры - функции)
EName_id CProcedure::GetResultId() const
{
	if (!FormalPars->Qualident) return id_CBaseName;
	return FormalPars->Qualident->TypeResultId;
}


//-----------------------------------------------------------------------------
//конструктор обработчика параметрической специализации
CCommonProc::CCommonProc(const CBaseName* parent) : CProcedure(parent),
	CommonPars(NULL), DefH(false)
{
	name_id = id_CCommonProc;
}//CCommonProc


//-----------------------------------------------------------------------------
//деструктор объекта CCommonProc
CCommonProc::~CCommonProc()
{
	delete CommonPars;
}


//-----------------------------------------------------------------------------
//проверка совпадения имен формальных и обобщающих параметров, конвертация записей в обобщенные переменные
int CCommonProc::CheckParams() {
	CBaseVarVector::iterator i;
	for (i = CommonPars->FPStore.begin(); i != CommonPars->FPStore.end(); ++i) {
		//проверка типа параметра (?)
		if ((*i)->name_id != id_CRecordVar) continue;

		//проверка совпадения имени обобщенного параметра с формальным
		if (FormalPars->FindName((*i)->name)) return s_e_CommonProcParamRedefined;

		//создание копии обобщающей переменной
		CCommonVar* CV = new CCommonVar(parent_element);
		//копирование названия типа в созданную переменную
		CV->SetName((*i)->name);
		CV->SetTypeName((*i)->GetTypeModuleAlias(), (*i)->GetTypeName());
		//замена переменной в списке обобщенных параметров
		delete *i;
		(*i) = CV;
	}
	return 0;
}


//-----------------------------------------------------------------------------
//инициализация объекта CCommonProc
int CCommonProc::Init(CLexBuf *lb)
{
	//переменная для получения номера ошибки
	int err_num;

	//создание имени IdentDef
	CIdentDef IdentDef(parent_element, false);
	if (err_num = IdentDef.Init(lb)) return err_num;

	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//получение очередного символа
	if (!lb->ReadLex(li)) return s_e_CommonProcCommonParam;

	//проверка наличия "." (допустимо только у обработчиков парам. спец-ции)
	if (lex_k_dot == li.lex) return s_m_HProcFound;

	//проверка наличия обобщающих параметров
	if (lex_k_op_brace != li.lex)
		return s_e_CommonProcCommonParam;
	else {
		CommonPars = new CCommonPars;
		err_num = CommonPars->Init(lb, this);
		if (err_num) return err_num;
	}

	//создание списка формальных параметров
	FormalPars = new CFormalPars;
	err_num = FormalPars->Init(lb, this);
	if (err_num) return err_num;

	//перенос описания типа возвращаемого результата из обобщенных параметров в формальные (если надо)
	if (CommonPars->Qualident) {
		//проверка двойного объявления типа возвращаемого значения (в случае процедуры - функции)
		if (FormalPars->Qualident) return s_e_CommonProcDoubleType;
		FormalPars->Qualident = CommonPars->Qualident;
		CommonPars->Qualident = NULL;
	}

	//проверка совпадения имен формальных и обобщающих параметров, конвертация записей в обобщенные переменные
	err_num = CheckParams();
	if (err_num) return err_num;

	//занесение найденного имени в текущий объект
	IdentDef.Assign(this);

	//проверка отсутствия тела процедуры (обработчика по умолчанию)
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_SemicolonMissing;
	if (lex_k_assign == li.lex) {
		if (!lb->ReadLex(li) || lex_d != li.lex || strcmp(li.st, "0")) return s_e_ZeroMissing;
		return 0;
	}

	//обработчик по умолчанию присутствует
	DefH = true;

	//проверка наличия ";"
	if (lex_k_semicolon != li.lex) return s_e_SemicolonMissing;

	//проверка наличия DeclSeq
	DeclSeq = new CDeclSeq(this);
	err_num = DeclSeq->Init(lb);
	if (err_num) return err_num;

	//чтение очередной лексемы (ключевого слова)
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;

	//проверка наличия операторов, если получен BEGIN
	if (lex_k_BEGIN == li.lex) {
		//проверка наличия послед. операторов
		StatementSeq = new CStatementSeq(this);
		err_num = StatementSeq->Init(lb);
		if (err_num) return err_num;
		//чтение очередной лексемы (ключевого слова)
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;
	}

	//проверка наличия RETURN в послед-ти операторов
	have_return = StatementSeq ? StatementSeq->HaveRet() : hr_No;
	//проверка присутствия RETURN в случае процедуры-функции
	if (FormalPars->Qualident) {
		if (hr_No == have_return)
			return s_e_FuncWithoutRETURN;
		else
			if (hr_NotAll == have_return) WriteWarn(s_w_NotAllPathsRETURN, lb);
	}

	//проверка наличия кл. слова END
	if (lex_k_END != li.lex) return s_e_END;

	//проверка наличия названия процедуры (в конце)
	if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	if (strcmp(li.st, name)) return s_e_ProcedureEndName;

	return 0;
}//CCommonProc::Init


//-----------------------------------------------------------------------------
//запись кода одного параметра при вызове обработчика параметрической специализации
void CCommonProc::WriteCPP_HPar(CPP_files &f, CBaseVar *par)
{
	//проверка наличия массива
	if (id_CArrayVar == par->name_id) {
		//запись собственно массива (с учетом префикса - т.к. массив не инициализирован)
		fprintf(f.fc, "O2M_ARR_%s, ", par->name);

		//проверка, является ли массив открытым, тогда требуется запись доп. параметров (размерностей)
		CArrayType* AT = static_cast<CArrayVar*>(par)->ArrayType;

		//запись списка размерностей (если есть ? -> ArrOfChar + "String")
		if (AT) {
			int dimension = 0;	//отсчет размерностей с 0
			while (id_CArrayType == AT->name_id) {
				//вывод размерности (в виде числа или имени переменной, содержащей размерность)
				if (AT->size)
					fprintf(f.fc, "%i, ", AT->size);
				else
					fprintf(f.fc, "O2M_ARR_%i_%s, ", dimension, par->name);
				//переход к следующей размерности
				++dimension;
				AT = static_cast<CArrayType*>(AT->Type);
			}//while
		}

	} else	//генерация кода обычного параметра (не массива)
		fprintf(f.fc, "%s, ", par->name);
}


//-----------------------------------------------------------------------------
//Запись кода CCommonProc
void CCommonProc::WriteCPP(CPP_files& f)
{
	///////////////////////////////////////////////////////
	//запись информации об обобщенной процедуре в .2ml файл

	//запись начала описания обобщенной процедуры в .2ml файл
	fprintf(f.f2ml, "\t<CProc Name=\"%s\"%s>\n", name, DefH ? " DefH=\"yes\"" : "");
	//запись списка обобщающих параметров в .2ml файл
	CBaseVarVector::const_iterator ci;
	for (ci = CommonPars->FPStore.begin(); ci != CommonPars->FPStore.end(); ci++) {
		CCommonType* CT = static_cast<CCommonType*>(parent_element->GetGlobalName((*ci)->GetTypeModuleName(), (*ci)->GetTypeName()));
		if (!CT) continue;
		const char *QN = CT->GetModuleName();
		fprintf(f.f2ml, "\t\t<CPar");
		if (QN) fprintf(f.f2ml, " Qual=\"%s\"", QN);
		fprintf(f.f2ml, " Name=\"%s\" />\n", CT->name);
	}
	//конец описания обобщенной процедуры в .2ml файл
	fprintf(f.f2ml, "\t</CProc>\n");


	//////////////////////////////////
	//запись кода обобщенной процедуры

	//запись комментариев
	fprintf(f.fc, "//COMMON PROCEDURE %s\n", name);

	//генерация неименованного namespace для локальной не связанной с типом процедуры
	bool req_unnm_nmsp = !external && !Receiver;
	if (req_unnm_nmsp) fprintf(f.fc, "namespace {\n");

	//генерация кода типа процедуры в описании
	FormalPars->WriteCPP_type(f, false, parent_element);

	//генерация объявления процедуры в заголовочном файле в случае ее экспорта
	if (external) {
		fprintf(f.fh, "//COMMON PROCEDURE %s\n", name);
		//генерация кода типа экспортируемой процедуры в объявлении
		FormalPars->WriteCPP_type(f, true, parent_element);
		//генерация названия процедуры в объявлении
		fprintf(f.fh, "%s(", name);
		//генерация namespace для экспорт-й не связанной с типом процедуры в описании
		fprintf(f.fc, "%s::", parent_element->name);
	}

	//генерация названия процедуры в описании
	fprintf(f.fc, "%s(", name);

	//генерация кода формальных параметров в описании (если надо)
	FormalPars->WriteCPP_pars(f, false);
	if (!FormalPars->FPStore.empty()) fprintf(f.fc, ", ");
	//генерация кода формальных параметров в объявлении (если надо)
	if (external) {
		FormalPars->WriteCPP_pars(f, true);
		if (!FormalPars->FPStore.empty()) fprintf(f.fh, ", ");
	}

	//генерация кода обобщенных параметров в описании
	CommonPars->WriteCPP_pars(f, false);
	fprintf(f.fc, ") {\n");
	//генерация кода обобщенных параметров в объявлении
	if (external) {
		CommonPars->WriteCPP_pars(f, true);
		fprintf(f.fh, ");\n");
	}


	/////////////////////////////////////////////////////////////////
	//генерация кода выбора обработчика параметрической специализайии

	//генерация объявления процедурного типа и переменной для вызова обработчика
	fprintf(f.fc, "\t//handler selection\n\ttypedef ");
	FormalPars->WriteCPP_type(f, false, parent_element);
	fprintf(f.fc, "(*O2M_TYPE) (");
	if (!FormalPars->FPStore.empty()) {
		FormalPars->WriteCPP_pars(f, false);
		fprintf(f.fc, ", ");
	}
	CommonPars->WriteCPP_pars(f, false);
	//генерация получения адреса обработчика парам. спец-ции
	fprintf(f.fc, ");\n\tO2M_TYPE O2M_PROC;\n\tO2M_PROC = (O2M_TYPE)O2M_COMMON_%s_%s(", parent_element->name, name);
	for (ci = CommonPars->FPStore.begin();;) {
		fprintf(f.fc, "%s", (*ci)->name);
		++ci;
		if (CommonPars->FPStore.end() == ci) break;
		fprintf(f.fc, ", ");
	}
	//генерация проверки наличия адреса и вызова обработчика при наличии адреса
	fprintf(f.fc, ");\n\tif (O2M_PROC) %sO2M_PROC(", FormalPars->Qualident ? "return " : "{\n\t\t");
	//генерация фактических параметров при вызове обработчика параметрической специализации
	//генерация обычных фактических параметров
	for (ci = FormalPars->FPStore.begin(); ci != FormalPars->FPStore.end(); ci++)
		WriteCPP_HPar(f, *ci);
	//генерация обобщенных фактических параметров
	for (ci = CommonPars->FPStore.begin();;) {
		fprintf(f.fc, "%s", (*ci)->name);
		++ci;
		if (CommonPars->FPStore.end() == ci) break;
		fprintf(f.fc, ", ");
	}
	fprintf(f.fc, ");\n");
	if (!FormalPars->Qualident) fprintf(f.fc, "\t\treturn;\n\t}\n");


	/////////////////////////////////////////////////////
	//генерация кода обработчика по умолчанию (если есть)

	//проверка наличия обработчика по умолчанию
	if (DefH) {

		//генерация переменной для хранения результатов процедуры-функции
		if (FormalPars->Qualident) {
			fprintf(f.fc, "\t");
			FormalPars->Qualident->WriteCPP_type(f, false, parent_element);
			fprintf(f.fc, " O2M_RESULT;\n");
		}
		//инициализация массивов-значений
		FormalPars->WriteCPP_begin(f);

		//генерация кода типов и переменных
		if (DeclSeq) {
			DeclSeq->WriteCPP_type(f);
			DeclSeq->WriteCPP_var(f);
		}

		//генерация кода для операторов
		if (StatementSeq) {
			fprintf(f.fc, "//BEGIN\n");
			StatementSeq->WriteCPP(f);
		}
		//деинициализация открытых массивов-значений
		FormalPars->WriteCPP_end(f, hr_No != have_return);
		//выдача результатов процедуры-функции
		if (FormalPars->Qualident) fprintf(f.fc, "\treturn O2M_RESULT;\n");

	} else
		fprintf(f.fc, "\t//default handler absent\n\texit(0);\n");	//генерация кода прерывания программы при отсутствии обработчика по умолчанию

	//в случае включения процедуры в неименованный namespace требуется доп. скобка '}'
	if (req_unnm_nmsp) fprintf(f.fc, "}");
	//завершение описания операторов процедуры
	fprintf(f.fc, "}\n//END %s;\n", name);
}//WriteCPP


//-----------------------------------------------------------------------------
//Запись в .dfn файл
void CCommonProc::WriteDFN(DFN_file& f)
{
	fprintf(f.f, "PROCEDURE ");
	//запись названия процедуры
	fprintf(f.f, "%s {\n", name);
	//запись объявления обобщенных параметров
	CommonPars->WriteDFN(f);
	fprintf(f.f, "\n}");
	//запись объявления формальных параметров
	if (!FormalPars->FPStore.empty()) {
		fprintf(f.f, "(\n");
		FormalPars->WriteDFN(f);
		fprintf(f.f, "\n)");
	}
	//запись типа процедуры (если есть)
	FormalPars->WriteDFN_type(f);
}//WriteDFN


//-----------------------------------------------------------------------------
//конструктор обработчика параметрической специализации
CHandlerProc::CHandlerProc(const CBaseName* parent) : CProcedure(parent),
	CommonPars(NULL), QualName(NULL), UID(GetGlobalUId())
{
	name_id = id_CHandlerProc;
}//CHandlerProc


//-----------------------------------------------------------------------------
//деструктор объекта CHandlerProc
CHandlerProc::~CHandlerProc()
{
	delete CommonPars;
	delete[] QualName;
}


//-----------------------------------------------------------------------------
//преобразование обобщающих параметров в формальные
int CHandlerProc::ConvertParams() {
	CBaseVarVector::const_iterator ci;
	for (ci = CommonPars->FPStore.begin(); ci != CommonPars->FPStore.end(); ++ci) {
		//проверка типа параметра (?)
		if ((*ci)->name_id != id_CCommonVar) continue;
		//проверка совпадения имени обобщенного параметра с формальным
		if (FormalPars->FindName((*ci)->name)) return s_e_HandlerProcParamRedefined;
		//создание копии обобщающего параметра
		CBaseVar* BV = static_cast<CCommonVar*>(*ci)->CreateVar(parent_element);
		if (!BV) return s_e_CommonTypeExpected;
		//добавление переменной в список формальных параметров
		FormalPars->FPStore.push_back(BV);
	}

	return 0;
}


//-----------------------------------------------------------------------------
//инициализация обработчика параметрической специализации
int CHandlerProc::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;
	//переменная для получения номера ошибки
	int err_num;

	//получение идентификатора (имя модуля или имя обработчика)
	if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	DECL_SAVE_POS
	//пока заносим найденный идент. в имя
	name = str_new_copy(li.st);
	//проверка следующего символа (если "." - найденный идент был именем модуля)
	if (!lb->ReadLex(li)) return s_e_HandlerProcCommonParam;
	if (lex_k_dot == li.lex) {
		//получение имени импортированного модуля из псевдонима
		CBaseName* BN = parent_element->GetGlobalName(name);
		if (!BN) {
			RESTORE_POS
			return s_e_UndeclaredIdent;
		}
		if (id_CImportModule != BN->name_id) {
			RESTORE_POS
			return s_e_IdentNotModule;
		}
		//получение имени обработчика
		if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
		//сохранение имени модуля и имени обработчика
		QualName = name;
		name = str_new_copy(li.st);
		//получение следующего символа
		if (!lb->ReadLex(li)) return s_e_HandlerProcCommonParam;
	}

	//поиск обобщенной процедуры, к которой привязан данный обработчик
	CBaseName* BN = parent_element->GetGlobalName(QualName, name);
	if (!BN || (id_CCommonProc != BN->name_id && id_CDfnCommonProc != BN->name_id)) {
		RESTORE_POS
		return s_e_FreeHProc;
	}

	//проверка отсутствия меток экспорта (недопустимы для обработчиков)
	if (lex_k_asterix == li.lex || lex_k_minus == li.lex) return s_e_IdentWrongMarked;

	//проверка наличия обобщающих параметров
	if (lex_k_op_brace != li.lex)
		return s_e_HandlerProcCommonParam;
	else {
		CommonPars = new CCommonPars;
		err_num = CommonPars->InitSpec(lb, this);
		if (err_num) return err_num;
	}

	//создание списка формальных параметров
	FormalPars = new CFormalPars;
	err_num = FormalPars->Init(lb, this);
	if (err_num) return err_num;

	//перенос описания типа возвращаемого результата из обобщенных параметров в формальные (если надо)
	if (CommonPars->Qualident) {
		//проверка двойного объявления типа возвращаемого значения (в случае процедуры - функции)
		if (FormalPars->Qualident) return s_e_CommonProcDoubleType;
		FormalPars->Qualident = CommonPars->Qualident;
		CommonPars->Qualident = NULL;
	}

	//преобразование обобщающих параметров в формальные
	err_num = ConvertParams();
	if (err_num) return err_num;

	//проверка наличия ";"
	if (!lb->ReadLex(li) || lex_k_semicolon != li.lex) return s_e_SemicolonMissing;

	//проверка наличия DeclSeq
	DeclSeq = new CDeclSeq(this);
	err_num = DeclSeq->Init(lb);
	if (err_num) return err_num;

	//чтение очередной лексемы (ключевого слова)
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;

	//проверка наличия операторов, если получен BEGIN
	if (lex_k_BEGIN == li.lex) {
		//проверка наличия послед. операторов
		StatementSeq = new CStatementSeq(this);
		err_num = StatementSeq->Init(lb);
		if (err_num) return err_num;
		//чтение очередной лексемы (ключевого слова)
		if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_END;
	}

	//проверка наличия RETURN в послед-ти операторов
	have_return = StatementSeq ? StatementSeq->HaveRet() : hr_No;
	//проверка присутствия RETURN в случае процедуры-функции
	if (FormalPars->Qualident) {
		if (hr_No == have_return)
			return s_e_FuncWithoutRETURN;
		else
			if (hr_NotAll == have_return) WriteWarn(s_w_NotAllPathsRETURN, lb);
	}

	//проверка наличия кл. слова END
	if (lex_k_END != li.lex) return s_e_END;

	//проверка наличия названия процедуры (в конце)
	if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	if (strcmp(li.st, name)) return s_e_ProcedureEndName;

	return 0;
}


//-----------------------------------------------------------------------------
//Запись кода обработчика параметрической специализации
void CHandlerProc::WriteCPP(CPP_files& f)
{
	//////////////////////////////////////////////////////////////
	//запись информации об обработчике парам. спец-ции в .2ml файл

	//запись начала описания обработчика парам. спец-ции в .2ml файл
	fprintf(f.f2ml, "\t<HProc ");
	if (QualName) {
		CBaseName* BN = parent_element->GetGlobalName(QualName);
		fprintf(f.f2ml, "Qual=\"%s\" ", static_cast<CImportModule*>(BN)->real_name);
	}
	fprintf(f.f2ml, "Name=\"%s\" UID=\"%i\">\n", name, UID);
	//запись списка параметризованных обобщающих параметров в .2ml файл
	CBaseVarVector::const_iterator ci;
	for (ci = CommonPars->FPStore.begin(); ci != CommonPars->FPStore.end(); ci++) {
		const char* QualSpecName = NULL;
		const char* SpecName = NULL;
		const char* TagName = NULL;
		static_cast<CCommonVar*>(*ci)->GetTagName(QualSpecName, SpecName, TagName);
		fprintf(f.f2ml, "\t\t<SPar");
		if (TagName)
			fprintf(f.f2ml, " Tag=\"%s\"", TagName);
		else {
			if (QualSpecName) fprintf(f.f2ml, " Qual=\"%s\"", QualSpecName);
			fprintf(f.f2ml, " Name=\"%s\"", SpecName);
		}
		fprintf(f.f2ml, " />\n");
	}
	//конец описания обработчика в .2ml файл
	fprintf(f.f2ml, "\t</HProc>\n");


	///////////////////////////////////////////////////////
	//запись кода обработчика параметрической специализации

	//запись комментариев
	fprintf(f.fc, "//HANDLER PROCEDURE %s\n", name);
	fprintf(f.fh, "//HANDLER PROCEDURE %s\n", name);

	//генерация кода типа процедуры в описании и объявлении
	FormalPars->WriteCPP_type(f, false, parent_element);
	FormalPars->WriteCPP_type(f, true, parent_element);

	//генерация namespace в описании процедуры
	fprintf(f.fc, "%s::", parent_element->name);

	//генерация названия процедуры в описании и объявлении
	fprintf(f.fc, "O2M_HANDLER_%s_%i(", name, UID);
	fprintf(f.fh, "O2M_HANDLER_%s_%i(", name, UID);

	//генерация кода формальных параметров в описании и объявлении
	FormalPars->WriteCPP_pars(f, false);
	fprintf(f.fc, ") {\n");
	FormalPars->WriteCPP_pars(f, true);
	fprintf(f.fh, ");\n");

	//генерация переменной для хранения результатов процедуры-функции
	if (FormalPars->Qualident) {
		fprintf(f.fc, "\t");
		FormalPars->Qualident->WriteCPP_type(f, false, parent_element);
		fprintf(f.fc, " O2M_RESULT;\n");
	}
	//инициализация массивов-значений
	FormalPars->WriteCPP_begin(f);

	//генерация кода деклараций типов
	DeclSeq->WriteCPP_type(f);
	//генерация кода переменных
	DeclSeq->WriteCPP_var(f);

	//генерация кода для операторов
	if (StatementSeq) {
		fprintf(f.fc, "//BEGIN\n");
		StatementSeq->WriteCPP(f);
	}
	//деинициализация открытых массивов-значений
	FormalPars->WriteCPP_end(f, hr_No != have_return);
	//выдача результатов процедуры-функции
	if (FormalPars->Qualident) fprintf(f.fc, "\treturn O2M_RESULT;\n");
	//завершение описания операторов процедуры
	fprintf(f.fc, "}\n//END %s;\n", name);
}


//-----------------------------------------------------------------------------
//конструктор объекта CPtrType
CPtrType::CPtrType(const CBaseName* parent) : CBaseType(id_CPtrType, parent)
{
	//сохранение имени типа (PTR)
	SetName("PTR");
}//CPtrType


//-----------------------------------------------------------------------------
//создание копии данного типа
int CPtrType::CreateType(CBaseType* &BaseType) const
{
	BaseType = new CPtrType(parent_element);
	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//создание переменной по типу
int CPtrType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	BaseVar = new CPtrVar(parent);
	//установка названия типа в случае именованного типа
	BaseVar->SetTypeName(GetModuleName(), name);
	return 0;
}//CPtrType::CreateVar


//-----------------------------------------------------------------------------
//Запись кода CPtrType
void CPtrType::WriteCPP(CPP_files& f, const bool to_h)
{
	fprintf(to_h ? f.fh : f.fc, "/* PTR TYPE %s */", name);
	fprintf(to_h ? f.fh : f.fc, "typedef bool %s;\n", name);
}


//-----------------------------------------------------------------------------
//запись в DFN объявления типа
void CPtrType::WriteDFN(DFN_file& f)
{
	throw error_Internal("CPtrType::WriteDFN");
}//WriteDFN


//-----------------------------------------------------------------------------
//инициализация объекта CBooleanType
int CBooleanType::Init(CLexBuf *lb)
{
	return 0;
}//Init CBooleanType


//-----------------------------------------------------------------------------
//Запись кода CBooleanType
void CBooleanType::WriteCPP(CPP_files& f, const bool to_h)
{
	fprintf(to_h ? f.fh : f.fc, "typedef %s %s;\n", GetCPPTypeName(), name);
}


//-----------------------------------------------------------------------------
//запись в DFN объявления типа
void CBooleanType::WriteDFN(DFN_file& f)
{
	fprintf(f.f, "BOOLEAN");
}//WriteDFN


//-----------------------------------------------------------------------------
//создание копии данного типа
int CBooleanType::CreateType(CBaseType* &BaseType) const
{
	BaseType = new CBooleanType(parent_element);
	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//создание переменной по типу
int CBooleanType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	BaseVar = new CBooleanVar(parent);
	//установка названия типа в случае именованного типа
	BaseVar->SetTypeName(GetModuleName(), name);
	return 0;
}//CBooleanType::CreateVar


//-----------------------------------------------------------------------------
//инициализация объекта CCharType
int CCharType::Init(CLexBuf *lb)
{
	return 0;
}//Init CCharType


//-----------------------------------------------------------------------------
//Запись кода CCharType
void CCharType::WriteCPP(CPP_files& f, const bool to_h)
{
	fprintf(to_h ? f.fh : f.fc, "typedef %s %s;\n", GetCPPTypeName(), name);
}


//-----------------------------------------------------------------------------
//запись в DFN объявления типа
void CCharType::WriteDFN(DFN_file& f)
{
	fprintf(f.f, "CHAR");
}//WriteDFN


//-----------------------------------------------------------------------------
//создание копии данного типа
int CCharType::CreateType(CBaseType* &BaseType) const
{
	BaseType = new CCharType(parent_element);
	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//создание переменной по типу
int CCharType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	BaseVar = new CCharVar(parent);
	//установка названия типа в случае именованного типа
	BaseVar->SetTypeName(GetModuleName(), name);
	return 0;
}//CCharType::CreateVar


//-----------------------------------------------------------------------------
//инициализация объекта CShortintType
int CShortintType::Init(CLexBuf *lb)
{
	return 0;
}//Init CShortintType


//-----------------------------------------------------------------------------
//Запись кода CShortintType
void CShortintType::WriteCPP(CPP_files& f, const bool to_h)
{
	fprintf(to_h ? f.fh : f.fc, "typedef %s %s;\n", GetCPPTypeName(), name);
}


//-----------------------------------------------------------------------------
//запись в DFN объявления типа
void CShortintType::WriteDFN(DFN_file& f)
{
	fprintf(f.f, "SHORTINT");
}//WriteDFN


//-----------------------------------------------------------------------------
//создание копии данного типа
int CShortintType::CreateType(CBaseType* &BaseType) const
{
	BaseType = new CShortintType(parent_element);
	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//создание переменной по типу
int CShortintType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	BaseVar = new CShortintVar(parent);
	//установка названия типа в случае именованного типа
	BaseVar->SetTypeName(GetModuleName(), name);
	return 0;
}//CShortintType::CreateVar


//-----------------------------------------------------------------------------
//инициализация объекта CIntegerType
int CIntegerType::Init(CLexBuf *lb)
{
	return 0;
}//Init CIntegerType


//-----------------------------------------------------------------------------
//Запись кода CIntegerType
void CIntegerType::WriteCPP(CPP_files& f, const bool to_h)
{
	fprintf(to_h ? f.fh : f.fc, "typedef %s %s;\n", GetCPPTypeName(), name);
}


//-----------------------------------------------------------------------------
//запись в DFN объявления типа
void CIntegerType::WriteDFN(DFN_file& f)
{
	fprintf(f.f, "INTEGER");
}//WriteDFN


//-----------------------------------------------------------------------------
//создание копии данного типа
int CIntegerType::CreateType(CBaseType* &BaseType) const
{
	BaseType = new CIntegerType(parent_element);
	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//создание переменной по типу
int CIntegerType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	BaseVar = new CIntegerVar(parent);
	//установка названия типа в случае именованного типа
	BaseVar->SetTypeName(GetModuleName(), name);
	return 0;
}//CIntegerType::CreateVar


//-----------------------------------------------------------------------------
//инициализация объекта CLongintType
int CLongintType::Init(CLexBuf *lb)
{
	return 0;
}//Init CLongintType


//-----------------------------------------------------------------------------
//Запись кода CLongintType
void CLongintType::WriteCPP(CPP_files& f, const bool to_h)
{
	fprintf(to_h ? f.fh : f.fc, "typedef %s %s;\n", GetCPPTypeName(), name);
}


//-----------------------------------------------------------------------------
//запись в DFN объявления типа
void CLongintType::WriteDFN(DFN_file& f)
{
	fprintf(f.f, "LONGINT");
}//WriteDFN


//-----------------------------------------------------------------------------
//создание копии данного типа
int CLongintType::CreateType(CBaseType* &BaseType) const
{
	BaseType = new CLongintType(parent_element);
	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//создание переменной по типу
int CLongintType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	BaseVar = new CLongintVar(parent);
	//установка названия типа в случае именованного типа
	BaseVar->SetTypeName(GetModuleName(), name);
	return 0;
}//CLongintType::CreateVar


//-----------------------------------------------------------------------------
//инициализация объекта CRealType
int CRealType::Init(CLexBuf *lb)
{
	return 0;
}//Init CRealType


//-----------------------------------------------------------------------------
//Запись кода CRealType
void CRealType::WriteCPP(CPP_files& f, const bool to_h)
{
	fprintf(to_h ? f.fh : f.fc, "typedef %s %s;\n", GetCPPTypeName(), name);
}


//-----------------------------------------------------------------------------
//запись в DFN объявления типа
void CRealType::WriteDFN(DFN_file& f)
{
	fprintf(f.f, "REAL");
}//WriteDFN


//-----------------------------------------------------------------------------
//создание копии данного типа
int CRealType::CreateType(CBaseType* &BaseType) const
{
	BaseType = new CRealType(parent_element);
	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//создание переменной по типу
int CRealType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	BaseVar = new CRealVar(parent);
	//установка названия типа в случае именованного типа
	BaseVar->SetTypeName(GetModuleName(), name);
	return 0;
}//CRealType::CreateVar


//-----------------------------------------------------------------------------
//инициализация объекта CLongrealType
int CLongrealType::Init(CLexBuf *lb)
{
	return 0;
}//Init CLongrealType


//-----------------------------------------------------------------------------
//Запись кода CLongrealType
void CLongrealType::WriteCPP(CPP_files& f, const bool to_h)
{
	fprintf(to_h ? f.fh : f.fc, "typedef %s %s;\n", GetCPPTypeName(), name);
}


//-----------------------------------------------------------------------------
//запись в DFN объявления типа
void CLongrealType::WriteDFN(DFN_file& f)
{
	fprintf(f.f, "LONGREAL");
}//WriteDFN


//-----------------------------------------------------------------------------
//создание копии данного типа
int CLongrealType::CreateType(CBaseType* &BaseType) const
{
	BaseType = new CLongrealType(parent_element);
	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//создание переменной по типу
int CLongrealType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	BaseVar = new CLongrealVar(parent);
	//установка названия типа в случае именованного типа
	BaseVar->SetTypeName(GetModuleName(), name);
	return 0;
}//CLongrealType::CreateVar


//-----------------------------------------------------------------------------
//инициализация объекта CSetType
int CSetType::Init(CLexBuf *lb)
{
	return 0;
}//Init CSetType


//-----------------------------------------------------------------------------
//Запись кода CSetType
void CSetType::WriteCPP(CPP_files& f, const bool to_h)
{
	fprintf(to_h ? f.fh : f.fc, "typedef %s %s;\n", GetCPPTypeName(), name);
}


//-----------------------------------------------------------------------------
//запись в DFN объявления типа
void CSetType::WriteDFN(DFN_file& f)
{
	fprintf(f.f, "SET");
}//WriteDFN


//-----------------------------------------------------------------------------
//создание копии данного типа
int CSetType::CreateType(CBaseType* &BaseType) const
{
	BaseType = new CSetType(parent_element);
	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//создание переменной по типу
int CSetType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	BaseVar = new CSetVar(parent);
	//установка названия типа в случае именованного типа
	BaseVar->SetTypeName(GetModuleName(), name);
	return 0;
}//CSetType::CreateVar


//-----------------------------------------------------------------------------
//инициализация объекта CQualidentType
int CQualidentType::Init(CLexBuf *lb)
{
	//инициализация Qualident (имени типа)
	Qualident = new CQualident;
	return Qualident->InitTypeName(lb, parent_element);
}//Init CQualidentType


//-----------------------------------------------------------------------------
//Запись кода CQualidentType
void CQualidentType::WriteCPP(CPP_files& f, const bool to_h)
{
	if (Qualident) {
		if (Qualident->pref_ident) {
			//получение типа по его имени (в случае наличия псевдонима он может потребоваться)
			CBaseType* BT = static_cast<CBaseType*>(parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident));
			//TYPE type1 = module1.type1
			fprintf(to_h ? f.fh : f.fc, "typedef %s::%s %s;\n", to_h ? BT->GetModuleName() : BT->GetModuleAlias(), Qualident->ident, name);
		} else	//TYPE type1 = type2;
			fprintf(to_h ? f.fh : f.fc, "typedef %s %s;\n", Qualident->ident, name);
	} else	//например в типе запись
		fprintf(f.fc, "%s", name);
}//WriteCPP


//-----------------------------------------------------------------------------
//запись в DFN объявления типа
void CQualidentType::WriteDFN(DFN_file& f)
{
	if (Qualident) Qualident->WriteDFN(f);
	else fprintf(f.f, "BYTE");
}//WriteDFN


//-----------------------------------------------------------------------------
//создание копии данного типа
int CQualidentType::CreateType(CBaseType* &BaseType) const
{
	CQualidentType *QT = new CQualidentType(parent_element);
	QT->Qualident = new CQualident();
	Qualident->Assign(QT->Qualident);

	BaseType = QT;
	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//создание переменной по типу
int CQualidentType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	//поиск типа по имеющемуся имени
	CBaseType *NamedType = NULL;
	int err_num = GetNamedType(NamedType, false);
	if (err_num) return err_num;
	//вызов процедуры создания переменной указанного типа
	err_num = NamedType->CreateVar(BaseVar, parent);
	if (err_num) return err_num;
	//проверка создания указателя (для него необходимо явно указать признак QualidentType)
	if (id_CPointerVar == BaseVar->name_id)
		static_cast<CPointerVar*>(BaseVar)->qualident_type = true;
	//занесение имени типа в переменную
	/**/
	//использование NamedType->GetModuleAlias() приводит к ошибке в случае использования типа указатель,
	//импортированного из модуля, который его в свою очередь импортирует
	//BaseVar->SetTypeName(Qualident->pref_ident ? NamedType->GetModuleAlias() : NULL, Qualident->ident);
	BaseVar->SetTypeName(Qualident->pref_ident, Qualident->ident);
	return 0;
}//CQualidentType::CreateVar


//-----------------------------------------------------------------------------
//получение ук. на действительный тип (не QualidentType), имя которого содержит QualidentType (с проверкой экспорта типа)
int CQualidentType::GetNamedType(CBaseType* &NamedType, const bool check_external) const
{
	CBaseName* BN = parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident);
	//проверка экспорта найденного типа (или его нахождение в DFN модуле)
	if (check_external && BN && !BN->external && (BN->parent_element->name_id != id_CDfnModule)) return s_e_TypeNotExternal;

	while (BN && (BN->name_id == id_CQualidentType)) {
		BN = BN->parent_element->GetGlobalName(static_cast<CQualidentType*>(BN)->Qualident->pref_ident, static_cast<CQualidentType*>(BN)->Qualident->ident);
		//проверка экспорта найденного типа (или его нахождение в DFN модуле)
		if (check_external && BN && !BN->external && (BN->parent_element->name_id != id_CDfnModule)) return s_e_TypeNotExternal;
	}

	//тип должен быть найден
	if (!BN || !CBaseType::IsTypeId(BN->name_id)) return s_m_Error;

	NamedType = static_cast<CBaseType*>(BN);
	return 0;
}


//-----------------------------------------------------------------------------
//получение результата выражения для типа, имя которого содержит QualidentType
EName_id CQualidentType::GetResultId() const
{
	CBaseType* BT;
	int err_num = GetNamedType(BT, false);
	if (err_num) return id_CBaseName;
	return BT->GetResultId();
}


//-----------------------------------------------------------------------------
//инициализация объекта CArrayType
int CArrayType::Init(CLexBuf *lb)
{
	DECL_SAVE_POS
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	CExpr* CountExpr = new CExpr(parent_element);
	int err_num = CountExpr->Init(lb);

	if (err_num) {
		//массив не имеет размера, восстанавливаем позицию в файле
		RESTORE_POS
		//уничтожаем созданное выражение
		delete CountExpr;
		//проверяем наличие "OF"
		if (!lb->ReadLex(li) || lex_k_OF != li.lex) return s_e_OF;
		//проверка наличия и создание типа
		return TypeSelector(lb, Type, parent_element);
	} else {
		//массив имеет размер, определяем его
		CBaseVar* BV = NULL;
		err_num = CountExpr->CreateConst(BV);
		delete CountExpr;
		if (err_num || !CBaseVar::IsIntId(BV->name_id)) {
			delete BV;
			return s_e_ExprNotPosIntConst;	//неверный тип выражения в размере массива
		}
		//получение и проверка допустимости размера массива
		size = BV->GetIntValue();
		delete BV;
		if (size <= 0) return s_e_ExprNotPosIntConst;
	}//else

	//массив имеет размер, читаем след. лексему ("," или "OF")
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_OF;

	//проверка наличия ","
	if (lex_k_comma == li.lex) {
		//имеется следующее измерение, интерпретируем его как тип эл-тов массива, являющийся массивом
		Type = new CArrayType(parent_element);
		err_num = static_cast<CArrayType*>(Type)->Init(lb);
		if (err_num) return err_num;
		//проверяем, является ли массив открытым (это недопустимо)
		if (0 == static_cast<CArrayType*>(Type)->size) return s_e_OpenArrayNotAllowed;
		return 0;
	}

	//проверка наличия "OF"
	if (lex_k_OF == li.lex) {
		//конец списка размерностей массива, создание типа эл-тов массива
		err_num = TypeSelector(lb, Type, parent_element);
		if (err_num) return err_num;
		//в случае, если типом эл-тов массива является массив, проверяем, является ли он открытым (это недопустимо)
		if (id_CArrayType == Type->name_id && 0 == static_cast<CArrayType*>(Type)->size) return s_e_OpenArrayNotAllowed;
		return 0;
	}

	return s_e_OF;
}//Init CArrayType


//-----------------------------------------------------------------------------
//запись кода объявления CArrayType
void CArrayType::WriteCPP(CPP_files& f, const bool to_h)
{
	//тип массив похож на тип ук. на запись - реально генерируется только код типа массива,
	//размерность (размерности) подставляются только при объявлении переменной, иначе не удастся
	//сгенерировать код для NEW()

	//установка имени (для создания RuntimeId и генерации кода) - опасная операция, т.к. InitRuntimeId или WriteCPP могут сгенерировать исключение
	Type->name = name;
	try {
		//для типа RECORD генерируем RuntimeId
		if (id_CRecordType == Type->name_id) static_cast<CRecordType*>(Type)->InitRuntimeId();
		Type->WriteCPP(f, to_h);
	}
	catch(error_Internal) {
		Type->name = NULL;
		throw;
	};

	//убираем установленное имя
	Type->name = NULL;

}//WriteCPP


//-----------------------------------------------------------------------------
//запись в DFN объявления типа
void CArrayType::WriteDFN(DFN_file &f)
{
	fprintf(f.f, "ARRAY");

	//вывод размеров по каждому измерению (если есть размер)
	CArrayType *AT = this;
	while (AT->name_id == id_CArrayType) {
		if (AT->size != 0) fprintf(f.f, "%s%i", AT == this ? " " : ",", AT->size);
		AT = static_cast<CArrayType*>(AT->Type);
	}

	//запись "OF" <Объявление типа эл-тов массива>
	fprintf(f.f, " OF ");
	AT->WriteDFN(f);

}


//-----------------------------------------------------------------------------
//проверка завершенности типа эл-тов массива
int CArrayType::CheckComplete(CLexBuf *lb) const
{
	//поиск типа эл-тов массива
	CBaseType *BT = FindLastType();
	switch (BT->name_id) {
	case id_CPointerType:
		return static_cast<CPointerType*>(BT)->CheckComplete(lb);
	case id_CRecordType:
		return static_cast<CRecordType*>(BT)->CheckComplete(lb);
	default:
		return 0;
	}//switch
}


//-----------------------------------------------------------------------------
//создание копии данного типа
int CArrayType::CreateType(CBaseType* &BaseType) const
{
	CArrayType* AT = new CArrayType(parent_element);
	//копирование размера
	AT->size = size;
	//копирование имени (если есть)
	if (name) AT->SetName(name);
	//копирование типа эл-тов массива
	int err_num = Type->CreateType( AT->Type );
	//возвращаем созданную копию типа
	BaseType = AT;
	return err_num;
}//CreateType


//-----------------------------------------------------------------------------
//создание переменной по типу
int CArrayType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	//создание переменной
	CArrayVar* AV = new CArrayVar(parent);
	CBaseType* BT;
	//копирование в переменную текущего типа
	CreateType(BT);
	AV->ArrayType = static_cast<CArrayType*>(BT);
	//возврат созданной переменной
	BaseVar = AV;
	//установка названия типа в случае именованного типа
	BaseVar->SetTypeName(GetModuleName(), name);

	return 0;
}//CArrayType::CreateVar


//-----------------------------------------------------------------------------
//деструктор объекта CTypeRecord
CRecordType::~CRecordType()
{
	delete Qualident;
	delete[] RuntimeId;
	//очистка списка полей записи (за исключением связанных с типом процедур)
	CBaseNameVector::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci)
		if (!CProcedure::IsProcId((*ci)->name_id)) delete *ci;
}//~CRecordType


//-----------------------------------------------------------------------------
//инициализация объекта CRecordType
int CRecordType::Init(CLexBuf *lb)
{
	DECL_SAVE_POS
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;
	//переменная для получения номера ошибки
	int err_num;

	//проверка наличия Qualident - проверка наличия "("
	if (lb->ReadLex(li) && lex_k_op_bracket == li.lex) {
		Qualident = new CQualident;
		err_num = Qualident->Init(lb, parent_element);
		if (err_num) return err_num;
		//проверка наличия базового типа записи, соответствующего Qualident
		CBaseType* BaseRec = static_cast<CBaseType*>( parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident) );
		if (!BaseRec) return s_e_UndeclaredIdent;
		//обработка типа QualidentType (получение типа, на кот. он ссылается)
		if (BaseRec->name_id == id_CQualidentType) err_num = static_cast<CQualidentType*>(BaseRec)->GetNamedType(BaseRec, external);
		else err_num = 0;
		if (err_num) return err_num;
		if (!BaseRec) return s_e_UndeclaredIdent;
		//проверка допустимости базового типа (должен быть тип запись)
		if (BaseRec->name_id != id_CRecordType) return s_e_IdentNotRecordType;
		//проверка наличия кл. слова ")"
		if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;
	}
	else {
		RESTORE_POS
	}

	//проверка, создан ли данный тип в локальной области видимости (среди parent_element есть только CModule, CDfnModule или CRecordType)
	const CBaseName* BN = parent_element;
	while (BN->parent_element && BN->name_id == id_CRecordType) BN = BN->parent_element;
	bool is_local = (BN->name_id != id_CModule) && (BN->name_id != id_CDfnModule);

	//инициализация списков полей (если есть)
	while (true) {
		CIdentList IdentList(this, is_local);	//создание списка имен переменных
		SAVE_POS
		err_num = IdentList.AssignVars(lb);
		if (err_num == s_m_IdentDefAbsent) {
			RESTORE_POS
		} else
			if (err_num) return err_num;

		//проверка наличия кл. слова ";"
		SAVE_POS
		if (!lb->ReadLex(li) || lex_k_semicolon != li.lex) {
			RESTORE_POS
			break;
		}
	}

	//проверка наличия кл. слова "END"
	if (!lb->ReadLex(li) || lex_k_END != li.lex) return s_e_END;

	return 0;
}//Init CRecordType


//-----------------------------------------------------------------------------
//инициализация строкового ид. типа времени исполнения
int CRecordType::InitRuntimeId()
{
	//проверка отсутствия созданного RuntimeId
	if (RuntimeId) return s_m_Error;

	//сохранение в RuntimeId название типа + название модуля
	if (name) {
		//получение ук. на модуль (или Dfn-модуль)
		const CBaseName* top_parent = parent_element;
		while ((top_parent->parent_element) && (top_parent->name_id != id_CDfnModule)) top_parent = top_parent->parent_element;
		//создание строки RuntimeId (вначале идет имя типа для ускорения операций сравнения строк)
		RuntimeId = new char[strlen(top_parent->name) + strlen(name) + 2];
		strcpy(RuntimeId, name);
		strcat(RuntimeId, "@");
		strcat(RuntimeId, top_parent->name);
	} else {
		//установка RuntimeId для неименованных типов
		RuntimeId = new char[16];
		strcpy(RuntimeId, "O2M_SYS_UNNAMED");
	}

	return 0;
}//InitRuntimeId


//-----------------------------------------------------------------------------
//Запись кода CRecordType
void CRecordType::WriteCPP(CPP_files& f, const bool to_h)
{
	//////////////////
	//запись в .h файл
	if (to_h) {
		fprintf(f.fh, "struct %s", name);
		//запись базового типа (если есть)
		if (Qualident) {
			fprintf(f.fh, " : ");
			CBaseName* BN = parent_element->GetGlobalName(Qualident->pref_ident);
			if (BN) fprintf(f.fh, "%s::", static_cast<CImportModule*>(BN)->real_name);
			fprintf(f.fh, "%s", Qualident->ident);
		}
		//начало описания кода полей записи
		fprintf(f.fh, " {\n");
		//запись кода метода для получения ид. типа времени исполнения во время исполнения
		fprintf(f.fh, "\t%sconst char* O2M_SYS_ID() {return \"%s\";};\n", Qualident ? "" : "virtual ", RuntimeId);
		//запись кода полей записи
		CBaseNameVector::const_iterator ci;
		for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci) {
			fprintf(f.fh, "\t");
			if (id_CArrayVar == (*ci)->name_id)	//запись кода массива
				static_cast<CArrayVar*>(*ci)->WriteCPP_rec(f, true);
			else {
				//проверка наличия связанной с типом процедуры
				if (CProcedure::IsProcId((*ci)->name_id))
					static_cast<CProcedure*>(*ci)->WriteCPP_RECORD(f, this, to_h);
				else
					static_cast<CBaseVar*>(*ci)->WriteCPP_fp(f, true);
			}
			fprintf(f.fh, ";\n");
		}//for
		//завершение описания кода записи
		fprintf(f.fh, "};\n");
		return;
	}//if

	////////////////////
	//запись в .cpp файл
	fprintf(f.fc, "struct %s", name);
	//запись базового типа (если есть)
	if (Qualident) {
		fprintf(f.fc, " : ");
		Qualident->WriteCPP_type(f, false, parent_element);
	}
	//начало описания кода полей записи
	fprintf(f.fc, " {\n");
	//запись кода метода для получения ид. типа времени исполнения во время исполнения
	fprintf(f.fc, "\t%sconst char* O2M_SYS_ID() {return \"%s\";};\n", Qualident ? "" : "virtual ", RuntimeId);
	//запись кода полей записи
	CBaseNameVector::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci) {
		fprintf(f.fc, "\t");
		if ((*ci)->name_id == id_CArrayVar)	//запись кода массива
			static_cast<CArrayVar*>(*ci)->WriteCPP_rec(f, false);
		else {
			//проверка наличия связанной с типом процедуры
			if (CProcedure::IsProcId((*ci)->name_id))
				static_cast<CProcedure*>(*ci)->WriteCPP_RECORD(f, this, false);
			else
				static_cast<CBaseVar*>(*ci)->WriteCPP_fp(f, false);
		}
		fprintf(f.fc, ";\n");
	}//for
	//завершение описания кода записи
	fprintf(f.fc, "};\n");

}//WriteCPP


//-----------------------------------------------------------------------------
//запись в DFN объявления типа
void CRecordType::WriteDFN(DFN_file &f)
{
	fprintf(f.f, "RECORD");
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
	CBaseNameVector::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci)
		if ((*ci)->external) {
			//проверка отсутствия связанной с типом процедуры
			if (!CProcedure::IsProcId((*ci)->name_id)) {
				(*ci)->WriteDFN(f);
				fprintf(f.f, ";\n");
				f.tab();
			}//if
		}//if

	//конец генерации кода записи
	fprintf(f.f, "END");

	//уменьшение отступа в DFN файле
	f.tab_level--;
}


//-----------------------------------------------------------------------------
//проверка, является ли данный тип расширением типа с указанным именем,
//возврат - ук. на базовый тип с указанным именем
CRecordType* CRecordType::IsExtension(const char *module_name, const char *type_name)
{
	//проверка, являются ли данный тип и тип с указанным именем равными типами
	if (IsSame(module_name, type_name)) return this;
	//проверка наличия непосредственного базового типа (direct extension)
	if (!Qualident) return NULL;
	//получение базового типа (необязательно непосредственного)
	CBaseType* BT = static_cast<CBaseType*>( parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident) );
	if (!BT) return NULL;
	int err_num = 0;
	if (id_CQualidentType == BT->name_id)
		err_num = static_cast<CQualidentType*>(BT)->GetNamedType(BT, external);
	if (err_num || id_CRecordType != BT->name_id) return NULL;
	//проверка, является ли непосредственный базовый тип расширением типа с указанным именем
	return static_cast<CRecordType*>(BT)->IsExtension(module_name, type_name);
}


//-----------------------------------------------------------------------------
//поиск имени в списке полей, NULL - если имя не найдено
CBaseName* CRecordType::FindName(const char* search_name) const
{
	//поиск имени в собственном списке полей типа
	CBaseNameVector::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci)
		if (!strcmp( (*ci)->name, search_name ))
			return *ci;

	//проверка наличия базового типа
	if (Qualident) {
		//получение ук. на объект базового типа
		CBaseName* BN = parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident);
		return static_cast<CRecordType*>(BN)->FindName(search_name);
	}

	//имя не найдено
	return NULL;
}//FindName


//-----------------------------------------------------------------------------
//добавление указанного эл-та в таблицу имен
void CRecordType::AddName(CBaseName* BN) const
{
	FieldStore.push_back(BN);
}//AddName


//-----------------------------------------------------------------------------
//добавление ук. на процедуру в локальную таблицу имен с заменой ForwardDecl
//и проверкой возможных ошибок (не ForwardDecl, несовпадение параметров с
//переопределяемой процедурой)
int CRecordType::AddProcReference(CProcedure* P) const
{
	/**/
	//вставить проверку совпадения параметров с переопределяемой процедурой

	//поиск объестов с именем, совпадающим с переданным объектом
	CBaseNameVector::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci)
		if (!strcmp(P->name, (*ci)->name)) {
			//проверка наличия опережающего описания (тогда добавлять объект не требуется)
			if (id_CForwardDecl == (*ci)->name_id) return 0;
			//обнаружен объект недопустимого типа
			return s_e_Redefinition;
		}
	//опережающее описание не обнаружено - обычное добавление в таблицу имен
	FieldStore.push_back(P);
	return 0;
}


//-----------------------------------------------------------------------------
//проверка завершенности всех типов указатель в составе типа запись
int CRecordType::CheckComplete(CLexBuf *lb) const
{
	//проверка рекурсивного вызова (если среди полей есть рекурсивный указатель)
	if (in_checking_complete)
		return 0;
	else
		in_checking_complete = true;
	//цикл перебора полей записи и вызова CheckComplete для каждой переменной
	CBaseNameVector::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci)
		if (CBaseVar::IsVarId((*ci)->name_id)) {
			int err_num = static_cast<CBaseVar*>(*ci)->CheckComplete(lb);
			if (err_num) return err_num;
		}
	return 0;
}


//-----------------------------------------------------------------------------
//создание копии данного типа
int CRecordType::CreateType(CBaseType* &BaseType) const
{
	BaseType = new CRecordType(parent_element);
	CRecordType* RT = static_cast<CRecordType*>(BaseType);
	//копирование списка полей (если есть)
	CBaseNameVector::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci) {
		//проверка наличия связанной с типом процедуры (не копируется в тип)
		if (CProcedure::IsProcId((*ci)->name_id)) continue;
		//копирование переменной
		CBaseVar* TempVar = static_cast<CBaseVar*>(*ci);
		TempVar = TempVar->CreateVar(RT);
		RT->AddName(TempVar);
	}
	//копирование названия базового типа (если есть)
	if (Qualident) Qualident->CreateCopy(RT->Qualident);
	//копирование RuntimeId (если есть)
	if (RuntimeId) RT->RuntimeId = str_new_copy(RuntimeId);
	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//создание переменной по типу
int CRecordType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	BaseVar = new CRecordVar(parent);
	//установка названия типа в случае именованного типа
	BaseVar->SetTypeName(GetModuleName(), name);
	//копирование списка переменных (если есть)
	CBaseNameVector::const_iterator ci;
	for (ci = FieldStore.begin(); ci != FieldStore.end(); ++ci) {
		//проверка наличия связанной с типом процедуры (не копируется в переменную)
		if (CProcedure::IsProcId((*ci)->name_id)) continue;
		//копирование переменной
		CBaseVar* TempVar = static_cast<CBaseVar*>(*ci);
		TempVar = TempVar->CreateVar(BaseVar);
		BaseVar->AddName(TempVar);
	}
	//копирование названия базового типа (если есть)
	if (Qualident)
		Qualident->CreateCopy(static_cast<CRecordVar*>(BaseVar)->Qualident);

	return 0;
}//CRecordType::CreateVar


//-----------------------------------------------------------------------------
//добавление эл-та обобщения в список
void CCommonType::AddSpec(const char *newTag, const char *newQual, const char *newName, const bool newIsExtended)
{
	SpecStore.push_back(NewSpec(newTag, newQual, newName, newIsExtended));
}


//-----------------------------------------------------------------------------
//проверка допустимости типа обобщения
int CCommonType::CheckSpecType(const char *pref_ident, const char *ident, const CBaseName* parent) const
{
	//проверка наличия объявленного типа специализации
	CBaseName* BN = parent->GetGlobalName(pref_ident, ident);
	if (!BN) return s_e_UndeclaredIdent;
	//проверка допустимости типа специализации
	switch (BN->name_id) {
	case id_CRecordType:
	case id_CArrayType:
	case id_CCommonType:
		return 0;
	default:
		return s_e_CommonTypeSpecType;
	}//switch
}


//-----------------------------------------------------------------------------
//очистка списка признаков
void CCommonType::ClearTmpTagStore()
{
	TmpTagStore_type::const_iterator ci_t;
	for (ci_t = TmpTagStore.begin(); ci_t != TmpTagStore.end(); ci_t++) delete[] *ci_t;
	TmpTagStore.clear();
}


//-----------------------------------------------------------------------------
//уничтожение эл-та обобщения
void CCommonType::DelSpec(CCommonType::SSpec *sp)
{
	delete sp->Tag;
	delete sp->QualName;
	delete sp->Name;
	delete sp;
}


//-----------------------------------------------------------------------------
//поиск спициализации по признаку (с учетом типа признака)
const CCommonType::SSpec* CCommonType::FindSpec(const char *QualName, const char *Name, const char *Tag)
{
	if (tt_Type == TagType)
		return FindSpecByName(QualName, Name);
	else
		return FindSpecByTag(Tag);
}


//-----------------------------------------------------------------------------
//поиск специализации по признаку
const CCommonType::SSpec* CCommonType::FindSpecByTag(const char *Tag) const
{
	//поиск в списке специализаций
	SpecStore_type::const_iterator ci;
	for (ci = SpecStore.begin(); ci != SpecStore.end(); ci++)
		if (!strcmp(Tag, (*ci)->Tag)) return *ci;
	//проверка специализации по умолчанию (если есть)
	if (DefaultSpec && !strcmp(Tag, DefaultSpec->Tag)) return DefaultSpec;
	//специализация не найдена
	return NULL;
}


//-----------------------------------------------------------------------------
//поиск специализации по имени типа
const CCommonType::SSpec* CCommonType::FindSpecByName(const char *QualName, const char *Name) const
{
	//в случае родительского эл-та - DFN модуля, проверка необходимости отбрасывать имя модуля
	if (QualName && id_CDfnModule == parent_element->name_id)
		if (static_cast<const CDfnModule*>(parent_element)->alias_name && !strcmp(QualName, static_cast<const CDfnModule*>(parent_element)->alias_name))
			QualName = NULL;
	//поиск в списке специализаций
	SpecStore_type::const_iterator ci;
	for (ci = SpecStore.begin(); ci != SpecStore.end(); ci++) {
		bool IsEqual = (QualName && (*ci)->QualName) || !(QualName || (*ci)->QualName);
		if (IsEqual && QualName) IsEqual = !strcmp(QualName, (*ci)->QualName);
		if (IsEqual) IsEqual = (Name && (*ci)->Name) || !(Name || (*ci)->Name);
		if (IsEqual && Name) IsEqual = !strcmp(Name, (*ci)->Name);
		if (IsEqual) return *ci;
	}//for
	//проверка специализации по умолчанию (если есть)
	if (DefaultSpec) {
		bool IsEqual = (QualName && DefaultSpec->QualName) || !(QualName || DefaultSpec->QualName);
		if (IsEqual && QualName) IsEqual = !strcmp(QualName, DefaultSpec->QualName);
		if (IsEqual) IsEqual = (Name && DefaultSpec->Name) || !(Name || DefaultSpec->Name);
		if (IsEqual && Name) IsEqual = !strcmp(Name, DefaultSpec->Name);
		if (IsEqual) return DefaultSpec;
	}//if
	//специализация не найдена
	return NULL;
}


//-----------------------------------------------------------------------------
//создание нового эл-та обобщения
CCommonType::SSpec* CCommonType::NewSpec(const char *newTag, const char *newQual, const char *newName, const bool newIsExtended)
{
	//создание эл-та обобщения
	SSpec* sp = new SSpec;
	//признак (Tag)
	if (newTag)
		sp->Tag = str_new_copy(newTag);
	else
		sp->Tag = NULL;
	//уточнение (для импортированного имени)
	if (newQual)
		sp->QualName = str_new_copy(newQual);
	else
		sp->QualName = NULL;
	//имя типа (записи)
	if (newName)
		sp->Name = str_new_copy(newName);
	else
		sp->Name = NULL;
	//признак добавления через обобщение
	sp->IsExtended = newIsExtended;
	//возврат созданного и инициализированного эл-та обобщения
	return sp;
}


//-----------------------------------------------------------------------------
//деструктор CCommonType
CCommonType::~CCommonType()
{
	//очистка списка признаков
	ClearTmpTagStore();
	//очистка списка эл-тов обобщения
	SpecStore_type::const_iterator ci_s;
	for (ci_s = SpecStore.begin(); ci_s != SpecStore.end(); ci_s++) DelSpec(*ci_s);
	//уничтожение ELSE - специализации (если есть)
	if (DefaultSpec) DelSpec(DefaultSpec);
}


//-----------------------------------------------------------------------------
//создание копии данного типа
int CCommonType::CreateType(CBaseType *&BaseType) const
{
	BaseType = new CCommonType(parent_element);
	/**/
	return 0;
}


//-----------------------------------------------------------------------------
//создание переменной по типу
int CCommonType::CreateVar(CBaseVar *&BaseVar, const CBaseName *parent) const
{
	BaseVar = new CCommonVar(parent);
	//установка названия типа в случае именованного типа
	BaseVar->SetTypeName(GetModuleName(), name);
	return 0;
}


//-----------------------------------------------------------------------------
//инициализация объекта CCommonType
int CCommonType::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;
	//переменная для получения номера ошибки
	int err_num;

	//проверка наличия типа признаков, признака локальности или OF
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_OF;
	switch (li.lex) {
	case lex_k_TYPE:
		TagType = tt_Type;
		break;
	case lex_k_LOCAL:
		IsLocal = true;
		break;
	case lex_k_OF:
		goto OF_received;
	default:
		return s_e_OF;
	}//switch

	//проверка наличия признака локальности или OF
	if (!lb->ReadLex(li) || lex_k_dot > li.lex) return s_e_OF;
	switch (li.lex) {
	case lex_k_LOCAL:
		IsLocal = true;
		break;
	case lex_k_OF:
		goto OF_received;
	default:
		return s_e_OF;
	}//switch

	//проверка наличия OF
	if (!lb->ReadLex(li) || lex_k_OF != li.lex) return s_e_OF;

OF_received:

	DECL_SAVE_POS
	//признак не первой специализации в обобщении
	bool is_not_first = false;
	//для получения Qualident
	CQualident qual;

	//инициализация списка специализаций (эл-тов обобщения)
	while (true) {
		//получение первой лексемы специализации или |, ELSE, END
		SAVE_POS
		if (!lb->ReadLex(li)) return s_e_END;
		//проверка наличия | (если надо), если есть - получение след. лексемы
		if (is_not_first) {
			if (lex_k_vertical == li.lex) {
				SAVE_POS
				if (!lb->ReadLex(li)) return s_e_IdentExpected;
			}
		} else
			is_not_first = true;
		//проверка идентификатора (признак или специализация), ELSE, END
		switch (li.lex) {
		case lex_i:
			break;
		case lex_k_ELSE:
			goto ELSE_received;
		case lex_k_END:
			goto END_received;
		default:
			return s_e_IdentExpected;
		}//switch
		//проверка необходимости получения списка признаков
		if (tt_Default == TagType) {

			//очистка списка признаков от предыдущих значений
			ClearTmpTagStore();

			//цикл загрузки списка признаков
			while (true) {
				//проверка отсутствия специализации с данным признаком
				if (FindSpecByTag(li.st)) return s_e_SpecRedefining;
				//запоминание очередного признака
				TmpTagStore.push_back(str_new_copy(li.st));
				//получение след. лексемы ("," или ":")
				if (!lb->ReadLex(li)) return s_e_ColonMissing;
				if (lex_k_colon == li.lex) break;
				if (lex_k_comma != li.lex) return s_e_ColonMissing;
				//получение след. идентификатора
				if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
			}

			//получение след. лексемы (ident или NIL)
			SAVE_POS
			if (!lb->ReadLex(li)) return s_e_IdentExpected;

			//в случае NIL завершение обработки текущего блока специализаций
			if (lex_k_NIL == li.lex) {
				TmpTagStore_type::const_iterator ci;
				for (ci = TmpTagStore.begin(); ci != TmpTagStore.end(); ci++) AddSpec(*ci, NULL, NULL, false);
				continue;
			} else {
				RESTORE_POS
			}

			//получение имени типа
			qual.Clear();
			err_num = qual.Init(lb, parent_element);
			if (err_num) return err_num;
			//проверка типа специализации
			err_num = CheckSpecType(qual.pref_ident, qual.ident, parent_element);
			if (err_num) return err_num;

			//завершение обработки текущего блока специализаций
			TmpTagStore_type::const_iterator ci;
			for (ci = TmpTagStore.begin(); ci != TmpTagStore.end(); ci++) AddSpec(*ci, qual.pref_ident, qual.ident, false);
			continue;

		}//if

		//получение имени типа
		RESTORE_POS
		qual.Clear();
		err_num = qual.Init(lb, parent_element);
		if (err_num) return err_num;
		//проверка отсутствия специализации с данным именем
		if (FindSpecByName(qual.pref_ident, qual.ident)) return s_e_SpecRedefining;
		//проверка типа специализации
		err_num = CheckSpecType(qual.pref_ident, qual.ident, parent_element);
		if (err_num) return err_num;

		//завершение обработки текущего блока специализаций
		AddSpec(NULL, qual.pref_ident, qual.ident, false);

	}//while

ELSE_received:
	//получение идентификатора (признак или специализация)
	qual.Clear();
	err_num = qual.Init(lb, parent_element);
	if (err_num) return err_num;
	//проверка типа признаков
	if (tt_Type == TagType) {
		//проверка отсутствия специализации с данным именем
		if (FindSpecByName(qual.pref_ident, qual.ident)) return s_e_SpecRedefining;
		//проверка типа специализации
		err_num = CheckSpecType(qual.pref_ident, qual.ident, parent_element);
		if (err_num) return err_num;
		//создание описания специализации по умолчанию
		DefaultSpec = NewSpec(NULL, qual.pref_ident, qual.ident, false);
	} else {
		//проверка отсутствия уточненного имени
		if (qual.pref_ident) return s_e_IdentExpected;
		//проверка отсутствия специализации с данным признаком
		if (FindSpecByTag(qual.ident)) return s_e_SpecRedefining;
		//проверка наличия ":"
		if (!lb->ReadLex(li) || lex_k_colon != li.lex) return s_e_ColonMissing;
		SAVE_POS
		//получение след. лексемы
		if (!lb->ReadLex(li)) return s_e_IdentExpected;
		if (lex_k_NIL == li.lex) {
			//создание описания специализации по умолчанию
			DefaultSpec = NewSpec(qual.ident, NULL, NULL, false);
		} else {
			RESTORE_POS
			//сохранение идентификатора
			char* ident = str_new_copy(qual.ident);
			qual.Clear();
			err_num = qual.Init(lb, parent_element);
			if (err_num) {
				delete ident;
				return err_num;
			}
			//создание описания специализации по умолчанию
			DefaultSpec = NewSpec(ident, qual.pref_ident, qual.ident, false);
			//уничтожение сохраненного идентификатора
			delete[] ident;
		}
	}

	//проверка наличия END
	if (!lb->ReadLex(li) || lex_k_END != li.lex) return s_e_END;

END_received:

	return 0;
}


//-----------------------------------------------------------------------------
//инициализация добавления в обобщение новых специализаций
int CCommonType::InitExtension(CLexBuf *lb, CDeclSeq* DS)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;
	//переменная для получения номера ошибки
	int err_num;

	//признак не первой специализации в обобщении
	bool is_not_first = false;
	//для получения Qualident
	CQualident qual;

	//объект для хранения информации о расширении импортированного обобщения (или NULL)
	CCommonExtensionType* CET = NULL;
	if (GetModuleAlias()) {
		CET = new CCommonExtensionType(parent_element);
		CET->TypeName = str_new_copy(name);
		CET->TypeModuleName = str_new_copy(GetModuleName());
		DS->AddName(CET);
	}

	//инициализация списка специализаций (эл-тов обобщения)
	while (true) {
		//получение первой лексемы специализации или |, ";"
		DECL_SAVE_POS
		if (!lb->ReadLex(li)) return s_e_SemicolonMissing;
		//проверка наличия | (если надо), если есть - получение след. лексемы
		if (is_not_first) {
			if (lex_k_vertical == li.lex) {
				SAVE_POS
				if (!lb->ReadLex(li)) return s_e_IdentExpected;
			}
		} else
			is_not_first = true;
		//проверка идентификатора (признак или специализация), ";"
		switch (li.lex) {
		case lex_i:
			break;
		case lex_k_semicolon:
			RESTORE_POS
			goto semicolon_received;
		default:
			return s_e_IdentExpected;
		}//switch
		//проверка необходимости получения списка признаков
		if (tt_Default == TagType) {

			//очистка списка признаков от предыдущих значений
			ClearTmpTagStore();

			//цикл загрузки списка признаков
			while (true) {
				//проверка отсутствия специализации с данным признаком
				if (FindSpecByTag(li.st)) return s_e_SpecRedefining;
				//запоминание очередного признака
				TmpTagStore.push_back(str_new_copy(li.st));
				//получение след. лексемы ("," или ":")
				if (!lb->ReadLex(li)) return s_e_ColonMissing;
				if (lex_k_colon == li.lex) break;
				if (lex_k_comma != li.lex) return s_e_ColonMissing;
				//получение след. идентификатора
				if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
			}

			//получение след. лексемы (ident или NIL)
			SAVE_POS
			if (!lb->ReadLex(li)) return s_e_IdentExpected;

			//в случае NIL завершение обработки текущего блока специализаций
			if (lex_k_NIL == li.lex) {
				TmpTagStore_type::const_iterator ci;
				for (ci = TmpTagStore.begin(); ci != TmpTagStore.end(); ci++) {
					AddSpec(*ci, NULL, NULL, true);
					if (CET) CET->AddSpec(*ci, NULL, NULL);
				}
				continue;
			} else {
				RESTORE_POS
			}

			//получение имени типа
			qual.Clear();
			err_num = qual.Init(lb, parent_element);
			if (err_num) return err_num;
			//проверка типа специализации
			err_num = CheckSpecType(qual.pref_ident, qual.ident, DS->parent_element);
			if (err_num) return err_num;

			//завершение обработки текущего блока специализаций
			TmpTagStore_type::const_iterator ci;
			for (ci = TmpTagStore.begin(); ci != TmpTagStore.end(); ci++) {
				AddSpec(*ci, qual.pref_ident, qual.ident, true);
				if (CET) CET->AddSpec(*ci, qual.pref_ident, qual.ident);
			}
			continue;

		}//if

		//получение имени типа
		RESTORE_POS
		qual.Clear();
		err_num = qual.Init(lb, parent_element);
		if (err_num) return err_num;
		//проверка отсутствия специализации с данным именем
		if (FindSpecByName(qual.pref_ident, qual.ident)) return s_e_SpecRedefining;
		//проверка типа специализации
		err_num = CheckSpecType(qual.pref_ident, qual.ident, DS->parent_element);
		if (err_num) return err_num;

		//завершение обработки текущего блока специализаций
		AddSpec(NULL, qual.pref_ident, qual.ident, true);
		if (CET) CET->AddSpec(NULL, qual.pref_ident, qual.ident);

	}//while

semicolon_received:

	//конец описания расширения обобщения
	return 0;
}


//-----------------------------------------------------------------------------
//генерация описания обобщения в .2ml файл
void CCommonType::WriteCPP(CPP_files& f, const bool to_h)
{
	//запись начала описания обобщения
	fprintf(f.f2ml, "\t<Case Name=\"%s\"%s%s>\n", name, (tt_Type == TagType) ? " TagType=\"type\"" : "", IsLocal ? " Local=\"yes\"" : "");
	//генерация специализаций обобщения (эл-тов обобщения)
	SpecStore_type::const_iterator ci;
	for (ci = SpecStore.begin(); ci != SpecStore.end(); ci++)
		WriteCPP_SpecInfo(f.f2ml, false, parent_element, *ci);
	//генерация специализации по умолчанию
	if (DefaultSpec)
		WriteCPP_SpecInfo(f.f2ml, true, parent_element, DefaultSpec);
	//конец описания обобщения
	fprintf(f.f2ml, "\t</Case>\n");
}


//-----------------------------------------------------------------------------
//занесение в .2ml файл информации об одном эл-те обобщения
void CCommonType::WriteCPP_SpecInfo(TFileType* f2ml, const bool IsDef, const CBaseName* parent_element, const CCommonType::SSpec *sp)
{
	fprintf(f2ml, "\t\t<Spec");
	if (IsDef) fprintf(f2ml, " Def=\"yes\"");
	if (sp->Tag) fprintf(f2ml, " Tag=\"%s\"", sp->Tag);
	if (sp->QualName) {
		//поиск имени модуля по псевдониму
		CBaseName* BN = parent_element->GetGlobalName(sp->QualName);
		const char* st = sp->QualName;
		if (BN && id_CImportModule == BN->name_id) st = static_cast<CImportModule*>(BN)->real_name;
		//запись настоящего названия модуля
		fprintf(f2ml, " Qual=\"%s\"", st);
	}
	if (sp->Name) fprintf(f2ml, " Name=\"%s\"", sp->Name);
	fprintf(f2ml, " />\n");
}


//-----------------------------------------------------------------------------
//запись в DFN объявления типа
void CCommonType::WriteDFN(DFN_file& f)
{
	//описание объявления нового обобщения (расширение обобщения не может
	//экспортироваться => WriteDFN вызывается только для объявления обобщения)
	fprintf(f.f, "CASE ");
	//генерация типа признака
	if (tt_Type == TagType) fprintf(f.f, "TYPE ");
	if (IsLocal) fprintf(f.f, "LOCAL ");
	fprintf(f.f, "OF\n\t\t");

	//генерация эл-тов обобщения
	bool is_not_first = false;
	SpecStore_type::const_iterator ci;
	for (ci = SpecStore.begin(); ci != SpecStore.end(); ci++) {
		//проверка необходимости записи разделителя
		if (is_not_first) fprintf(f.f, "\n\t\t| "); else is_not_first = true;
		//проверка необходимости записи признака
		if ((*ci)->Tag) fprintf(f.f, "%s : ", (*ci)->Tag);
		//проверка необходимости записи уточнения
		if ((*ci)->QualName) fprintf(f.f, "%s.", (*ci)->QualName);
		//запись имени или NIL
		fprintf(f.f, "%s", (*ci)->Name ? (*ci)->Name : "NIL");
	}

	//код завершения объявления нового обобщения
	//проверка наличия специализации по умолчанию
	if (DefaultSpec) {
		fprintf(f.f, "\n\t\tELSE ");
		if (DefaultSpec->Tag) fprintf(f.f, "%s : ", DefaultSpec->Tag);
		if (DefaultSpec->QualName) fprintf(f.f, "%s.", DefaultSpec->QualName);
		fprintf(f.f, "%s", DefaultSpec->Name ? DefaultSpec->Name : "NIL");
	}
	//собственно запись завершения объявления обобщения
	fprintf(f.f, "\n\t\tEND");
}


//-----------------------------------------------------------------------------
//добавление эл-та обобщения в список
void CCommonExtensionType::AddSpec(const char *newTag, const char *newQual, const char *newName)
{
	SpecStore.push_back(CCommonType::NewSpec(newTag, newQual, newName, true));
}


//-----------------------------------------------------------------------------
//деструктор CCommonExtensionType
CCommonExtensionType::~CCommonExtensionType()
{
	delete[] TypeModuleName;
	delete[] TypeName;
	//очистка списка эл-тов обобщения
	CCommonType::SpecStore_type::const_iterator ci_s;
	for (ci_s = SpecStore.begin(); ci_s != SpecStore.end(); ci_s++) CCommonType::DelSpec(*ci_s);
}


//-----------------------------------------------------------------------------
//генерация описания расширения обобщения в .2ml файл
void CCommonExtensionType::WriteCPP(CPP_files &f, const bool to_h)
{
	//запись начала описания обобщения
	fprintf(f.f2ml, "\t<Case Qual=\"%s\" Name=\"%s\">\n", TypeModuleName, TypeName);
	//генерация специализаций обобщения (эл-тов обобщения)
	CCommonType::SpecStore_type::const_iterator ci;
	for (ci = SpecStore.begin(); ci != SpecStore.end(); ci++) {
		fprintf(f.f2ml, "\t\t<Spec");
		if ((*ci)->Tag) fprintf(f.f2ml, " Tag=\"%s\"", (*ci)->Tag);
		if ((*ci)->QualName) fprintf(f.f2ml, " Qual=\"%s\"", (*ci)->QualName);
		if ((*ci)->Name) fprintf(f.f2ml, " Name=\"%s\"", (*ci)->Name);
		fprintf(f.f2ml, " />\n");
	}

	//конец описания обобщения
	fprintf(f.f2ml, "\t</Case>\n");
}


//-----------------------------------------------------------------------------
//инициализация объекта CSpecType
int CSpecType::Init(CLexBuf *lb)
{
	//получение обобщения (Qualident инициализируется в TypeSelector)
	CBaseName* BN = parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident);
	if (!BN || (id_CCommonType != BN->name_id)) return s_e_CommonTypeExpected;

	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//получение "<" (ее наличие проверено в TypeSelector)
	if (!lb->ReadLex(li) || lex_k_lt != li.lex) return s_e_OpAngleMissing;

	//проверка наличия ">" (используется специализация по умолчанию)
	DECL_SAVE_POS
	if (!lb->ReadLex(li) || lex_k_gt != li.lex) {
		RESTORE_POS
	} else {
		//получение параметров специализации по умолчанию
		const CCommonType::SSpec* spec = static_cast<CCommonType*>(BN)->GetDefaultSpec();
		if (!spec) return s_e_NoDefaultSpec;
		//сохранение параметров специализации по умолчанию
		if (spec->Tag)
			TagName = str_new_copy(spec->Tag);
		else {
			TagName = str_new_copy(spec->Name);
			if (spec->QualName) QualTagName = str_new_copy(spec->QualName);
		}
		//инициализация завершена
		return 0;
	}

	//получение признака (в зависимости от типа признака)
	if (CCommonType::tt_Type == static_cast<CCommonType*>(BN)->TagType) {
		//инициализация описания признака
		CQualident qual;
		int err_num = qual.Init(lb, parent_element);
		if (err_num) return err_num;
		//проверка допустимого признака
		if (!static_cast<CCommonType*>(BN)->FindSpecByName(qual.pref_ident, qual.ident)) return s_e_SpecTypeTag;
		//сохранение названия признака
		QualTagName = qual.pref_ident;
		qual.pref_ident = NULL;
		TagName = qual.ident;
		qual.ident = NULL;
	} else {
		//получение названия признака
		if (!lb->ReadLex(li)) return s_e_SpecTypeTag;
		//проверка допустимого признака
		if (!static_cast<CCommonType*>(BN)->FindSpecByTag(li.st)) return s_e_SpecTypeTag;
		//сохранение названия признака
		TagName = str_new_copy(li.st);
	}

	//проверка закрывающей скобки ">"
	if (!lb->ReadLex(li) || lex_k_gt != li.lex) return s_e_ClAngleMissing;

	return 0;
}//Init CSpecType


//-----------------------------------------------------------------------------
//Запись кода CSpecType
void CSpecType::WriteCPP(CPP_files& f, const bool to_h)
{
	if (Qualident->pref_ident)
		//TYPE type1 = module1.type1
		fprintf(to_h ? f.fh : f.fc, "typedef %s::%s %s;\n", Qualident->pref_ident, Qualident->ident, name);
	else
		//TYPE type1 = type2;
		fprintf(to_h ? f.fh : f.fc, "typedef %s %s;\n", Qualident->ident, name);
}//WriteCPP


//-----------------------------------------------------------------------------
//запись в DFN объявления типа
void CSpecType::WriteDFN(DFN_file &f)
{
	Qualident->WriteDFN(f);
	fprintf(f.f, "<");
	if (QualTagName) fprintf(f.f, "%s.", QualTagName);
	fprintf(f.f, "%s>", TagName);
}


//-----------------------------------------------------------------------------
//создание копии данного типа
int CSpecType::CreateType(CBaseType *&BaseType) const
{
	CSpecType* ST = new CSpecType(parent_element);
	Qualident->CreateCopy(ST->Qualident);
	if (QualTagName) ST->QualTagName = str_new_copy(QualTagName);
	if (TagName) ST->TagName = str_new_copy(TagName);
	BaseType = ST;
	return 0;
}


//-----------------------------------------------------------------------------
//создание переменной по типу
int CSpecType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	BaseVar = new CCommonVar(parent);
	//установка названия обобщающего типа
	BaseVar->SetTypeName(Qualident->pref_ident, Qualident->ident);
	//установка названия признака
	return static_cast<CCommonVar*>(BaseVar)->SetTagName(QualTagName, TagName);
}//CSpecType::CreateVar


//-----------------------------------------------------------------------------
//поиск имени в списке полей записи, NULL - если имя не найдено
CBaseName* CSpecType::FindName(const char *search_name) const
{
	//получение ук. на обобщенный тип
	CCommonType* CT = static_cast<CCommonType*>(parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident));
	if (!CT || id_CCommonType != CT->name_id) return NULL;

	//получение описания типа специализации по обобщенному типу и признаку
	const CCommonType::SSpec* spec = CT->FindSpec(QualTagName, TagName, TagName);
	if (!spec) return NULL;

	//получение названия модуля, содержащего тип специализации
	const char* ModuleName = spec->QualName ? spec->QualName : (spec->IsExtended ? NULL : CT->GetModuleAlias());

	//получение типа специализации из текущей области видимости по описанию типа специализации
	CBaseName* BN = parent_element->GetGlobalName(ModuleName, spec->Name);
	if (!BN || id_CRecordType != BN->name_id) return NULL;

	return static_cast<CRecordType*>(BN)->FindName(search_name);
}


//-----------------------------------------------------------------------------
//поиск имени в списке полей записи, NULL - если имя не найдено
const char* CSpecType::GetQualSpecName() const
{
	//получение ук. на обобщенный тип
	CCommonType* CT = static_cast<CCommonType*>(parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident));
	if (!CT || id_CCommonType != CT->name_id) return NULL;

	//проверка типа признаков
	if (CCommonType::tt_Type != CT->TagType) return NULL;

	//проверка локальных признаков
	if (!QualTagName) return GetParentModule()->name;

	//поиск имени модуля по псевдониму
	CBaseName* BN = parent_element->GetGlobalName(QualTagName);
	const char* st = QualTagName;
	if (BN && id_CImportModule == BN->name_id) st = static_cast<CImportModule*>(BN)->real_name;

	//возврат настоящего названия модуля
	return st;
}


//-----------------------------------------------------------------------------
//проверка завершенности типа (наличия именованного типа, на кот. ссылается ук.)
//возврат - 0 или код ошибки (тип не объявлен, недопустимый тип) c установкой
//позиции на ошибку в потоке лексем
int CPointerType::CheckComplete(CLexBuf *lb) const
{
	//проверяется только наличие типа, который еще не был объявлен на момент объявления ук.
	if (!forward) return 0;
	//получение типа
	CBaseName* BN = FindType();
	//проверка наличия объявленного типа
	if (!BN) {
		lb->SetCurrPos(TypePos);
		return s_e_UndeclaredIdent;
	}
	//проверка допустимости типа (допускаются только типы запись, массив и обобщение)
	switch (BN->name_id) {
	case id_CArrayType:
		return static_cast<CArrayType*>(BN)->CheckComplete(lb);
	case id_CRecordType:
		return static_cast<CRecordType*>(BN)->CheckComplete(lb);
	case id_CCommonType:
	case id_CSpecType:
		return 0;
	default:
		lb->SetCurrPos(TypePos);
		return s_e_PointerTypeKind;
	}//switch
}


//-----------------------------------------------------------------------------
//деструктор объекта CPointerType
CPointerType::~CPointerType()
{
	delete Type;
	delete Qualident;
}//~CPointerType


//-----------------------------------------------------------------------------
//инициализация объекта CPointerType
int CPointerType::Init(CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия кл. слова TO
	if (!lb->ReadLex(li) || lex_k_TO != li.lex) return s_e_TO;

	//для проверки наличия QualidentType
	DECL_SAVE_POS

	//получение след. лексемы
	if (!lb->ReadLex(li)) return s_e_PointerTypeKind;

	//проверка наличия Qualident, ARRAY или RECORD
	switch (li.lex) {

	//найден идент. (тип Qualident), это ук. на именованный тип
	case lex_i:
		{
			RESTORE_POS
			//создание и инициализация имени типа
			Qualident = new CQualident;
			int err_num = Qualident->Init(lb, parent_element);
			if (err_num) return err_num;
			//проверка отсутствия указания в Qualident базового типа
			if (id_CBaseName != Qualident->TypeResultId) return s_e_PointerTypeKind;
			//поиск имени типа, на кот. указывает указатель
			CBaseType* BT = FindType();
			//проверка указания уже объявленного типа
			if (BT) {
				//проверка указания недопустимого типа
				switch (BT->name_id) {
				case id_CArrayType:
				case id_CRecordType:
				case id_CSpecType:
					break;
				case id_CCommonType:
					//проверка наличия неименованного SpecType
					SAVE_POS
					err_num = !lb->ReadLex(li) || lex_k_lt != li.lex;
					RESTORE_POS
					if (!err_num) {
						//найден SpecType, инициализируем его (используем уже загруженный Qualident)
						Type = new CSpecType(parent_element);
						static_cast<CSpecType*>(Type)->Qualident = Qualident;
						Qualident = NULL;
						err_num = Type->Init(lb);
						if (err_num) return err_num;
					}
					break;
				default:
					return s_e_PointerTypeKind;
				}//switch
			} else {
				//тип еще не объявлен, устанавливаем признак необходимости опережающего описания
				forward = true;
				//запоминаем позицию в буфере лексем (на случай ошибки)
				TypePos = lb->GetCurrPos();
			}
			return 0;
		}

	//найдено описание неименованного типа ARRAY
	case lex_k_ARRAY:
		Type = new CArrayType(parent_element);
		return Type->Init(lb);

	//найдено описание неименованного типа RECORD (запись или обобщение)
	case lex_k_RECORD:
		SAVE_POS
		if (!lb->ReadLex(li) || lex_k_CASE != li.lex) {
			RESTORE_POS
			Type = new CRecordType(parent_element);
		} else
			Type = new CCommonType(parent_element);
		//инициализация созданного типа
		return Type->Init(lb);

	//недопустимый тип указателя
	default:
		return s_e_PointerTypeKind;
	}//switch
}//CPointerType::Init


//-----------------------------------------------------------------------------
//Запись кода CPointerType
void CPointerType::WriteCPP(CPP_files& f, const bool to_h)
{
	//в коде C++ указатели неименованного типа запись на самом деле являются записями
	//указатели неименованного типа массив являются массивами (?)

	//для генерации кода ук. на необъявленный тип его нужно вначале получить, потом сгенерировать опережающее
	//описание, в зависимости от типа

	if (Type) {
		//генерация кода типа, на кот. указывает указатель
		//установка имени (для создания RuntimeId и генерации кода) - опасная операция, т.к. InitRuntimeId или WriteCPP могут сгенерировать исключение
		Type->name = name;
		try {
			//для типа RECORD генерируем RuntimeId
			if (id_CRecordType == Type->name_id) static_cast<CRecordType*>(Type)->InitRuntimeId();
			Type->WriteCPP(f, to_h);
		}
		catch(error_Internal) {
			Type->name = NULL;
			throw;
		};
		//убираем установленное имя
		Type->name = NULL;
	} else {
		//именованный тип - используем имя для объявления типа POINTER TO

		//получение типа по его имени (в случае наличия псевдонима он может потребоваться)
		CBaseType* BT = static_cast<CBaseType*>(parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident));
		const char* name_alias;
		TFileType* ff;
		//проверка, куда будет записано объявление
		if (to_h) {
			name_alias = BT->GetModuleName();
			ff = f.fh;
		} else {
			name_alias = BT->GetModuleAlias();
			ff = f.fc;
		}

		//проверка необходимости генерации опережающего описания
		if (forward) {
			if (id_CRecordType == BT->name_id || id_CCommonType == BT->name_id) {
				//генерация объявления типа в случае записи (обобщения)
				fprintf(ff, "struct ");
				if (name_alias) fprintf(ff, "%s::", name_alias);
				fprintf(ff, "%s;\n", Qualident->ident);
			} else {
				//в случае массива полностью генерируем тип (в итоге он будет сгенерирован дважды)
				if (id_CArrayType != BT->name_id) throw error_Internal("CPointerType::WriteCPP");
				static_cast<CArrayType*>(BT)->WriteCPP(f, to_h);
			}
		}

		//собственно генерация кода объявления типа
		fprintf(ff, "typedef ");
		if (name_alias) fprintf(ff, "%s::", name_alias);
		fprintf(ff, "%s %s;\n", Qualident->ident, name);
	}//else
}


//-----------------------------------------------------------------------------
//запись в DFN объявления типа
void CPointerType::WriteDFN(DFN_file& f)
{
	fprintf(f.f, "POINTER TO ");
	//запись объявления неименованного или имени именованного типа
	if (Type)
		Type->WriteDFN(f);
	else
		Qualident->WriteDFN(f);
}//WriteDFN


//-----------------------------------------------------------------------------
//создание копии данного типа
int CPointerType::CreateType(CBaseType* &BaseType) const
{
	//создание нового типа
	BaseType = new CPointerType(parent_element);
	//копирование уникальных атрибутов объекта
	static_cast<CPointerType*>(BaseType)->forward = forward;
	static_cast<CPointerType*>(BaseType)->TypePos = TypePos;
	if (Qualident)
		Qualident->CreateCopy(static_cast<CPointerType*>(BaseType)->Qualident);
	if (Type)
		Type->CreateType(static_cast<CPointerType*>(BaseType)->Type);

	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//создание переменной по типу
int CPointerType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	//проверка указания именованного типа (в этом случае известно имя типа, сам тип может еще не быть создан)
	if (Qualident) {
		//создаем переменную
		BaseVar = new CPointerVar(parent);
		//устанавливаем название типа
		BaseVar->SetTypeName(Qualident->pref_ident, Qualident->ident);
	} else {
		//тип указателя на неименованный тип, создаем его копию и заносим в переменную
		CBaseType* BT;
		int err_num = Type->CreateType(BT);
		if (err_num) return err_num;
		//создаем переменную с передачей описания ее типа
		BaseVar = new CPointerVar(parent, BT);
	}

	//занесение текущей позиции в буфере лексем (если есть) в переменную
	if (forward) static_cast<CPointerVar*>(BaseVar)->TypePos = TypePos;

	return 0;
}//CPointerType::CreateVar


//-----------------------------------------------------------------------------
//получение типа, на кот. указывает указатель
CBaseType* CPointerType::FindType() const
{
	//проверка наличия неименованного типа
	if (Type) return Type;

	//поиск по цепочке именованных типов
	CBaseName* BN = parent_element->GetGlobalName(Qualident->pref_ident, Qualident->ident);
	while (BN && id_CQualidentType == BN->name_id)
		BN = BN->parent_element->GetGlobalName(static_cast<CQualidentType*>(BN)->Qualident->pref_ident, static_cast<CQualidentType*>(BN)->Qualident->ident);

	//возврат найденного типа (или NULL)
	return static_cast<CBaseType*>(BN);
}


//-----------------------------------------------------------------------------
//получение типа результата выражения (для процедуры - функции) или id_CBaseName
EName_id CProcedureType::GetResultId() const
{
	//проверка переменной-процедуры (не имеет типа результата)
	if (!FormalPars.Qualident) return id_CBaseName;
	//возврат типа результата выражения
	return FormalPars.Qualident->TypeResultId;
}


//-----------------------------------------------------------------------------
//инициализация объекта CProcedureType
int CProcedureType::Init(CLexBuf *lb)
{
	return FormalPars.Init(lb, parent_element);
}


//-----------------------------------------------------------------------------
//запись кода объявления процедурного типа (CProcedureType)
void CProcedureType::WriteCPP(CPP_files& f, const bool to_h)
{
	//для упрощения выбора целевого файла (.h или .cpp)
	TFileType* const ff = to_h ? f.fh : f.fc;

	fprintf(ff, "typedef ");
	//запись типа для функции (если есть) или void
	if (FormalPars.Qualident) FormalPars.Qualident->WriteCPP_type(f, to_h, parent_element);
	else fprintf(ff, "void");
	fprintf(ff, " (*%s) (", name);
	FormalPars.WriteCPP_pars(f, to_h);
	fprintf(ff, ");\n");
}


//-----------------------------------------------------------------------------
//запись в DFN объявления типа
void CProcedureType::WriteDFN(DFN_file& f)
{
	fprintf(f.f, "PROCEDURE");

	//запись объявления формальных параметров
	fprintf(f.f, " (\n\t\t");
	FormalPars.WriteDFN(f);
	fprintf(f.f, ")");
	//запись типа процедуры (если есть)
	FormalPars.WriteDFN_type(f);
}//WriteDFN


//-----------------------------------------------------------------------------
//создание копии данного типа
int CProcedureType::CreateType(CBaseType* &BaseType) const
{
	//создание нового типа
	BaseType = new CProcedureType(parent_element);
	//копирование списка формальных параметров
	FormalPars.Assign(static_cast<CProcedureType*>(BaseType)->FormalPars, parent_element);
	return 0;
}//CreateType


//-----------------------------------------------------------------------------
//создание переменной по типу
int CProcedureType::CreateVar(CBaseVar* &BaseVar, const CBaseName* parent) const
{
	//создание процедурной переменной
	BaseVar = new CProcedureVar(parent);
	//установка названия типа в случае именованного типа
	BaseVar->SetTypeName(GetModuleName(), name);

	//копирование формальных параметров
	FormalPars.Assign(static_cast<CProcedureVar*>(BaseVar)->FormalPars, parent_element);

	return 0;
}//CProcedureType::CreateVar


//-----------------------------------------------------------------------------
//деструктор объекта CReceiver
CReceiver::~CReceiver()
{
	delete[] name;
	delete[] type_name;
	delete Recv;
}


//-----------------------------------------------------------------------------
//поиск имени в таблице имен, NULL - если имя не найдено
CBaseName* CReceiver::FindName(const char *search_name) const
{
	if (!strcmp(search_name, name)) return Recv;
	return NULL;
}


//-----------------------------------------------------------------------------
//инициализация объекта CReceiver
int CReceiver::Init(CLexBuf *lb, const CBaseName* parent_element)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия лексемы
	if (!lb->ReadLex(li)) return s_e_IdentExpected;

	//проверка наличия VAR
	if (lex_k_VAR == li.lex) {
		is_var = true;
		if (!lb->ReadLex(li)) return s_e_IdentExpected;
	}

	//проверка наличия ид.
	if (lex_i != li.lex) return s_e_IdentExpected;

	//запоминаем полученное имя
	name = str_new_copy(li.st);

	//проверка наличия ":"
	if (!lb->ReadLex(li) || lex_k_colon != li.lex) return s_e_ColonMissing;

	//проверка наличия ид. или кл. слова
	if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;

	//проверка существования объекта типа в глобальной области видимости
	CBaseName* RType = parent_element->GetGlobalName(li.st);
	if (!RType) return s_e_UndeclaredIdent;

	//проверка типа и способа объявления (VAR) приемника
	switch (RType->name_id) {
	case id_CRecordType:	//тип запись, должен быть объявлен как VAR
		if (!is_var) return s_e_ReceiverVAR;
		break;
	case id_CPointerType:	//тип ук., должен быть объявлен без VAR
		if (is_var) return s_e_ReceiverVAR;
		//проверка типа указателя (должен быть ук. на запись)
		if (id_CRecordType != static_cast<CPointerType*>(RType)->FindType()->name_id) return s_e_ReceiverVAR;
	}

	//копирование переменной - приемника (уже убедились, что указан тип)
	int err_num = static_cast<CBaseType*>(RType)->CreateVar(Recv, parent_element);
	if (err_num) return err_num;
	//в случае ук. установка признака ук. на запись (уже проверено)
	if (id_CPointerVar == Recv->name_id) static_cast<CPointerVar*>(Recv)->SetIsRecord();
	//копирование признака экспорта
	Recv->external = RType->external;
	//копирование в Recv имени приемника (для дальнейшей работы с Recv как с обычной именованной переменной)
	Recv->SetName(name);
	//установка признака VAR для записи
	Recv->is_var = is_var;

	//запоминаем полученное имя типа
	type_name = str_new_copy(li.st);

	//проверка наличия ")"
	if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

	return 0;
}//Init CReceiver


//-----------------------------------------------------------------------------
//запись кода приемника (устаревший вариант)
void CReceiver::WriteCPP_fp(CPP_files &f, const bool external, const bool single_param)
{
	fprintf(f.fc, "%s* %s%s", type_name, name, single_param ? "" : ", ");
	if (external) fprintf(f.fh, "%s* %s%s", type_name, name, single_param ? "" : ", ");
}


//-----------------------------------------------------------------------------
//деструктор объекта CFPSection
CFPSection::~CFPSection()
{
	//очистка списка строк
	TmpIdents_type::const_iterator ci;
	for (ci = tmp_idents.begin(); ci != tmp_idents.end(); ci++)
		delete[] *ci;
	//очистка типа
	delete BaseType;
}//~CFPSection


//-----------------------------------------------------------------------------
//инициализация списка объектов FPVector, в ReceiverName - имя приемника или NULL
int CFPSection::AssignFPElems(CLexBuf *lb, CBaseVarVector &FPVector, const char* ReceiverName)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//загрузка первой лексемы (должен быть идентификатор или VAR)
	if (!lb->ReadLex(li) || (lex_k_VAR != li.lex && lex_i != li.lex)) return s_e_IdentExpected;

	//проверка наличия "VAR", за ним должен следовать идентификатор
	if (is_var = (lex_k_VAR == li.lex))
		if (!lb->ReadLex(li) || lex_i != li.lex)
			return s_e_IdentExpected;

	while (true) {
		//проверка отсутствия совпадения имени с именем приемника (если есть имя приемника)
		if (ReceiverName)
			if (!strcmp(ReceiverName, li.st))
				return s_e_Redefinition;

		//проверка отсутствия имени в списке имен FPSection
		TmpIdents_type::const_iterator ci;
		for (ci = tmp_idents.begin(); ci != tmp_idents.end(); ++ci)
			if (!strcmp(*ci, li.st))
				return s_e_Redefinition;

		//проверка отсутствия имени в списке имен FormalPars
		CBaseVarVector::const_iterator fp_ci;
		for (fp_ci = FPVector.begin(); fp_ci != FPVector.end(); ++fp_ci)
			if ( !strcmp((*fp_ci)->name, li.st) )
				return s_e_Redefinition;

		//запоминаем очередное считанное имя
		tmp_idents.push_back(str_new_copy(li.st));

		//проверка отсутствия ","
		if (!lb->ReadLex(li) || lex_k_comma != li.lex) break;

		//считывание следующей лексемы
		if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	}//while

	//проверка наличия ":"
	if (lex_k_colon != li.lex) return s_e_ColonMissing;

	//создание объекта типа из потока лексем
	int err_num = TypeSelector(lb, BaseType, parent_element);
	if (err_num) return err_num;
	if (!BaseType) return s_m_Error;

	//создание объявленных формальных параметров по указанному типу
	TmpIdents_type::iterator it;
	for (it = tmp_idents.begin(); it != tmp_idents.end(); ++it) {
		CBaseVar* FPVar = NULL;
		//создание нового эл-та списка формальных параметров
		err_num = BaseType->CreateVar(FPVar, parent_element);
		if (err_num) {
			delete FPVar;
			return err_num;
		}

		//проверка наличия непараметризованной обобщенной переменной
		if (id_CCommonVar == FPVar->name_id && static_cast<CCommonVar*>(FPVar)->IsPureCommon()) {
			//уничтожение обобщенной переменной
			delete FPVar;
			//при отсутствии VAR - найдено объявление непараметризованной обобщенной переменной,
			//при отсутствии QualidentType - найдено объявление неименованного обобщенного типа в параметрах процедуры
			if (!is_var || id_CQualidentType != BaseType->name_id) return s_e_SpecTypeExpected;
			//создание переменной типа указатель на непараметризованное обобщение
			FPVar = new CPointerVar(parent_element);
			const CQualident* Q = static_cast<CQualidentType*>(BaseType)->Qualident;
			FPVar->SetTypeName(Q->pref_ident, Q->ident);
		}

		//инициализация нового эл-та списка формальных параметров
		FPVar->is_var = is_var;
		FPVar->name = *it;
		*it = NULL;
		//занесение созданного эл-та в список
		FPVector.push_back(FPVar);
	}

	return 0;
}


//-----------------------------------------------------------------------------
//деструктор объекта CFormalPars
CFormalPars::~CFormalPars()
{
	delete Qualident;
	CBaseVarVector::const_iterator ci;
	for (ci = FPStore.begin(); ci != FPStore.end(); ++ci) delete *ci;
}//~CFormalPars


//-----------------------------------------------------------------------------
//копирование содержимого объекта в указанные формальные параметры
void CFormalPars::Assign(CFormalPars& FP, const CBaseName* parent_element) const
{
	//копирование списка переменных
	CBaseVarVector::const_iterator ci = FPStore.begin();
	for (; ci != FPStore.end(); ci++)
		FP.FPStore.push_back((*ci)->CreateVar(parent_element));

	//копирование названия типа (если есть)
	if (Qualident) Qualident->CreateCopy(FP.Qualident);
}


//-----------------------------------------------------------------------------
//инициализация объекта CFormalPars
int CFormalPars::Init(CLexBuf *lb, const CBaseName* parent_element)
{
	DECL_SAVE_POS
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия "("
	if (lb->ReadLex(li) && lex_k_op_bracket == li.lex) {
		SAVE_POS

		//для хранения имени приемника в случае процедуры, связанной с типом
		const char* ReceiverName = NULL;
		if (CProcedure::IsProcId(parent_element->name_id))
			if (static_cast<const CProcedure*>(parent_element)->Receiver)
				ReceiverName = static_cast<const CProcedure*>(parent_element)->Receiver->name;

		//проверка отсутствия ")" (есть список формальных параметров)
		if (!lb->ReadLex(li) || lex_k_cl_bracket != li.lex) {
			RESTORE_POS

			while (true) {
				//загрузка текущей FPSection
				CFPSection FPSection (parent_element);
				int err_num = FPSection.AssignFPElems(lb, FPStore, ReceiverName);
				if (err_num) return err_num;
	
				//проверка наличия ";"
				if (!lb->ReadLex(li) || lex_k_semicolon != li.lex) break;
			}

			//проверка наличия ")" (в конце списка формальных параметров)
			if (lex_k_cl_bracket != li.lex) return s_e_ClBracketMissing;

			//выполнение доп. проверок корректности формальных параметров
			int err_num = CheckCompleteRoot(lb);
			if (err_num) return err_num;

		}//if

	} else {
		//отсутствует "(" => нет списка формальных параметров
		RESTORE_POS
		//завершаем обработку, т.к. тип результата может быть записан только после "()"
		return 0;
	}

	//проверка наличия параметров-значений типа массив (требуется для определения необходимости инициализации массивов)
	CBaseVarVector::const_iterator ci;
	for (ci = FPStore.begin(); ci != FPStore.end(); ++ci)
		if (id_CArrayVar == (*ci)->name_id && !(*ci)->is_var) {
			have_arrays = true;
			break;
		}

	//проверка наличия ":" (отсутствие типа возвращаемого результата)
	SAVE_POS
	if (!lb->ReadLex(li) || lex_k_colon != li.lex) {
		RESTORE_POS
		return 0;
	}

	//создание описания имени типа возвращаемого результата
	Qualident = new CQualident;
	return Qualident->InitTypeName(lb, parent_element);

}//Init CFormalPars


//-----------------------------------------------------------------------------
//Запись кода CFormalPars (тип процедуры)
void CFormalPars::WriteCPP_type(CPP_files& f, const bool to_h, const CBaseName* parent_element)
{
	//собственно запись типа процедуры (функции)
	if (!Qualident) {
		fprintf(to_h ? f.fh : f.fc, "void ");
	} else {
		Qualident->WriteCPP_type(f, to_h, parent_element);
		fprintf(to_h ? f.fh : f.fc, " ");
	}
}//WriteCPP_type


//-----------------------------------------------------------------------------
//запись объявления формальных параметров процедуры в dfn файл
void CFormalPars::WriteDFN(DFN_file& f)
{
	//запись кода параметров (если есть) процедуры
	if (!FPStore.empty()) {
		CBaseVarVector::const_iterator ci = FPStore.begin();
		while (true) {
			f.tab();
			if ((*ci)->is_var) fprintf(f.f, "VAR ");
			(*ci)->WriteDFN(f);
			++ci;
			if (FPStore.end() == ci) break;
			fprintf(f.f, ";\n");
		}
	}
}//WriteDFN


//-----------------------------------------------------------------------------
//запись типа процедуры в dfn файл
void CFormalPars::WriteDFN_type(DFN_file& f)
{
	//запись кода типа процедуры (если есть)
	if (Qualident) {
		fprintf(f.f, " : ");
		Qualident->WriteDFN(f);
	}
}//WriteDFN


//-----------------------------------------------------------------------------
//Запись кода CFormalPars (формальные параметры)
void CFormalPars::WriteCPP_pars(CPP_files& f, const bool to_h)
{
	//проверка наличия формальных параметров
	if (FPStore.empty()) return;
	//запись первого формального параметра
	CBaseVarVector::const_iterator i = FPStore.begin();
	(*i)->WriteCPP_fp(f, to_h);
	//запись следующих формальных параметров
	for(++i; i != FPStore.end(); ++i) {
		fprintf(to_h ? f.fh : f.fc, ", ");
		(*i)->WriteCPP_fp(f, to_h);
	}
}//WriteCPP_pars


//-----------------------------------------------------------------------------
//запись списка имен формальных параметров через ","
void CFormalPars::WriteCPP_names(CPP_files& f, const bool to_h)
{
	//проверка наличия формальных параметров
	if (FPStore.empty()) return;
	//запись первого формального параметра
	CBaseVarVector::const_iterator ci = FPStore.begin();
	fprintf(to_h ? f.fh : f.fc, "%s", (*ci)->name);
	//запись остальных формальных параметров
	for(++ci; ci != FPStore.end(); ++ci) {
		fprintf(to_h ? f.fh : f.fc, ", ");
		fprintf(to_h ? f.fh : f.fc, "%s", (*ci)->name);
	}
}


//-----------------------------------------------------------------------------
//генерация кода для инициализации массивов-значений
void CFormalPars::WriteCPP_begin(CPP_files& f)
{
	//проверка отсутствия массивов в качестве параметров-значений
	if (!have_arrays) return;

	f.tab_level_c++;
	f.tab_fc();
	fprintf(f.fc, "//arrays initialisation\n");

	//перебор списка формальных параметров
	CBaseVarVector::const_iterator ci;
	for (ci = FPStore.begin(); ci != FPStore.end(); ++ci)
		//проверка наличия параметра-значения типа массив
		if ( ((*ci)->name_id == id_CArrayVar)&&(!(*ci)->is_var) ) {

			//получение ук. на тип массива для текущей переменной
			CArrayType *AT = static_cast<CArrayVar*>(*ci)->ArrayType;
			//проверка наличия открытого массива
			if (AT->size == 0) {	//создание открытого массива и инициализация переданным массивом
				AT = static_cast<CArrayType*>(AT->FindLastType());
				//объявление локальной переменной для копирования переданной
				CBaseVar *BV;
				AT->CreateVar(BV, AT->parent_element);
				f.tab_fc();
				const char* module_alias = BV->GetTypeModuleAlias();
				if (module_alias) fprintf(f.fc, "%s::", module_alias);
				fprintf(f.fc, "%s *%s;\n{\n", BV->GetTypeName(), (*ci)->name);
				//генерация кода выделения памяти под массив
				fprintf(f.fc, "\tconst int O2M_COUNT = O2M_ARR_0_%s", (*ci)->name);
				int dimention = 1;
				AT = static_cast<CArrayType*>( static_cast<CArrayVar*>(*ci)->ArrayType->Type );
				while (AT->name_id == id_CArrayType) {
					fprintf(f.fc, "*O2M_ARR_%i_%s", dimention, (*ci)->name);
					++dimention;
					AT = static_cast<CArrayType*>(AT->Type);
				}
				fprintf(f.fc, ";\n\t%s = new ", (*ci)->name);
				if (module_alias) fprintf(f.fc, "%s::", module_alias);
				fprintf(f.fc, "%s[O2M_COUNT];\n", BV->GetTypeName());
				//генерация инициализирующего кода
				fprintf(f.fc, "\tfor(int O2M_I=0; O2M_I<O2M_COUNT; ++O2M_I) %s[O2M_I] = O2M_ARR_%s[O2M_I];\n}\n", (*ci)->name, (*ci)->name);
				delete BV;

			} else {	//создание обычного массива и инициализация переданным массивом

				//объявление локальной переменной аналогичной переданной
				f.tab_fc();
				(*ci)->WriteCPP(f);
				fprintf(f.fc, ";\n{");
				//генерация инициализирующего кода
				int dimention = 0;
				while (AT->name_id == id_CArrayType) {
					//генерация кода цикла перебора массива по очередному измерению
					fprintf(f.fc, "\n\tfor(int O2M_I_%i = 0; O2M_I_%i < %i; ++O2M_I_%i)", dimention, dimention, AT->size, dimention);
					++dimention;
					AT = static_cast<CArrayType*>(AT->Type);
				}
				//генерация кода инициализации текущего эл-та массива
				fprintf(f.fc, " %s", (*ci)->name);
				int i;
				for (i = 0; i < dimention; i++) fprintf(f.fc, "[O2M_I_%i]", i);
				fprintf(f.fc, " = O2M_ARR_%s", (*ci)->name);
				for (i = 0; i < dimention; i++) fprintf(f.fc, "[O2M_I_%i]", i);
				fprintf(f.fc, ";\n}\n");
			}//else
		}//if

	f.tab_level_c--;
}//WriteCPP_arrays


//-----------------------------------------------------------------------------
//генерация кода для деинициализации массивов-значений
void CFormalPars::WriteCPP_end(CPP_files& f, const bool ret_present)
{
	//в случае наличия оператора RETURN генерация метки для поддержки его работы
	if (ret_present) {
		f.tab_fc();
		fprintf(f.fc, "\tO2M_RETURN:;\n");
	}

	//проверка отсутствия массивов в качестве параметров-значений
	if (!have_arrays) return;

	f.tab_level_c++;

	f.tab_fc();
	fprintf(f.fc, "//delete arrays\n");

	//перебор списка формальных параметров
	CBaseVarVector::const_iterator ci;
	for (ci = FPStore.begin(); ci != FPStore.end(); ++ci)
		//проверка наличия параметра-значения типа массив
		if ( ((*ci)->name_id == id_CArrayVar) && (!(*ci)->is_var) )
			if (static_cast<CArrayVar*>(*ci)->ArrayType->size == 0) {
				f.tab_fc();
				fprintf(f.fc, "delete[] %s;\n", (*ci)->name);
			}

	f.tab_level_c--;
}//WriteCPP_delete


//-----------------------------------------------------------------------------
//инициализация списка объектов FPVector, в ReceiverName - имя приемника или NULL
int CCommonFPSection::AssignFPElems(CLexBuf *lb, CBaseVarVector &FPVector)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//загрузка первой лексемы (должен быть идентификатор или VAR)
	if (!lb->ReadLex(li) || (lex_k_VAR != li.lex && lex_i != li.lex)) return s_e_IdentExpected;

	//для установки позиции ошибки при неверном отсутствии/наличии VAR
	DECL_SAVE_POS

	//проверка наличия "VAR", за ним должен следовать идентификатор
	if (is_var = (lex_k_VAR == li.lex))
		if (!lb->ReadLex(li) || lex_i != li.lex)
			return s_e_IdentExpected;

	while (true) {

		//проверка отсутствия имени в списке имен FPSection
		TmpIdents_type::const_iterator ci;
		for (ci = tmp_idents.begin(); ci != tmp_idents.end(); ++ci)
			if (!strcmp(*ci, li.st))
				return s_e_Redefinition;

		//проверка отсутствия имени в списке имен FormalPars
		CBaseVarVector::const_iterator fp_ci;
		for (fp_ci = FPVector.begin(); fp_ci != FPVector.end(); ++fp_ci)
			if ( !strcmp((*fp_ci)->name, li.st) )
				return s_e_Redefinition;

		//запоминаем очередное считанное имя
		tmp_idents.push_back(str_new_copy(li.st));

		//проверка отсутствия ","
		if (!lb->ReadLex(li) || lex_k_comma != li.lex) break;

		//считывание следующей лексемы
		if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	}//while

	//проверка наличия "<" (признак наличия обработчика парам. спец-ции)
	if (lex_k_lt == li.lex) return s_m_HProcFound;

	//проверка наличия ":"
	if (lex_k_colon != li.lex) return s_e_ColonMissing;

	//создание объекта типа из потока лексем
	int err_num = TypeSelector(lb, BaseType, parent_element);
	if (err_num) return err_num;
	if (!BaseType) return s_m_Error;


	/////////////////////////////////////
	//проверка получения обобщенного типа

	//для получения обобщенного типа через именованный тип или указатель
	CBaseType* BT;

	//проверка полученного типа
	switch (BaseType->name_id) {
	case id_CQualidentType:
		//получение типа через именованный тип
		err_num = static_cast<CQualidentType*>(BaseType)->GetNamedType(BT, false);
		if (err_num) return err_num;
		break;
	case id_CPointerType:
		//установка BT для дальнейшей проверки типа указатель
		BT = BaseType;
		break;
	case id_CSpecType:
		//обнаружен обработчик парам. спец-ции
		return s_m_HProcFound;
	default:
		return s_e_CommonTypeExpected;
	}

	//проверка типа, полученного из именованного, или проверка типа указатель
	switch (BT->name_id) {
	case id_CPointerType:
		//получение типа через тип указатель
		BT = static_cast<CPointerType*>(BT)->FindType();
		if (id_CHandlerProc == BT->name_id) return s_m_HProcFound;
		if (id_CCommonType != BT->name_id) return s_e_CommonTypeExpected;
		if (is_var) {
			RESTORE_POS
			return s_e_CommonParVAR;
		}
		break;
	case id_CCommonType:
		if (!is_var) {
			RESTORE_POS
			return s_e_CommonParVAR;
		}
		break;
	case id_CSpecType:
		return s_m_HProcFound;
	default:
		return s_e_CommonTypeExpected;
	}


	///////////////////////////////////////////////////////////////
	//создание объявленных формальных параметров по указанному типу
	TmpIdents_type::iterator it;
	for (it = tmp_idents.begin(); it != tmp_idents.end(); ++it) {
		CBaseVar* FPVar = NULL;
		//инициализация нового эл-та списка формальных параметров
		err_num = BaseType->CreateVar(FPVar, parent_element);
		if (err_num) {
			delete FPVar;
			return err_num;
		}
		if (!FPVar)
			return s_m_Error;
		FPVar->is_var = is_var;
		FPVar->name = *it;
		*it = NULL;
		//занесение созданного эл-та в список
		FPVector.push_back(FPVar);
	}

	return 0;
}


//-----------------------------------------------------------------------------
//деструктор
CSpecFPSection::~CSpecFPSection()
{
	//очистка списка загруженных специализированных параметров
	TSFPElemStore::const_iterator ci;
	for (ci = SFPElemStore.begin(); ci != SFPElemStore.end(); ci++)
		delete *ci;
	//очистка типа
	delete BaseType;
}


//-----------------------------------------------------------------------------
//инициализация списка объектов FPVector
int CSpecFPSection::AssignFPElems(CLexBuf *lb, CBaseVarVector &FPVector)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//загрузка первой лексемы (должен быть идентификатор или VAR)
	if (!lb->ReadLex(li) || (lex_k_VAR != li.lex && lex_i != li.lex)) return s_e_IdentExpected;

	//проверка наличия "VAR", за ним должен следовать идентификатор
	if (is_var = (lex_k_VAR == li.lex))
		if (!lb->ReadLex(li) || lex_i != li.lex)
			return s_e_IdentExpected;

	//признак необходимости наличия обобщенного (непараметризованного) типа
	bool CommonTypeNeed = false;

	while (true) {
		//проверка отсутствия имени в списке имен SpecFPSection
		TSFPElemStore::const_iterator ci;
		for (ci = SFPElemStore.begin(); ci != SFPElemStore.end(); ++ci)
			if (!strcmp((*ci)->ident, li.st)) return s_e_Redefinition;

		//проверка отсутствия имени в списке имен FormalPars
		CBaseVarVector::const_iterator fp_ci;
		for (fp_ci = FPVector.begin(); fp_ci != FPVector.end(); ++fp_ci)
			if (!strcmp((*fp_ci)->name, li.st)) return s_e_Redefinition;

		//создание эл-та для хранения информации о текущем параметре
		SSFPElem* fpe = new SSFPElem;
		fpe->ident = str_new_copy(li.st);
		fpe->IsNeedDefaultSpec = false;
		fpe->QualTagName = NULL;
		fpe->TagName = NULL;
		//сохранение текущей позиции (для определения позиции ошибки при проверке названия признака)
		fpe->pos = lb->GetCurrPos();
		//сохранение считанного идентификатора в списке эл-тов
		SFPElemStore.push_back(fpe);

		//получение след. символа
		if (!lb->ReadLex(li)) return s_e_ColonMissing;

		//проверка наличия "," (переход к обработке след. эл-та)
		if (lex_k_comma == li.lex) {
			if (CommonTypeNeed) return s_e_OpAngleMissing;
			if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
			continue;
		}

		//проверка отсутствия "<" (конец обработки секции параметров)
		if (lex_k_lt != li.lex) break;

		//есть "<", требуется обобщенный (непараметризованный) тип
		CommonTypeNeed = true;

		//проверка наличия ">" (используется специализация по умолчанию)
		DECL_SAVE_POS
		if (!lb->ReadLex(li) || lex_k_gt != li.lex) {
			//">" отсутствует, восстанавливаем позицию в буфере лексем
			RESTORE_POS
			//инициализация описания признака
			CQualident qual;
			int err_num = qual.Init(lb, parent_element);
			if (err_num) return err_num;
			//сохранение названия признака
			fpe->QualTagName = qual.pref_ident;
			fpe->TagName = qual.ident;
			qual.pref_ident = NULL;
			qual.ident = NULL;
			//проверка наличия ">"
			if (!lb->ReadLex(li) || lex_k_gt != li.lex) return s_e_ClAngleMissing;
		} else	//установка признака необходимости использования спец-ции по умолчанию
			fpe->IsNeedDefaultSpec = true;

		//проверка отсутствия ","
		if (!lb->ReadLex(li) || lex_k_comma != li.lex) break;

		//считывание следующей лексемы
		if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	}//while

	//проверка наличия ":"
	if (lex_k_colon != li.lex) return s_e_ColonMissing;

	//создание объекта типа из потока лексем
	int err_num = TypeSelector(lb, BaseType, parent_element);
	if (err_num) return err_num;
	//ук. на тип для обработки случая QualidentType
	CBaseType* BT = BaseType;
	if (id_CQualidentType == BT->name_id) {
		//в случае QualidentType получаем именованный тип
		err_num = static_cast<CQualidentType*>(BT)->GetNamedType(BT, false);
		if (err_num) return err_num;
	}

	//проверка наличия требуемого обобщенного (возможно специализированного) типа
	if (CommonTypeNeed) {
		if (id_CCommonType != BT->name_id) return s_e_CommonTypeExpected;
	} else
		if (id_CSpecType != BT->name_id) return s_e_SpecTypeExpected;

	//создание объявленных формальных параметров по указанному типу
	TSFPElemStore::const_iterator ci;
	for (ci = SFPElemStore.begin(); ci != SFPElemStore.end(); ++ci) {
		//проверка отсутствия требуемого признака у текущего эл-та (наличие признака когда он не требуется проверено выше)
		if (!(*ci)->TagName && !(*ci)->IsNeedDefaultSpec && CommonTypeNeed) {
			lb->SetCurrPos((*ci)->pos);
			lb->ReadLex(li);
			return s_e_OpAngleMissing;
		}
		//ук. на создаваемую переменную
		CBaseVar* FPVar = NULL;
		//инициализация нового эл-та списка формальных параметров
		err_num = BT->CreateVar(FPVar, parent_element);
		if (err_num) {
			delete FPVar;
			return err_num;
		}
		//установка свойств созданной переменной
		FPVar->is_var = is_var;
		FPVar->name = (*ci)->ident;
		(*ci)->ident = NULL;

		//при наличии обобщенного типа требуется установка признака
		if (CommonTypeNeed) {
			//проверка необходимости установки признака по умолчанию
			if ((*ci)->IsNeedDefaultSpec) {
				const CCommonType::SSpec* spec = static_cast<CCommonType*>(BT)->GetDefaultSpec();
				if (!spec) {
					//специализация по умолчанию не найдена, выдача сообщения об ошибке
					delete FPVar;
					lb->SetCurrPos((*ci)->pos);
					lb->ReadLex(li);
					return s_e_SpecTypeExpected;
				}
				//установка имени специализации по умолчанию
				err_num = static_cast<CCommonVar*>(FPVar)->SetTagName(spec->QualName, spec->Name);
			} else
				err_num = static_cast<CCommonVar*>(FPVar)->SetTagName((*ci)->QualTagName, (*ci)->TagName);
			//проверка наличия ошибки после установки признака
			if (err_num) {
				delete FPVar;
				lb->SetCurrPos((*ci)->pos);
				lb->ReadLex(li);
				return err_num;
			}
		}

		//занесение созданного эл-та в список
		FPVector.push_back(FPVar);
	}

	return 0;
}


//-----------------------------------------------------------------------------
//инициализация обобщенных параметров обобщающей процедуры
int CCommonPars::Init(CLexBuf *lb, CBaseName* parent_element)
{
	DECL_SAVE_POS
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка отсутствия "}" (есть список обобщающих параметров)
	if (!lb->ReadLex(li) || lex_k_cl_brace != li.lex) {
		RESTORE_POS

		while (true) {
			//загрузка текущей FPSection
			CCommonFPSection CommonFPSection(parent_element);
			int err_num = CommonFPSection.AssignFPElems(lb, FPStore);
			if (err_num) return err_num;

			//проверка наличия ";"
			if (!lb->ReadLex(li) || lex_k_semicolon != li.lex) break;
		}

		//проверка наличия "}" (в конце списка обобщающих параметров)
		if (lex_k_cl_brace != li.lex) return s_e_ClBraceMissing;
	}//if

	//проверка наличия ":" (отсутствие типа возвращаемого результата)
	SAVE_POS
	if (!lb->ReadLex(li) || lex_k_colon != li.lex) {
		RESTORE_POS
		return 0;
	}

	//создание описания имени типа возвращаемого результата
	Qualident = new CQualident;
	return Qualident->InitTypeName(lb, parent_element);

}//Init CCommonPars


//-----------------------------------------------------------------------------
//инициализация обобщенных параметров обработчика парам. спец-ции
int CCommonPars::InitSpec(CLexBuf *lb, CBaseName *parent_element)
{
	DECL_SAVE_POS
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка отсутствия "}" (есть список обобщающих параметров)
	if (!lb->ReadLex(li) || lex_k_cl_brace != li.lex) {
		RESTORE_POS

		while (true) {
			//загрузка текущей SpecFPSection
			CSpecFPSection SpecFPSection(parent_element);
			int err_num = SpecFPSection.AssignFPElems(lb, FPStore);
			if (err_num) return err_num;

			//проверка наличия ";"
			if (!lb->ReadLex(li) || lex_k_semicolon != li.lex) break;
		}

		//проверка наличия "}" (в конце списка обобщающих параметров)
		if (lex_k_cl_brace != li.lex) return s_e_ClBraceMissing;
	}//if

	//проверка наличия ":" (отсутствие типа возвращаемого результата)
	SAVE_POS
	if (!lb->ReadLex(li) || lex_k_colon != li.lex) {
		RESTORE_POS
		return 0;
	}

	//создание описания имени типа возвращаемого результата
	Qualident = new CQualident;
	return Qualident->InitTypeName(lb, parent_element);

}


//-----------------------------------------------------------------------------
//инициализация импортируемых модулей (вызывается только для модулей)
void CDeclSeq::WriteCPP_mod_init(CPP_files &f)
{
	//проверка отсутствия повторного вызова процедуры инициализации
	fprintf(f.fc, "\t//imports initialisation\n");
	fprintf(f.fc, "\tstatic bool %swas_started(false);\n", O2M_SYS_);
	fprintf(f.fc, "\tif (%swas_started) return;\n", O2M_SYS_);
	fprintf(f.fc, "\t%swas_started = true;\n", O2M_SYS_);

	//поиск импорт. модулей и вызов процедур их инициализации
	CBaseNameVector::const_iterator i;
	for (i = BaseNameStore.begin(); i != BaseNameStore.end(); ++i)
		if ((*i)->name_id == id_CImportModule)
			fprintf(f.fc, "\t%s::%s%s();\n", 
				static_cast<CImportModule*>(*i)->name,
				O2M_SYS_,
				static_cast<CImportModule*>(*i)->real_name);
	//комментарий - начало исполняемого кода модуля
	fprintf(f.fc, "//BEGIN\n");
}


//-----------------------------------------------------------------------------
//создание файла _O2M_main.cpp с функцией O2M_SYS_main_init
bool CModule::WriteCPP_main()
{
	//имя .h файла _O2M_main
	const char* const _O2M_main_h = "CPP/_O2M_main.h";
	//имя .cpp файла _O2M_main
	const char* const _O2M_main_cpp = "CPP/_O2M_main.cpp";

	//создание заголовочного файла _O2M_main.h
	FILE *f = fopen(_O2M_main_h, "w");
	if (!f) {
		fprintf(output, textCannotOpenW, _O2M_main_h);
		return false;
	}
	//запись комментария
	fprintf(f, comment_format, comment_line_cpp, comment_title, comment_line_cpp);

	//предотвращение повторного объявления
	fprintf(f, "#ifndef O2M_H_FILE__O2M_main\n");
	fprintf(f, "#define O2M_H_FILE__O2M_main\n\n");

	//запись имени функции инициализации модуля
	fprintf(f, "#include \"%s.h\"\n\n", name);
	fprintf(f, "void %smain_init();\n\n", O2M_SYS_);
	fprintf(f, "#endif\n");

	//закрытие файла
	if (fclose(f)) {
		fprintf(output, textCannotClose, _O2M_main_h);
		return false;
	}

	//создание файла _O2M_main.cpp
	f = fopen(_O2M_main_cpp, "w");
	if (!f) {
		fprintf(output, textCannotOpenW, _O2M_main_cpp);
		return false;
	}
	//запись комментария
	fprintf(f, comment_format, comment_line_cpp, comment_title, comment_line_cpp);

	//запись включения заголовочного файла главного файла проекта, и файла с механизмами PPP
	fprintf(f, "#include \"%s.h\"\n#include \"_O2M_ppp.cpp\"\n\n", name);

	//запись функции инициализации модуля
	fprintf(f, "void %smain_init() {\n", O2M_SYS_);
	fprintf(f, "\t%s::%s%s();\n}\n", name, O2M_SYS_, name);

	//закрытие файла
	if (fclose(f)) {
		fprintf(output, textCannotClose, _O2M_main_cpp);
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
//запись в .dfn файл
void CModule::WriteDFN(DFN_file &f)
{
	fprintf(f.f, "DEFINITION %s;\n", name);
	DeclSeq->WriteDFN(f);
	fprintf(f.f, "\n\nEND %s.\n", name);
}


//-----------------------------------------------------------------------------
//деструктор объекта CDfnModule
CDfnModule::~CDfnModule()
{
	delete DfnDeclSeq;
	delete[] full_path;
	delete[] alias_name;
}//~CDfnModule


//-----------------------------------------------------------------------------
//Запись кода CDfnModule
void CDfnModule::WriteCPP(CPP_files& f)
{
	fprintf(f.fh, "#include \"%s%s.h\"\n", full_path ? full_path : "", name);
}


//-----------------------------------------------------------------------------
//инициализация объекта CDfnModule (загрузка .dfn файла)
int CDfnModule::Init(const CProject *project, CLexBuf *lb)
{
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия кл. слова DEFINITION
	if (!lb->ReadLex(li) || lex_k_DEFINITION != li.lex) return s_e_DEFINITION;

	//проверка наличия названия модуля
	if (!lb->ReadLex(li) || lex_i != li.lex) return s_e_IdentExpected;
	SetName(li.st);

	//проверка наличия ";"
	if (!lb->ReadLex(li) || lex_k_semicolon != li.lex) return s_e_SemicolonMissing;

	//создание DfnDeclSeq (для хранения импортируемых модулей)
	DfnDeclSeq = new CDfnDeclSeq(this);
	
	//загрузка списка импортируемых модулей (если есть)
	int err_num = DfnDeclSeq->ImportListInit(project, lb);
	if (err_num) return err_num;

	//проверка наличия DfnDeclSeq
	err_num = DfnDeclSeq->Init(lb);
	if (err_num) return err_num;

	//проверка наличия кл. слова END
	if (!lb->ReadLex(li) || lex_k_END != li.lex) return s_e_END;

	//проверка наличия названия модуля (в конце)
	if (!lb->ReadLex(li) || lex_i != li.lex || strcmp(li.st, name)) return s_e_ModuleEndName;

	//проверка наличия "."
	if (!lb->ReadLex(li) || lex_k_dot != li.lex) return s_e_DotMissing;

	return 0;
}//CDfnModule::Init


//-----------------------------------------------------------------------------
//добавление указанного эл-та в таблицу имен
void CDfnModule::AddName(CBaseName* BN) const
{
	DfnDeclSeq->AddName(BN);
}


//-----------------------------------------------------------------------------
//поиск импортированного имени в таблице имен, NULL - если имя не найдено
CBaseName* CDfnModule::FindImportedName(const char *module_name, const char *search_name) const
{
	//поиск имени (псевдонима) импортированного модуля
	const CBaseName* BaseIM = FindName(module_name);
	if (!BaseIM || (BaseIM->name_id != id_CImportModule))
		return NULL;	//указанный псевдоним не найден
	//проверка наличия импортированных модулей
	if (!DfnDeclSeq->DfnModuleSeq) return NULL;
	//поиск импортированного модуля по реальному имени, полученному из псевдонима
	CDfnModule* DM = DfnDeclSeq->DfnModuleSeq->FindName(static_cast<const CImportModule*>(BaseIM)->real_name);
	if (!DM) return NULL;	//нет загруженного dfn модуля
	//собственно поиск указанного уточненного имени
	return DM->FindName(search_name);	
}


//-----------------------------------------------------------------------------
//поиск имени в таблице имен, NULL - если имя не найдено
CBaseName* CDfnModule::FindName(const char* search_name) const
{
	if (DfnDeclSeq) return DfnDeclSeq->FindName(search_name);
	return NULL;
}


//-----------------------------------------------------------------------------
//конструктор объекта CDfnModuleSystem
CDfnModuleSystem::CDfnModuleSystem(const CBaseName* parent) : CDfnModule(parent)
{
	name_id = id_CDfnModuleSystem;
}//CDfnModuleSystem


//-----------------------------------------------------------------------------
//процедура инициализации модуля SYSTEM (спец. объекты генерируются без .dfn)
int CDfnModuleSystem::Init(const CProject *project, CLexBuf *lb)
{
	//вызов базовой процедуры инициализации
	CDfnModule::Init(project, lb);

	//создание спец. объектов

	//создание типа PTR
	CBaseType *BT = new CPtrType(this);
	AddName(BT);

	return 0;
}


//-----------------------------------------------------------------------------
//инициализация объекта PROCEDURE из потока лексем в dfn модуле
int CDfnDeclSeq::ProcInit(CLexBuf *lb)
{
	DECL_SAVE_POS
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;
	//создаваемый объект
	CBaseName *BaseName = NULL;

	//проверка отсутствия кл. слова ^
	if (!lb->ReadLex(li) || lex_k_up_arrow != li.lex) {
		//инициализация процедуры
		RESTORE_POS
		BaseName = new CDfnProcedure(parent_element);

		//инициализация процедуры и проверка отсутствия обобщений в ее параметрах
		int err_num = BaseName->Init(lb);
		if (err_num && (s_m_CommonProcFound != err_num)) {
			delete BaseName;
			return err_num;
		}

		//проверка обнаружения обобщенной процедуры
		if (s_m_CommonProcFound == err_num) {
			//найдена обобщающая процедура или обработчик
			delete BaseName;
			//попытка инициализации обобщающей параметрической процедуры
			RESTORE_POS
			BaseName = new CDfnCommonProc(parent_element);
			err_num = BaseName->Init(lb);
			//в DFN не допускается наличие обработчика
			if (s_m_HProcFound == err_num) err_num = s_e_HProcInDfn;
			//проверка наличия ошибок при обработке обобщенной процелуры
			if (err_num) {
				delete BaseName;
				return err_num;
			}
		}

		//занесение считанного объекта в список
		AddName(BaseName);
	} else	//найдено опережающее объявление (^)
		return s_e_ForwardDeclDfnFile;

	return 0;
}//CDfnDeclSeq::ProcInit


//-----------------------------------------------------------------------------
//конструктор объекта CDfnProcedure
CDfnProcedure::CDfnProcedure(const CBaseName* parent) : CProcedure(parent)
{
	name_id = id_CDfnProcedure;
}


//-----------------------------------------------------------------------------
//инициализация объекта CDfnProcedure
int CDfnProcedure::Init(CLexBuf *lb)
{
	DECL_SAVE_POS
	//буфер для чтения информации о текущей лексеме
	CLexInfo li;
	//переменная для получения номера ошибки
	int err_num;

	//проверка наличия приемника
	if (!lb->ReadLex(li) || lex_k_op_bracket != li.lex) {
		RESTORE_POS
	} 
	else {
		Receiver = new CReceiver(this);
		err_num = Receiver->Init(lb, this);
		if (err_num) return err_num;
	}

	//проверка наличия имени и занесение его в текущий объект
	CIdentDef IdentDef(parent_element, false);
	if (err_num = IdentDef.Init(lb)) return err_num;
	IdentDef.Assign(this);

	//проверка наличия списка обобщенных параметров (тогда это обобщенная прцедура или обработчик)
	SAVE_POS
	if (!lb->ReadLex(li) || lex_k_op_brace == li.lex)
		return s_m_CommonProcFound;
	else {
		RESTORE_POS
	}

	//создание списка формальных параметров
	FormalPars = new CFormalPars;
	err_num = FormalPars->Init(lb, this);
	if (err_num) return err_num;

	//в случае наличия приемника необходимо занести ссылку на процедуру в таблицу имен
	//типа, с которым она связана
	if (Receiver) {
		//поиск именованного типа запись (должен быть - проверено в приемнике)
		CBaseName* BN = parent_element->GetGlobalName(Receiver->type_name);
		if (id_CPointerType == BN->name_id) BN = static_cast<CPointerType*>(BN)->FindType();
		//занесение в объект с проверкой отсутствия одноименных полей в объекте
		return static_cast<CRecordType*>(BN)->AddProcReference(this);
	}

	return 0;
}//CDfnProcedure::Init


//-----------------------------------------------------------------------------
//конструктор объекта CDfnCommonProc
CDfnCommonProc::CDfnCommonProc(const CBaseName* parent) : CCommonProc(parent)
{
	name_id = id_CDfnCommonProc;
}


//-----------------------------------------------------------------------------
//инициализация объекта CDfnCommonProc
int CDfnCommonProc::Init(CLexBuf *lb)
{
	//переменная для получения номера ошибки
	int err_num;

	//создание имени IdentDef
	CIdentDef IdentDef(parent_element, false);
	if (err_num = IdentDef.Init(lb)) return err_num;

	//буфер для чтения информации о текущей лексеме
	CLexInfo li;

	//проверка наличия обобщающих параметров
	if (!lb->ReadLex(li) || lex_k_op_brace != li.lex) {
		return s_e_CommonProcCommonParam;
	} else {
		CommonPars = new CCommonPars;
		err_num = CommonPars->Init(lb, this);
		if (err_num) return err_num;
	}

	//создание списка формальных параметров
	FormalPars = new CFormalPars;
	err_num = FormalPars->Init(lb, this);
	if (err_num) return err_num;

	//перенос описания типа возвращаемого результата из обобщенных параметров в формальные (если надо)
	if (CommonPars->Qualident) {
		//проверка двойного объявления типа возвращаемого значения (в случае процедуры - функции)
		if (FormalPars->Qualident) return s_e_CommonProcDoubleType;
		FormalPars->Qualident = CommonPars->Qualident;
		CommonPars->Qualident = NULL;
	}

	//проверка совпадения имен формальных и обобщающих параметров, конвертация записей в обобщенные переменные
	err_num = CheckParams();
	if (err_num) return err_num;

	//занесение найденного имени в текущий объект
	IdentDef.Assign(this);

	return 0;
}//CDfnCommonProc::Init


//-----------------------------------------------------------------------------
//загрузка указанного dfn модуля
int CDeclSeq::LoadDfnModule(const CProject *project, const char* module_name, const char* alias_name)
{
	//поиск dfn модуля в списке каталогов
	char *full_name = new char[project->PathMaxLen + strlen(module_name) + strlen(".dfn_")];
	CProject::Paths_type::const_iterator ci;
	for (ci = project->Paths.begin(); ci != project->Paths.end(); ++ci) {
		strcpy(full_name, *ci);
		strcat(full_name, module_name);
		strcat(full_name, ".dfn");
		FILE *f = fopen(full_name, "r");
		if (f) {
			fclose(f);
			break;
		}
	}
	//проверка наличия найденного модуля
	if (ci == project->Paths.end()) {
		fprintf(output, "ERROR: required file \"%s.dfn\" not found\n", module_name);
		delete[] full_name;
		return s_e_DfnFileNotFound;
	}
	//запуск синт. анализа указанного модуля
	CLexBuf *lb = CLexBuf::AnalyseFile(full_name, project->TabSize);
	if (!lb) {
		delete[] full_name;
		return s_e_DfnFileError;
	}
	//создание нового dfn модуля
	CDfnModule *DM;
	//проверка, для модуля SYSTEM создается объект CDfnModuleSystem
	if (strcmp(module_name, "SYSTEM"))
		DM = new CDfnModule(parent_element);
	else
		DM = new CDfnModuleSystem(parent_element);
	int err_num = DM->Init(project, lb);
	//проверка наличия ошибок
	if (err_num) {
		//вывод сообщения об ошибке
		WriteErr(err_num, lb);
		fprintf(output, "Syntactic analysis of \"%s\" - FAILED\n", full_name);
		//уничтожение временных объектов
		delete lb;
		delete[] full_name;
		delete DM;
		//возврат признака ошибки
		return s_e_DfnFileError;
	}
	//уничтожение буфера лексем
	delete lb;
	delete[] full_name;
	//создание псевдонима модуля
	DM->alias_name = str_new_copy(alias_name);
	//создание (при необходимости) полного пути к модулю
	if (strcmp("DFN/", (*ci))) DM->full_path = str_new_copy(*ci);
	//создание (при необходимости) контейнера dfn модулей
	if (!DfnModuleSeq) DfnModuleSeq = new CDfnModuleSeq(parent_element);
	//добавление эл-та в список загруженных модулей
	DfnModuleSeq->AddModule(DM);

	return 0;
}


//-----------------------------------------------------------------------------
//поиск импортированного имени в таблице имен, NULL - если имя не найдено
CBaseName* CModule::FindImportedName(const char *module_name, const char *search_name) const
{
	//поиск имени (псевдонима) импортированного модуля
	const CBaseName* BaseIM = FindName(module_name);
	if (!BaseIM || (BaseIM->name_id != id_CImportModule))
		return NULL;	//указанный псевдоним не найден
	//проверка наличия импортированных модулей
	if (!DeclSeq->DfnModuleSeq) return NULL;
	//поиск импортированного модуля по реальному имени, полученному из псевдонима
	CDfnModule* DM = DeclSeq->DfnModuleSeq->FindName(static_cast<const CImportModule*>(BaseIM)->real_name);
	if (!DM) return NULL;	//нет загруженного dfn модуля
	//собственно поиск указанного уточненного имени
	return DM->FindName(search_name);	
}


//-----------------------------------------------------------------------------
//проверка завершенности всех объектов, входящих в область деклараций
int CFormalPars::CheckCompleteRoot(CLexBuf *lb)
{
	//для получения номера ошибки
	int err_num;
	//цикл перебора всех переменных (в формальных параметрах могут быть только объявления переменных)
	CBaseVarVector::const_iterator ci;
	for(ci = FPStore.begin(); ci != FPStore.end(); ++ci)
		if (CBaseVar::IsVarId((*ci)->name_id)) {
			err_num = static_cast<CBaseVar*>(*ci)->CheckComplete(lb);
			if (err_num) return err_num;
		}
	//проверка завершена
	return 0;
}


//-----------------------------------------------------------------------------
//поиск указанного имени в списке формальных параметров
CBaseName* CFormalPars::FindName(const char *search_name) const
{
	CBaseVarVector::const_iterator ci;
	for (ci = FPStore.begin(); ci != FPStore.end(); ++ci)
		if ( !strcmp((*ci)->name, search_name) )
			return *ci;
	return NULL;
}


//-----------------------------------------------------------------------------
//поиск имени в списке формальных параметров по номеру
CBaseName* CFormalPars::GetNameByIndex(int index)
{
	if (index >= FPStore.size()) return NULL;
	return FPStore[index];
}


//-----------------------------------------------------------------------------
//запись в .dfn файл
void CImportModule::WriteDFN(DFN_file &f)
{
	if (strcmp(name, real_name)) fprintf(f.f, "%s := ", name);
	fprintf(f.f, "%s", real_name);
}


//-----------------------------------------------------------------------------
//запись кода переменной именованного типа в параметры процедуры
void CBaseVar::WriteCPP_fp_named_type(CPP_files &f, const bool to_h)
{
	if (!to_h) {//запись кода в .cpp файл (с добавлением "&" для VAR переменных)
		if (type_module_name) fprintf(f.fc, "%s::", type_module_name);
		fprintf(f.fc, "%s %s%s", type_name, is_var ? "&" : "", name);
	}
	else {//запись кода в .h файл
		if (type_module_name) {
			//поиск модуля по его псевдониму
			CBaseName* BN = parent_element->GetGlobalName(type_module_name);
			//запись префикса по настоящему имени модуля
			if ( BN && (BN->name_id == id_CImportModule) )
				fprintf(f.fh, "%s::", static_cast<CImportModule*>(BN)->real_name);
		}
		//запись типа и названия переменной (и "&" для VAR переменной)
		fprintf(f.fh, "%s %s%s", type_name, is_var ? "&" : "", name);
	}
}


//-----------------------------------------------------------------------------
//вычисление максимального количества размерностей массива
long CArrayType::GetDimCount()
{
	long DimCount = 1;
	CArrayType *AT = static_cast<CArrayType*>(Type);
	while (AT->name_id == id_CArrayType) {
		++DimCount;
		AT = static_cast<CArrayType*>(AT->Type);
	}
	return DimCount;
}


//-----------------------------------------------------------------------------
//получение результата выражения для указанного измерения, отсчет измерений с 0
//при недопустимом количестве измерений возврат id_CBaseName
EName_id CArrayType::GetResultId(int dimension) const
{
	//перебор пока есть измерения, или пока не достигли указанного измерения
	CBaseType* BT = const_cast<CArrayType*>(this);
	while (id_CArrayType == BT->name_id && dimension) {
		--dimension;
		BT = static_cast<CArrayType*>(BT)->Type;
	}
	//проверка случая, когда размерностей меньше чем затребовано
	if (dimension) return id_CBaseName;
	//проверка получения QualidentType
	if (id_CQualidentType == BT->name_id) {
		//получен QualidentType, извлекаем из него тип
		int err_num = static_cast<CQualidentType*>(BT)->GetNamedType(BT, false);
		if (err_num) return id_CBaseName;
	}
	//получение id результата на основании типа
	return BT->GetResultId();
}


//-----------------------------------------------------------------------------
//получение ук. на тип эл-тов массива (последний тип в цепочке типов, не являющийся массивом)
CBaseType* CArrayType::FindLastType() const
{
	CBaseType *BT = Type;
	while (BT->name_id == id_CArrayType) BT = static_cast<CArrayType*>(BT)->Type;
	return BT;
}


//-----------------------------------------------------------------------------
//получение типа для указанной размерности массива, отсчет размерностей с 0
//при указании недопустимой размерности возврат NULL
CBaseType* CArrayType::GetType(const int dimension)
{
	//начало поиска с текущего объекта
	CBaseType* BT = this;
	//перебор типов с подсчетом номера размерности
	int i = 0;
	while (i < dimension) {
		i++;
		BT = static_cast<CArrayType*>(BT)->Type;
		if (id_CArrayType != BT->name_id) break;
	}
	//проверка, что цепочка типов не кончилась раньше достижения требуемой размерности
	if (dimension != i) return NULL;
	//проверка нахождения именованного типа (требуется вызов метода поиска типа)
	if (id_CQualidentType == BT->name_id)
		static_cast<CQualidentType*>(BT)->GetNamedType(BT, false);
	//выдача найденного типа
	return BT;
}

