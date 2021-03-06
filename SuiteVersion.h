#ifndef SUITEVERSION_H
#define SUITEVERSION_H

#define SNK_HS_TITLE	L"HotkeySuite"	

#define _HS_DEV_BUILD	1				//SET TO 0 ON RELEASE COMMIT

#define _HS_MAJ_VERSION	1				//UPDATE THIS DEFINE ON VERSION CHANGE
#define _HS_MIN_VERSION	2				//UPDATE THIS DEFINE ON VERSION CHANGE

#define __STRINGIFY(x)	#x
#define _STRINGIFY(x)	__STRINGIFY(x)

#if _HS_DEV_BUILD==1
#define HS_STR_VERSION 	_STRINGIFY(_HS_MAJ_VERSION._HS_MIN_VERSION) "-dev"
#else
#define HS_STR_VERSION 	_STRINGIFY(_HS_MAJ_VERSION._HS_MIN_VERSION)
#endif
#define HS_VSVI_VERSION	_HS_MAJ_VERSION,_HS_MIN_VERSION,0,0
#define HS_CRIGHT_YEARS	"2016-2017"		//UPDATE THIS DEFINE ON VERSION CHANGE

#endif //SUITEVERSION_H
