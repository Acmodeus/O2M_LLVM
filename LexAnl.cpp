//===============================================================
// Описание процедур, необходимых для лексической обработки
//===============================================================

#include "LexAnl.h"


//---------------------------------------------------------------
//определение потока для вывода текстовой информации
TFileType *output = stdout;


//---------------------------------------------------------------
//деструктор буфера лексем
CLexBuf::~CLexBuf()
{
    /*
	//отладка: сохранение содержимого буфера лексем в файле
	FILE *f = fopen("lexbuf.tmp", "w");
	CurrStNum = 1;
	if (f)
		for (CurrLex = Buf.begin(); CurrLex != Buf.end(); CurrLex++) {
			fprintf(f, "---------------------\n");
			switch ((*CurrLex).lex) {
			case lex_n: fprintf(f, "конец строки исх. файла\nst: %i; pos: %i;\n", CurrStNum, (*CurrLex).pos);
				++CurrStNum;
				continue;
			case lex_d: fprintf(f, "десятичное число");
				break;
			case lex_h: fprintf(f, "шестнадцат. число");
				break;
			case lex_r: fprintf(f, "число типа REAL");
				break;
			case lex_l: fprintf(f, "число типа LONGREAL");
				break;
			case lex_i: fprintf(f, "идентификатор");
				break;
			case lex_s: fprintf(f, "строка");
				break;
			case lex_c: fprintf(f, "символьная константа");
				break;
			default:
				fprintf(f, "ключевое слово\nst: %i; pos: %i;\n", CurrStNum, (*CurrLex).pos);
				if (lex_k_TO == (*CurrLex).lex) fprintf(f, "TO\n");
				continue;
			}
			fprintf(f, "\nst: %i; pos: %i;\n\"%s\"\n", CurrStNum, (*CurrLex).pos, (*CurrLex).st);
		}//for
	fclose(f);
	//конец отладки
    //*/

	//очистка содержимого буфера
	for (CurrLex = Buf.begin(); CurrLex != Buf.end(); CurrLex++)
		delete[] (*CurrLex).st;
}


//---------------------------------------------------------------
//чтение информации о лексеме из буфера
bool CLexBuf::ReadLex(CLexInfo &LexInfo)
{
	//цикл пропуска признаков конца строки с подсчетом строк
	while (Buf.end() != CurrLex) {
		//обработка признака конца строки
		if (lex_n == (*CurrLex).lex) {
			//переход к след. строке и след. лексеме
			++CurrLex;
			++CurrStNum;
			continue;
		}
		//найдена лексема - не конец строки
		LexInfo.lex = (*CurrLex).lex;
		LexInfo.st = (*CurrLex).st;
		//текущая позиция - на пред. лексему, т.к. при ошибке сбойная лексема считывается,
		//следовательно текущая лексема будет следующей за сбойной
		CurrStPos = (*CurrLex).pos;
		//установка следующей лексемы в качестве текущей
		++CurrLex;
		return true;
	}
	//сброс позиции на случай обнаружения ошибки в конце файла
	CurrStPos = 1;
	return false;
}


//---------------------------------------------------------------
//установка текущей позиции и номера строки на начало буфера
void CLexBuf::ResetCurrPos()
{
	CurrLex = Buf.begin();
	CurrStNum = 1;
	CurrStPos = 1;
}


//---------------------------------------------------------------
//получение текущей позиции буфера лексем
CLexBufPos CLexBuf::GetCurrPos()
{
	CLexBufPos LBPos;
	LBPos.CurrLex = CurrLex;
	LBPos.CurrStNum = CurrStNum;
	LBPos.CurrStPos = CurrStPos;
	return LBPos;
}


//---------------------------------------------------------------
//установка текущей позиции буфера лексем
void CLexBuf::SetCurrPos(CLexBufPos LexBufPos)
{
	CurrLex = LexBufPos.CurrLex;
	CurrStNum = LexBufPos.CurrStNum;
	CurrStPos = LexBufPos.CurrStPos;
}


