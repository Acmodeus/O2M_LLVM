//===============================================================
// Объявление процедур, необходимых для лексической обработки
//===============================================================

#ifndef O2M_LexAnl_h
#define O2M_LexAnl_h

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <vector>


//---------------------------------------------------------------
//список типов лексем
enum ELexType {
	lex_n,		//конец строки исх. файла
	lex_d,		//десятичное число
	lex_h,		//шестнадцат. число
	lex_r,		//число типа REAL
	lex_l,		//число типа LONGREAL
	lex_i,		//идентификатор
	lex_s,		//строка
	lex_c,		//символьная константа
	//ключевые слова, признак отсутствия кл. слова: lex_k_dot > LexType
	lex_k_dot,			// .
	lex_k_dots,			// ..
	lex_k_assign,		// :=
	lex_k_add_assign,	// +=
	lex_k_le,			// <=
	lex_k_ge,			// >=
	lex_k_semicolon,	// ;
	lex_k_colon,		// :
	lex_k_lt,			// <
	lex_k_gt,			// >
	lex_k_op_bracket,	// (
	lex_k_cl_bracket,	// )
	lex_k_op_square,	// [
	lex_k_cl_square,	// ]
	lex_k_op_brace,		// {
	lex_k_cl_brace,		// }
	lex_k_plus,			// +
	lex_k_minus,		// -
	lex_k_asterix,		// *
	lex_k_slash,		// /
	lex_k_eq,			// =
	lex_k_ne,			// #
	lex_k_negation,		// ~
	lex_k_and,			// &
	lex_k_comma,		// ,
	lex_k_up_arrow,		// ^
	lex_k_vertical,		// |
	lex_k_BY,
	lex_k_DO,
	lex_k_IF,
	lex_k_IN,
	lex_k_IS,
	lex_k_OF,
	lex_k_OR,
	lex_k_TO,
	lex_k_DIV,
	lex_k_END,
	lex_k_FOR,
	lex_k_MOD,
	lex_k_NIL,
	lex_k_VAR,
	lex_k_ABS,
	lex_k_ASH,
	lex_k_CAP,
	lex_k_CHR,
	lex_k_DEC,
	lex_k_INC,
	lex_k_LEN,
	lex_k_MAX,
	lex_k_MIN,
	lex_k_NEW,
	lex_k_ODD,
	lex_k_ORD,
	lex_k_SET,
	lex_k_CASE,
	lex_k_ELSE,
	lex_k_EXIT,
	lex_k_LOOP,
	lex_k_THEN,
	lex_k_TYPE,
	lex_k_WITH,
	lex_k_CHAR,
	lex_k_COPY,
	lex_k_EXCL,
	lex_k_HALT,
	lex_k_INCL,
	lex_k_LONG,
	lex_k_REAL,
	lex_k_SIZE,
	lex_k_TRUE,
	lex_k_BEGIN,
	lex_k_CONST,
	lex_k_ELSIF,
	lex_k_ARRAY,
	lex_k_UNTIL,
	lex_k_WHILE,
	lex_k_FALSE,
	lex_k_LOCAL,
	lex_k_SHORT,
	lex_k_ASSERT,
	lex_k_IMPORT,
	lex_k_MODULE,
	lex_k_RECORD,
	lex_k_REPEAT,
	lex_k_RETURN,
	lex_k_ENTIER,
	lex_k_POINTER,
	lex_k_BOOLEAN,
	lex_k_INTEGER,
	lex_k_LONGINT,
	lex_k_LONGREAL,
	lex_k_SHORTINT,
	lex_k_PROCEDURE,
	lex_k_DEFINITION
};


//---------------------------------------------------------------
//тип обрабатываемых файлов
typedef FILE TFileType;


//---------------------------------------------------------------
//структура для загрузки необходимой для синт. анализа информации о текущей лексеме
struct CLexInfo
{
	ELexType lex;		//тип лексемы
	const char* st;		//строка с содержимым лексемы
};


//---------------------------------------------------------------
//структура, содержащая полную информацию об 1 лексеме
struct CLex {
	ELexType lex;		//тип лексемы
	char* st;			//строка с содержимым лексемы
	int pos;			//позиция начала лексемы в строке
};


