///////////////////////////////////////////////////////////////////////////
// Класс для работы с настройками файлов проектов O2M, часть проекта O2M //
///////////////////////////////////////////////////////////////////////////

#ifndef O2M_Project_h
#define O2M_Project_h

#include "Common.h"
#include "xml_lib.h"


//-----------------------------------------------------------------------------
//класс для хранения настроек проекта
class CProject {
public:
	CProject();
	~CProject();
	//создание (перезапись) O2M makefile
	bool Create2mk();
	//оболочка для вызова AddMakeFile в случае использования полного пути к файлу
	void AddFullPathMakeFile(const char *FullPath, const char *ModuleName);
	//добавление файла в список файлов с проверкой отсутствия дублирования
	void AddMakeFile(const char *FilePath, const char *ModuleName);
	//загрузка конфигурации из указанного файла
	int Init(const char *f_name);
	//проверка, является ли указанный файл главным файлом проекта
	bool IsMainFile(const char* file_name);
	//открытие O2M makefile (если есть)
	bool Load2mk();
	//обработка файла O2M makefile для компоновщика (Link2M) и сборщика проекта (Make2M)
	bool Write2mk(const bool is_main, const char* ModuleName);
	//максимальная длина пути (для буфера)
	int PathMaxLen;
	//список директорий с dfn модулями
	typedef StringVector Paths_type;
	Paths_type Paths;
	//размер табуляции
	int TabSize;
private:
	//имя исполняемого файл проекта (без .exe)
	char *EXEC;
	//главный файл проекта
	char *MAIN;
	//объект для обработки O2M makefile
	CXMLFile xmlFile;
};


#endif //O2M_Project_h
