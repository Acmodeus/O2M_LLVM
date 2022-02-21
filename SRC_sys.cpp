//=============================================================================
// Создание файлов _O2M_sys.h, _O2M_sys.cpp
//=============================================================================

#include "SRC_sys.h"
#include "Common.h"


//-----------------------------------------------------------------------------
//создание _O2M_sys файлов с проверкой наличия директории CPP
bool create_O2M_sys_files()
{
	//имя .h файла _O2M_sys
	const char* const _O2M_sys_h = "CPP/_O2M_sys.h";
	//имя .cpp файла _O2M_sys
	const char* const _O2M_sys_cpp = "CPP/_O2M_sys.cpp";

	//открытие .h файла _O2M_sys для записи
	FILE *f = fopen(_O2M_sys_h, "w");
	if (!f) {
		fprintf(output, textCannotOpenW, _O2M_sys_h);
		return false;
	}
	//запись содержимого .h файла _O2M_sys
	fprintf(f, comment_format, comment_line_cpp, comment_title, comment_line_cpp);
	fprintf(f, "#ifndef O2M_H_FILE__O2M_sys\n#define O2M_H_FILE__O2M_sys\n\n");
	int i;
	for (i = 0; i < O2M_sys_h_size; i++)
		fprintf(f, "%s\n", O2M_sys_h[i]);
	fprintf(f, "#endif\n");
	//закрытие файла
	if (fclose(f)) {
		fprintf(output, textCannotClose, _O2M_sys_h);
		return false;
	};

	//открытие .cpp файла _O2M_sys для записи
	f = fopen(_O2M_sys_cpp, "w");
	if (!f) {
		fprintf(output, textCannotOpenW, _O2M_sys_cpp);
		return false;
	}
	//запись содержимого .cpp файла _O2M_sys
	fprintf(f, comment_format, comment_line_cpp, comment_title, comment_line_cpp);
	for (i = 0; i < O2M_sys_cpp_size; i++)
		fprintf(f, "%s\n", O2M_sys_cpp[i]);
	//закрытие файла
	if (fclose(f)) {
		fprintf(output, textCannotClose, _O2M_sys_cpp);
		return false;
	}

	return true;
}
