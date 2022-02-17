//===============================================================
// Точка входа в программу
//===============================================================

#include "LexAnl.h"
#include "Type.h"
#include "SRC_sys.h"
#include "Project.h"
#include "llvmdriver.h"


//---------------------------------------------------------------
//класс для работы с ресурсами (автоматическая инициализация/деинициализация)
class CResourceManager {
	//временная ссылка на файл для вывода текстовой информации
	//если output_file != NULL, необходимо закрытие файла и восстановкление output
	TFileType* output_file;
public:
	//переопределение вывода в указанный файл вместо stdout
	bool SetOutput(const char* fname);
	//конструктор (инициализация ресурсов)
	CResourceManager() : output_file(NULL)
	{
		//инициализация строки с наибольшим целым числом
		sprintf(CLexBuf::st_LONG_MAX, "%li", LONG_MAX);
		//определение длины полученной строки
		CLexBuf::st_LONG_MAX_len = strlen(CLexBuf::st_LONG_MAX);
	};
	//деструктор (освобождение ресурсов)
	~CResourceManager()
	{
		//закрытие файла для вывода текстовых сообщений (см. ключ '-f')
		if (output_file) {
			output = stdout;
			if (fclose(output_file)) fprintf(output, "ERROR: Cannot close file, specified by switch -f\n");
		}
	};
} ResourceManager;


//---------------------------------------------------------------
//переопределение вывода в указанный файл вместо stdout
bool CResourceManager::SetOutput(const char *fname)
{
	//открытие файла для вывода текстовой информации
	output_file = fopen(fname, "w");
	if (!output_file) {
		fprintf(output, textCannotOpenW, fname);
		return false;
	}
	//установка потока для вывода текстовых сообщений
	output = output_file;
	return true;
}


