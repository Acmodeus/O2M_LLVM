///////////////////////////////////////////////////////
// Набор классов для разбора файла в формате XML 1.0 //
// часть проекта Link2M                              //
// версия библиотеки: 1.0.003 от 2004.08.26          //
///////////////////////////////////////////////////////

#ifndef xml_lib_h_file
#define xml_lib_h_file

#include <ctype.h>
#include <stdio.h>
#include <vector>
#include <cstring>

namespace xml_lib {

//размер буфера для чтения из файла блока информации (имени тэга, параметра, значения)
const int BufSize = 1000;

//чтение из указанного файла символа с пропуском пробельных символов
bool freadc(FILE* f, char& ch);
//чтение из указанного файла слова (цифры/буквы) с пропуском пробельных символов,
//читается также первый не пробельный символ за идентификатором
bool freads(FILE* f, char* buf, char& ch);
//чтение из файла с проверкой совпадения указанной послед-ти символов
bool freadb(FILE* f, const char* st);
//пропуск тэга комментария (смиволы "<!" должны быть уже считаны)
bool SkipComment(FILE* f);
//загрузка строкового параметра (символ '"' (или '>') должен быть уже считан)
bool ReadText(FILE* f, char* buf, char& ch);
//преобразование записанного в файле числа в символ (переполнение символа игнорируется)
bool ReadNumber(FILE* f, char& ch);

}//namespace


//-----------------------------------------------------------------------------
//коды ошибок
enum EXMLErrors {
	EXMLNoError,
	//код ошибки открытия xml файла
	EXMLOpen,
	//код ошибки закрытия xml файла
	EXMLClose,
	//код ошибки - неверный формат файла
	EXMLFormat
};


//-----------------------------------------------------------------------------
//тип пары параметр-значение одного xml эл-та
class CXMLAttribute {
public:
	CXMLAttribute(const char* newName, const char* newValue);
	~CXMLAttribute() {
		delete[] Name;
		delete[] Value;
	};
	//название параметра
	char* Name;
	//значение параметра (может отсутствовать)
	char* Value;
};


//-----------------------------------------------------------------------------
//класс для поддержки одного xml эл-та
class CXMLEntry {
	void AddContent(const char* buf);
public:
	void AddExtContent(const char* buf);
	//получение значения атрибута (или NULL) по имени
	const char* AttribValue(const char* AttribName) const;
	//конструктор
	CXMLEntry() : Content(NULL), ExtContent(NULL), TagName(NULL) {};
	~CXMLEntry();
	bool Init(FILE* f, char& ch);
	//тип списка пар параметр-значение
	typedef std::vector<CXMLAttribute*> TAttribStore;
	//список пар параметр-значение
	TAttribStore AttribStore;
	//текст внутри тэга (до первого вложенного тэга)
	char* Content;
	//текст, следующий после тэга (добавляется родительским тэгом)
	char* ExtContent;
	//имя xml тэга
	char* TagName;
	//тип списка xml эл-тов
	typedef std::vector<CXMLEntry*> TXMLEntryStore;
	//список xml эл-тов
	TXMLEntryStore XMLEntryStore;
};


//-----------------------------------------------------------------------------
//класс для обработки одного xml файла
class CXMLFile {
public:
	CXMLFile() : XMLEntry(NULL) {};
	~CXMLFile() {
		Clear();
	};
	//очистка содержимого объекта, подготовка к повторному вызову метода Load
	void Clear() {
		delete XMLEntry;
		XMLEntry = NULL;
	};
	//загрузка файла в формате XML
	EXMLErrors Load(const char* fname);
	//главный тэг документа
	CXMLEntry* XMLEntry;
};


#endif	//xml_lib_h_file
