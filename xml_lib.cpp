///////////////////////////////////////////////////////
// Набор классов для разбора файла в формате XML 1.0 //
// часть проекта Link2M                              //
// версия библиотеки: см. xml_lib.h                  //
///////////////////////////////////////////////////////

#include "xml_lib.h"

using namespace xml_lib;


//-----------------------------------------------------------------------------
//чтение из указанного файла символа с пропуском пробельных символов
bool xml_lib::freadc(FILE* f, char& ch)
{
	while (true) {
		ch = fgetc(f);
		if (feof(f)) return false;
		switch (ch) {
		case ' ':
		case '\t':
		case 0x0D:
		case 0x0A:
			continue;
		}
		return true;
	}
}


//-----------------------------------------------------------------------------
//чтение из указанного файла слова (цифры/буквы) с пропуском пробельных символов,
//читается также первый не пробельный символ за идентификатором
bool xml_lib::freads(FILE* f, char* buf, char& ch)
{
	if (!freadc(f, buf[0])) return false;
	if (!isalnum(buf[0])) return false;
	int i = 1;
	while (!feof(f) && i < BufSize) {
		buf[i] = fgetc(f);
		if (!isalnum(buf[i])) {
			ch = buf[i];
			buf[i] = 0;
			switch (ch) {
			case ' ':
			case '\t':
			case 0x0D:
			case 0x0A:
				if (!freadc(f, ch)) return false;
			}
			return true;
		}
		i++;
	}
	return false;
}


//-----------------------------------------------------------------------------
//чтение из файла с проверкой совпадения указанной послед-ти символов
bool xml_lib::freadb(FILE* f, const char* st)
{
	char buf[BufSize];
	unsigned int len = strlen(st);
	if (len != fread(&buf, sizeof(char), len, f)) return false;
	buf[len] = 0;
	if (strcmp(st, buf)) return false;
	return true;
}


//-----------------------------------------------------------------------------
//пропуск тэга комментария (смиволы "<!" должны быть уже считаны)
bool xml_lib::SkipComment(FILE* f)
{
	if (!freadb(f, "--")) return false;
	char ch;
	while (true) {
		if (!freadc(f, ch)) return false;
		if ('-' == ch) {
			if (feof(f)) return false;
			if ('-' != fgetc(f)) continue;
			if (feof(f)) return false;
			if ('>' != fgetc(f)) continue;
			return true;
		}
	}//while
}


//-----------------------------------------------------------------------------
//преобразование записанного в файле числа в символ (переполнение символа игнорируется)
bool xml_lib::ReadNumber(FILE* f, char& ch)
{
	//чтение текущего символа
	ch = fgetc(f);
	if (feof(f)) return false;

	int n = 0;
	int num = 0;

	//обработка числа в зависимости от типа (десятичное/шестнадцатеричное)
	if ('x' == ch) {
		//обработка шестнадцатеричного числа
		ch = fgetc(f);
		if (';' == ch) return false;
		while (!feof(f)) {
			switch (ch) {
			case '0': num = 0; break;
			case '1': num = 1; break;
			case '2': num = 2; break;
			case '3': num = 3; break;
			case '4': num = 4; break;
			case '5': num = 5; break;
			case '6': num = 6; break;
			case '7': num = 7; break;
			case '8': num = 8; break;
			case '9': num = 9; break;
			case 'a':
			case 'A': num = 10; break;
			case 'b':
			case 'B': num = 11; break;
			case 'c':
			case 'C': num = 12; break;
			case 'd':
			case 'D': num = 13; break;
			case 'e':
			case 'E': num = 14; break;
			case 'f':
			case 'F': num = 15; break;
			case ';':
				ch = n;
				return true;
			default:
				return false;
			}
			n = 16 * n + num;
			//получение след. символа
			ch = fgetc(f);
		}//while
	} else {
		//обработка десятичного числа
		if (';' == ch) return false;
		while (!feof(f)) {
			switch (ch) {
			case '0': num = 0; break;
			case '1': num = 1; break;
			case '2': num = 2; break;
			case '3': num = 3; break;
			case '4': num = 4; break;
			case '5': num = 5; break;
			case '6': num = 6; break;
			case '7': num = 7; break;
			case '8': num = 8; break;
			case '9': num = 9; break;
			case ';':
				ch = n;
				return true;
			default:
				return false;
			}
			n = 10 * n + num;
			//получение след. символа
			ch = fgetc(f);
		}//while
	}//else

	//не найден завершающий символ ";"
	return false;
}


