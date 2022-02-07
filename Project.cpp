///////////////////////////////////////////////////////////////////////////
// ����� ��� ������ � ����������� ������ �������� O2M, ����� ������� O2M //
///////////////////////////////////////////////////////////////////////////

#include "Project.h"
#include <stdlib.h>

static const int Max_St_Len = 10000;

//�������� O2M makefile
static const char* const O2M_makefile = "CPP/_O2M_make.2mk";


//-----------------------------------------------------------------------------
//����������� ������ �������
CProject::CProject() : TabSize(1), EXEC(NULL), MAIN(NULL)
{
	//�������� � ������ ���������� �� ��������� (/DFN)
	char *st = new char[strlen("DFN/_")];
	strcpy(st, "DFN/");
	Paths.push_back(st);
	PathMaxLen = strlen(st);
}


//-----------------------------------------------------------------------------
//���������� ������ �������
CProject::~CProject()
{
	delete[] MAIN;
	delete[] EXEC;
	//������� ������ ����������
	Paths_type::const_iterator cpi;
	for (cpi = Paths.begin(); cpi != Paths.end(); ++cpi)
		delete[] (*cpi);
}


//-----------------------------------------------------------------------------
//�������� ��� ������ AddMakeFile � ������ ������������� ������� ���� � �����
void CProject::AddFullPathMakeFile(const char *FullPath, const char *ModuleName)
{
	//����� ������ ����� �����
	int i;
	for (i = strlen(FullPath); i >= 0; i--)
		if ('/' == FullPath[i] || '\\' == FullPath[i]) break;
	//��������� ���� � ����� (���� ����)
	char *path = NULL;
	if (0 <= i) {
		++i;
		path = new char[i + 1];
		strncpy(path, FullPath, i);
		path[i] = '\0';
	}
	//���������� ����� � ������
	AddMakeFile(path, ModuleName);
	//������������ ������
	delete[] path;
}


//-----------------------------------------------------------------------------
//���������� ����� � ������ ������ � ��������� ���������� ������������
void CProject::AddMakeFile(const char *FilePath, const char *ModuleName)
{
	const char* Name;
	const char* Path;
	//�������� ���������� ��������� ��������� (�������� ���� � ������ �� ���������)
	CXMLEntry::TXMLEntryStore::const_iterator ci;
	for (ci = xmlFile.XMLEntry->XMLEntryStore.begin(); ci != xmlFile.XMLEntry->XMLEntryStore.end(); ++ci) {
		//�������� ���������� ���� XML ��-��
		if (strcmp("File", (*ci)->TagName)) continue;
		//����� ��������� ��������� XML ��-�� (��� ��������)
		Name = (*ci)->AttribValue("Name");
		Path = (*ci)->AttribValue("Path");
		//���������� ��������� ����� ����� � ����
		if (!strcmp(Name, ModuleName)) {
			if (!Path && !FilePath) return;
			if (Path && FilePath && !strcmp(Path, FilePath)) return;
		}
	}
	//���������� ����� � ������
	//���������� ��. �� ���� (� ����������� XML �������� ������ ���������� NULL)
	Path = FilePath ? FilePath : "\0";
	//��������  � ��������� XML ��-�� (��������� � ���� ��������� � ������ ����� � �����)
	CXMLEntry* xmle = new CXMLEntry;
	xmle->TagName = str_new_copy("File");
	xmle->AttribStore.push_back(new CXMLAttribute("Name", ModuleName));
	xmle->AttribStore.push_back(new CXMLAttribute("Path", Path));
	//��������� XML ��-�� � ������
	xmlFile.XMLEntry->XMLEntryStore.push_back(xmle);
}