//---------------------------------------------------------------
//для объявления формального параметра типа CLexBufPos
struct CLexBufPos;


//---------------------------------------------------------------
//буфер лексем, содержит лексемы, полученные после обработки 1 модуля
class CLexBuf
{
public:
	CLexBuf() : CurrStNum(1), CurrStPos(1) {};
	~CLexBuf();
	//тип контейнера лексем
	typedef std::vector <CLex> BufType;
	//тип итератора для хранения позиции в буфере лексем
	typedef BufType::const_iterator CurrLexType;
	//получение текущей позиции буфера лексем
	CLexBufPos GetCurrPos();
	//получение текущего номера строки
	int GetCurrStNum() const {return CurrStNum;};
	//получение позиции в строке исходного файла последней считанной лексемы
	int GetCurrStPos() const {return CurrStPos;};
	//установка текущей позиции буфера лексем
	void SetCurrPos(CLexBufPos LexBufPos);
	//установка текущей позиции и номера строки на начало буфера
	void ResetCurrPos();
	//чтение информации о лексеме из буфера
	bool ReadLex(CLexInfo &LexInfo);
	//запуск cинтаксического анализатора, возврат - ук. на буфер лексем
	static CLexBuf* AnalyseFile(const char* fname, const int tab_size);
	//переменная для хранения строки с максимальным целым числом, инициализируется в ResourceManager
	//100 символов должно хватить даже для 256 разрядной системы :)
	static char st_LONG_MAX[100];
	//реальная длина строки с максимальным целым числом
    static int st_LONG_MAX_len;
private:
	//контейнер лексем
	BufType Buf;
	//для хранения текущей позиции в контейнере лексем
	CurrLexType CurrLex;
	//номер строки в исходном файле для текущей лексемы
	int CurrStNum;
	//позиция последней считанной лексемы в исходном файле
	int CurrStPos;
	//добавление в буфер лексемы
	void WriteLex(const enum ELexType LexType, const char* st, const int pos);
	//запуск лексического анализа файла f, tab_size - размер табуляции
	int analyser (TFileType *f, const int tab_size);
	//проверка слишком большого целого числа, true - число слишком велико
	bool test_big_number(const char* st, int st_len);
	//проверка числа в позиции i строки st (buf - буфер)
	//если число найдено, i - на последний символ числа
	int test_digit (const char *st, char *buf, int &i, const int pos);
	//проверка наличия в строке st (не менее 2 символов, включая 0) ключевого слова
	//если найдено кл. слово, прибавление к i и pos длины найденного слова - 1
	bool test_keyword(const char *st, int &i, int &pos);
};//CLexBuf


//---------------------------------------------------------------
//структура для сохранения/восстановления текущей позиции в буфере лексем
struct CLexBufPos {
	//итератор для сохранения позиции в буфере лексем
	CLexBuf::CurrLexType CurrLex;
	//текущий номер обрабатываемой строки в исходном файле
	int CurrStNum;
	//позиция текущей лексемы в исходном файле
	int CurrStPos;
};


//---------------------------------------------------------------
//объявление потока для вывода текстовой информации
extern TFileType *output;


//---------------------------------------------------------------
//размер буфера для считывания строки
const int MaxStLeng = 32000;


