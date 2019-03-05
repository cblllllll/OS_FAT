#include <windows.h>
#include<iostream>
#include<time.h>
#include<conio.h>
#include<string.h>
#include<io.h>
#include<fstream>

#include "defines.h"
using namespace std;

SuperBlock SB;
CurrentStatus CS;
static FILE *fpd = NULL;
static FILE *fpd1 = NULL;
FileType FT;
FCB F;//һ���ļ���
static char FC[BLOCK_SIZE];	//�ļ�����,��ת

void create_dir(char Name[])//����Ŀ¼
{

	if (!CheckFileName(Name))
	{
		cout << ("����,�ļ����Ѵ���!") << endl;
		return;
	}
	strcpy(F.FileName, Name);
	FT = ADIR;
	F.Type = FT;
	CreateFileElement(CS.CurrentUser, F.FileName, F.Type, FC, CS.ParentIndexId);
	show_dir();
}


void create(char Name[])
{

	FT = AFILE;
	if (!CheckFileName(Name))
	{
		cout << ("����,�ļ����Ѵ���!") << endl;
		return;
	}
	strcpy(F.FileName, Name);
	FT = AFILE;
	F.Type = FT;
	cout << "������Ҫ�������ļ�,�ļ����ݣ�";
	scanf("%s", FC);
	CreateFileElement(CS.CurrentUser, F.FileName, F.Type, FC, CS.ParentIndexId);
	cout << "-----------------------------------------�ļ��������" << endl;
	show_dir();
}

void show_path()//չʾ��ǰ·��
{
	cout << "��ǰ·���ǣ�" << CS.CurrentPath << endl;
}


void delet(char Name[])
{

	for (int i = 0; i < FILE_INDEX_NUM; i++)		//��������
	{
		if (SB.FI[i].ParentIndexId == CS.ParentIndexId &&
			SB.FI[i].FileLevel == CS.CurrentLevel &&
			strcmp(SB.FI[i].FileName, Name) == 0 &&//���ҵ���
			SB.FI[i].IndexEnable != 0)
		{
			if (strcmp(SB.FI[i].Creator.UserName, CS.CurrentUser.UserName) != 0)
			{
				cout << ("�Բ�����û��Ȩ��ɾ������ļ�(��)��\t") << endl;
				return;
			}
			if (SB.FI[i].FileLevel != 2)
			{
				//ɾ����Ӧ�ļ�����������
				int fatx[1024];
				int j = 1;

				int x = SB.FI[i].FileBlockId;
				int fat_copy[1024];
				for (int m = 0; m < 1024; m++)
				{
					fat_copy[m] = SB.FAT[m];
				}
				fatx[0] = SB.FI[i].FileBlockId;
				fatx[1] = SB.FAT[x];
				while (fat_copy[x] != -1)//�����Ǹ��ڵ�ʱ
				{

					fat_copy[x] = fat_copy[fat_copy[x]];
					j++;
					fatx[j] = fat_copy[x];
				}

				//����fatx������������е��ļ��̿���FATidΪ��ͷ����-1Ϊ��β
				int n = 0;//fatx[0]���ŵ���FAT����
				while (fatx[n] != -1)
				{

					fseek(fpd, fatx[n] * BLOCK_SIZE, SEEK_SET);//ת��ָ�룬д�ļ�
					char c[BLOCK_SIZE];
					memset(c, '\0', BLOCK_SIZE);
					fwrite(c, BLOCK_SIZE, 1, fpd);	//д�ļ�����
					n++;
				}//ȫ�����Ϊ'\0'
				for (int m = 0; m < 1024; m++)
				{
					fat_copy[m] = SB.FAT[m];
				}
				//ɾ����Ӧ��FAT������,����
				for (int x = SB.FI[i].FileBlockId;; fat_copy[x] = fat_copy[fat_copy[x]])//�����Ǹ��ڵ�ʱ
				{
					if (fat_copy[x] != -1)
					{
						SB.FAT[x] = 0;
						SB.FAT[fat_copy[x]] = 0;
					}
					else
					{
						SB.FAT[x] = 0;
						break;
					}
				}
				//ɾ����Ӧ�ļ�Ŀ¼������FI[I]��ֱ�����ڴ����޸�
				SB.FI[i].IndexEnable = false;
				memset(SB.FI[i].CreateTime, '\0', sizeof(SB.FI[i].CreateTime));
				memset(SB.FI[i].LastModTime, '\0', sizeof(SB.FI[i].LastModTime));
				cout << "-----------------------------------ɾ���ļ���" << Name << "�ɹ�" << endl;
			}
			else
			{
				cout << ("�Բ����㲻��ɾ����ļ��У�\t") << endl;
				return;
			}
		}
	}
	show_dir();
}

