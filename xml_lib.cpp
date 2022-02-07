///////////////////////////////////////////////////////
// ����� ������� ��� ������� ����� � ������� XML 1.0 //
// ����� ������� Link2M                              //
// ������ ����������: ��. xml_lib.h                  //
///////////////////////////////////////////////////////

#include "xml_lib.h"

using namespace xml_lib;


//-----------------------------------------------------------------------------
//������ �� ���������� ����� ������� � ��������� ���������� ��������
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
//������ �� ���������� ����� ����� (�����/�����) � ��������� ���������� ��������,
//�������� ����� ������ �� ���������� ������ �� ���������������
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
//������ �� ����� � ��������� ���������� ��������� ������-�� ��������
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
//������� ���� ����������� (������� "<!" ������ ���� ��� �������)
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
//�������������� ����������� � ����� ����� � ������ (������������ ������� ������������)
bool xml_lib::ReadNumber(FILE* f, char& ch)
{
	//������ �������� �������
	ch = fgetc(f);
	if (feof(f)) return false;

	int n = 0;
	int num = 0;

	//��������� ����� � ����������� �� ���� (����������/�����������������)
	if ('x' == ch) {
		//��������� ������������������ �����
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
			//��������� ����. �������
			ch = fgetc(f);
		}//while
	} else {
		//��������� ����������� �����
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
			//��������� ����. �������
			ch = fgetc(f);
		}//while
	}//else

	//�� ������ ����������� ������ ";"
	return false;
}