//---------------------------------------------------------------
//Точка входа в программу
int main(int argc, char *argv[])
{
	//версия O2M
	const char* const Version = "1.8.162";
    const char* const VersionLLVM = "0.2";

	//код ошибки, возвращаемый в операционную систему
	const int ErrExitCode = 1;

	//текст, выводимый при получении неизвестной ошибки
	const char* const textInternalError = "UNKNOWN INTERNAL ERROR: %s\n";

	//для хранения состояния переключателей, задаваемых в командной строке
	bool sw_main = false;
	bool sw_drop = false;
	bool sw_about = true;
	bool sw_version = false;
	bool sw_help = false;
	bool sw_arg_err = false;
    bool sw_LLVM = false;

	//для временного хранения значения табуляции (-1 - недопустимое значение)
	int tab_size = -1;

	//обработка параметров командной строки
	if (argc > 1) {
		for (int i = 1; i < argc; i++)
			if ('-' == argv[i][0] || '/' == argv[i][0]) {
				//получение проверяемой буквы ключа
				char sw = toupper(argv[i][1]);

				//проверка ключа 'a'
				if ('A' == sw && '\0' == argv[i][2]) sw_about = false;
				else

				//проверка ключа 't'
				if ('T' == sw && '\0' == argv[i][2]) {
					//получение размера табуляции из параметров командной строки
					if (++i >= argc) {
						fprintf(output, "ERROR: Not found tab size after -t in command line\n");
						sw_arg_err = true;
						break;
					}
					//преобразование строки в число
					tab_size = atoi(argv[i]);
					//проверка корректности указанного размера табуляции
					if (0 > tab_size || (0 == tab_size && ('0' != argv[i][0] || 0 != argv[i][1]))) {
						fprintf(output, "ERROR: Incorrect tab size format\n");
						sw_arg_err = true;
						break;
					}
				} else

				//проверка ключа 'f'
				if ('F' == sw && '\0' == argv[i][2]) {
					//получение имени файла из параметров командной строки
					if (++i >= argc) {
						fprintf(output, "ERROR: Not found file name after -f in command line\n");
						sw_arg_err = true;
						break;
					}
					//установка файла с заданным именем для вывода текстовых сообщений
					if (!ResourceManager.SetOutput(argv[i])) return ErrExitCode;
				} else

				//проверка ключа 'm'
				if ('M' == sw && '\0' == argv[i][2]) sw_main = true;
				else

				//проверка ключа 'd'
				if ('D' == sw && '\0' == argv[i][2]) sw_drop = true;
				else

				//проверка ключа 'v'
				if ('V' == sw && '\0' == argv[i][2]) sw_version = true;
				else

				//проверка ключа 'h'
				if (('H' == sw || '?' == sw) && '\0' == argv[i][2]) sw_help = true;
                else

                //проверка ключа 'l'
                if ('L' == sw && '\0' == argv[i][2]) sw_LLVM = true;
				else {
					fprintf(output, "ERROR: Unknown command line option %s\n", argv[i]);
					sw_arg_err = true;
					break;
				}
			}
	} else sw_help = true;

	//отображение справочной информации (если надо)
	if (sw_version) {
		fprintf(output, "O2M compiler version %s\n", Version);
        fprintf(output, "Driver LLVM compiler version %s\n",VersionLLVM);
		return 0;
	}
    if (sw_about) {
        fprintf(output, "\nO2M compiler v%s\nCopyright(c) Schwetz Dmitry, 2004\n\n", Version);
        fprintf(output, "Driver LLVM compiler v%s\nCopyright(c) Fomichev Maxim, 2022\n\n",VersionLLVM);
    }
	if (sw_help || sw_arg_err) {
		fprintf(output, "Usage:      O2M <source_file> [<project_file>] [-<switch>...]\n\n<Examples>\n  O2M source.o2m project.pro -a\n  O2M \"source file.o2m\" -m -f \"log.txt\" -a\n\n<Switches>\n");
		fprintf(output, "  m         Specify current file as main file in project\n");
		fprintf(output, "  d         Drop O2M makefile (for first compiled file in project)\n");
		fprintf(output, "  t <size>  Specify tab size (1 by default)\n");
		fprintf(output, "  a         Disable about info\n");
		fprintf(output, "  f <file>  Specify file for output messages instead of stdout\n");
		fprintf(output, "  v         Version info\n");
		fprintf(output, "  h,?       This help\n");
        fprintf(output, "  l         Choose compile file with LLVM-compiler\n");
		return sw_arg_err ? ErrExitCode : 0;
	}

	//создание описания текущего проекта и проверка, помечен ли в настройках проекта текущий файл как главный
	CProject project;
	if (argc > 2 && '-' != argv[2][0] && '/' != argv[2][0])
		if (project.Init(argv[2]))
			fprintf(output, "ERROR: cannot open project file\n");
		else
			if (!sw_main && project.IsMainFile(argv[1])) sw_main = true;

	//занесение в проект размера табуляции (если указан), перекрывая значение в проекте
	if (0 <= tab_size) project.TabSize = tab_size;


	//////////////////////////////////////////////////////////////
	//создание буфера лексем и лексический анализ указанного файла
	CLexBuf *lb = CLexBuf::AnalyseFile(argv[1], project.TabSize);
	if (!lb) return ErrExitCode;


	//////////////////////////////////////////////////////////////
	//синтаксический анализ текущего модуля
	CModule *M = new CModule(NULL);
	int err_num = s_m_Error;
	//инициализация объекта "Модуль"
	try {
		err_num = M->Init(&project, lb);
	}
	catch (error_Internal e_I) {
		fprintf(output, textInternalError, e_I.error_message);
	}
	//проверка отсутствия ошибок после синтаксического анализа
	if (err_num) {
		WriteErr(err_num, lb);
		fprintf(output, "Syntactic analysis of \"%s\" - FAILED\n", argv[1]);
		delete lb;
		delete M;
		return ErrExitCode;
	}
	//уничтожение буфера лексем
	delete lb;


	//////////////////////////////////////////////////////////////
	//подготовка DFN файла и генерация его содержимого
	err_num = s_m_Error;
	try {
		DFN_file dfn_f;
		if (!dfn_f.Init(M->name)) {
			M->WriteDFN(dfn_f);
			err_num = 0;
		}
	}
	catch (error_Internal e_I) {
		fprintf(output, textInternalError, e_I.error_message);
	}
	//проверка отсутствия ошибок после генерации DFN файла
	if (err_num) goto fault_exit;

    if (!sw_LLVM){
        //////////////////////////////////////////////////////////////
        //подготовка файлов и генерация кода C++
        err_num = s_m_Error;
        try {
            CPP_files cpp_f;
            if (!cpp_f.Init(M->name)) {
                M->WriteCPP(cpp_f);
                err_num = 0;
            }
        }
        catch (error_Internal e_I) {
            fprintf(output, textInternalError, e_I.error_message);
        }
        //проверка отсутствия ошибок после генерации кода C++
        if (err_num) goto fault_exit;

    }
    else{

        //////////////////////////////////////////////////////////////
        //подготовка файлов и генерация кода LLVM
        err_num = s_m_Error;
        try {
            LLVMDriver llvmDriver;
            if (!llvmDriver.Init(M))
                err_num = 0;
        }
        catch (error_Internal e_I) {
            fprintf(output, textInternalError, e_I.error_message);
        }
        //проверка отсутствия ошибок после генерации кода LLVM
        if (err_num) goto fault_exit;

    }
    //////////////////////////////////////////////////////////////

	//обработка O2M makefile и главного файла проекта
	//проверка необходимости перезаписи или считывания O2M makefile
	if (sw_drop) {
		if (!project.Create2mk()) goto fault_exit;
	} else
		if (!project.Load2mk()) goto fault_exit;
	//проверка, является ли файл главным файлом проекта
	if (sw_main) {
		//создание _O2M_sys файлов
		if (!create_O2M_sys_files()) goto fault_exit;
		//генерация кода ф-ции main для главного файла проекта
		if (!M->WriteCPP_main()) goto fault_exit;
	} else {
		//занесение текущего файла в список файлов проекта
		project.AddFullPathMakeFile(argv[1], M->name);
	}
	//подготовка доп. списка файлов проекта, на которые имеется ссылка из главного модуля
	if (M->DeclSeq->DfnModuleSeq) M->DeclSeq->DfnModuleSeq->EnumDfnModules(project);
	//запись дополненного O2M makefile (_O2M_make.2mk)
	if (!project.Write2mk(sw_main, M->name)) goto fault_exit;


	//////////////////////////////////////////////////////////////
	//удачное завершение обработки модуля
	fprintf(output, "Module \"%s\" from \"%s\" - OK\n", M->name, argv[1]);
	delete M;
	return 0;

fault_exit:
	//неудачное завершение обработки модуля
	fprintf(output, "Module \"%s\" from \"%s\" - FAILED\n", M->name, argv[1]);
	delete M;
	return ErrExitCode;
};