//---------------------------------------------------------------
//добавление в буфер лексемы
void CLexBuf::WriteLex(const enum ELexType LexType, const char *st, const int pos)
{
	CLex lex;
	//сохранение типа лексемы
	lex.lex = LexType;
	//сохранение позиции начала лексемы в строке исх. файла и в буфере лексем
	lex.pos = pos;
	CurrStPos = pos;
	//создание строки с содержимым лексемы (если есть)
	if (st) {
		lex.st = new char[strlen(st) + 1];
		strcpy(lex.st, st);
	} else
		lex.st = NULL;
	//занесение лексемы в список
	Buf.push_back(lex);
	//при получении признака конца строки обновление счетчика строк
	if (lex_n == LexType) ++CurrStNum;
}


//---------------------------------------------------------------
//выдача сообщения об ошибке
void WriteErr(int err_num, const CLexBuf *lb)
{
	fprintf(output,"ERROR (st %d, pos %d): %s\n", lb->GetCurrStNum(), lb->GetCurrStPos(),
		(err_num >= s_m_Error && err_num < s_m_MaxErrorCode) ? err_arr[err_num] : err_arr[s_m_ZeroErrorCode]);
};//WriteErr


//---------------------------------------------------------------
//выдача предупреждения (Warning)
void WriteWarn(int warn_num, const CLexBuf *lb)
{
	fprintf(output,"WARNING (st %d, pos %d): %s\n", lb->GetCurrStNum(), lb->GetCurrStPos(),
		(warn_num >= s_m_Error && warn_num < s_m_MaxErrorCode) ? err_arr[warn_num] : err_arr[s_m_ZeroErrorCode]);
};//WriteWarn


//---------------------------------------------------------------
//проверка наличия шестнадцатеричной цифры оберона (0..9, A..F)
static bool isO2Mxdigit(int c) {
	if (isdigit(c) || (c >= 'A' && c <= 'F')) return true;
	return false;
}


//---------------------------------------------------------------
//проверка слишком большого целого числа, true - число слишком велико
inline bool CLexBuf::test_big_number(const char *st, int st_len)
{
	//пропуск нулей в начале числа
	while ('0' == st[0]) {
		st++;
		st_len--;
	}
	//проверка длины числа
	if (st_len < st_LONG_MAX_len) return false;
	if (st_len > st_LONG_MAX_len) return true;
	//проверка значения числа при совпадении длин
	for (int i = 0; i < st_len; i++)
		if (st[i] > st_LONG_MAX[i]) return true; else if (st[i] < st_LONG_MAX[i]) break;
	return false;
}


//---------------------------------------------------------------
//проверка числа в позиции i строки st (buf - буфер)
//если число найдено, i - на последний символ числа
inline int CLexBuf::test_digit (const char *st, char *buf, int &i, const int pos)
{
	int m = i;						//запоминаем текущую позицию
	while (isdigit(st[++i]));		//поиск окончания числа

	//проверка, какой тип числа найден
	switch (st[i]) {
	case 'H':
		//шестнадцатеричное (конец)
		memcpy(buf, &st[m], i - m);
		buf[i - m] = 0;
		WriteLex(lex_h, buf, pos);
		return 0;
	case 'X':
		//символ (конец)
		memcpy(buf, &st[m], i - m);
		buf[i - m] = 0;
		WriteLex(lex_c, buf, pos);
		return 0;
	case '.':
		//проверка наличия доп. точки (keyword "..")
		if (st[i + 1] != '.') {

			//действительное
			while (isdigit(st[++i]));		//поиск окончания числа
			//проверка, какой тип числа найден
			switch (st[i]){
			case 'D':
				//тип LONGREAL
				++i;
				if (st[i] == '+' || st[i] == '-') ++i;
				if (!isdigit(st[i])){		//нет цифры после точки
					return l_e_LastScaleDigit;
				}//if
				while (isdigit(st[++i]));	//поиск окончания числа
				//тип REAL (конец)
				memcpy(buf, &st[m], i - m);
				buf[i - m] = 0;
				--i;						//i - на конец числа
				WriteLex(lex_l, buf, pos);
				return 0;
			case 'E':
				//тип REAL
				++i;
				if (st[i] == '+' || st[i] == '-') ++i;
				if (!isdigit(st[i])){		//нет цифры после точки
					return l_e_LastScaleDigit;
				}//if
				while (isdigit(st[++i]));	//поиск окончания числа
			default:
				//тип REAL (конец)
				memcpy(buf, &st[m], i - m);
				buf[i - m] = 0;
				--i;						//i - на конец числа
				WriteLex(lex_r, buf, pos);
				return 0;
			}//switch

		}//if not '.'

	default:
		//проверка отсутствия HEX цифр
		if (!isO2Mxdigit(st[i])) {
			//десятичное число
			int len = i - m;
			memcpy(buf, &st[m], len);
			buf[len] = 0;
			--i;							//i - на конец числа
			WriteLex(lex_d, buf, pos);
			//проверка слишком большого целого числа
			if (test_big_number(buf, len)) return l_e_NumberTooLarge;
			return 0;
		}
		//число - шестнадцатеричное
		while (isO2Mxdigit(st[++i]));		//поиск окончания числа
		//проверка - символ или HEX
		switch (st[i]){
		case 'H':
			//шестнадцатеричное (конец)
			memcpy(buf, &st[m], i - m);
			buf[i - m] = 0;
			WriteLex(lex_h, buf, pos);
			return 0;
		case 'X':
			//символ (конец)
			memcpy(buf, &st[m], i - m);
			buf[i - m] = 0;
			WriteLex(lex_c, buf, pos);
			return 0;
		}//switch
	}//switch

	//некорректное окончание HEX числа
	return l_e_LastHexDigit;
}//test_digit