//-----------------------------------------------------------------------------
//загрузка строкового параметра (символ '"' (или '>') должен быть уже считан)
bool xml_lib::ReadText(FILE* f, char* buf, char& ch)
{
	int i = 0;
	while (i < BufSize) {
		//чтение текущего символа
		ch = fgetc(f);
		if (feof(f)) return false;
		//проверка получения закрывающей кавычки '"' или начала тэга '<'
		if ('"' == ch || '<' == ch) {
			buf[i] = 0;
			return true;
		}
		//проверка наличия "&" (начало "&amp;", "&lt;", "&gt;", "&quot;", "&#<число>;" или "&#x<шестн. число>;")
		if ('&' == ch) {
			//чтение след. символа
			ch = fgetc(f);
			if (feof(f)) return false;
			//проверка считанного символа
			switch (ch) {
			case 'a':
				if (!freadb(f, "mp;")) return false;
				buf[i++] = '&';
				continue;
			case 'l':
				if (!freadb(f, "t;")) return false;
				buf[i++] = '<';
				continue;
			case 'g':
				if (!freadb(f, "t;")) return false;
				buf[i++] = '>';
				continue;
			case 'q':
				if (!freadb(f, "uot;")) return false;
				buf[i++] = '"';
				continue;
			case '#':
				if (!ReadNumber(f, ch)) return false;
				buf[i++] = ch;
				continue;
			default:
				return false;
			}//switch
		}
		//сохранение символа и переход к след. итерации
		buf[i++] = ch;
		continue;
	}//while
	//переполнение буфера
	return false;
}


//-----------------------------------------------------------------------------
//конструктор
CXMLAttribute::CXMLAttribute(const char* newName, const char* newValue)
{
	//имя параметра всегда должно присутствовать
	Name = new char[strlen(newName) + 1];
	strcpy(Name, newName);
	//значение параметра может отсутствовать (состоять из одного символа '\0')
	if ('\0' == newValue[0])
		Value = NULL;
	else {
		Value = new char[strlen(newValue) + 1];
		strcpy(Value, newValue);
	}//else
};


//-----------------------------------------------------------------------------
//сохранение текста внутри тэга (<TagName>Text<InternalTag/>more text</TagName>)
void CXMLEntry::AddContent(const char *buf)
{
	//проверка наличия вложенных тэгов
	if (XMLEntryStore.empty()) {
		//текст внутри тэга до появления вложенных тэгов
		Content = new char[strlen(buf) + 1];
		strcpy(Content, buf);
	} else	//текст после появления вложенных тэгов
		(XMLEntryStore.back())->AddExtContent(buf);
}


//-----------------------------------------------------------------------------
//сохранение текста, следующего после тэга (вызывается родительским тэгом)
void CXMLEntry::AddExtContent(const char *buf)
{
	//на случай ошибочного повторного вызова
	delete[] ExtContent;
	//сохранение переданного текста
	ExtContent = new char[strlen(buf) + 1];
	strcpy(ExtContent, buf);
}


//-----------------------------------------------------------------------------
//получение значения атрибута (или NULL) по имени
const char* CXMLEntry::AttribValue(const char *AttribName) const
{
	TAttribStore::const_iterator ci;
	for (ci = AttribStore.begin(); ci != AttribStore.end(); ci++)
		if (!strcmp(AttribName, (*ci)->Name))
			return (*ci)->Value;
	return NULL;
}


//-----------------------------------------------------------------------------
//деструктор
CXMLEntry::~CXMLEntry() {
	//очистка строковых переменных
	delete[] TagName;
	delete[] ExtContent;
	delete[] Content;
	//очистка списка пар параметр-значение
	TAttribStore::const_iterator ci_at;
	for (ci_at = AttribStore.begin(); ci_at != AttribStore.end(); ci_at++)
		delete *ci_at;
	//очистка списка xml эл-тов
	TXMLEntryStore::const_iterator ci_x;
	for (ci_x = XMLEntryStore.begin(); ci_x != XMLEntryStore.end(); ci_x++)
		delete *ci_x;
};


