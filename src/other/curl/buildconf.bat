@echo off
REM set up a CVS tree to build when there's no autotools
REM $Revision: 1.4 $
REM $Date: 2007-01-29 00:51:02 $

REM create ca-bundle.h
echo /* This file is generated automatically */ >lib\ca-bundle.h

REM create hugehelp.c
copy src\hugehelp.c.cvs src\hugehelp.c

REM create Makefile
copy Makefile.dist Makefile