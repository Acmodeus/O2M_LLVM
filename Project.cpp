///////////////////////////////////////////////////////////////////////////
// Класс для работы с настройками файлов проектов O2M, часть проекта O2M //
///////////////////////////////////////////////////////////////////////////

#include "Project.h"
#include <stdlib.h>

static const int Max_St_Len = 10000;

//название O2M makefile
static const char* const O2M_makefile = "CPP/_O2M_make.2mk";


//-----------------------------------------------------------------------------
//конструктор класса проекта
CProject::CProject() : TabSize(1), EXEC(NULL), MAIN(NULL)
{
	//загрузка в список директории по умолчанию (/DFN)
	char *st = new char[strlen("DFN/_")];
	strcpy(st, "DFN/");
	Paths.push_back(st);
	PathMaxLen = strlen(st);
}


//-----------------------------------------------------------------------------
//деструктор класса проекта
CProject::~CProject()
{
	delete[] MAIN;
	delete[] EXEC;
	//очистка списка директорий
	Paths_type::const_iterator cpi;
	for (cpi = Paths.begin(); cpi != Paths.end(); ++cpi)
		delete[] (*cpi);
}


//-----------------------------------------------------------------------------
//оболочка для вызова AddMakeFile в случае использования полного пути к файлу
void CProject::AddFullPathMakeFile(const char *FullPath, const char *ModuleName)
{
	//поиск начала имени файла
	int i;
	for (i = strlen(FullPath); i >= 0; i--)
		if ('/' == FullPath[i] || '\\' == FullPath[i]) break;
	//получение пути к файлу (если есть)
	char *path = NULL;
	if (0 <= i) {
		++i;
		path = new char[i + 1];
		strncpy(path, FullPath, i);
		path[i] = '\0';
	}
	//добавление файла в список
	AddMakeFile(path, ModuleName);
	//освобождение памяти
	delete[] path;
}


//-----------------------------------------------------------------------------
//добавление файла в список файлов с проверкой отсутствия дублирования
void CProject::AddMakeFile(const char *FilePath, const char *ModuleName)
{
	const char* Name;
	const char* Path;
	//проверка отсутствия повторных вхождений (повторно файл в список не заносится)
	CXMLEntry::TXMLEntryStore::const_iterator ci;
	for (ci = xmlFile.XMLEntry->XMLEntryStore.begin(); ci != xmlFile.XMLEntry->XMLEntryStore.end(); ++ci) {
		//проверка требуемого типа XML эл-та
		if (strcmp("File", (*ci)->TagName)) continue;
		//поиск требуемых атрибутов XML эл-та (для скорости)
		Name = (*ci)->AttribValue("Name");
		Path = (*ci)->AttribValue("Path");
		//собственно сравнение имени файла и пути
		if (!strcmp(Name, ModuleName)) {
			if (!Path && !FilePath) return;
			if (Path && FilePath && !strcmp(Path, FilePath)) return;
		}
	}
	//добавление файла в список
	//подготовка ук. на путь (в конструктор XML атрибута нельзя передавать NULL)
	Path = FilePath ? FilePath : "\0";
	//создание  и настройка XML эл-та (занесение в него атрибутов с именем файла и путем)
	CXMLEntry* xmle = new CXMLEntry;
	xmle->TagName = str_new_copy("File");
	xmle->AttribStore.push_back(new CXMLAttribute("Name", ModuleName));
	xmle->AttribStore.push_back(new CXMLAttribute("Path", Path));
	//занесение XML эл-та в список
	xmlFile.XMLEntry->XMLEntryStore.push_back(xmle);
}


