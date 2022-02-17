//===============================================================
// ����� ����� � ���������
//===============================================================

#include "LexAnl.h"
#include "Type.h"
#include "SRC_sys.h"
#include "Project.h"
#include "llvmdriver.h"


//---------------------------------------------------------------
//����� ��� ������ � ��������� (�������������� �������������/���������������)
class CResourceManager {
	//��������� ������ �� ���� ��� ������ ��������� ����������
	//���� output_file != NULL, ���������� �������� ����� � ��������������� output
	TFileType* output_file;
public:
	//��������������� ������ � ��������� ���� ������ stdout
	bool SetOutput(const char* fname);
	//����������� (������������� ��������)
	CResourceManager() : output_file(NULL)
	{
		//������������� ������ � ���������� ����� ������
		sprintf(CLexBuf::st_LONG_MAX, "%li", LONG_MAX);
		//����������� ����� ���������� ������
		CLexBuf::st_LONG_MAX_len = strlen(CLexBuf::st_LONG_MAX);
	};
	//���������� (������������ ��������)
	~CResourceManager()
	{
		//�������� ����� ��� ������ ��������� ��������� (��. ���� '-f')
		if (output_file) {
			output = stdout;
			if (fclose(output_file)) fprintf(output, "ERROR: Cannot close file, specified by switch -f\n");
		}
	};
} ResourceManager;


//---------------------------------------------------------------
//��������������� ������ � ��������� ���� ������ stdout
bool CResourceManager::SetOutput(const char *fname)
{
	//�������� ����� ��� ������ ��������� ����������
	output_file = fopen(fname, "w");
	if (!output_file) {
		fprintf(output, textCannotOpenW, fname);
		return false;
	}
	//��������� ������ ��� ������ ��������� ���������
	output = output_file;
	return true;
}


