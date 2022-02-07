//===============================================================
// �������� ��������, ����������� ��� ����������� ���������
//===============================================================

#include "LexAnl.h"


//---------------------------------------------------------------
//����������� ������ ��� ������ ��������� ����������
TFileType *output = stdout;


//---------------------------------------------------------------
//���������� ������ ������
CLexBuf::~CLexBuf()
{
    /*
	//�������: ���������� ����������� ������ ������ � �����
	FILE *f = fopen("lexbuf.tmp", "w");
	CurrStNum = 1;
	if (f)
		for (CurrLex = Buf.begin(); CurrLex != Buf.end(); CurrLex++) {
			fprintf(f, "---------------------\n");
			switch ((*CurrLex).lex) {
			case lex_n: fprintf(f, "����� ������ ���. �����\nst: %i; pos: %i;\n", CurrStNum, (*CurrLex).pos);
				++CurrStNum;
				continue;
			case lex_d: fprintf(f, "���������� �����");
				break;
			case lex_h: fprintf(f, "����������. �����");
				break;
			case lex_r: fprintf(f, "����� ���� REAL");
				break;
			case lex_l: fprintf(f, "����� ���� LONGREAL");
				break;
			case lex_i: fprintf(f, "�������������");
				break;
			case lex_s: fprintf(f, "������");
				break;
			case lex_c: fprintf(f, "���������� ���������");
				break;
			default:
				fprintf(f, "�������� �����\nst: %i; pos: %i;\n", CurrStNum, (*CurrLex).pos);
				if (lex_k_TO == (*CurrLex).lex) fprintf(f, "TO\n");
				continue;
			}
			fprintf(f, "\nst: %i; pos: %i;\n\"%s\"\n", CurrStNum, (*CurrLex).pos, (*CurrLex).st);
		}//for
	fclose(f);
	//����� �������
    //*/

	//������� ����������� ������
	for (CurrLex = Buf.begin(); CurrLex != Buf.end(); CurrLex++)
		delete[] (*CurrLex).st;
}


//---------------------------------------------------------------
//������ ���������� � ������� �� ������
bool CLexBuf::ReadLex(CLexInfo &LexInfo)
{
	//���� �������� ��������� ����� ������ � ��������� �����
	while (Buf.end() != CurrLex) {
		//��������� �������� ����� ������
		if (lex_n == (*CurrLex).lex) {
			//������� � ����. ������ � ����. �������
			++CurrLex;
			++CurrStNum;
			continue;
		}
		//������� ������� - �� ����� ������
		LexInfo.lex = (*CurrLex).lex;
		LexInfo.st = (*CurrLex).st;
		//������� ������� - �� ����. �������, �.�. ��� ������ ������� ������� �����������,
		//������������� ������� ������� ����� ��������� �� �������
		CurrStPos = (*CurrLex).pos;
		//��������� ��������� ������� � �������� �������
		++CurrLex;
		return true;
	}
	//����� ������� �� ������ ����������� ������ � ����� �����
	CurrStPos = 1;
	return false;
}


//---------------------------------------------------------------
//��������� ������� ������� � ������ ������ �� ������ ������
void CLexBuf::ResetCurrPos()
{
	CurrLex = Buf.begin();
	CurrStNum = 1;
	CurrStPos = 1;
}


//---------------------------------------------------------------
//��������� ������� ������� ������ ������
CLexBufPos CLexBuf::GetCurrPos()
{
	CLexBufPos LBPos;
	LBPos.CurrLex = CurrLex;
	LBPos.CurrStNum = CurrStNum;
	LBPos.CurrStPos = CurrStPos;
	return LBPos;
}


//---------------------------------------------------------------
//��������� ������� ������� ������ ������
void CLexBuf::SetCurrPos(CLexBufPos LexBufPos)
{
	CurrLex = LexBufPos.CurrLex;
	CurrStNum = LexBufPos.CurrStNum;
	CurrStPos = LexBufPos.CurrStPos;
}