//-----------------------------------------------------------------------------
//загрузка файла проекта
int CProject::Init(const char *f_name)
{
	//открытие файлп проекта для чтения
	FILE *f = fopen(f_name, "r");
	if (!f) return 1;

	//буфер для чтения одной строки из файла проекта
	char st[Max_St_Len];
	//переменные для сохранения координат названия параметра в считанной строке
	int var_beg, var_len;
	//переменные для сохранения координат значения в считанной строке
	int val_beg, val_len;

	//цикл построчного чтения файла проекта
	while (fgets(st, Max_St_Len, f)) {

		//выделение названия параметра и значения
		//пропуск пробельных символов
		int beg = 0;
		while ('\t' == st[beg] || ' ' == st[beg]) beg++;
		//пропуск комментариев
		if (';' == st[beg] || '#' == st[beg]) continue;
		//найдено начало названия параметра, ищем его конец
		int end = beg;
		while (isalnum(st[end]) || '_' == st[end]) end++;
		//проверка наличия названия параметра
		if (end == beg) continue;
		//запоминаем координаты названия параметра
		var_beg = beg;
		var_len = end - beg;
		//пропуск пробельных символов
		beg = end;
		while ('\t' == st[beg] || ' ' == st[beg]) beg++;
		//проверка наличия символа '='
		if ('=' != st[beg++]) continue;
		//пропуск пробельных символов
		while ('\t' == st[beg] || ' ' == st[beg]) beg++;
		//поиск координаты конца значения
		end = strlen(st) - 1;
		switch (st[end]) {
		case '\t':
		case ' ':
		case '\r':
		case '\n':
			--end;
		}
		//проверка наличия открывающей кавычки
		if ('"' == st[beg] || '\'' == st[beg]) {
			//проверка наличия закрывающей кавычки, соответствующей открывающей
			if (st[beg] != st[end]) continue;
			//корректировка координаты начала с учетом кавычки
			++beg;
		} else
			++end;	//end - за последний символ строки
		//проверка наличия значения (пустое значение является допустимым)
		if (end < beg) continue;
		//запоминаем координаты значения
		val_beg = beg;
		val_len = end - beg;
		//добавление завершающих символов '\0' для названия параметра и значения
		st[var_beg + var_len] = '\0';
		st[end] = '\0';

		//обработка полученных параметра и значения
		//преобразование названия параметра в верхний регистр
		for (beg = var_beg; st[beg]; beg++) st[beg] = toupper(st[beg]);
		//проверка первого символа названия параметра (для скорости)
		switch (st[var_beg]) {
		case 'M':	//MAIN
			if (!strcmp(st + var_beg + 1, "AIN")) {
				//очищаем строку на случай повторного объявления
				delete[] MAIN;
				//сохраняем имя главного файла с преобразованием в верхний регистр
				MAIN = new char[val_len + 1];
				for (beg = 0; beg <= val_len; beg++) MAIN[beg] = toupper(st[val_beg + beg]);
			}
			break;
		case 'E':	//EXEC
			if (!strcmp(st + var_beg + 1, "XEC")) {
				//очищаем строку на случай повторного объявления
				delete[] EXEC;
				//убираем .exe в конце имени исполняемого файла (если есть)
				const char* ste = st + val_beg;
				if (4 < val_len &&
					'.' == ste[val_len - 4] &&
					'E' == toupper(ste[val_len - 3]) &&
					'X' == toupper(ste[val_len - 2]) &&
					'E' == toupper(ste[val_len - 1])
					) val_len -= 4;
				//сохраняем имя исполняемого файла
				EXEC = new char[val_len + 1];
				strncpy(EXEC, ste, val_len);
				EXEC[val_len] = '\0';
			}
			break;
		case 'I':	//IMPORT
			if (!strcmp(st + var_beg + 1, "MPORT")) {
				char *tmp = new char[val_len + 1];
				strcpy(tmp, st + val_beg);
				Paths.push_back(tmp);
			}
			break;
		case 'T':	//TABSIZE
			if (!strcmp(st + var_beg + 1, "ABSIZE")) {
				//преобразование строки в число
				TabSize = atoi(st + val_beg);
				//проверка корректности полученного размера табуляции
				if (0 > TabSize || (0 == TabSize && ('0' != st[val_beg] || 0 != st[val_beg + 1])))
					TabSize = 1;	//установка TabSize по умолчанию в случае некорректного значения
			}
			break;
		}//switch

	}//while

	//вычисление наибольшей длины пути
	Paths_type::const_iterator i;
	for (i = Paths.begin(); i != Paths.end(); ++i) {
		int NewLen = strlen(*i);
		if (NewLen > PathMaxLen) PathMaxLen = NewLen;
	}

	return 0;
}