//---------------------------------------------------------------
//����� ����� � ���������
int main(int argc, char *argv[])
{
	//������ O2M
	const char* const Version = "1.8.162";
    const char* const VersionLLVM = "0.2";

	//��� ������, ������������ � ������������ �������
	const int ErrExitCode = 1;

	//�����, ��������� ��� ��������� ����������� ������
	const char* const textInternalError = "UNKNOWN INTERNAL ERROR: %s\n";

	//��� �������� ��������� ��������������, ���������� � ��������� ������
	bool sw_main = false;
	bool sw_drop = false;
	bool sw_about = true;
	bool sw_version = false;
	bool sw_help = false;
	bool sw_arg_err = false;
    bool sw_LLVM = false;

	//��� ���������� �������� �������� ��������� (-1 - ������������ ��������)
	int tab_size = -1;

	//��������� ���������� ��������� ������
	if (argc > 1) {
		for (int i = 1; i < argc; i++)
			if ('-' == argv[i][0] || '/' == argv[i][0]) {
				//��������� ����������� ����� �����
				char sw = toupper(argv[i][1]);

				//�������� ����� 'a'
				if ('A' == sw && '\0' == argv[i][2]) sw_about = false;
				else

				//�������� ����� 't'
				if ('T' == sw && '\0' == argv[i][2]) {
					//��������� ������� ��������� �� ���������� ��������� ������
					if (++i >= argc) {
						fprintf(output, "ERROR: Not found tab size after -t in command line\n");
						sw_arg_err = true;
						break;
					}
					//�������������� ������ � �����
					tab_size = atoi(argv[i]);
					//�������� ������������ ���������� ������� ���������
					if (0 > tab_size || (0 == tab_size && ('0' != argv[i][0] || 0 != argv[i][1]))) {
						fprintf(output, "ERROR: Incorrect tab size format\n");
						sw_arg_err = true;
						break;
					}
				} else

				//�������� ����� 'f'
				if ('F' == sw && '\0' == argv[i][2]) {
					//��������� ����� ����� �� ���������� ��������� ������
					if (++i >= argc) {
						fprintf(output, "ERROR: Not found file name after -f in command line\n");
						sw_arg_err = true;
						break;
					}
					//��������� ����� � �������� ������ ��� ������ ��������� ���������
					if (!ResourceManager.SetOutput(argv[i])) return ErrExitCode;
				} else

				//�������� ����� 'm'
				if ('M' == sw && '\0' == argv[i][2]) sw_main = true;
				else

				//�������� ����� 'd'
				if ('D' == sw && '\0' == argv[i][2]) sw_drop = true;
				else

				//�������� ����� 'v'
				if ('V' == sw && '\0' == argv[i][2]) sw_version = true;
				else

				//�������� ����� 'h'
				if (('H' == sw || '?' == sw) && '\0' == argv[i][2]) sw_help = true;
                else

                //�������� ����� 'l'
                if ('L' == sw && '\0' == argv[i][2]) sw_LLVM = true;
				else {
					fprintf(output, "ERROR: Unknown command line option %s\n", argv[i]);
					sw_arg_err = true;
					break;
				}
			}
	} else sw_help = true;

	//����������� ���������� ���������� (���� ����)
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

	//�������� �������� �������� ������� � ��������, ������� �� � ���������� ������� ������� ���� ��� �������
	CProject project;
	if (argc > 2 && '-' != argv[2][0] && '/' != argv[2][0])
		if (project.Init(argv[2]))
			fprintf(output, "ERROR: cannot open project file\n");
		else
			if (!sw_main && project.IsMainFile(argv[1])) sw_main = true;

	//��������� � ������ ������� ��������� (���� ������), ���������� �������� � �������
	if (0 <= tab_size) project.TabSize = tab_size;


	//////////////////////////////////////////////////////////////
	//�������� ������ ������ � ����������� ������ ���������� �����
	CLexBuf *lb = CLexBuf::AnalyseFile(argv[1], project.TabSize);
	if (!lb) return ErrExitCode;


	//////////////////////////////////////////////////////////////
	//�������������� ������ �������� ������
	CModule *M = new CModule(NULL);
	int err_num = s_m_Error;
	//������������� ������� "������"
	try {
		err_num = M->Init(&project, lb);
	}
	catch (error_Internal e_I) {
		fprintf(output, textInternalError, e_I.error_message);
	}
	//�������� ���������� ������ ����� ��������������� �������
	if (err_num) {
		WriteErr(err_num, lb);
		fprintf(output, "Syntactic analysis of \"%s\" - FAILED\n", argv[1]);
		delete lb;
		delete M;
		return ErrExitCode;
	}
	//����������� ������ ������
	delete lb;


	//////////////////////////////////////////////////////////////
	//���������� DFN ����� � ��������� ��� �����������
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
	//�������� ���������� ������ ����� ��������� DFN �����
	if (err_num) goto fault_exit;

    if (!sw_LLVM){
        //////////////////////////////////////////////////////////////
        //���������� ������ � ��������� ���� C++
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
        //�������� ���������� ������ ����� ��������� ���� C++
        if (err_num) goto fault_exit;

    }
    else{

        //////////////////////////////////////////////////////////////
        //���������� ������ � ��������� ���� LLVM
        err_num = s_m_Error;
        try {
            LLVMDriver llvmDriver;
            if (!llvmDriver.Init(M))
                err_num = 0;
        }
        catch (error_Internal e_I) {
            fprintf(output, textInternalError, e_I.error_message);
        }
        //�������� ���������� ������ ����� ��������� ���� LLVM
        if (err_num) goto fault_exit;

    }
    //////////////////////////////////////////////////////////////

	//��������� O2M makefile � �������� ����� �������
	//�������� ������������� ���������� ��� ���������� O2M makefile
	if (sw_drop) {
		if (!project.Create2mk()) goto fault_exit;
	} else
		if (!project.Load2mk()) goto fault_exit;
	//��������, �������� �� ���� ������� ������ �������
	if (sw_main) {
		//�������� _O2M_sys ������
		if (!create_O2M_sys_files()) goto fault_exit;
		//��������� ���� �-��� main ��� �������� ����� �������
		if (!M->WriteCPP_main()) goto fault_exit;
	} else {
		//��������� �������� ����� � ������ ������ �������
		project.AddFullPathMakeFile(argv[1], M->name);
	}
	//���������� ���. ������ ������ �������, �� ������� ������� ������ �� �������� ������
	if (M->DeclSeq->DfnModuleSeq) M->DeclSeq->DfnModuleSeq->EnumDfnModules(project);
	//������ ������������ O2M makefile (_O2M_make.2mk)
	if (!project.Write2mk(sw_main, M->name)) goto fault_exit;


	//////////////////////////////////////////////////////////////
	//������� ���������� ��������� ������
	fprintf(output, "Module \"%s\" from \"%s\" - OK\n", M->name, argv[1]);
	delete M;
	return 0;

fault_exit:
	//��������� ���������� ��������� ������
	fprintf(output, "Module \"%s\" from \"%s\" - FAILED\n", M->name, argv[1]);
	delete M;
	return ErrExitCode;
};