//---------------------------------------------------------------
//���������� � ����� �������
void CLexBuf::WriteLex(const enum ELexType LexType, const char *st, const int pos)
{
	CLex lex;
	//���������� ���� �������
	lex.lex = LexType;
	//���������� ������� ������ ������� � ������ ���. ����� � � ������ ������
	lex.pos = pos;
	CurrStPos = pos;
	//�������� ������ � ���������� ������� (���� ����)
	if (st) {
		lex.st = new char[strlen(st) + 1];
		strcpy(lex.st, st);
	} else
		lex.st = NULL;
	//��������� ������� � ������
	Buf.push_back(lex);
	//��� ��������� �������� ����� ������ ���������� �������� �����
	if (lex_n == LexType) ++CurrStNum;
}


//---------------------------------------------------------------
//������ ��������� �� ������
void WriteErr(int err_num, const CLexBuf *lb)
{
	fprintf(output,"ERROR (st %d, pos %d): %s\n", lb->GetCurrStNum(), lb->GetCurrStPos(),
		(err_num >= s_m_Error && err_num < s_m_MaxErrorCode) ? err_arr[err_num] : err_arr[s_m_ZeroErrorCode]);
};//WriteErr


//---------------------------------------------------------------
//������ �������������� (Warning)
void WriteWarn(int warn_num, const CLexBuf *lb)
{
	fprintf(output,"WARNING (st %d, pos %d): %s\n", lb->GetCurrStNum(), lb->GetCurrStPos(),
		(warn_num >= s_m_Error && warn_num < s_m_MaxErrorCode) ? err_arr[warn_num] : err_arr[s_m_ZeroErrorCode]);
};//WriteWarn


//---------------------------------------------------------------
//�������� ������� ����������������� ����� ������� (0..9, A..F)
static bool isO2Mxdigit(int c) {
	if (isdigit(c) || (c >= 'A' && c <= 'F')) return true;
	return false;
}


//---------------------------------------------------------------
//�������� ������� �������� ������ �����, true - ����� ������� ������
inline bool CLexBuf::test_big_number(const char *st, int st_len)
{
	//������� ����� � ������ �����
	while ('0' == st[0]) {
		st++;
		st_len--;
	}
	//�������� ����� �����
	if (st_len < st_LONG_MAX_len) return false;
	if (st_len > st_LONG_MAX_len) return true;
	//�������� �������� ����� ��� ���������� ����
	for (int i = 0; i < st_len; i++)
		if (st[i] > st_LONG_MAX[i]) return true; else if (st[i] < st_LONG_MAX[i]) break;
	return false;
}


//---------------------------------------------------------------
//�������� ����� � ������� i ������ st (buf - �����)
//���� ����� �������, i - �� ��������� ������ �����
inline int CLexBuf::test_digit (const char *st, char *buf, int &i, const int pos)
{
	int m = i;						//���������� ������� �������
	while (isdigit(st[++i]));		//����� ��������� �����

	//��������, ����� ��� ����� ������
	switch (st[i]) {
	case 'H':
		//����������������� (�����)
		memcpy(buf, &st[m], i - m);
		buf[i - m] = 0;
		WriteLex(lex_h, buf, pos);
		return 0;
	case 'X':
		//������ (�����)
		memcpy(buf, &st[m], i - m);
		buf[i - m] = 0;
		WriteLex(lex_c, buf, pos);
		return 0;
	case '.':
		//�������� ������� ���. ����� (keyword "..")
		if (st[i + 1] != '.') {

			//��������������
			while (isdigit(st[++i]));		//����� ��������� �����
			//��������, ����� ��� ����� ������
			switch (st[i]){
			case 'D':
				//��� LONGREAL
				++i;
				if (st[i] == '+' || st[i] == '-') ++i;
				if (!isdigit(st[i])){		//��� ����� ����� �����
					return l_e_LastScaleDigit;
				}//if
				while (isdigit(st[++i]));	//����� ��������� �����
				//��� REAL (�����)
				memcpy(buf, &st[m], i - m);
				buf[i - m] = 0;
				--i;						//i - �� ����� �����
				WriteLex(lex_l, buf, pos);
				return 0;
			case 'E':
				//��� REAL
				++i;
				if (st[i] == '+' || st[i] == '-') ++i;
				if (!isdigit(st[i])){		//��� ����� ����� �����
					return l_e_LastScaleDigit;
				}//if
				while (isdigit(st[++i]));	//����� ��������� �����
			default:
				//��� REAL (�����)
				memcpy(buf, &st[m], i - m);
				buf[i - m] = 0;
				--i;						//i - �� ����� �����
				WriteLex(lex_r, buf, pos);
				return 0;
			}//switch

		}//if not '.'

	default:
		//�������� ���������� HEX ����
		if (!isO2Mxdigit(st[i])) {
			//���������� �����
			int len = i - m;
			memcpy(buf, &st[m], len);
			buf[len] = 0;
			--i;							//i - �� ����� �����
			WriteLex(lex_d, buf, pos);
			//�������� ������� �������� ������ �����
			if (test_big_number(buf, len)) return l_e_NumberTooLarge;
			return 0;
		}
		//����� - �����������������
		while (isO2Mxdigit(st[++i]));		//����� ��������� �����
		//�������� - ������ ��� HEX
		switch (st[i]){
		case 'H':
			//����������������� (�����)
			memcpy(buf, &st[m], i - m);
			buf[i - m] = 0;
			WriteLex(lex_h, buf, pos);
			return 0;
		case 'X':
			//������ (�����)
			memcpy(buf, &st[m], i - m);
			buf[i - m] = 0;
			WriteLex(lex_c, buf, pos);
			return 0;
		}//switch
	}//switch

	//������������ ��������� HEX �����
	return l_e_LastHexDigit;
}//test_digit