void modify(char Name[])
{
	//���޸ĺ���ļ����ж�ȡ
	FILE *f = fopen("C:\\filesys\\read.txt", "r");
	char *a = new char[1000];
	int s = 0;
	int ch;
	ch = getc(f);
	while (ch != EOF)
	{
		a[s] = ch;
		ch = getc(f);
		s++;
	}
	int ID;//��ȡ���ļ���FCB���
	for (int i = 0; i < FILE_INDEX_NUM; i++)		//��������
	{
		if (SB.FI[i].ParentIndexId == CS.ParentIndexId &&
			SB.FI[i].FileLevel == CS.CurrentLevel && SB.FI[i].IndexEnable == 1 &&
			strcmp(SB.FI[i].FileName, Name) == 0)
		{
			ID = i;
		}
	}
	if (SB.FI[ID].FileLevel != 2)
	{//1)ɾ����Ӧ�ļ�����������
		int fatx[1024];
		int j = 1;

		int FATId = SB.FI[ID].FileBlockId;
		int fat_copy[1024];
		for (int m = 0; m < 1024; m++)
		{
			fat_copy[m] = SB.FAT[m];
		}
		fatx[0] = SB.FI[ID].FileBlockId;
		fatx[1] = SB.FAT[FATId];
		while (fat_copy[FATId] != -1)//�����Ǹ��ڵ�ʱ
		{

			fat_copy[FATId] = fat_copy[fat_copy[FATId]];
			j++;
			fatx[j] = fat_copy[FATId];
		}
		//����fatx������������е��ļ��̿���FATidΪ��ͷ����-1Ϊ��β
		int n = 0;//fatx[0]���ŵ���FAT����
		while (fatx[n] != -1)
		{

			fseek(fpd, fatx[n] * BLOCK_SIZE, SEEK_SET);//ת��ָ�룬д�ļ�
			char c[BLOCK_SIZE];
			memset(c, '\0', BLOCK_SIZE);
			fwrite(c, BLOCK_SIZE, 1, fpd);	//д�ļ�����
			n++;
		}//ȫ�����Ϊ'\0'
		for (int m = 0; m < 1024; m++)
		{
			fat_copy[m] = SB.FAT[m];
		}
		//ɾ����Ӧ��FAT������,����
		for (int x = SB.FI[ID].FileBlockId;; fat_copy[x] = fat_copy[fat_copy[x]])//�����Ǹ��ڵ�ʱ
		{
			if (fat_copy[x] != -1)
			{
				SB.FAT[x] = 0;
				SB.FAT[fat_copy[x]] = 0;
			}
			else
			{
				SB.FAT[x] = 0;
				break;
			}
		}

		//�޸��ļ��޸�ʱ�䣬FILELEN
		int x = strlen(a) / FILEDATA_SIZE;
		int y = strlen(a) % FILEDATA_SIZE;
		int l = 0;
		for (int i = 0; i < DATA_NUM; i++)
		{
			if (SB.FAT[i] == 0)
			{
				l++;
			}
		}
		int k;
		k = get_num(x, y);//����Ҫ���̿���
		if (l < k)
		{
			cout << ("����,�Ҳ����㹻�հ׵��ļ���,����ϵͳ����!") << endl;
			return;
		}
		FCB FE;//�ļ��б������FAT��ַ

		GetTimeStr(FE.LastModTime);
		FE.FileLen = strlen(a);

		int FATS[1024];
		//��x��0
		if (x == 0)
		{
			fseek(fpd, FATId * BLOCK_SIZE, SEEK_SET);//ת��ָ�룬д�ļ�
			fwrite(a, strlen(a), 1, fpd);	//д�ļ�����

			UpdateFAT(FATId, x, FATS);//�ɿ�ʼ��FAT����
		}
		else if (y == 0)//����x���̿�
		{
			UpdateFAT(FATId, x, FATS);

			for (int i = 0; i < x; i++)
			{

				fseek(fpd, FATS[i] * BLOCK_SIZE, SEEK_SET);//ת��ָ�룬д�ļ�
				char c[BLOCK_SIZE];
				memset(c, '\0', BLOCK_SIZE);
				int m = i * BLOCK_SIZE;
				int n = i * BLOCK_SIZE + BLOCK_SIZE;
				for (int i = m, j = 0; i < n; i++, j++)
				{
					if (a[i] == '\0')
					{
						break;
					}
					c[j] = a[i];
				}
				fwrite(c, strlen(c), 1, fpd);	//д�ļ�����
			}
		}
		else
		{//x!=0,y!=0
			//����x+1���̿�
			UpdateFAT(FATId, x + 1, FATS);
			for (int i = 0; i < x + 1; i++)
			{
				fseek(fpd, FATS[i] * BLOCK_SIZE, SEEK_SET);//ת��ָ�룬д�ļ�
				char c[BLOCK_SIZE];
				memset(c, '\0', BLOCK_SIZE);
				int m = i * BLOCK_SIZE;
				int n = i * BLOCK_SIZE + BLOCK_SIZE;
				for (int i = m, j = 0; i < n; i++, j++)
				{
					if (a[i] == '\0')
					{
						break;
					}
					c[j] = a[i];
				}
				fwrite(c, strlen(c), 1, fpd);	//д�ļ�����
			}
		}
		fclose(f);
		UpdateFCB(ID, FE.LastModTime, FE.FileLen);
	}
}


void read(char Name[])
{
	if (CheckFileName(Name))
	{
		cout << ("����,�ļ���������!") << endl;
		return;
	}
	FT = AFILE;
	FILE *fpr = fopen("C:\\filesys\\read.txt", "w");
	int i;
	for (i = 0; i < FILE_INDEX_NUM; i++)		//��������
	{
		if (SB.FI[i].ParentIndexId == CS.ParentIndexId &&
			SB.FI[i].FileLevel == CS.CurrentLevel && SB.FI[i].IndexEnable == 1 &&
			strcmp(SB.FI[i].FileName, Name) == 0)//�ҵ���Ӧ���ļ�������
			if (SB.FI[i].share==1)
			{
				Go_share(SB.FI[i].share_path);
				return;
			}
			else
			{
				//����FAT����ʼ��
				//ɾ����Ӧ�ļ�����������
				int fatx[1024];
				int j = 1;
				int x = SB.FI[i].FileBlockId;
				int fat_copy[1024];
				for (int m = 0; m < 1024; m++)
				{
					fat_copy[m] = SB.FAT[m];
				}
				fatx[0] = SB.FI[i].FileBlockId;//�����������
				fatx[1] = SB.FAT[x];

				while (fat_copy[x] != -1)//�����Ǹ��ڵ�ʱ
				{
					fat_copy[x] = fat_copy[fat_copy[x]];
					j++;
					fatx[j] = fat_copy[x];
				}//���fatx��Ϊ��������
				int y = SB.FI[i].FileLen % DATA_SIZE;
				int n = 0;
				while (fatx[n] != -1)
				{
					if (fatx[n + 1] == -1)
					{
						fseek(fpd, fatx[n] * BLOCK_SIZE, SEEK_SET);//ת��ָ�룬д�ļ�
						fread(FC, y, 1, fpd);//������ļ�����FC��
						fwrite(FC, strlen(FC), 1, fpr);
						n++;
					}
					else
					{
						fseek(fpd, fatx[n] * BLOCK_SIZE, SEEK_SET);//ת��ָ�룬д�ļ�
						fread(FC, BLOCK_SIZE, 1, fpd);//������ļ�����FC��
						fwrite(FC, strlen(FC), 1, fpr);
						n++;
					}

				}//ȫ�����Ϊ'\0'
				//�����޸ĺ���ļ�
				break;
			}
		
	}

	fclose(fpr);
	cout << "-----------------------------------------�ļ���ȡ���" << endl;
}

