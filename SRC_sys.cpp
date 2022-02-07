//=============================================================================
// �������� ������ _O2M_sys.h, _O2M_sys.cpp
//=============================================================================

#include "SRC_sys.h"
#include "Common.h"


//-----------------------------------------------------------------------------
//�������� _O2M_sys ������ � ��������� ������� ���������� CPP
bool create_O2M_sys_files()
{
	//��� .h ����� _O2M_sys
	const char* const _O2M_sys_h = "CPP/_O2M_sys.h";
	//��� .cpp ����� _O2M_sys
	const char* const _O2M_sys_cpp = "CPP/_O2M_sys.cpp";

	//�������� .h ����� _O2M_sys ��� ������
	FILE *f = fopen(_O2M_sys_h, "w");
	if (!f) {
		fprintf(output, textCannotOpenW, _O2M_sys_h);
		return false;
	}
	//������ ����������� .h ����� _O2M_sys
	fprintf(f, comment_format, comment_line_cpp, comment_title, comment_line_cpp);
	fprintf(f, "#ifndef O2M_H_FILE__O2M_sys\n#define O2M_H_FILE__O2M_sys\n\n");
	int i;
	for (i = 0; i < O2M_sys_h_size; i++)
		fprintf(f, "%s\n", O2M_sys_h[i]);
	fprintf(f, "#endif\n");
	//�������� �����
	if (fclose(f)) {
		fprintf(output, textCannotClose, _O2M_sys_h);
		return false;
	};

	//�������� .cpp ����� _O2M_sys ��� ������
	f = fopen(_O2M_sys_cpp, "w");
	if (!f) {
		fprintf(output, textCannotOpenW, _O2M_sys_cpp);
		return false;
	}
	//������ ����������� .cpp ����� _O2M_sys
	fprintf(f, comment_format, comment_line_cpp, comment_title, comment_line_cpp);
	for (i = 0; i < O2M_sys_cpp_size; i++)
		fprintf(f, "%s\n", O2M_sys_cpp[i]);
	//�������� �����
	if (fclose(f)) {
		fprintf(output, textCannotClose, _O2M_sys_cpp);
		return false;
	}

	return true;
}
