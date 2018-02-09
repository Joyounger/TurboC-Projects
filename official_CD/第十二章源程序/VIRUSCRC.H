/*
VIRUSCRC.H
����Ƿ�����CRCУ��VALIDCRC.C��VIRSUDAT.C��ͷ�ļ�
*/


typedef
  unsigned long crc32_t;

union filecrc
  {
  char searchstr[8];	/* Ҫ�������ַ���. */

  struct
    {
    crc32_t polynomial;	/* ����ļ��Ķ���ʽ. */
    crc32_t crc;	/* Ϊ����ļ��������CRC. */
    } x;
  };


extern const union filecrc _viruscrc;


/* CRCУ��ķ���ֵ. */
enum
  {
  CRC_VALID,	/* CRC��Ч. */
  CRC_INVALID,	/* CRC��Ч */
  CRC_ISZERO,	/* CRC����ʽ������Ϊ0 */
  CRC_NOMEM,	/* �ڴ治�� */
  CRC_FILEERR	/* δ�ҵ������ļ� */
  };


int validatecrc(const char *progname);

#define isvalidcrc(progname)  (validatecrc(progname) == CRC_VALID)