//-----------------------------------------------------------------------------
//�������� ����� �������
int CProject::Init(const char *f_name)
{
	//�������� ����� ������� ��� ������
	FILE *f = fopen(f_name, "r");
	if (!f) return 1;

	//����� ��� ������ ����� ������ �� ����� �������
	char st[Max_St_Len];
	//���������� ��� ���������� ��������� �������� ��������� � ��������� ������
	int var_beg, var_len;
	//���������� ��� ���������� ��������� �������� � ��������� ������
	int val_beg, val_len;

	//���� ����������� ������ ����� �������
	while (fgets(st, Max_St_Len, f)) {

		//��������� �������� ��������� � ��������
		//������� ���������� ��������
		int beg = 0;
		while ('\t' == st[beg] || ' ' == st[beg]) beg++;
		//������� ������������
		if (';' == st[beg] || '#' == st[beg]) continue;
		//������� ������ �������� ���������, ���� ��� �����
		int end = beg;
		while (isalnum(st[end]) || '_' == st[end]) end++;
		//�������� ������� �������� ���������
		if (end == beg) continue;
		//���������� ���������� �������� ���������
		var_beg = beg;
		var_len = end - beg;
		//������� ���������� ��������
		beg = end;
		while ('\t' == st[beg] || ' ' == st[beg]) beg++;
		//�������� ������� ������� '='
		if ('=' != st[beg++]) continue;
		//������� ���������� ��������
		while ('\t' == st[beg] || ' ' == st[beg]) beg++;
		//����� ���������� ����� ��������
		end = strlen(st) - 1;
		switch (st[end]) {
		case '\t':
		case ' ':
		case '\r':
		case '\n':
			--end;
		}
		//�������� ������� ����������� �������
		if ('"' == st[beg] || '\'' == st[beg]) {
			//�������� ������� ����������� �������, ��������������� �����������
			if (st[beg] != st[end]) continue;
			//������������� ���������� ������ � ������ �������
			++beg;
		} else
			++end;	//end - �� ��������� ������ ������
		//�������� ������� �������� (������ �������� �������� ����������)
		if (end < beg) continue;
		//���������� ���������� ��������
		val_beg = beg;
		val_len = end - beg;
		//���������� ����������� �������� '\0' ��� �������� ��������� � ��������
		st[var_beg + var_len] = '\0';
		st[end] = '\0';

		//��������� ���������� ��������� � ��������
		//�������������� �������� ��������� � ������� �������
		for (beg = var_beg; st[beg]; beg++) st[beg] = toupper(st[beg]);
		//�������� ������� ������� �������� ��������� (��� ��������)
		switch (st[var_beg]) {
		case 'M':	//MAIN
			if (!strcmp(st + var_beg + 1, "AIN")) {
				//������� ������ �� ������ ���������� ����������
				delete[] MAIN;
				//��������� ��� �������� ����� � ��������������� � ������� �������
				MAIN = new char[val_len + 1];
				for (beg = 0; beg <= val_len; beg++) MAIN[beg] = toupper(st[val_beg + beg]);
			}
			break;
		case 'E':	//EXEC
			if (!strcmp(st + var_beg + 1, "XEC")) {
				//������� ������ �� ������ ���������� ����������
				delete[] EXEC;
				//������� .exe � ����� ����� ������������ ����� (���� ����)
				const char* ste = st + val_beg;
				if (4 < val_len &&
					'.' == ste[val_len - 4] &&
					'E' == toupper(ste[val_len - 3]) &&
					'X' == toupper(ste[val_len - 2]) &&
					'E' == toupper(ste[val_len - 1])
					) val_len -= 4;
				//��������� ��� ������������ �����
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
				//�������������� ������ � �����
				TabSize = atoi(st + val_beg);
				//�������� ������������ ����������� ������� ���������
				if (0 > TabSize || (0 == TabSize && ('0' != st[val_beg] || 0 != st[val_beg + 1])))
					TabSize = 1;	//��������� TabSize �� ��������� � ������ ������������� ��������
			}
			break;
		}//switch

	}//while

	//���������� ���������� ����� ����
	Paths_type::const_iterator i;
	for (i = Paths.begin(); i != Paths.end(); ++i) {
		int NewLen = strlen(*i);
		if (NewLen > PathMaxLen) PathMaxLen = NewLen;
	}

	return 0;
}