//---------------------------------------------------------------
//�������� ������� � ������ st (�� ����� 2 ��������, ������� 0) ��������� �����
//���� ������� ��. �����, ����������� � i � pos ����� ���������� ����� - 1
inline bool CLexBuf::test_keyword(const char *st, int &i, int &pos)
{
	//��� �������� ���� �������
	ELexType lt = lex_i;

	//�������� �������� �� 2 �������� (":=", "+=", "<=", ">=", "..")
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

	//�������� ����������� �������� �� 2 ��������
	if (lex_i != lt) {
		WriteLex(lt, NULL, pos);
		++i;
		++pos;
		return true;
	}

	//�������� ��������
	switch (st[0]) {
	case ';':
		lt = lex_k_semicolon;
		break;
	case '.':	//��� ��������� ���������� ".."
		lt = lex_k_dot;
		break;
	case ':':	//��� ��������� ���������� ":="
		lt = lex_k_colon;
		break;
	case '<':	//��� ��������� ���������� "<="
		lt = lex_k_lt;
		break;
	case '>':	//��� ��������� ���������� ">="
		lt = lex_k_gt;
		break;
	case '(':	//����������� �� ���������� � ������ �������� ����
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

	//�������� ����������� �������
	if (lex_i != lt) {
		WriteLex(lt, NULL, pos);
		return true;
	}

	//����������� ����� ��������� ����� (���� ����), ��� ����. ������� ��� ���������
	int st_len = 0;
	while ('A' <= st[st_len] && 'Z' >= st[st_len]) ++st_len;
	if (isalnum(st[st_len])) return false;

	//����. ����� ��. ����� (DEFINITION)
	const int st_DEFINITION_len = 10;

	//�������� ������������ ��������� ����� (��������� ������� ����������)
	if (2 > st_len || st_DEFINITION_len < st_len) return false;

	//����������� ���������� � �����
	char buf[st_DEFINITION_len + 1];
	memcpy(buf, st, st_len);
	buf[st_len] = 0;

	//�������� ���������� ��������� �����
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

	//��������� ���������� � �������
	WriteLex(lt, NULL, pos);
	//���������� ������� �������
	--st_len;
	i += st_len;
	pos += st_len;	//� ��. ������ ��� ���������

	return true;
}


//---------------------------------------------------------------
//������ ������������ ������� ����� f, tab_size - ������ ���������
int CLexBuf::analyser (TFileType *f, const int tab_size)
{
	char st[MaxStLeng];		//������ ��������� �����
	char buf[MaxStLeng];	//����� ��� ��������� ������
	int i;					//������� ������� � ������
	int m;					//���������� ������� � ������
	int in_comment = 0;		//������� ���������� (�����������)
	int in_commas = 0;		//�������� ��� ������� �������
	int err_num = 0;		//������� ������� ������

	int beg_pos;			//������� ������ ������� � ������ (� ������ ���������)
	int pos;				//������� ������� � ������ (� ������ ���������)

	//���� ���������� ��������� ��������� �����
	while (fgets(st, MaxStLeng, f)) {
		//���������� ���������� ��� ��������� ������
		i = -1;
		beg_pos = 0;
		pos = 0;
		//���� ��������� ��������� ������
		while (st[++i]) {

			//�������� ������� � ������� ���������� ��������
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

			//�������� ������� �����������
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

			//�������� ������� �������
			if (in_commas) { 
				if (in_commas == st[i]) {
					in_commas = 0;
					memcpy(buf, &st[m], i - m);
					buf[i - m] = 0;
					//���������� ����������� ������� ������ ��������� ������
					WriteLex(lex_s, buf, beg_pos);
				}
				continue;
			}

			//�������� ����������� ������ �����������
			if (st[i] == '(' && st[i+1] == '*') {
				++in_comment;
				++i;
				++pos;
				continue;
			}

			//�������� ����������� �������
			if (st[i] == '\'' || st[i] == '\"') {
				in_commas = st[i];	//������� ������� (� ���) �������
				m = i + 1;			//���������� ������ ��������� ������
				beg_pos = pos + 1;	//���������� ������� ������ ��������� ������
				continue;
			}

			//�������� ������� �����
			if (isdigit(st[i])) {
				m = i;						//���������� ������ �����
				err_num = test_digit(st, buf, i, pos);
				pos += i - m;				//����� �� ����� ��������� ���������
				if (err_num) {
					CurrStPos = pos;		//������� ������� - �� ������� ������
					return err_num;
				}
				continue;
			}

			//�������� ������� ��������� c����
			if (test_keyword(st + i, i, pos)) continue;

			//�������� ������� ��������������
			if (isalpha(st[i])) {
				m = i;						//���������� ������ ��.
				while (isalnum(st[++i]));	//����� ��������� ��. (������� ������ ��. �� ����������)
				memcpy(buf, &st[m], i - m);	//�������� ��.
				buf[i - m] = 0;
				WriteLex(lex_i, buf, pos);
				--i;						//������� ������ - �� ����� ��.
				pos += i - m;				//��. �� ����� ��������� ���������
				continue;
			}//if

			//������ - ������ ������������ ������
			CurrStPos = pos;				//������� ������� - �� ������������ ������
			return l_e_IncorrectSymbol;

		}//while

		//�������� �������� ������� � �������
		if (in_commas) {
			CurrStPos = beg_pos - 1;		//������� ������� - �� ����������� �������
			return l_e_OpenCommas;
		}
		//����� ��������� ��������� ������
		WriteLex(lex_n, "", pos);
	}//while

	//�������� �������� ������������ ������
	if (in_comment) {
		--CurrStNum;			//����� �� ���������� ������
		CurrStPos = pos;		//������� ������� - �� ��������� ������ � ������
		return l_e_OpenComment;
	}

	return 0;
};//analyser


//---------------------------------------------------------------
//������ c�������������� �����������, ������� - ��. �� ����� ������
CLexBuf* CLexBuf::AnalyseFile(const char* fname, const int tab_size)
{
	//�������� ���������� �����
	FILE *f = fopen(fname, "r");
	if (!f) {
		fprintf(output, "ERROR: Cannot open file \"%s\" for reading\n", fname);
		return NULL;
	}
	//�������� ���������� ������
	CLexBuf *lb = new CLexBuf;

	//����������� ������
	int err_num = lb->analyser(f, tab_size);

	//�������� �����
	fclose(f);

	if (err_num) {
		WriteErr(err_num, lb);
		fprintf(output, "Lexical analysis of \"%s\" - FAILED\n", fname);
		delete lb;
		return NULL;
	}

	//���������� ������ � ������ ������
	lb->ResetCurrPos();
	return lb;
}


//---------------------------------------------------------------
//���������� ��� �������� ������ � ������������ ����� ������, ���������������� � ResourceManager
//100 �������� ������ ������� ���� ��� 256 ��������� ������� :)
char CLexBuf::st_LONG_MAX[100];
//�������� ����� ������ � ������������ ����� ������
int CLexBuf::st_LONG_MAX_len;