void rename(char Name[], char Rename[])
{//ǰ��ԭ�ļ�����������Ŀ���ļ���

	if (CheckFileName(Name))
	{
		cout << ("����,�ļ�������!") << endl;
		return;
	}
	if (!CheckFileName(Rename))
	{
		cout << ("����,�ļ����Ѵ���!") << endl;
		return;
	}
	for (int i = 0; i < FILE_INDEX_NUM; i++)		//��������
	{
		if (SB.FI[i].ParentIndexId == CS.ParentIndexId &&
			SB.FI[i].FileLevel == CS.CurrentLevel &&
			strcmp(SB.FI[i].FileName, Name) == 0 &&
			SB.FI[i].IndexEnable == 1)
		{
			strcpy(SB.FI[i].FileName, Rename);
		}
	}
}

int share_i;

void shared(char Path[], char Name[])
{
	//������
	//��ȡĳ���ļ���·����FCB�����һ�ΪĿ����ĵ�ַ
	char *a = new char[256];
	for (int i = 0; i < strlen(Path); i++)
	{
		a[i] = CS.CurrentPath[i];
	}

	Go(Path);
	Go(a);
	int i;
	for (i = 0; i < FILE_INDEX_NUM; i++)//�����е�FCB���б������ң������õ�FCB��
	{
		if (SB.FI[i].IndexEnable == 0)
		{
			break;
		}
		else continue;
	}

	//��Ҫ�����û�пյ��ļ���
	strcpy(SB.FI[i].FileName,Name);
	SB.FI[i].FileLevel = CS.CurrentLevel;
	SB.FI[i].IndexEnable = 1;
	SB.FI[i].ParentIndexId = CS.ParentIndexId;
	SB.FI[i].Type = SB.FI[share_i].Type;
	SB.FI[i].Creator = CS.CurrentUser;
	GetTimeStr(SB.FI[share_i].CreateTime);
	GetTimeStr(SB.FI[share_i].LastModTime);
	SB.FI[i].FileLen = SB.FI[share_i].FileLen;
	SB.FI[i].number = i;
	SB.FI[i].share = 1;
	strcpy(SB.FI[i].share_path, Path);
}




void Cd_1(char Name[])
{
	int SplitNum = 0;	//�ָ�����ֵĴ���
	{
		{
			int i;
			for (i = 0; i < FILE_INDEX_NUM; i++)
			{
				if (SB.FI[i].ParentIndexId == CS.ParentIndexId &&
					SB.FI[i].FileLevel == CS.CurrentLevel && SB.FI[i].IndexEnable != 0 &&
					strcmp(SB.FI[i].FileName, Name) == 0)
				{
					share_i = i;
					if (SB.FI[i].Type == ADIR)
					{
						CS.ParentIndexId = i;//�ڼ����ļ�������
						CS.CurrentLevel++;	//��ǰ�������
						//�����ǰ·������һλ�ľͼ��Ϸָ���
						if (strlen(CS.CurrentPath) != 1)
							strcat(CS.CurrentPath, "\\");
						//����֮���ټ���·�������ǵ�ǰ·����
						strcat(CS.CurrentPath, Name);
					}
					else
					{
						read(Name);
					}
					break;
				}
			}
			if (i >= FILE_INDEX_NUM)
			{
				cout << "λ�ò�����, ����·���Ƿ���ȷ" << endl;
				return;
			}
		}
	}
}
void show_fat()
{
	FILE *fp = fopen("C:\\filesys\\fat.txt", "w");
	for (int i = 0; i < FAT_NUM; i++)
	{
		fprintf(fp, "%d ", SB.FAT[i]);
	}
	fclose(fp);

	cout << "-------------------------------FAT��������" << endl;
	cout << "---------------------------------��ַΪ��C:\\filesys\\fat.txt" << endl;
}


void logout()
{
	Cd_back();

	cout << "��ϵͳ���������û�root��cbl" << endl;
	cout << "��ѡ���û����е�¼" << endl;
	userchoose();
}


void userchoose()
{
	while (1)
	{
		cout << "������ѡ����û�����";
		char c[100];
		scanf("%s", c);

		if (!strcmp(c, "root"))
		{
			while (1)
			{
				cout << "root�û����ã��������������룺";
				char ch[10];
				int i = 0;
				while (1)
				{
					ch[i] = _getch();
					if (ch[i] == '\r')
						break;
					printf("*");
					i++;
				}
				ch[i] = '\0';
				if (!strcmp(ch, "root"))
				{
					CS.CurrentUser = SB.SU[0];
					CS.CurrentLevel = 3;//��Ŀ¼��ǰ�ȼ���2,�ʼ��1
					strcat(CS.CurrentPath, "root");//���Ϸָ���
					CS.ParentIndexId = 1;
					cout << endl;
					cout << ("-------------------------------------------------------------\n");
					cout << "root�û�����ӭ����FAT�����ļ�����ϵͳ��" << endl;
					cout << "��ǰ·���ǣ�" << CS.CurrentPath << endl;
					show_dir();
					break;
				}
				else
				{
					cout << "�����������������" << endl;
				}
			}
			break;
		}
		else if (!strcmp(c, "cbl"))
		{
			while (1)
			{
				cout << "cbl�û����ã��������������룺";
				char c[10];
				int i = 0;
				while (1)
				{
					c[i] = _getch();
					if (c[i] == '\r')
						break;
					printf("*");
					i++;
				}
				c[i] = '\0';
				if (!strcmp(c, "cbl"))
				{
					CS.CurrentUser = SB.SU[1];
					CS.CurrentLevel = 3;//��Ŀ¼��ǰ�ȼ���2,�ʼ��1
					strcat(CS.CurrentPath, "cbl");//���Ϸָ���
					CS.ParentIndexId = 2;
					cout << endl;
					cout << ("-------------------------------------------------------------\n");
					cout << "cbl�û�����ӭ����FAT�����ļ�����ϵͳ��" << endl;
					cout << "��ǰ·���ǣ�" << CS.CurrentPath << endl;
					show_dir();
					break;
				}
				else
				{
					cout << "�����������������" << endl;
				}
			}
			break;
		}
		else
			cout << "�����������������" << endl;
	}
}