//-----------------------------------------------------------------------------
//�������� ���������� ��������� (������ '"' (��� '>') ������ ���� ��� ������)
bool xml_lib::ReadText(FILE* f, char* buf, char& ch)
{
	int i = 0;
	while (i < BufSize) {
		//������ �������� �������
		ch = fgetc(f);
		if (feof(f)) return false;
		//�������� ��������� ����������� ������� '"' ��� ������ ���� '<'
		if ('"' == ch || '<' == ch) {
			buf[i] = 0;
			return true;
		}
		//�������� ������� "&" (������ "&amp;", "&lt;", "&gt;", "&quot;", "&#<�����>;" ��� "&#x<�����. �����>;")
		if ('&' == ch) {
			//������ ����. �������
			ch = fgetc(f);
			if (feof(f)) return false;
			//�������� ���������� �������
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
		//���������� ������� � ������� � ����. ��������
		buf[i++] = ch;
		continue;
	}//while
	//������������ ������
	return false;
}


//-----------------------------------------------------------------------------
//�����������
CXMLAttribute::CXMLAttribute(const char* newName, const char* newValue)
{
	//��� ��������� ������ ������ ��������������
	Name = new char[strlen(newName) + 1];
	strcpy(Name, newName);
	//�������� ��������� ����� ������������� (�������� �� ������ ������� '\0')
	if ('\0' == newValue[0])
		Value = NULL;
	else {
		Value = new char[strlen(newValue) + 1];
		strcpy(Value, newValue);
	}//else
};


//-----------------------------------------------------------------------------
//���������� ������ ������ ���� (<TagName>Text<InternalTag/>more text</TagName>)
void CXMLEntry::AddContent(const char *buf)
{
	//�������� ������� ��������� �����
	if (XMLEntryStore.empty()) {
		//����� ������ ���� �� ��������� ��������� �����
		Content = new char[strlen(buf) + 1];
		strcpy(Content, buf);
	} else	//����� ����� ��������� ��������� �����
		(XMLEntryStore.back())->AddExtContent(buf);
}


//-----------------------------------------------------------------------------
//���������� ������, ���������� ����� ���� (���������� ������������ �����)
void CXMLEntry::AddExtContent(const char *buf)
{
	//�� ������ ���������� ���������� ������
	delete[] ExtContent;
	//���������� ����������� ������
	ExtContent = new char[strlen(buf) + 1];
	strcpy(ExtContent, buf);
}


//-----------------------------------------------------------------------------
//��������� �������� �������� (��� NULL) �� �����
const char* CXMLEntry::AttribValue(const char *AttribName) const
{
	TAttribStore::const_iterator ci;
	for (ci = AttribStore.begin(); ci != AttribStore.end(); ci++)
		if (!strcmp(AttribName, (*ci)->Name))
			return (*ci)->Value;
	return NULL;
}


//-----------------------------------------------------------------------------
//����������
CXMLEntry::~CXMLEntry() {
	//������� ��������� ����������
	delete[] TagName;
	delete[] ExtContent;
	delete[] Content;
	//������� ������ ��� ��������-��������
	TAttribStore::const_iterator ci_at;
	for (ci_at = AttribStore.begin(); ci_at != AttribStore.end(); ci_at++)
		delete *ci_at;
	//������� ������ xml ��-���
	TXMLEntryStore::const_iterator ci_x;
	for (ci_x = XMLEntryStore.begin(); ci_x != XMLEntryStore.end(); ci_x++)
		delete *ci_x;
};


//-----------------------------------------------------------------------------
//�������� xml ��-��, ch - ������ ������ ������������ ���� (������ ���� ��� ��������)
bool CXMLEntry::Init(FILE *f, char& ch)
{
	//�������� ���������� �������� ���� (1-� ������ ��� �������� � ch)
	char buf[BufSize];
	buf[0] = ch;
	if (!freads(f, &buf[1], ch)) return false;
	TagName = new char[strlen(buf) + 1];
	strcpy(TagName, buf);

	//�������� �������, ������� ����� ����

	//�������� ������� ����� (����� ��� ���� ��������-��������)
	while (isalpha(ch)) {
		//�������� ���������� �������� �������� ���������
		buf[0] = ch;
		if (!freads(f, &buf[1], ch)) return false;
		//���������� �������� ���������
		char nm[BufSize];
		strcpy(nm, buf);
		//�������� ������� "=" � '"'
		if ('=' != ch) return false;
		if (!freadc(f, ch)) return false;
		if ('"' != ch) return false;
		//�������� ���������� ��������� (��������)
		if (!ReadText(f, buf, ch)) return false;
		//�������� ������� '"' � ���������� ���� � ������
		if ('"' != ch) return false;
		CXMLAttribute* attr = new CXMLAttribute(nm, buf);
		AttribStore.push_back(attr);
		//��������� ����. �������
		if (!freadc(f, ch)) return false;
	}//while

	//�������� ������� "/" (����������� ��������� xml ��-��)
	if ('/' == ch) {
		if (!freadb(f, ">")) return false;
		return true;
	}

	//�������� ������� ">" (����� �������������� ��������� xml ��-��)
	if ('>' != ch) return false;

	//����� ��������� xml ��-���
	while (!feof(f)) {
		//����� "<" �������� ����������� ������� ������
		if (!ReadText(f, buf, ch)) return false;
		if ('<' != ch) return false;
		//����� ����������� ������ ���� �� ������������ (���� ������� ����� '\0')
		if ('\0' != buf[0]) AddContent(buf);
		//�������� �������, ���������� �� "<"
		if (feof(f)) return false;
		ch = fgetc(f);
		//�������� ����������� ������������ ����
		if ('/' == ch) {
			if (!freadb(f, TagName)) return false;
			if (!freadc(f, ch)) return false;
			if ('>' != ch) return false;
			return true;
		}
		//�������� ������� �����������
		if ('!' == ch) {
			SkipComment(f);
			continue;
		}
		//�������� ������� ������ �������������� (����)
		if (!isalpha(ch)) return false;
		//�������� ���������� xml ���� ���-��
		CXMLEntry* xmle = new CXMLEntry;
		if (!xmle->Init(f, ch)) {
			delete xmle;
			return false;
		}
		//��������� ������������ xml ��-�� � ������
		XMLEntryStore.push_back(xmle);
	}//while

	return false;
}


//-----------------------------------------------------------------------------
//�������� ����� � ������� XML
EXMLErrors CXMLFile::Load(const char *fname)
{
	//����� ��� ������ ����������
	char buf[BufSize];
	char ch;

	//�������� ����� ��� ������
	FILE* f = fopen(fname, "r");
	if (!f) return EXMLOpen;

	//������ xml ���������
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

	//����� �������� �������� ���� ���������
	while (true) {
		//����� "<"
		if (!freadc(f, ch)) goto err;
		if ('<' != ch) goto err;
		//��������� �������, ���������� �� "<"
		if (feof(f)) goto err;
		ch = fgetc(f);
		//�������� ������� �����������
		if ('!' == ch) {
			SkipComment(f);
			continue;
		}
		//�������� ������� ������ �������������� (����)
		if (!isalpha(ch)) goto err;
		//�������� �������� ���� ���-�� (���� �� �����������)
		XMLEntry = new CXMLEntry;
		if (XMLEntry->Init(f, ch))
			break;
		else
			goto err;
	}//while

	//����� ������������ ����� �������� ���� ���-��
	while (true) {
		//����� "<" (���� ��� - ����� ��������� �����)
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
	//�������� ������ �����
	if (fclose(f)) return EXMLClose;
	return EXMLFormat;
}