//---------------------------------------------------------------
//проверка наличия в строке st (не менее 2 символов, включая 0) ключевого слова
//если найдено кл. слово, прибавление к i и pos длины найденного слова - 1
inline bool CLexBuf::test_keyword(const char *st, int &i, int &pos)
{
	//для проверки типа лексемы
	ELexType lt = lex_i;

	//проверка операций из 2 символов (":=", "+=", "<=", ">=", "..")
	switch (st[1]) {
	case '=':
		switch (st[0]) {
		case ':':
			lt = lex_k_assign;
			break;
		case '+':
			lt = lex_k_add_assign;
			break;
		case '<':
			lt = lex_k_le;
			break;
		case '>':
			lt = lex_k_ge;
		}//switch
		break;
	case '.':
		if ('.' == st[0]) lt = lex_k_dots;
	}//switch

	//проверка обнаружения операции из 2 символов
	if (lex_i != lt) {
		WriteLex(lt, NULL, pos);
		++i;
		++pos;
		return true;
	}

	//проверка символов
	switch (st[0]) {
	case ';':
		lt = lex_k_semicolon;
		break;
	case '.':	//уже проверено отсутствие ".."
		lt = lex_k_dot;
		break;
	case ':':	//уже проверено отсутствие ":="
		lt = lex_k_colon;
		break;
	case '<':	//уже проверено отсутствие "<="
		lt = lex_k_lt;
		break;
	case '>':	//уже проверено отсутствие ">="
		lt = lex_k_gt;
		break;
	case '(':	//комментарий не включается в список ключевых слов
		lt = lex_k_op_bracket;
		break;
	case ')':
		lt = lex_k_cl_bracket;
		break;
	case '[':
		lt = lex_k_op_square;
		break;
	case ']':
		lt = lex_k_cl_square;
		break;
	case '{':
		lt = lex_k_op_brace;
		break;
	case '}':
		lt = lex_k_cl_brace;
		break;
	case '+':
		lt = lex_k_plus;
		break;
	case '-':
		lt = lex_k_minus;
		break;
	case '*':
		lt = lex_k_asterix;
		break;
	case '/':
		lt = lex_k_slash;
		break;
	case '=':
		lt = lex_k_eq;
		break;
	case '#':
		lt = lex_k_ne;
		break;
	case '~':
		lt = lex_k_negation;
		break;
	case '&':
		lt = lex_k_and;
		break;
	case ',':
		lt = lex_k_comma;
		break;
	case '^':
		lt = lex_k_up_arrow;
		break;
	case '|':
		lt = lex_k_vertical;
	}//switch

	//проверка обнаружения символа
	if (lex_i != lt) {
		WriteLex(lt, NULL, pos);
		return true;
	}

	//определение длины ключевого слова (если есть), все спец. символы уже проверены
	int st_len = 0;
	while ('A' <= st[st_len] && 'Z' >= st[st_len]) ++st_len;
	if (isalnum(st[st_len])) return false;

	//макс. длина кл. слова (DEFINITION)
	const int st_DEFINITION_len = 10;

	//проверка допустимости найденной длины (одинарные символы обработаны)
	if (2 > st_len || st_DEFINITION_len < st_len) return false;

	//копирование информации в буфер
	char buf[st_DEFINITION_len + 1];
	memcpy(buf, st, st_len);
	buf[st_len] = 0;

	//проверка нахождения ключевого слова
	switch (st_len) {
	case 2:
		if (!strcmp(buf,"BY")) lt = lex_k_BY; else
		if (!strcmp(buf,"DO")) lt = lex_k_DO; else
		if (!strcmp(buf,"IF")) lt = lex_k_IF; else
		if (!strcmp(buf,"IN")) lt = lex_k_IN; else
		if (!strcmp(buf,"IS")) lt = lex_k_IS; else
		if (!strcmp(buf,"OF")) lt = lex_k_OF; else
		if (!strcmp(buf,"OR")) lt = lex_k_OR; else
		if (!strcmp(buf,"TO")) lt = lex_k_TO; else return false;
		break;
	case 3:
		if (!strcmp(buf,"DIV")) lt = lex_k_DIV; else
		if (!strcmp(buf,"END")) lt = lex_k_END; else
		if (!strcmp(buf,"FOR")) lt = lex_k_FOR; else
		if (!strcmp(buf,"MOD")) lt = lex_k_MOD; else
		if (!strcmp(buf,"NIL")) lt = lex_k_NIL; else
		if (!strcmp(buf,"VAR")) lt = lex_k_VAR; else
		if (!strcmp(buf,"ABS")) lt = lex_k_ABS; else
		if (!strcmp(buf,"ASH")) lt = lex_k_ASH; else
		if (!strcmp(buf,"CAP")) lt = lex_k_CAP; else
		if (!strcmp(buf,"CHR")) lt = lex_k_CHR; else
		if (!strcmp(buf,"DEC")) lt = lex_k_DEC; else
		if (!strcmp(buf,"INC")) lt = lex_k_INC; else
		if (!strcmp(buf,"LEN")) lt = lex_k_LEN; else
		if (!strcmp(buf,"MAX")) lt = lex_k_MAX; else
		if (!strcmp(buf,"MIN")) lt = lex_k_MIN; else
		if (!strcmp(buf,"NEW")) lt = lex_k_NEW; else
		if (!strcmp(buf,"ODD")) lt = lex_k_ODD; else
		if (!strcmp(buf,"ORD")) lt = lex_k_ORD; else
		if (!strcmp(buf,"SET")) lt = lex_k_SET; else return false;
		break;
	case 4:
		if (!strcmp(buf,"CASE")) lt = lex_k_CASE; else
		if (!strcmp(buf,"ELSE")) lt = lex_k_ELSE; else
		if (!strcmp(buf,"EXIT")) lt = lex_k_EXIT; else
		if (!strcmp(buf,"LOOP")) lt = lex_k_LOOP; else
		if (!strcmp(buf,"THEN")) lt = lex_k_THEN; else
		if (!strcmp(buf,"TYPE")) lt = lex_k_TYPE; else
		if (!strcmp(buf,"WITH")) lt = lex_k_WITH; else
		if (!strcmp(buf,"CHAR")) lt = lex_k_CHAR; else
		if (!strcmp(buf,"COPY")) lt = lex_k_COPY; else
		if (!strcmp(buf,"EXCL")) lt = lex_k_EXCL; else
		if (!strcmp(buf,"HALT")) lt = lex_k_HALT; else
		if (!strcmp(buf,"INCL")) lt = lex_k_INCL; else
		if (!strcmp(buf,"LONG")) lt = lex_k_LONG; else
		if (!strcmp(buf,"REAL")) lt = lex_k_REAL; else
		if (!strcmp(buf,"SIZE")) lt = lex_k_SIZE; else
		if (!strcmp(buf,"TRUE")) lt = lex_k_TRUE; else return false;
		break;
	case 5:
		if (!strcmp(buf,"BEGIN")) lt = lex_k_BEGIN; else
		if (!strcmp(buf,"CONST")) lt = lex_k_CONST; else
		if (!strcmp(buf,"ELSIF")) lt = lex_k_ELSIF; else
		if (!strcmp(buf,"ARRAY")) lt = lex_k_ARRAY; else
		if (!strcmp(buf,"UNTIL")) lt = lex_k_UNTIL; else
		if (!strcmp(buf,"WHILE")) lt = lex_k_WHILE; else
		if (!strcmp(buf,"FALSE")) lt = lex_k_FALSE; else
		if (!strcmp(buf,"LOCAL")) lt = lex_k_LOCAL; else
		if (!strcmp(buf,"SHORT")) lt = lex_k_SHORT; else return false;
		break;
	case 6:
		if (!strcmp(buf,"ASSERT")) lt = lex_k_ASSERT; else
		if (!strcmp(buf,"IMPORT")) lt = lex_k_IMPORT; else
		if (!strcmp(buf,"MODULE")) lt = lex_k_MODULE; else
		if (!strcmp(buf,"RECORD")) lt = lex_k_RECORD; else
		if (!strcmp(buf,"REPEAT")) lt = lex_k_REPEAT; else
		if (!strcmp(buf,"RETURN")) lt = lex_k_RETURN; else
		if (!strcmp(buf,"ENTIER")) lt = lex_k_ENTIER; else return false;
		break;
	case 7:
		if (!strcmp(buf,"POINTER")) lt = lex_k_POINTER; else
		if (!strcmp(buf,"BOOLEAN")) lt = lex_k_BOOLEAN; else
		if (!strcmp(buf,"INTEGER")) lt = lex_k_INTEGER; else
		if (!strcmp(buf,"LONGINT")) lt = lex_k_LONGINT; else return false;
		break;
	case 8:
		if (!strcmp(buf,"LONGREAL")) lt = lex_k_LONGREAL; else
		if (!strcmp(buf,"SHORTINT")) lt = lex_k_SHORTINT; else return false;
		break;
	case 9:
		if (!strcmp(buf,"PROCEDURE")) lt = lex_k_PROCEDURE; else return false;
		break;
	case 10:
		if (!strcmp(buf,"DEFINITION")) lt = lex_k_DEFINITION; else return false;
		break;
	default:
		return false;
	}//switch

	//занесение информации в лексему
	WriteLex(lt, NULL, pos);
	//обновление текущих позиций
	--st_len;
	i += st_len;
	pos += st_len;	//в кл. словах нет табуляций

	return true;
}