//---------------------------------------------------------------
//список номеров ошибок
enum
{
//отсутствие ошибки, сообщение используется для "Internal error: Wrong error code"
	s_m_ZeroErrorCode,
//системные сообщения, не являются кодами ошибок, не имеют текстов сообщений
	s_m_IdentDefAbsent,		//ident absent
	s_m_CaseAbsent,			//CaseAbsent
	s_m_CommonProcFound,	//Common or handler procedure was found
	s_m_HProcFound,			//Handler procedure was found
//отладочное сообщение об ошибке, номер первого допустимого сообщения об ошибке (кроме s_m_ZeroErrorCode)
	s_m_Error,				//Unknown internal error
//предупреждения (Warnings)
	s_w_NotAllPathsRETURN,	//Not all control paths return a value
//лексические ошибки
	l_e_OpenComment,		//Comment not closed
	l_e_OpenCommas,			//Commas not closed
	l_e_LastScaleDigit,		//Not found last digit in scale factor
	l_e_LastHexDigit,		//Hex number not terminated
	l_e_IncorrectSymbol,	//Incorrect symbol
	l_e_CharConstTooLong,	//Char constant too long
	l_e_NumberTooLarge,		//Number too large
//синтаксические ошибки Оберон-2
	s_e_UndeclaredIdent,	//Undeclared identifier
	s_e_Redefinition,		//Multiply defined identifier
	s_e_Incompatible,		//Incompatible assignment
	s_e_IdentExpected,		//Identifier expected
	s_e_IdentWrongMarked,	//Illegally marked identifier
	s_e_IdentNotType,		//Identifier does not denote a type
	s_e_IdentNotRecordType,	//Identifier does not denote a record type
	s_e_ModuleEndName,		//Identifier does not match module name
	s_e_ProcedureEndName,	//Identifier does not match procedure name
	s_e_EqualSignExpected,	//"=" expected
	s_e_DotMissing,			//"." missing
	s_e_SemicolonMissing,	//";" missing
	s_e_CommaMissing,		//"," missing
	s_e_ColonMissing,		//":" missing
	s_e_AssignMissing,		//":=" missing
	s_e_OpBracketMissing,	//"(" missing
	s_e_ClBracketMissing,	//")" missing
	s_e_ClSquareMissing,	//"]" missing
	s_e_ClBraceMissing,		//"}" missing
	s_e_DfnFileNotFound,	//Definition file of imported module not found
	s_e_DfnFileError,		//Error in definition file of imported module
	s_e_RecursiveImport,	//Recursive import not allowed
	s_e_ReceiverVAR,		//Pointer or VAR record required as receiver
	s_e_Statement,			//Statement starts with incorrect symbol
	s_e_Factor,				//Factor starts with incorrect symbol
	s_e_AssignConst,		//Assignment to constant
	s_e_AssignReadonly,		//Assignment to read-only variable (field)
	s_e_ExprCompatibility,	//Compatibility error in expression
	s_e_ExprNotConst,		//Expression should be constant
	s_e_ExprNotIntConst,	//Expression should be integer constant
	s_e_ExprNotPosIntConst,	//Expression should be positive integer constant
	s_e_SetElemType,		//Set element type is not an integer
	s_e_SetElemRange,		//Set element greater than MAX(SET) or less than 0
	s_e_SetRange,			//Lower bound of set range greater than higher bound
	s_e_DivisionByZero,		//Division by zero
	s_e_RETURN_ExprInModule,//Returning expression from module
	s_e_RETURN_ExprInProc,	//Returning expression from procedure
	s_e_EXIT_NotInLoop,		//EXIT not within loop statement
	s_e_FOR_VarNotInt,		//Control variable must be integer
	s_e_FOR_BY_Zero,		//Step in FOR statement equals zero
	s_e_UNTIL,				//UNTIL missing
	s_e_UNTIL_ExprType,		//Type of expression following UNTIL is not BOOLEAN
	s_e_WHILE_ExprType,		//Type of expression following WHILE is not BOOLEAN
	s_e_ASSERT_ExprType,	//Type of expression following ASSERT is not BOOLEAN
	s_e_IF_ExprType,		//Type of expression following IF or ELSIF is not BOOLEAN
	s_e_CASE_Expr,			//Type of case expression is neither INTEGER nor CHAR
	s_e_CASE_LabelType,		//Inadmissible type of case label
	s_e_CASE_WrongLabelType,//Wrong type of case label
	s_e_CASE_LabelExists,	//Case label value occured more than once
	s_e_DO,					//DO missing
	s_e_TO,					//TO missing
	s_e_OF,					//OF missing
	s_e_END,				//END missing
	s_e_THEN,				//THEN missing
	s_e_BEGIN,				//BEGIN missing
	s_e_DEFINITION,			//DEFINITION expected
	s_e_MODULE,				//MODULE expected
	s_e_MODULE_SYSTEM,		//Name SYSTEM reserved for SYSTEM module
	s_e_CallNotProc,		//Called object is not a procedure
	s_e_CallFuncAsProc,		//Procedure call of a function
	s_e_DesNotPointer,		//Dereferenced variable is not a pointer
	s_e_DesNotArray,		//Indexed variable is not an array
	s_e_DesRecordField,		//Undefined record field
	s_e_OperandNotVariable,	//Operand is not a variable
	s_e_OperandReadOnly,	//Operand is read only
	s_e_OperandInappl,		//Inapplicable operand
	s_e_ParamNotMatch,		//Parameter does not match
	s_e_ParamCountNotMatch,	//Number of parameters does not match
	s_e_ParamNotIntConst,	//Parameter must be an integer constant
	s_e_ParsFewer,			//Fewer actual than formal parameters
	s_e_ParsMore,			//More actual than formal parameters
	s_e_GuardVarNotRecOrP,	//Guarded variable is neither a pointer nor a VAR-parameter record
	s_e_GuardTypeNotP,		//Guard type is not a pointer
	s_e_GuardTypeNotRec,	//Guard type is not a record
	s_e_GuardTypeNotExt,	//Guard type is not an extension of variable type
	s_e_LEN_NotArray,		//LEN not applied to array
	s_e_LEN_Dimension,		//Dimension in LEN too large or negative
	s_e_MAX_NotType,		//MAX not applied to basic type
	s_e_MIN_NotType,		//MIN not applied to basic type
	s_e_ForwardDeclUnsat,	//Unsatisfied forward procedure
	s_e_ForwardDeclDfnFile,	//Forward procedure in definition file
	s_e_TypeDefinition,		//Type definition starts with incorrect symbol
	s_e_TypeNotExternal,	//All base/synonym types must be exported/external together
	s_e_OpenArrayNotAllowed,//Open array not allowed as variable, record field or array element
	s_e_FuncWithoutRETURN,	//Function without RETURN
/**/
//неустоявшиеся синтаксические ошибки
	s_e_CommonProcCommonParam,		//Absent common parameters in common procedure
	s_e_CommonProcParamRedefined,	//Common parameter redefined in common proc
	s_e_CommonProcDoubleType,		//Double definition of function type
	s_e_HandlerProcCommonParam,		//Absent common parameters in handler procedure
	s_e_HandlerProcParamRedefined,	//Common parameter redefined in handler proc
//синтаксические ошибки, характерные только для О2М
	s_e_IdentNotModule,		//Identifier does not denote a module
	s_e_AddAssignMissing,	//"+=" missing
	s_e_OpAngleMissing,		//"<" missing
	s_e_ClAngleMissing,		//">" missing
	s_e_ZeroMissing,		//"0" missing
	s_e_CommonTypeExpected,	//Common type expected
	s_e_CommonTypeSpecType,	//Incorrect type of specialization in common type
	s_e_CommonTypeLocalExt,	//Extension of imported common type with LOCAL attribute
	s_e_SpecTypeExpected,	//Specialized common type expected
	s_e_SpecRedefining,		//Specialization multiply defined
	s_e_SpecTypeTag,		//Incorrect tag in specialization
	s_e_NoDefaultSpec,		//Default specialization not declared
	s_e_PointerTypeKind,	//Pointer not bound to record, array or common type
	s_e_CommonProcCallMixed,//Mixed different styles of common procedure call
	s_e_CommonParVAR,		//Pointer or VAR common parameter required
	s_e_CommonParsExpected,	//Actual common parameters expected
	s_e_CommonParsFewer,	//Fewer actual than formal common parameters
	s_e_CommonParsMore,		//More actual than formal common parameters
	s_e_HProcInDfn,			//Handler procedure in definition file
	s_e_FreeHProc,			//Handler procedure not bound to common procedure
//максимальный код ошибки, размер массива сообщений об ошибках
	s_m_MaxErrorCode		//сообщение отсутствует
};