//-----------------------------------------------------------------------------
//загрузка xml эл-та, ch - первый символ открывающего тэга (должен быть уже проверен)
bool CXMLEntry::Init(FILE *f, char& ch)
{
	//загрузка оставшихся символов тэга (1-й символ уже загружен в ch)
	char buf[BufSize];
	buf[0] = ch;
	if (!freads(f, &buf[1], ch)) return false;
	TagName = new char[strlen(buf) + 1];
	strcpy(TagName, buf);

	//проверка символа, идущего после тэга

	//проверка наличия буквы (тогда это пара параметр-значение)
	while (isalpha(ch)) {
		//загрузка оставшихся символов названия параметра
		buf[0] = ch;
		if (!freads(f, &buf[1], ch)) return false;
		//сохранение названия параметра
		char nm[BufSize];
		strcpy(nm, buf);
		//проверка наличия "=" и '"'
		if ('=' != ch) return false;
		if (!freadc(f, ch)) return false;
		if ('"' != ch) return false;
		//загрузка строкового параметра (значения)
		if (!ReadText(f, buf, ch)) return false;
		//проверка наличия '"' и сохранение пары в списке
		if ('"' != ch) return false;
		CXMLAttribute* attr = new CXMLAttribute(nm, buf);
		AttribStore.push_back(attr);
		//получение след. символа
		if (!freadc(f, ch)) return false;
	}//while

	//проверка наличия "/" (отсутствуют вложенные xml эл-ты)
	if ('/' == ch) {
		if (!freadb(f, ">")) return false;
		return true;
	}

	//проверка наличия ">" (могут присутствовать вложенные xml эл-ты)
	if ('>' != ch) return false;

	//поиск вложенных xml эл-тов
	while (!feof(f)) {
		//поиск "<" учитывая возможность наличия текста
		if (!ReadText(f, buf, ch)) return false;
		if ('<' != ch) return false;
		//текст добавляется только если он присутствует (есть символы кроме '\0')
		if ('\0' != buf[0]) AddContent(buf);
		//проверка символа, следующего за "<"
		if (feof(f)) return false;
		ch = fgetc(f);
		//проверка обнаружения закрывающего тэга
		if ('/' == ch) {
			if (!freadb(f, TagName)) return false;
			if (!freadc(f, ch)) return false;
			if ('>' != ch) return false;
			return true;
		}
		//проверка наличия комментария
		if ('!' == ch) {
			SkipComment(f);
			continue;
		}
		//проверка наличия начала идентификатора (тэга)
		if (!isalpha(ch)) return false;
		//загрузка вложенного xml тэга док-та
		CXMLEntry* xmle = new CXMLEntry;
		if (!xmle->Init(f, ch)) {
			delete xmle;
			return false;
		}
		//занесение загруженного xml эл-та в список
		XMLEntryStore.push_back(xmle);
	}//while

	return false;
}


//-----------------------------------------------------------------------------
//загрузка файла в формате XML
EXMLErrors CXMLFile::Load(const char *fname)
{
	//буфер для чтения информации
	char buf[BufSize];
	char ch;

	//открытие файла для чтения
	FILE* f = fopen(fname, "r");
	if (!f) return EXMLOpen;

	//чтение xml заголовка
	if (!freadb(f, "<?xml")) goto err;
	if (!freads(f, buf, ch)) goto err;
	if (strcmp("version", buf)) goto err;
	if ('=' != ch) goto err;
	if (!freadc(f, ch)) goto err;
	if ('"' != ch) goto err;
	if (!freadb(f, "1.0")) goto err;
	if (!freadc(f, ch)) goto err;
	if ('"' != ch) goto err;
	if (!freadc(f, ch)) goto err;
	if ('?' != ch) goto err;
	if (!freadb(f, ">")) goto err;

	//поиск описания главного тэга документа
	while (true) {
		//поиск "<"
		if (!freadc(f, ch)) goto err;
		if ('<' != ch) goto err;
		//получение символа, следующего за "<"
		if (feof(f)) goto err;
		ch = fgetc(f);
		//проверка наличия комментария
		if ('!' == ch) {
			SkipComment(f);
			continue;
		}
		//проверка наличия начала идентификатора (тэга)
		if (!isalpha(ch)) goto err;
		//загрузка главного тэга док-та (если не комментарий)
		XMLEntry = new CXMLEntry;
		if (XMLEntry->Init(f, ch))
			break;
		else
			goto err;
	}//while

	//поиск комментариев после главного тэга док-та
	while (true) {
		//поиск "<" (если нет - конец обработки файла)
		if (!freadc(f, ch)) break;
		if ('<' != ch) goto err;
		if (feof(f)) goto err;
		if ('!' != fgetc(f)) goto err;
		SkipComment(f);
	}//while

	//All OK
	if (fclose(f)) return EXMLClose;
	return EXMLNoError;

err:
	//неверный формат файла
	if (fclose(f)) return EXMLClose;
	return EXMLFormat;
}


