#ifndef INSTALL_H
#define INSTALL_H

#define INST_NONE	0
#define INST_SETINSTDIR	1
#define INST_SETSYSDIR	2
#define INST_SETWINDIR	3
#define INST_PUSHDIR	4
#define INST_POPDIR	5
#define INST_FILE	6
#define INST_DLL	7
#define INST_REGKEY	8
#define INST_REGSTRING	9
#define INST_REGDWORD	10
#define INST_ADDREADME	11
#define INST_ADDSHORTCUT 12

typedef void		(*SetNameCB)(const char*);
typedef void		(*SetMeterCB)(int);
typedef void		(*ErrorCB)(const char*);

int			getRequiredSpace();
int			install(const char* dir,
				ErrorCB, SetNameCB fileName,
				SetMeterCB file, SetMeterCB total,
				SetNameCB readmeName);

#endif