//---------------------------------------------------------------
//сообщения об ошибках
static char *err_arr[s_m_MaxErrorCode] = 
{
	"Internal error: Wrong error code",				//s_m_ZeroErrorCode
	"",												//s_m_IdentDefAbsent
	"",												//s_m_CaseAbsent
	"",												//s_m_CommonProcFound
	"",												//s_m_HProcFound
	"Unknown internal error",						//s_m_Error
//предупреждения (Warnings)
	"Not all control paths return a value",			//s_w_NotAllPathsRETURN
//лексические ошибки
	"Comment not closed",							//l_e_OpenComment
	"Commas not closed",							//l_e_OpenCommas
	"Not found last digit in scale factor",			//l_e_LastScaleDigit
	"Hex number not terminated",					//l_e_LastHexDigit
	"Incorrect symbol",								//l_e_IncorrectSymbol
	"Char constant too long",						//l_e_CharConstTooLong
	"Number too large",								//l_e_NumberTooLarge
//синтаксические ошибки Оберон-2
	"Undeclared identifier",						//s_e_UndeclaredIdent
	"Multiply defined identifier",					//s_e_Redefinition
	"Incompatible assignment",						//s_e_Incompatible
	"Identifier expected",							//s_e_IdentExpected
	"Illegally marked identifier",					//s_e_IdentWrongMarked
	"Identifier does not denote a type",			//s_e_IdentNotType
	"Identifier does not denote a record type",		//s_e_IdentNotRecordType
	"Identifier does not match module name",		//s_e_ModuleEndName
	"Identifier does not match procedure name",		//s_e_ProcedureEndName
	"\"=\" expected",								//s_e_EqualSignExpected
	"\".\" missing",								//s_e_DotMissing
	"\";\" missing",								//s_e_SemicolonMissing
	"\",\" missing",								//s_e_CommaMissing
	"\":\" missing",								//s_e_ColonMissing
	"\":=\" missing",								//s_e_AssignMissing
	"\"(\" missing",								//s_e_OpBracketMissing
	"\")\" missing",								//s_e_ClBracketMissing
	"\"]\" missing",								//s_e_ClSquareMissing
	"\"}\" missing",								//s_e_ClBraceMissing
	"Definition file of imported module not found",	//s_e_DfnFileNotFound
	"Error in definition file of imported module",	//s_e_DfnFileError
	"Recursive import not allowed",					//s_e_RecursiveImport
	"Pointer or VAR record required as receiver",	//s_e_ReceiverVAR
	"Statement starts with incorrect symbol",		//s_e_Statement
	"Factor starts with incorrect symbol",			//s_e_Factor
	"Assignment to constant",						//s_e_AssignConst
	"Assignment to read-only variable (field)",		//s_e_AssignReadonly
	"Compatibility error in expression",			//s_e_ExprCompatibility
	"Expression should be constant",				//s_e_ExprNotConst
	"Expression should be integer constant",		//s_e_ExprNotIntConst
	"Expression should be positive integer constant",		//s_e_ExprNotPosIntConst
	"Set element type is not an integer",			//s_e_SetElemType
	"Set element greater than MAX(SET) or less than 0",		//s_e_SetElemRange
	"Lower bound of set range greater than higher bound",	//s_e_SetRange
	"Division by zero",								//s_e_DivisionByZero
	"Returning expression from module",				//s_e_RETURN_ExprInModule
	"Returning expression from procedure",			//s_e_RETURN_ExprInProc
	"EXIT not within loop statement",				//s_e_EXIT_NotInLoop
	"Control variable must be integer",				//s_e_FOR_VarNotInt
	"Step in FOR statement equals zero",			//s_e_FOR_BY_Zero
	"UNTIL missing",								//s_e_UNTIL
	"Type of expression following UNTIL is not BOOLEAN",	//s_e_UNTIL_ExprType
	"Type of expression following WHILE is not BOOLEAN",	//s_e_WHILE_ExprType
	"Type of expression following ASSERT is not BOOLEAN",	//s_e_ASSERT_ExprType
	"Type of expression following IF or ELSIF is not BOOLEAN",	//s_e_IF_ExprType
	"Type of case expression is neither INTEGER nor CHAR",	//s_e_CASE_Expr
	"Inadmissible type of case label",				//s_e_CASE_LabelType
	"Wrong type of case label",						//s_e_CASE_WrongLabelType
	"Case label value occured more than once",		//s_e_CASE_LabelExists
	"DO missing",									//s_e_DO
	"TO missing",									//s_e_TO
	"OF missing",									//s_e_OF
	"END missing",									//s_e_END
	"THEN missing",									//s_e_THEN
	"BEGIN missing",								//s_e_BEGIN
	"DEFINITION expected",							//s_e_DEFINITION
	"MODULE expected",								//s_e_MODULE
	"Name SYSTEM reserved for SYSTEM module",		//s_e_MODULE_SYSTEM
	"Called object is not a procedure",				//s_e_CallNotProc
	"Procedure call of a function",					//s_e_CallFuncAsProc
	"Dereferenced variable is not a pointer",		//s_e_DesNotPointer
	"Indexed variable is not an array",				//s_e_DesNotArray
	"Undefined record field",						//s_e_DesRecordField
	"Operand is not a variable",					//s_e_OperandNotVariable
	"Operand is read only",							//s_e_OperandReadOnly
	"Inapplicable operand",							//s_e_OperandInappl
	"Parameter does not match",						//s_e_ParamNotMatch
	"Number of parameters does not match",			//s_e_ParamCountNotMatch
	"Parameter must be an integer constant",		//s_e_ParamNotIntConst
	"Fewer actual than formal parameters",			//s_e_ParsFewer
	"More actual than formal parameters",			//s_e_ParsMore
	"Guarded variable is neither a pointer nor a VAR-parameter record",	//s_e_GuardVarNotRecOrP
	"Guard type is not a pointer",					//s_e_GuardTypeNotP
	"Guard type is not a record",					//s_e_GuardTypeNotRec
	"Guard type is not an extension of variable type",	//s_e_GuardTypeNotExt
	"LEN not applied to array",						//s_e_LEN_NotArray
	"Dimension in LEN too large or negative",		//s_e_LEN_Dimension
	"MAX not applied to basic type",				//s_e_MAX_NotType
	"MIN not applied to basic type",				//s_e_MIN_NotType
	"Unsatisfied forward procedure",				//s_e_ForwardDeclUnsat
	"Forward procedure in definition file",			//s_e_ForwardDeclDfnFile
	"Type definition starts with incorrect symbol",	//s_e_TypeDefinition
	"All base/synonym types must be exported/external together",		//s_e_TypeNotExternal
	"Open array not allowed as variable, record field or array element",//s_e_OpenArrayNotAllowed
	"Function without RETURN",						//s_e_FuncWithoutRETURN
/**/
//неустоявшиеся синтаксические ошибки
	"Absent common parameters in common procedure",	//s_e_CommonProcCommonParam
	"Common parameter redefined in common proc",	//s_e_CommonProcParamRedefined
	"Double definition of function type",			//s_e_CommonProcDoubleType
	"Absent common parameters in handler procedure",//s_e_HandlerProcCommonParam
	"Common parameter redefined in handler proc",	//s_e_HandlerProcParamRedefined
//синтаксические ошибки, характерные только для О2М
	"Identifier does not denote a module",				//s_e_IdentNotModule
	"\"+=\" missing",									//s_e_AddAssignMissing
	"\"<\" missing",									//s_e_OpAngleMissing
	"\">\" missing",									//s_e_ClAngleMissing
	"\"0\" missing",									//s_e_ZeroMissing
	"Common type expected",								//s_e_CommonTypeExpected
	"Incorrect type of specialization in common type",	//s_e_CommonTypeSpecType
	"Extension of imported common type with LOCAL attribute",	//s_e_CommonTypeLocalExt
	"Specialized common type expected",					//s_e_SpecTypeExpected
	"Specialization multiply defined",					//s_e_SpecRedefining
	"Incorrect tag in specialization",					//s_e_SpecTypeTag
	"Default specialization not declared",				//s_e_NoDefaultSpec
	"Pointer not bound to record, array or common type",//s_e_PointerTypeKind
	"Mixed different styles of common procedure call",	//s_e_CommonProcCallMixed
	"Pointer or VAR common parameter required",			//s_e_CommonParVAR
	"Actual common parameters expected",				//s_e_CommonParsExpected
	"Fewer actual than formal common parameters",		//s_e_CommonParsFewer
	"More actual than formal common parameters",		//s_e_CommonParsMore
	"Handler procedure in definition file",				//s_e_HProcInDfn
	"Handler procedure not bound to common procedure",	//s_e_FreeHProc
};


//---------------------------------------------------------------
//выдача сообщения об ошибке
void WriteErr(int err_num, const CLexBuf *lb);


//---------------------------------------------------------------
//выдача предупреждения (Warning)
void WriteWarn(int warn_num, const CLexBuf *lb);


#endif	//O2M_LexAnl_h
