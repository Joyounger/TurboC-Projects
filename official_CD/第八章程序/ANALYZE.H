/****************************************************/
/* �ļ���analyze.h                                  */
/* TINY compiler���������﷨�����ӿ��ļ�		    */
/****************************************************/

#ifndef _ANALYZE_H_
#define _ANALYZE_H_

/*
 * ����buildSymtabͨ��ǰ������﷨�����������ű�
 */
void buildSymtab(TreeNode *);

/* 
 * ����typeCheckͨ�����������﷨���������ͼ��
 */
void typeCheck(TreeNode *);

#endif