void show_att()//չʾ����
{
	char Name[256];
	cout << "������Ҫ��ѯ���Ե��ļ����У���:";
	scanf("%s", Name);
	for (int i = 0; i < FILE_INDEX_NUM; i++)		//��������
	{
		if (SB.FI[i].ParentIndexId == CS.ParentIndexId &&
			SB.FI[i].FileLevel == CS.CurrentLevel &&
			strcmp(SB.FI[i].FileName, Name) == 0)
		{
			//��ȡ�ļ�

			cout << "���ƣ�" << SB.FI[i].FileName << endl;
			if (!F.Type)
			{
				cout << "���ͣ��ļ�" << endl;
				cout << "���ȣ�" << SB.FI[i].FileLen << endl;	//�ļ�Ԫ�صĳ���
			}
			else
			{
				cout << "���ͣ��ļ���" << endl;
			}
			cout << "�����ߣ�" << SB.FI[i].Creator.UserName << endl;
			cout << "����ʱ�䣺"<<endl;
			
			cout << SB.FI[i].CreateTime[0] << "��" << SB.FI[i].CreateTime[1] << "��" << SB.FI[i].CreateTime[2] <<
				"��" << SB.FI[i].CreateTime[3] << "ʱ" << SB.FI[i].CreateTime[4] << "��"<<SB.FI[i].CreateTime[5] << "��" << endl;
	
			cout << "����޸�ʱ�䣺"<< endl;
			cout << SB.FI[i].LastModTime[0] << "��" << SB.FI[i].LastModTime[1] << "��" << SB.FI[i].LastModTime[2] <<
				"��" << SB.FI[i].LastModTime[3] << "ʱ" << SB.FI[i].LastModTime[4] << "��" << SB.FI[i].LastModTime[5] << "��" << endl;
		}
	}
}

//��ʼ��2���û���һ���ǹ���Ա
void InitAllUsers()
{
	//��ʼ��Ԥ���û�
	strcpy(SB.SU[0].UserName, "root");
	SB.SU[0].Type = ADMIN;//����Ա
	strcpy(SB.SU[0].UserPw, "root");
	SB.SU[0].UserRoot = 1;
	strcpy(SB.SU[1].UserName, "cbl");
	SB.SU[1].Type = COMM;
	strcpy(SB.SU[1].UserPw, "cbl");
	SB.SU[1].UserRoot = 2;//�û����ڵ�
}

void Cd_back()//�ص��û�ע�ᴦ
{
	CS.CurrentLevel = 2;//��Ŀ¼��ǰ�ȼ���2,�ʼ��1
	strcpy(CS.CurrentPath, "\\");//���Ϸָ���
	CS.ParentIndexId = 0;
}

void back()//����һ��
{
	int SplitNum = 0;
	int i;

	if (CS.CurrentLevel > 2)//�����ǰ�ȼ�����2
	{
		CS.CurrentLevel--;
		CS.ParentIndexId = SB.FI[CS.ParentIndexId].ParentIndexId;//ǰ��һ��
		for (i = strlen(CS.CurrentPath) - 1; i > 0; i--)//�������ҷָ��������ǰ��һ�������԰������һ��·���ĵ�
		{
			if (CS.CurrentPath[i] == '\\')
			{
				SplitNum++;//�ָ����һ
				if (SplitNum == 1)	//�ѹ��˵�һ��Ŀ¼��
					break;
			}
		}
		char TempPath[256];		//����·�����ݴ�����
		strcpy(TempPath, CS.CurrentPath);//�Ȱ����ڵ�·�����Ƶ�TempPath�У���
		memset(CS.CurrentPath, '\0', 256);//��CurrentPathȫ��0
		if (i == 0)	i++;//���i=0������ͷ�ˣ�i=1
		strncpy(CS.CurrentPath, TempPath, i);//��TempPath��ǰi������ǰ·��
		show_path();
		if (CS.CurrentLevel == 2)
		{
			cout << "��ϵͳ���������û�root��cbl" << endl;
			cout << "��ѡ���û����е�¼" << endl;
			userchoose();
		}
	}
}
//����Ŀ¼����
//�ص���Ŀ¼
void Cd(char Name[])
{
	int SplitNum = 0;	//�ָ�����ֵĴ���
	{
		{
			int i;
			for (i = 0; i < FILE_INDEX_NUM; i++)
			{
				if (SB.FI[i].ParentIndexId == CS.ParentIndexId &&
					SB.FI[i].FileLevel == CS.CurrentLevel && SB.FI[i].IndexEnable != 0 &&
					strcmp(SB.FI[i].FileName, Name) == 0)
				{
					if (SB.FI[i].share == 1)
					{
						Go_share(SB.FI[i].share_path);
						return;
					}
					
					if ((strcmp(CS.CurrentUser.UserName, SB.FI[i].Creator.UserName) != 0) &&
						(CS.CurrentUser.Type != ADMIN ))
					{
						cout << ("�Բ���,����Ȩ���ʸ��ļ����ļ���!") << endl;
						return;
					}
					if (SB.FI[i].Type == ADIR)
					{
						CS.ParentIndexId = i;//�ڼ����ļ�������
						CS.CurrentLevel++;	//��ǰ�������
						//�����ǰ·������һλ�ľͼ��Ϸָ���
						if (strlen(CS.CurrentPath) != 1)
							strcat(CS.CurrentPath, "\\");
						//����֮���ټ���·�������ǵ�ǰ·����
						strcat(CS.CurrentPath, Name);
					}
					else
					{
						read(Name);
					}
					break;
				}
			}
			if (i == FILE_INDEX_NUM)
			{
				cout << "λ�ò�����, ����·���Ƿ���ȷ" << endl;
			}
		}
	}
}
bool CheckFileName(char Name[])
{
	//����Ƿ��������
	for (int i = 0; i < FILE_INDEX_NUM; i++)
	{
		if (SB.FI[i].ParentIndexId == CS.ParentIndexId &&
			SB.FI[i].FileLevel == CS.CurrentLevel &&
			SB.FI[i].IndexEnable != 0 &&
			strcmp(SB.FI[i].FileName, Name) == 0)
			return 0;
	}
	return 1;
}