//---------------------------------------------------------------
//запуск лексического анализа файла f, tab_size - размер табуляции
int CLexBuf::analyser (TFileType *f, const int tab_size)
{
	char st[MaxStLeng];		//строка исходного файла
	char buf[MaxStLeng];	//буфер для обработки строки
	int i;					//текущая позиция в строке
	int m;					//отмеченная позиция в строке
	int in_comment = 0;		//наличие коментария (вложенность)
	int in_commas = 0;		//содержит тип текущих кавычек
	int err_num = 0;		//признак наличия ошибки

	int beg_pos;			//позиция начала лексемы в строке (с учетом табуляций)
	int pos;				//текущая позиция в строке (с учетом табуляций)

	//цикл построчной обработки исходного файла
	while (fgets(st, MaxStLeng, f)) {
		//подготовка переменных для обработки строки
		i = -1;
		beg_pos = 0;
		pos = 0;
		//цикл обработки очередной строки
		while (st[++i]) {

			//проверка наличия и пропуск пробельных символов
			switch (st[i]) {
			case ' ':
			case '\r':	//13
			case '\n':	//10
				++pos;
				continue;
			case '\t':
				pos += tab_size;
				continue;
			default:
				++pos;
			}//switch

			//проверка наличия комментария
			if (in_comment) {
				if (st[i] == '(' && st[i+1] == '*') {
					++in_comment;
					++i;
					++pos;
				}
				if (st[i] == '*' && st[i+1] == ')') {
					--in_comment;
					++i;
					++pos;
				}
				continue;
			}

			//проверка наличия кавычек
			if (in_commas) { 
				if (in_commas == st[i]) {
					in_commas = 0;
					memcpy(buf, &st[m], i - m);
					buf[i - m] = 0;
					//используем запомненную позицию начала текстовой строки
					WriteLex(lex_s, buf, beg_pos);
				}
				continue;
			}

			//проверка открывающей скобки комментария
			if (st[i] == '(' && st[i+1] == '*') {
				++in_comment;
				++i;
				++pos;
				continue;
			}

			//проверка открывающих кавычек
			if (st[i] == '\'' || st[i] == '\"') {
				in_commas = st[i];	//признак наличия (и тип) кавычек
				m = i + 1;			//запоминаем начало текстовой строки
				beg_pos = pos + 1;	//запоминаем позицию начала текстовой строки
				continue;
			}

			//проверка наличия числа
			if (isdigit(st[i])) {
				m = i;						//запоминаем начало числа
				err_num = test_digit(st, buf, i, pos);
				pos += i - m;				//число не может содержать табуляций
				if (err_num) {
					CurrStPos = pos;		//текущую позицию - на позицию ошибки
					return err_num;
				}
				continue;
			}

			//проверка наличия ключевого cлова
			if (test_keyword(st + i, i, pos)) continue;

			//проверка наличия идентификатора
			if (isalpha(st[i])) {
				m = i;						//запоминаем начало ид.
				while (isalnum(st[++i]));	//поиск окончания ид. (позиция начала ид. не изменяется)
				memcpy(buf, &st[m], i - m);	//выделяем ид.
				buf[i - m] = 0;
				WriteLex(lex_i, buf, pos);
				--i;						//текущий символ - на конец ид.
				pos += i - m;				//ид. не может содержать табуляций
				continue;
			}//if

			//ошибка - найден недопустимый символ
			CurrStPos = pos;				//текущую позицию - на недопустимый символ
			return l_e_IncorrectSymbol;

		}//while

		//проверка закрытия кавычек в строках
		if (in_commas) {
			CurrStPos = beg_pos - 1;		//текущую позицию - на открывающую кавычку
			return l_e_OpenCommas;
		}
		//конец обработки очередной строки
		WriteLex(lex_n, "", pos);
	}//while

	//проверка закрытия комментарных скобок
	if (in_comment) {
		--CurrStNum;			//откат на предыдущую строку
		CurrStPos = pos;		//текущую позицию - на последний символ в строке
		return l_e_OpenComment;
	}

	return 0;
};//analyser


//---------------------------------------------------------------
//запуск cинтаксического анализатора, возврат - ук. на буфер лексем
CLexBuf* CLexBuf::AnalyseFile(const char* fname, const int tab_size)
{
	//открытие указанного файла
	FILE *f = fopen(fname, "r");
	if (!f) {
		fprintf(output, "ERROR: Cannot open file \"%s\" for reading\n", fname);
		return NULL;
	}
	//создание временного буфера
	CLexBuf *lb = new CLexBuf;

	//лексический анализ
	int err_num = lb->analyser(f, tab_size);

	//закрытие файла
	fclose(f);

	if (err_num) {
		WriteErr(err_num, lb);
		fprintf(output, "Lexical analysis of \"%s\" - FAILED\n", fname);
		delete lb;
		return NULL;
	}

	//подготовка буфера к чтению лексем
	lb->ResetCurrPos();
	return lb;
}


//---------------------------------------------------------------
//переменная для хранения строки с максимальным целым числом, инициализируется в ResourceManager
//100 символов должно хватить даже для 256 разрядной системы :)
char CLexBuf::st_LONG_MAX[100];
//реальная длина строки с максимальным целым числом
int CLexBuf::st_LONG_MAX_len;
