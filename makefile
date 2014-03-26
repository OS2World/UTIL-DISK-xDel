# makefile
# Created by IBM WorkFrame/2 MakeMake at 1:18:59 on 8 Aug 1999
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker

.SUFFIXES:

.SUFFIXES: \
    .c .obj 

.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /Ss /O /Gm /C %s

{e:\data\c\xdel}.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /Ss /O /Gm /C %s

all: \
    .\xDel.exe

.\xDel.exe: \
    .\xdel.obj \
    {$(LIB)}os2386.lib \
    {$(LIB)}cppom30.lib \
    {$(LIB)}xdel.def
    @echo " Link::Linker "
    icc.exe @<<
     /B" /exepack:2 /packd /optfunc"
     /FexDel.exe 
     os2386.lib 
     cppom30.lib 
     xdel.def
     .\xdel.obj
<<

.\xdel.obj: \
    e:\data\c\xdel\xdel.c \
    {e:\data\c\xdel;$(INCLUDE);}xdel.h
