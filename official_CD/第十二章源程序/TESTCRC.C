/*
TESTCRC.C

�������֤������VALIDCRC.C�ж���ķ������㷨����Ч�ԡ�
*/


#include <stdio.h>
#include <stdlib.h>

#include "viruscrc.h"


/***/
main()
{
switch (validatecrc("TESTCRC.EXE"))
  {
  case CRC_VALID:
    puts("CRC is valid.");
    break;
    
  case CRC_INVALID:
  case CRC_ISZERO:
    fputs("*** WARNING *** Program's CRC is invalid.\n"
	  "This program may have been infected by a virus.\n", stderr);
    break;

  case CRC_NOMEM:
    fputs("Insufficient memory to run CRC calculation.\n", stderr);
    break;

  case CRC_FILEERR:
    fputs("Program file not found; cannot calculate CRC.\n", stderr);
    break;
  }

return (0);
}