//�����ļ���������������id��Ԫ�������ļ��л����ļ���Ȩ��
//����֪id�ļ����������и���
void UpdateFCB(int FIId, int time[], long y)
{
	for (int i = 0; i < 6; i++)
	{
		SB.FI[FIId].LastModTime[i]=time[i];
	}

	SB.FI[FIId].FileLen = y;
}


void exit()
{
	SaveVirtualDisk();
	fclose(fpd);
	fclose(fpd1);
}


int get_num(int x, int y)
{
	if (x == 0)
	{
		return 1;
	}
	else if (y == 0)
	{
		return x;
	}
	else
	{
		return x + 1;
	}

}

//����һ�ļ�Ԫ��,�����ļ�����FAT����ʼ�ļ����
void CreateFileElement(User Creator, char FileName[], FileType Type, char Content[], int ParentId)
{

	if (!CheckFileName(FileName))
	{
		cout << ("����,�ļ����Ѵ���!") << endl;
		return;
	}

	if (Type == ADIR)
	{
		FCB FE;
		FE.FileLevel = CS.CurrentLevel;		//�����ڵ�ǰ��
		FE.Type = Type;

		GetTimeStr(FE.CreateTime);
		GetTimeStr(FE.LastModTime);
		strcpy(FE.FileName, FileName);
		FE.Creator = Creator;

		FE.FileLen = sizeof(FCB);
		AddFCB(0, FE.FileLevel, FE.FileName, FE.Type, Creator, ParentId, FE.CreateTime, FE.LastModTime, FE.FileLen);
	}
	else
	{
		int FATId;
		FATId = GetBlankFileBlockId();//��ʼ����ʱ��ÿ���ļ�����һ��FATId
		if (FATId >= DATA_NUM)
		{
			cout << ("����,�Ҳ����հ׵��ļ���,����ϵͳ����!") << endl;
			return;
		}
		int x = strlen(Content) / FILEDATA_SIZE;
		int y = strlen(Content) % FILEDATA_SIZE;
		int l = 0;
		for (int i = 0; i < DATA_NUM; i++)
		{
			if (SB.FAT[i] == 0)
			{
				l++;
			}
		}
		int k;
		k = get_num(x, y);//����Ҫ���̿���
		if (l < k)
		{
			cout << ("����,�Ҳ����㹻�հ׵��ļ���,����ϵͳ����!") << endl;
			return;
		}
		FCB FE;
		FE.FileBlockId = FATId;//�ļ��б������FAT��ַ
		FE.FileLevel = CS.CurrentLevel;		//�����ڵ�ǰ��
		FE.Type = Type;

		GetTimeStr(FE.CreateTime);
		GetTimeStr(FE.LastModTime);

		strcpy(FE.FileName, FileName);
		FE.Creator = Creator;
		FE.FileLen = strlen(Content);

		int FATS[1024];
		//��x��0
		if (x == 0)
		{
			fseek(fpd, FATId * BLOCK_SIZE, SEEK_SET);//ת��ָ�룬д�ļ�
			fwrite(Content, strlen(Content), 1, fpd);	//д�ļ�����
			//fpd����ָ��
			//��������
			AddFCB(FATId, FE.FileLevel, FE.FileName, FE.Type, Creator, ParentId, FE.CreateTime, FE.LastModTime, FE.FileLen);

			//����FAT
			UpdateFAT(FATId, x, FATS);//�ɿ�ʼ��FAT����
		}
		else if (y == 0)//����x���̿�
		{
			UpdateFAT(FATId, x, FATS);

			for (int i = 0; i < x; i++)
			{

				fseek(fpd, FATS[i] * BLOCK_SIZE, SEEK_SET);//ת��ָ�룬д�ļ�
				char c[BLOCK_SIZE];
				memset(c, '\0', BLOCK_SIZE);
				int m = i * BLOCK_SIZE;
				int n = i * BLOCK_SIZE + BLOCK_SIZE;
				for (int i = m, j = 0; i < n; i++, j++)
				{
					if (Content[i] == '\0')
					{
						break;
					}
					c[j] = Content[i];
				}
				fwrite(c, strlen(c), 1, fpd);	//д�ļ�����
			}
			AddFCB(FATId, FE.FileLevel, FE.FileName, FE.Type, Creator, ParentId, FE.CreateTime, FE.LastModTime, FE.FileLen);

		}
		else {//x!=0,y!=0
			//����x+1���̿�
			UpdateFAT(FATId, x + 1, FATS);
			for (int i = 0; i < x + 1; i++)
			{
				fseek(fpd, FATS[i] * BLOCK_SIZE, SEEK_SET);//ת��ָ�룬д�ļ�
				char c[BLOCK_SIZE];
				memset(c, '\0', BLOCK_SIZE);
				int m = i * BLOCK_SIZE;
				int n = i * BLOCK_SIZE + BLOCK_SIZE;
				for (int i = m, j = 0; i < n; i++, j++)
				{
					if (Content[i] == '\0')
					{
						break;
					}
					c[j] = Content[i];
				}
				fwrite(c, strlen(c), 1, fpd);	//д�ļ�����
			}
			AddFCB(FATId, FE.FileLevel, FE.FileName, FE.Type, Creator, ParentId, FE.CreateTime, FE.LastModTime, FE.FileLen);
		}
	}
}