//-----------------------------------------------------------------------------
//проверка, является ли указанный файл главным файлом проекта
bool CProject::IsMainFile(const char *file_name)
{
/*
	//возможно, в file_name требуется отделять путь к файлу от имени файла?

	//поиск конца расширения файла
	const int len = strlen(file_name);
	int i;
	for (i = len; i >= 0; i--)
		if ('.' == file_name[i]) break;
	if (0 > i) return false;
	int dot = i;
	//поиск начала имени файла
	int beg;
	for (i = dot; i >= 0; i--)
		if ('/' == file_name[i] || '\\' == file_name[i]) break;
	beg = i;
	//копирование пути и имени файла
	char *path = NULL;
	if (0 < beg) {
		path = str_new_copy(file_name);
		path[beg] = '\0';
	}
	//получение названия обрабатываемого файла без расширения
	char *file = str_new_copy(file_name + beg + 1);
	file[dot] = '\0';
	//получение названия обрабатываемого файла с расширением
	char* file_ext = str_new_copy(file_name + beg + 1);
*/

	//проверка наличия обоих названий файлов
	if (!file_name || !MAIN) return false;
	//получение названия обрабатываемого файла в UPPER CASE
	char* upper_f_n = NULL;
	if (file_name) {
		upper_f_n = str_new_copy(file_name);
		for (int i = 0; i < strlen(upper_f_n); i++) upper_f_n[i] = toupper(upper_f_n[i]);
	}
	//проверка совпадения имен
	if (strcmp(upper_f_n, MAIN ? MAIN : "")) {
		delete[] upper_f_n;
		return false;
	}
	delete[] upper_f_n;
	return true;
}


//-----------------------------------------------------------------------------
//создание (перезапись) O2M makefile
bool CProject::Create2mk()
{
	//создание главного XML эл-та
	if (!xmlFile.XMLEntry) xmlFile.XMLEntry = new CXMLEntry;
	return true;
}


//-----------------------------------------------------------------------------
//открытие O2M makefile (если есть)
bool CProject::Load2mk()
{
	//чтение xml файла с перечнем файлов проекта _O2M_make.2mk (если есть)
	switch (xmlFile.Load(O2M_makefile)) {
	case EXMLNoError:
		//проверка допустимости содержимого файла (наличие эл-та Project)
		if (strcmp("Project", xmlFile.XMLEntry->TagName)) {
			fprintf(output, "ERROR: Incorrect O2M makefile (\"%s\") content\n", O2M_makefile);
			break;
		}
		return true;
	case EXMLOpen:
		//при ошибке открытия файл будет создаваться заново
		return Create2mk();
	case EXMLFormat:
		fprintf(output, "ERROR: Incorrect O2M makefile (\"%s\") format\n", O2M_makefile);
		break;
	case EXMLClose:
		fprintf(output, textCannotClose, O2M_makefile);
		break;
	default:
		fprintf(output, "ERROR: Unknown XML error\n");
		break;
	}//switch
	//признак неудачного чтения
	return false;
}


//-----------------------------------------------------------------------------
//обработка файла O2M makefile для компоновщика (Link2M) и сборщика проекта (Make2M)
bool CProject::Write2mk(const bool is_main, const char *ModuleName)
{
	//ук. на имя исполняемого файла
	const char* ExecName;

	//проверка, является ли файл главным файлом проекта
	if (is_main)	//имя исполняемого файла берется из настроек или из имени модуля
		ExecName = EXEC ? EXEC : ModuleName;
	else {			//установка имени исп. файла и назв. модуля из O2M makefile (если есть)
		ModuleName = xmlFile.XMLEntry->AttribValue("Main");
		if (!ModuleName) ModuleName = "";
		ExecName = xmlFile.XMLEntry->AttribValue("Exec");
		if (!ExecName) ExecName = "";
	}

	//создание файла O2M makefile для компоновщика (Link2M) и сборщика проекта (Make2M)
	FILE *ff = fopen(O2M_makefile, "w");
	if (!ff) {
		fprintf(output, textCannotOpenW, O2M_makefile);
		return false;
	}

	//запись заголовка в 2mk файл
	fprintf(ff, "<?xml version=\"1.0\" ?>\n<!-- %s -->\n<Project Main=\"%s\" Exec=\"%s\">\n", comment_title, ModuleName, ExecName);
	//запись названий всех c++ файлов проекта, требующих компиляции (кроме главного и файлов _O2M_main, _O2M_sys)
	CXMLEntry::TXMLEntryStore::const_iterator ci;
	for (ci = xmlFile.XMLEntry->XMLEntryStore.begin(); ci != xmlFile.XMLEntry->XMLEntryStore.end(); ++ci) {
		if (strcmp("File", (*ci)->TagName)) continue;
		const char* Path = (*ci)->AttribValue("Path");
		fprintf(ff, "\t<File Name=\"%s\" Path=\"%s\" />\n", (*ci)->AttribValue("Name"), Path ? Path : "");
	}
	//запись закрывающего тэга в 2mk файл
	fprintf(ff, "</Project>\n");

	//закрытие файла
	if (fclose(ff)) {
		fprintf(output, textCannotClose, O2M_makefile);
		return false;
	}

	return true;
}

