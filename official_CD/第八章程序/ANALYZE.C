/****************************************************/
/* �ļ��� analyze.c                                  */
/* �����������ʵ��					                 */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"

/* �����ڴ�λ�ü�����counter for variable memory locations */
static int location = 0;

/* ����traverse��һ��ͨ�õĵݹ�����﷨������
 * ��ͨ������preProc������ʵ��ǰ�������
 * ���ú���postProcʵ�ֺ������ 
 */
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

/* ����nullProc��һ��ʲô�������ĺ�����
 * Ŀ���ǲ�����ǰ����ߵ���������ı�������
 */
static void nullProc(TreeNode * t)
{ if (t==NULL) return;
  else return;
}

/* ����insertNode����t���﷨�����д洢�Ľڵ������ű�
 */
static void insertNode( TreeNode * t)
{ switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { case AssignK:
        case ReadK:
          if (st_lookup(t->attr.name) == -1)
          /* û���ڷ��ű����ҵ�����������ʶ�����յ��µĶ��崦�� */
            st_insert(t->attr.name,t->lineno,location++);
          else
          /* �Ѿ��ڷ��ű����ҵ������Ժ���λ����Ϣ������ʹ������������к� */ 
            st_insert(t->attr.name,t->lineno,0);
          break;
        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      { case IdK:
          if (st_lookup(t->attr.name) == -1)
           /* û���ڷ��ű����ҵ�����������ʶ�����յ��µĶ��崦�� */
            st_insert(t->attr.name,t->lineno,location++);
          else
          /* �Ѿ��ڷ��ű����ҵ������Ժ���λ����Ϣ������ʹ������������к� */ 
			st_insert(t->attr.name,t->lineno,0);
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

/* ����buildSymtabͨ��ǰ������﷨���������ű�
 */
void buildSymtab(TreeNode * syntaxTree)
{ traverse(syntaxTree,insertNode,nullProc);
  if (TraceAnalyze)
  { fprintf(listing,"\nSymbol table:\n\n");
    printSymTab(listing);
  }
}
/* ��ӡ������Ϣ */
static void typeError(TreeNode * t, char * message)
{ fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}

/* ����checkNode���﷨����һ���ڵ�ִ�����ͼ��
 */
static void checkNode(TreeNode * t)
{ switch (t->nodekind)
  { 
  /* ���ʽ */
	case ExpK:
      switch (t->kind.exp)
      { 
	  /* ��ֵ���㣬Ҫ��������������� */
		case OpK:
          if ((t->child[0]->type != Integer) ||
              (t->child[1]->type != Integer))
            typeError(t,"Op applied to non-integer");
		  /* ��ֵ�Ƚ����㣬����ǲ����� */
          if ((t->attr.op == EQ) || (t->attr.op == LT))
            t->type = Boolean;
		  /* ������ֵ���㣬��������� */
          else
            t->type = Integer;
          break;
		  /* �����ͱ���������Ϊ���� */
        case ConstK:
        case IdK:
          t->type = Integer;
          break;
        default:
          break;
      }
      break;
	/* ��� */
    case StmtK:
      switch (t->kind.stmt)
      { 
		/* IF��䣬�����ǲ����� */
	    case IfK:
          if (t->child[0]->type == Integer)
            typeError(t->child[0],"if test is not Boolean");
          break;
		/* ��ֵ�����䣬���������� */
        case AssignK:
          if (t->child[0]->type != Integer)
            typeError(t->child[0],"assignment of non-integer value");
          break;
		/* �����䣬���������� */
        case WriteK:
          if (t->child[0]->type != Integer)
            typeError(t->child[0],"write of non-integer value");
          break;
		/* ѭ����䣬���������ǲ����� */
        case RepeatK:
          if (t->child[1]->type == Integer)
            typeError(t->child[1],"repeat test is not Boolean");
          break;
        default:
          break;
      }
      break;
    default:
      break;

  }
}

/* ����typeCheckͨ����������﷨����ִ�����ͼ��
 */
void typeCheck(TreeNode * syntaxTree)
{ traverse(syntaxTree,nullProc,checkNode);
}