//-----------------------------------------------------------------------------
//��������, �������� �� ��������� ���� ������� ������ �������
bool CProject::IsMainFile(const char *file_name)
{
/*
	//��������, � file_name ��������� �������� ���� � ����� �� ����� �����?

	//����� ����� ���������� �����
	const int len = strlen(file_name);
	int i;
	for (i = len; i >= 0; i--)
		if ('.' == file_name[i]) break;
	if (0 > i) return false;
	int dot = i;
	//����� ������ ����� �����
	int beg;
	for (i = dot; i >= 0; i--)
		if ('/' == file_name[i] || '\\' == file_name[i]) break;
	beg = i;
	//����������� ���� � ����� �����
	char *path = NULL;
	if (0 < beg) {
		path = str_new_copy(file_name);
		path[beg] = '\0';
	}
	//��������� �������� ��������������� ����� ��� ����������
	char *file = str_new_copy(file_name + beg + 1);
	file[dot] = '\0';
	//��������� �������� ��������������� ����� � �����������
	char* file_ext = str_new_copy(file_name + beg + 1);
*/

	//�������� ������� ����� �������� ������
	if (!file_name || !MAIN) return false;
	//��������� �������� ��������������� ����� � UPPER CASE
	char* upper_f_n = NULL;
	if (file_name) {
		upper_f_n = str_new_copy(file_name);
		for (int i = 0; i < strlen(upper_f_n); i++) upper_f_n[i] = toupper(upper_f_n[i]);
	}
	//�������� ���������� ����
	if (strcmp(upper_f_n, MAIN ? MAIN : "")) {
		delete[] upper_f_n;
		return false;
	}
	delete[] upper_f_n;
	return true;
}


//-----------------------------------------------------------------------------
//�������� (����������) O2M makefile
bool CProject::Create2mk()
{
	//�������� �������� XML ��-��
	if (!xmlFile.XMLEntry) xmlFile.XMLEntry = new CXMLEntry;
	return true;
}


//-----------------------------------------------------------------------------
//�������� O2M makefile (���� ����)
bool CProject::Load2mk()
{
	//������ xml ����� � �������� ������ ������� _O2M_make.2mk (���� ����)
	switch (xmlFile.Load(O2M_makefile)) {
	case EXMLNoError:
		//�������� ������������ ����������� ����� (������� ��-�� Project)
		if (strcmp("Project", xmlFile.XMLEntry->TagName)) {
			fprintf(output, "ERROR: Incorrect O2M makefile (\"%s\") content\n", O2M_makefile);
			break;
		}
		return true;
	case EXMLOpen:
		//��� ������ �������� ���� ����� ����������� ������
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
	//������� ���������� ������
	return false;
}


//-----------------------------------------------------------------------------
//��������� ����� O2M makefile ��� ������������ (Link2M) � �������� ������� (Make2M)
bool CProject::Write2mk(const bool is_main, const char *ModuleName)
{
	//��. �� ��� ������������ �����
	const char* ExecName;

	//��������, �������� �� ���� ������� ������ �������
	if (is_main)	//��� ������������ ����� ������� �� �������� ��� �� ����� ������
		ExecName = EXEC ? EXEC : ModuleName;
	else {			//��������� ����� ���. ����� � ����. ������ �� O2M makefile (���� ����)
		ModuleName = xmlFile.XMLEntry->AttribValue("Main");
		if (!ModuleName) ModuleName = "";
		ExecName = xmlFile.XMLEntry->AttribValue("Exec");
		if (!ExecName) ExecName = "";
	}

	//�������� ����� O2M makefile ��� ������������ (Link2M) � �������� ������� (Make2M)
	FILE *ff = fopen(O2M_makefile, "w");
	if (!ff) {
		fprintf(output, textCannotOpenW, O2M_makefile);
		return false;
	}

	//������ ��������� � 2mk ����
	fprintf(ff, "<?xml version=\"1.0\" ?>\n<!-- %s -->\n<Project Main=\"%s\" Exec=\"%s\">\n", comment_title, ModuleName, ExecName);
	//������ �������� ���� c++ ������ �������, ��������� ���������� (����� �������� � ������ _O2M_main, _O2M_sys)
	CXMLEntry::TXMLEntryStore::const_iterator ci;
	for (ci = xmlFile.XMLEntry->XMLEntryStore.begin(); ci != xmlFile.XMLEntry->XMLEntryStore.end(); ++ci) {
		if (strcmp("File", (*ci)->TagName)) continue;
		const char* Path = (*ci)->AttribValue("Path");
		fprintf(ff, "\t<File Name=\"%s\" Path=\"%s\" />\n", (*ci)->AttribValue("Name"), Path ? Path : "");
	}
	//������ ������������ ���� � 2mk ����
	fprintf(ff, "</Project>\n");

	//�������� �����
	if (fclose(ff)) {
		fprintf(output, textCannotClose, O2M_makefile);
		return false;
	}

	return true;
}