//���һ�ļ�����
void AddFCB(int FBId, int Level, char FileName[], FileType Type, User Creator, int ParentId, int  time1[], int time2[], long y)
{
	//��ѡ��հ�FCB����в���
	int i;
	for (i = 0; i < FILE_INDEX_NUM; i++)//�����е�FCB���б������ң������õ�FCB��
	{
		if (SB.FI[i].IndexEnable == 0)
		{
			break;
		}
		else continue;
	}

	//��Ҫ�����û�пյ��ļ���
	strcpy(SB.FI[i].FileName, FileName);
	SB.FI[i].FileLevel = Level;
	SB.FI[i].IndexEnable = 1;
	SB.FI[i].FileBlockId = FBId;
	SB.FI[i].ParentIndexId = ParentId;
	SB.FI[i].Type = Type;
	SB.FI[i].Creator = Creator;
	for (int j = 0; j < 6; j++)
	{
		SB.FI[i].CreateTime[j] = time1[j];
	}
	for (int j = 0; j < 6; j++)
	{
		SB.FI[i].LastModTime[j] = time2[j];
	}

	SB.FI[i].FileLen = y;
	SB.FI[i].number = i;
	SB.FI[i].share = 0;
}

void SaveVirtualDisk()
{
	fseek(fpd1, 0, SEEK_SET);
	fwrite(&SB, sizeof(SuperBlock), 1, fpd1);		//д���ļ����ƿ�

}


//��FAT����ѡ��һ���հױ���
int GetBlankFileBlockId()
{
	for (int i = 0; i < FAT_NUM; i++)
	{
		if (SB.FAT[i] == 0)
		{

			return i;
		}
		else continue;
	}
	return FAT_NUM;
}


void UpdateFAT(int FATId, int x, int FATS[])//��ʼ�̿�Ÿ���
{//�Ȳ������пյ��̿�ţ�Ȼ����ǰx������
	if (x == 0 || x == 1)//��ʾֻ��һ��FAT����
	{
		SB.FAT[FATId] = -1;
		FATS[0] = FATId;
		return;
	}

	int FATX[1024];
	for (int i = 0; i < 1024; i++)
	{
		FATX[i] = -2;
	}
	for (int i = 0; i < FAT_NUM; i++)
	{
		if (i == FATId)
		{
			continue;
		}
		if (SB.FAT[i] == 0)
		{
			int j = 0;
			for (j = 0; j < 1024; j++)
			{
				if (FATX[j] == -2)
					break;
			}
			FATX[j] = i;

		}
	}//�õ����п��е�FAT������

	if (x == 2)//���������̿�ʱ
	{
		SB.FAT[FATId] = FATX[0];//��ֵ
		SB.FAT[FATX[0]] = -1;//��ֵ
		FATS[0] = FATId;
		FATS[1] = FATX[0];
		return;
	}
	//x>=3ʱ
	SB.FAT[FATId] = FATX[0];//��ֵ
	for (int i = 0; i < x - 2; i++)
	{
		SB.FAT[FATX[i]] = FATX[i + 1];
	}
	SB.FAT[FATX[x - 2]] = -1;

	FATS[0] = FATId;//��FATS�а�˳��洢�ļ���Ϣ�����FAT��
	for (int i = 0; i < x - 1; i++)
	{
		FATS[i + 1] = FATX[i];
	}

}

void Go(char Path[])//����ǰ��ĳ���ĵ�ַ
{
	int Pos = 1, j = 0;
	string Str = Path;
	string Sub[10];
	if (!strcmp(Path, "\\"))
	{
		Cd_back();
		return;
	}
	for (int i = 0; i < Str.length(); i++)		//����Ŀ¼�����ָ����
	{
		if (Path[i] == '\\')
			j++;
	}
	int i;

	Cd_back();	//�ص���Ŀ¼

	//j�Ƿָ�����
	for (i = 0; i < j; i++)   //�����Ŀ¼
	{
		Str = Str.substr(1, Str.length() - 1);//ȥ�������/
		Pos = Str.find("\\", 0);//�����0������ʼ�����ҵ�һ�����ַָ������ַ�λ��,��0�ʼ����

		if (Pos != -1)
		{
			Sub[i] = Str.substr(0, Pos);//ȡstrǰ���ָ�����λ��
			//��string���н�ȡ
			Str = Str.substr(Pos);
		}
		else//���һ��
			Sub[i] = Str;//��Ȼ�Ͳ�����

		
		Cd(const_cast<char *>(Sub[i].c_str()));	//һ��һ��CD,�м�㲻��ʾ������һ������ָ����С��д�ڴ�ĳ�Ա����
	}
}

void Go_share(char Path[])//����ǰ��ĳ���ĵ�ַ
{
	int Pos = 1, j = 0;
	string Str = Path;
	string Sub[10];
	if (!strcmp(Path, "\\"))
	{
		Cd_back();
		return;
	}
	for (int i = 0; i < Str.length(); i++)		//����Ŀ¼�����ָ����
	{
		if (Path[i] == '\\')
			j++;
	}
	int i;

	Cd_back();	//�ص���Ŀ¼

	//j�Ƿָ�����
	for (i = 0; i < j; i++)   //�����Ŀ¼
	{
		Str = Str.substr(1, Str.length() - 1);//ȥ�������/
		Pos = Str.find("\\", 0);//�����0������ʼ�����ҵ�һ�����ַָ������ַ�λ��,��0�ʼ����

		if (Pos != -1)
		{
			//����ҵ���
			Sub[i] = Str.substr(0, Pos);//ȡstrǰ���ָ�����λ��
			Str=Str.substr(Pos);//ɾ����0��ʼ��pos���ַ�
		}
		else
			Sub[i] = Str;//��Ȼ�Ͳ�����

		Cd_1(const_cast<char *>(Sub[i].c_str()));	//һ��һ��CD,���һ����ʾ
	}
}

