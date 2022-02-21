//=============================================================================
// Содержимое файлов _O2M_sys.h, _O2M_sys.cpp
//=============================================================================

#ifndef O2M_SRC_sys_h
#define O2M_SRC_sys_h


//содержимое файла _O2M_sys.h
static const int O2M_sys_h_size = 9;

static char *O2M_sys_h[O2M_sys_h_size] =
{
	"#include <string.h>",
	"#include <stdlib.h>",
	"#include <ctype.h>",
	"#include <math.h>",
	"#include \"_O2M_ppp.h\"\n",
	"long MOD(const long x, const long y);",
	"int ORD(const char ch);",
	"void COPY(const char *x, const int x_size, char *v, const int v_size);",
	"int O2M_SET_RANGE(const int l, const int h);\n"
};

//содержимое файла _O2M_sys.cpp
static const int O2M_sys_cpp_size = 17;

static char *O2M_sys_cpp[O2M_sys_cpp_size] =
{
	"#include <math.h>\n",
	"long MOD(const long x, const long y) {",
	"\treturn x - long(floor(double(x) / y)) * y;",
	"}\n",
	"int ORD(const char ch) {",
	"\treturn (ch < 0) ? (ch + 256) : ch;",
	"}\n",
	"void COPY(const char *x, const int x_size, char *v, const int v_size) {",
	"\tint min = x_size;",
	"\tif (v_size < x_size) min = v_size;",
	"\tfor (int i = 0; i <= min-2; i++) v[i] = x[i];",
	"\tv[min-1] = 0;",
	"}\n",
	"int O2M_SET_RANGE(const int l, const int h) {",
	"\tint res = 0;",
	"\tfor (int i = l; i <= h; i++) res |= 1 << i;",
	"\treturn res;\n}"
};

//создание _O2M_sys файлов с проверкой наличия директории CPP
bool create_O2M_sys_files();

#endif	//O2M_SRC_sys_h
