/*��̽���һͨѶ����������������绰���롢סַ��Ȼ���ͨ�Ų����в��ҡ���ӡ��޸ļ�ɾ����*/
#include<stdio.h>
struct person
{
	char name[8];
    char tel[15];
    char addr[50];
};

char filename[20];
FILE *fp;

void creat();
void output();
void search();
void append();
void modify();
void delete();

main()
{
	int m;
  
	creat();   
	while(1)
	{
		printf("\n\n��ӣ��밴1");
 		printf("\n���ң��밴2");
		printf("\n�޸ģ��밴3");
		printf("\nɾ�����밴4");
		printf("\n������밴5");
		printf("\n�˳����밴0\n");
		scanf("%d",&m);
		if(m>=0&&m<=5)
		{
			switch(m)
    		{
			case 1: append();
    				break;
			case 2: search();
					break;
			case 3: modify();
					break;
			case 4: delete();
					break;
			case 5: output();
					break;
			case 0: exit();
			}
		 printf("\n\n������ϣ����ٴ�ѡ��");
		 }
		else
			printf("\n\nѡ��������ٴ�ѡ��");
	}
}


void creat()
{
	struct person one;
    long s1;

    printf("\n������ͨѶ������");
    scanf("%s",filename);
    if((fp=fopen(filename,"w"))==NULL)
    {
		printf("\n���ܽ���ͨѶ����");
		exit();
    }
    fprintf(fp,"%-10s%-20s%-50s\n","����","�绰����","סַ");
    printf("\n�������������绰���뼰סַ����0������\n");
    scanf("%s",one.name);
    while(strcmp(one.name,"0"))
    {
		scanf("%s%s",one.tel,one.addr);
		fprintf(fp,"%-10s%-20s%-50s\n",one.name,one.tel,one.addr);
		scanf("%s",one.name);
    }
    fclose(fp);
}

void output()
{
	struct person one;

    if((fp=fopen(filename,"r"))==NULL)
    {
		printf("\n���ܴ�ͨѶ����");
		exit();
    }
    printf("\n\n%20s\n","ͨ Ѷ ��");
    while(!feof(fp))
    {
		fscanf(fp,"%s%s%s\n",one.name,one.tel,one.addr);
		printf("%-10s%-20s%-50s",one.name,one.tel,one.addr);
    }
    fclose(fp);
}

void append()
{
	struct person one;

    if((fp=fopen(filename,"a"))==NULL)
    {
		printf("\n���ܴ�ͨѶ����");
		exit();
    }
    printf("\n��������ӵ��������绰���뼰סַ\n");
    scanf("%s%s%s",one.name,one.tel,one.addr);
    fprintf(fp,"%-10s%-20s%-50s\n",one.name,one.tel,one.addr);
    fclose(fp);
}

void search()
{  
	int k=0;
    char namekey[8];
    struct person one;

    printf("\n����������:");
    scanf("%s",namekey);

    if((fp=fopen(filename,"rb"))==NULL)
    {  
		printf("\n���ܴ�ͨѶ����");
		exit();
    }
    while(!feof(fp))
    {  
		fscanf(fp,"%s%s%s\n",one.name,one.tel,one.addr);
		if(!strcmp(namekey,one.name))
		{
			printf("\n\n�Ѳ鵽����¼Ϊ��");
			printf("\n%-10s%-18s%-50s",one.name,one.tel,one.addr);
			k=1;
        }
    }
    if(!k)  
		printf("\n\n�Բ���ͨѶ����û�д��˵ļ�¼��");
    fclose(fp);
}

void modify()
{  
	int m,k=0;
    long offset;
    char namekey[8];
    struct person one;

    printf("\n����������:");
    scanf("%s",namekey);

    if((fp=fopen(filename,"r+"))==NULL)
    {  
		printf("\n���ܴ�ͨѶ����");
		exit();
    }
    while(!feof(fp))
    {  
		offset=ftell(fp);
		fscanf(fp,"%s%s%s\n",one.name,one.tel,one.addr);
		if(!strcmp(namekey,one.name))
		{
			k=1;
			break;  
		}
    }
    if(k)
	{  
		printf("\n�Ѳ鵽����¼Ϊ��");
        printf("\n%-10s%-18s%-50s",one.name,one.tel,one.addr);
        printf("\n���������������绰���뼰סַ��");
		scanf("%s%s%s",one.name,one.tel,one.addr);
		fseek(fp,offset,SEEK_SET);
		printf("%ld",ftell(fp));
		fprintf(fp,"%-10s%-20s%-50s\n",one.name,one.tel,one.addr);
	}
    else
		printf("\n�Բ���ͨѶ����û�д��˵ļ�¼��");
    fclose(fp);
}

void delete()
{  
	int m,k=0,flag;
    long offset1,offset2;
    char namekey[8], valid[4];
    struct person one;

    printf("\n����������:");
    scanf("%s",namekey);
    if((fp=fopen(filename,"r+"))==NULL)
    {  
		printf("\n���ܴ�ͨѶ����");
		exit();
    }
    while(!feof(fp))
    {  
		offset1=ftell(fp);
		fscanf(fp,"%s%s%s\n",one.name,one.tel,one.addr);
		if(!strcmp(namekey,one.name))
		{   
			k=1;
			break; 
		}
    }
    if(k)
	{ 
		printf("\n�Ѳ鵽����¼Ϊ");
        printf("\n%-10s%-18s%-50s",one.name,one.tel,one.addr);
        printf("\nȷʵҪɾ��,��1����ɾ������0��");
        scanf("%d",&m);
        if(m)
		{   
			fseek(fp,offset1,SEEK_SET);
			fprintf(fp,"%-10s%-20s%-50s\n","","","");
		}
	}
    else
		printf("\n�Բ���ͨѶ����û�д��˵ļ�¼��");
    fclose(fp);
}