void show_space()//չʾ���õ�FCB�ͻ��յ�FCB,�����̿�ռ��
{
	//����FCB
	int x = 0;
	for (int i = 0; i < FILE_INDEX_NUM; i++)
	{
		if (SB.FI[i].IndexEnable != 0)//�ҵ���Ӧ���ļ�������
		{
			x++;
		}
	}
	cout << "FCB���ã�" << x << "�飨һ��100��)" << endl;

	//���������̿�
	int y = 0;
	for (int i = 0; i < DATA_NUM; i++)
	{
		if (SB.FAT[i] == 0)
		{
			y++;
		}
	}
	cout << "���ݿ����ã�" << 1024 - y << "�飨һ��1024�飩" << endl;
}


//��ʾ��ǰĿ¼�������ļ����ļ���
void show_dir()
{
	for (int i = 0; i < FILE_INDEX_NUM; i++)//���е��ļ��������б���
	{//����ļ������ĸ��׽��͵�ǰ�ĸ��׽����ȶ��ҵ�ǰ�����ͬ
		if (SB.FI[i].ParentIndexId == CS.ParentIndexId && SB.FI[i].FileLevel == CS.CurrentLevel&&SB.FI[i].IndexEnable != 0)
		{
			cout << "�ڵ�ǰĿ¼�£�" << endl;
			if (SB.FI[i].Type == ADIR)//���ļ���
				cout << "���ļ��У�" << SB.FI[i].FileName << endl;
			else
				cout << "���ļ���" << SB.FI[i].FileName << endl;
		}
	}
}

void Format()//�Դ��̿ռ���и�ʽ��
{
	InitAllUsers();//�����û���ʼ��
	for (int i = 0; i < FAT_NUM; i++)	//FAT���ʼ������
	{
		SB.FAT[i] = 0;
	}
	//��ʼ���ļ�����,��Ŀ¼,��һ��

	for (int i = 0; i < FILE_INDEX_NUM; i++)
	{
		SB.FI[i].IndexEnable = 0;//��ʼ���ǿյ�
		memset(SB.FI[i].CreateTime, '\0', sizeof(SB.FI[i].CreateTime));
		memset(SB.FI[i].FileName, '\0', sizeof(SB.FI[i].FileName));	//Ԫ����
	}

	strcpy(SB.FI[0].FileName, "\\");
	SB.FI[0].ParentIndexId = FILE_INDEX_NUM + 1;//��ʾΪ��
	SB.FI[0].FileBlockId = DATA_NUM + 1;
	SB.FI[0].FileLevel = 1;
	SB.FI[0].IndexEnable = 1;
	SB.FI[0].Type = ADIR;//�ļ���


	fpd = fopen("C:\\filesys\\filesys.disk", "w+b");
	fseek(fpd, DATA_SIZE, SEEK_CUR);		//����ռ�
	fprintf(fpd, "VitualDiskEnd");

	fpd1 = fopen("C:\\filesys\\superblock.disk", "w+b");
	fseek(fpd1, sizeof(SuperBlock), SEEK_CUR);
	fprintf(fpd1, "superblockEnd");

	cout << ("--------------------------------���̿ռ䴴���ɹ�") << endl;

	Cd_back();//�����Ŀ¼
	//��ʼ���ļ�,���������û��ļ���cbl,root,�ڶ���
	CreateFileElement(SB.SU[0], "root", ADIR, "", 0);
	CreateFileElement(SB.SU[1], "cbl", ADIR, "", 0);
	SaveVirtualDisk();
	fclose(fpd);
	fclose(fpd1);
	cout << ("---------------------------------��ʽ���������!") << endl;
	LoadVirtualDisk();
}

void GetTimeStr(int YMD[])//��õ�ǰʱ�䣬17λchar����
{
	time_t t;
	struct tm *p;
	time(&t);
	p = localtime(&t);
	YMD[0] = p->tm_year;
	YMD[1] = p->tm_mon;
	YMD[2] = p->tm_mday;
	YMD[3] = p->tm_hour;
	YMD[4] = p->tm_min;
	YMD[5] = p->tm_sec;
}

void LoadVirtualDisk()//���������������
{
	fpd1 = fopen("C:\\filesys\\superblock.disk", "rb+");
	fpd = fopen("C:\\filesys\\filesys.disk", "rb+");//���ݿռ�
	if (fpd == NULL) {//���û�м�Ϊ�գ��޷����ش���
		cout << "�޷�������������ļ�!" << endl;
		exit(0);
	}
	fseek(fpd1, 0, SEEK_SET);//��SEEK_SET�ļ���ͷΪ��׼���ƽ�0������fpd��λ����ͷ
	fread(&SB, sizeof(SuperBlock), 1, fpd1);//��һ��Ԫ�أ�ÿ��Ԫ�� sizeof(SuperBlock)���ֽ�
	//��ȡֱ�Ӷ�ȡ��SuperBlock
	Cd_back();
}


