/*
VALIDCRC.C

	���ģ�����������֤���ӵĳ����CRC��
	������һ���������㷨��CRC��һ����ⲡ�����ֵķǳ���Ч�İ취��
	�κβ��������������Ƕ������Ȼ��ı�Դ�����CRC��
*/


#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "viruscrc.h"


/* ��һ����ȡ�õ��ֽں͸��ֽڵĺ�. */
#define lowb(x)  (*(unsigned char *)&(x))
#define hib(x)   (*((unsigned char *)&(x) + 1))

/* ��һ��˫��ȡ�õ��ֺ͸��ֵĺ�. */
#define loww(x)  (*(unsigned short *)&(x))
#define hiw(x)   (*((unsigned short *)&(x) + 1))


void *bufalloc(size_t *size, size_t minsize);


#if defined(__TURBOC__)

#include <dir.h>
#define findprog(progname, pn)  strcpy(pn, searchpath(progname))
#define argv0  _argv[0]

#elif defined(_MSC_VER) || defined(_QC)

#define findprog(progname, pn)  _searchenv(progname, getenv("PATH"), pn)
extern char **__argv;
#define argv0  __argv[0]

#else
#error Unknown compiler.
#endif


/***/
/* ���㵱ǰ����ĳ����CRC�����Ұ�������_viruscrc�����CRC�Ƚ�. */
int validatecrc(const char *progname)
{
int retcode;			/* ���������� */

if (_viruscrc.x.polynomial != 0)
  {
  unsigned char *buffer;	/* ����λͼ�Ļ��� */
  size_t bufsize;		/* ����Ĵ�С. */

  /* ������ܣ�����8k���ڴ棻���ٱ�֤����512�ֽ�. */
  bufsize = 8192;
  buffer = bufalloc(&bufsize, 512);

  if (buffer)
    {
    char pn[80];		/* �����ȫ�� */
    FILE *progfile;		/* �ļ�ָ�� */

    if (_osmajor < 3)
      /* ������prognameָ����·�������ļ�. */
      findprog(progname, pn);
    else
      strcpy(pn, argv0);

    if ((progfile = fopen(pn, "rb")) != NULL)
      {
      crc32_t table[256];	/* CRC�� */
      register crc32_t *halfi;	/* i/2��CRCָ�� */
      crc32_t crc;		/* ��ǰ��CRC */

      register size_t i;	/* �ֽڼ�����. */
      unsigned char *bufptr;	/* ����������ָ�� */

      /* Ϊ�˿��ټ��������һ��CRC���ұ� */
      for (i = 0, halfi = table, table[0] = 0; i < 256; i += 2, halfi++)
	if (hib(hiw(*halfi)) & 0x80)
	  table[i] = (table[i + 1] = *halfi << 1) ^ _viruscrc.x.polynomial;
	else
	  table[i + 1] = (table[i] = *halfi << 1) ^ _viruscrc.x.polynomial;

      crc = 0;
      while ((i = fread(buffer, 1, bufsize, progfile)) != 0)
	for (bufptr = buffer; i--; bufptr++)
	  crc = (crc << 8) ^ table[hib(hiw(crc)) ^ *bufptr];

      fclose(progfile);

      retcode = crc == _viruscrc.x.crc ? CRC_VALID : CRC_INVALID;
      }
    else
      retcode = CRC_FILEERR;

    free(buffer);
    }
  else
    retcode = CRC_NOMEM;
  }
else
  /* CRC����ʽ���벻Ϊ0 */
  retcode = CRC_ISZERO;

return (retcode);
}
