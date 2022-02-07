MAIN_NAME = main
MAIN_NAME_1 = add
EXE_NAME = T

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\$(EXE_NAME).exe"


CLEAN :
	-@erase "$(INTDIR)\$(MAIN_NAME_1).obj"
	-@erase "$(INTDIR)\$(MAIN_NAME).obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\$(EXE_NAME).exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\$(EXE_NAME).pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\$(EXE_NAME).bsc"
BSC32_SBRS= \


LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\$(EXE_NAME).pdb" /machine:I386 /out:"$(OUTDIR)\$(EXE_NAME).exe"
LINK32_OBJS= \
	"$(INTDIR)\$(MAIN_NAME_1).obj" \
	"$(INTDIR)\$(MAIN_NAME).obj" 

"$(OUTDIR)\$(EXE_NAME).exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<