int main()
{
	cout << ("---------Welcome To My Rom System Of File(FAT)---------\n");
	cout << ("\n======================readme��=======================\n");
	cout << ("--------------------------------------------------------------\n");
	cout << ("|| format            :��ʽ������.                   || \n");
	cout << ("|| create            :�����ļ�.                     || \n");
	cout << ("|| create_dir        :�����ļ���.                   || \n");
	cout << ("|| delete            :ɾ���ļ�                      || \n");
	cout << ("|| logout            :ע����ǰ�û�.                 || \n");
	cout << ("|| show_att          :��ʾ��ǰ�ļ�/�ļ��е�����.    || \n");
	cout << ("|| show_fat          :չʾfat��.                    || \n");
	cout << ("|| read              ����ȡ�ļ�����.                || \n");
	cout << ("|| cd                �������ļ���.                  || \n");
	cout << ("|| back              :����.                         || \n");
	cout << ("|| rename            :������.                       || \n");
	cout << ("|| go                : ����·��ֱ��ǰ��             || \n");
	cout << ("|| exit              :�˳����ر�ϵͳ.               || \n");
	cout << ("|| show_dir          ���鿴��ǰĿ¼                 || \n");
	cout << ("|| shared            ��ʵ��������                   || \n");
	cout << ("-------------------------------------------------------------\n");

	if (access("C:\\filesys\\filesys.disk", 0)==-1|| access("C:\\filesys\\superblock.disk", 0) == -1)
	{
		cout << "���̿ռ䲻����, ����ͨ��Ĭ�����ò����¸�ʽ�������ļ�" << endl;
		Format();//��ʽ��
	}
	else
	{
		cout << "�Ѵ������̿ռ䣬��ȴ������ļ�����" << endl;
		LoadVirtualDisk();
		cout << "-----------------------------------��������(filesys.disk)�������" << endl;
		cout << "-------------------------------------�������һ������" << endl;
	}
	cout << "��ǰ�Ѿ������FAT�����ļ�����ϵͳ�Ĵ���" << endl;
	cout << "��������С: " << DATA_SIZE << "�ֽ�" << endl;
	cout << "��������С: " << sizeof(SuperBlock) << "�ֽ�" << endl;
	cout << "FAT������С: " << FAT_SIZE << "�ֽڼ�" << FAT_SIZE / 1024 << "KB" << endl;
	cout << "���ݴ���λ����C:\\filesys\\filesys.disk" << endl;
	cout << "�������λ����C:\\filesys\\superblock.disk" << endl;
	cout << ("-------------------------------------------------------------\n");
	cout << "��ϵͳ���������û�root��cbl" << endl;
	cout << "��ѡ���û����е�¼" << endl;

	userchoose();
	while (1)
	{
		cout << "����������ָ�" << endl;
		char cmd[1000];
		scanf("%s", cmd);

		if (!strcmp(cmd, "format"))//��ʽ������ʵ��
		{
			Format();
			logout();
		}
		else if (!strcmp(cmd, "create_dir"))//�����ļ���
		{
			cout << "������Ҫ�����ļ������ƣ�";

			char Name[10];
			scanf("%s", Name);
			create_dir(Name);
		}
		else if (!strcmp(cmd, "delete"))//ɾ���ļ�
		{
			cout << "������Ҫɾ�����ļ�����" << endl;
			char Name[10];
			scanf("%s", Name);
			delet(Name);

		}
		else if (!strcmp(cmd, "back"))//ɾ���ļ�
		{
			back();
		}
		else if (!strcmp(cmd, "logout"))//ע���û�
		{
			cout << ("--------------------------------------------------ע����ǰ�û���¼�ɹ�\n");
			logout();

		}
		else if (!strcmp(cmd, "create"))//�����ļ�
		{
			cout << "������Ҫ�������ļ����ƣ�";
			char Name[10];
			scanf("%s", Name);
			create(Name);
		}
		else if (!strcmp(cmd, "show_att"))//չʾ�ļ�����
		{
			show_att();

		}
		else if (!strcmp(cmd, "modify"))//�޸��ļ�
		{
			char Name[32];
			cout << "������Ҫ�޸ĵ��ļ���" << endl;
			scanf("%s", Name);
			if (CheckFileName(Name))
			{
				cout << ("����,�ļ���������!") << endl;
				continue;
			}
			read(Name);
			char c[3];
			cout << "��ȷ���Ƿ��޸����" << endl;
			scanf("%s", c);
			if (!strcmp(c, "ok"))
			{
				modify(Name);
				cout << "-----------------------------------�޸ĳɹ�" << endl;
			}

		}
		else if (!strcmp(cmd, "read"))//��ȡ�ļ�
		{
			cout << "������Ҫ��ȡ���ļ�����";
			char Name[10];
			scanf("%s", Name);
			read(Name);

		}
		else if (!strcmp(cmd, "show_fat"))
		{
			show_fat();//չʾ��ǰϵͳ��fat��

		}
		else if (!strcmp(cmd, "show_dir"))
		{
			show_dir();

		}
		else if (!strcmp(cmd, "show_space"))
		{
			show_space();

		}

		else if (!strcmp(cmd, "shared"))
		{

			cout << "������Ҫ���������ӵ�Դ�ļ�·����" << endl;
			char Path[256];
			scanf("%s", Path);


			cout << "�����뱾�ļ����ƣ�" << endl;
			char Name[32];
			scanf("%s", Name);
			shared(Path, Name);//ʵ��������
		}
		else if (!strcmp(cmd, "rename"))
		{
			cout << "������Ҫ�޸ĵ��ļ����ƣ�";
			char  Name[10];
			scanf("%s", Name);
			if (CheckFileName(Name))
			{
				cout << ("����,�ļ���������!") << endl;
				continue;
			}
			cout << "������Ҫ�޸�Ϊ���ļ����ƣ�";
			char Rename[10];
			scanf("%s", Rename);
			rename(Name, Rename);
			cout << "---------------------------rename���" << endl;
		}
		else if (!strcmp(cmd, "go"))
		{
			char Path[256];
			cout << "������Ҫǰ�����ļ�/�ļ���·��";
			scanf("%s", Path);
			Go(Path);
		}
		else if (!strcmp(cmd, "show_path"))
		{
			show_path();
		}
		else if (!strcmp(cmd, "cd"))
		{
			char Name[256];
			cout << "�����뵱ǰĿ¼��Ҫǰ�����ļ���";
			scanf("%s", Name);
			Cd(Name);
			show_path();
		}
		else if (!strcmp(cmd, "exit"))
		{
			exit();
			break;
		}
		else
			cout << "��Чָ��,����������:" << endl;
	}

	cout << "------------------------------�˳��ɹ�" << endl;
	cout << ("-------------Thank you for using FAT file system!\n");
	system("pause");
	return 0;
}