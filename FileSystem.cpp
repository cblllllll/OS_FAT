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
FCB F;//一个文件项
static char FC[BLOCK_SIZE];	//文件内容,中转

void create_dir(char Name[])//创建目录
{

	if (!CheckFileName(Name))
	{
		cout << ("错误,文件名已存在!") << endl;
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
		cout << ("错误,文件名已存在!") << endl;
		return;
	}
	strcpy(F.FileName, Name);
	FT = AFILE;
	F.Type = FT;
	cout << "请输入要创建的文件,文件内容：";
	scanf("%s", FC);
	CreateFileElement(CS.CurrentUser, F.FileName, F.Type, FC, CS.ParentIndexId);
	cout << "-----------------------------------------文件创建完成" << endl;
	show_dir();
}

void show_path()//展示当前路径
{
	cout << "当前路径是：" << CS.CurrentPath << endl;
}


void delet(char Name[])
{

	for (int i = 0; i < FILE_INDEX_NUM; i++)		//搜索索引
	{
		if (SB.FI[i].ParentIndexId == CS.ParentIndexId &&
			SB.FI[i].FileLevel == CS.CurrentLevel &&
			strcmp(SB.FI[i].FileName, Name) == 0 &&//查找到了
			SB.FI[i].IndexEnable != 0)
		{
			if (strcmp(SB.FI[i].Creator.UserName, CS.CurrentUser.UserName) != 0)
			{
				cout << ("对不起，你没有权限删除这个文件(夹)！\t") << endl;
				return;
			}
			if (SB.FI[i].FileLevel != 2)
			{
				//删除对应文件数据区内容
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
				while (fat_copy[x] != -1)//当不是根节点时
				{

					fat_copy[x] = fat_copy[fat_copy[x]];
					j++;
					fatx[j] = fat_copy[x];
				}

				//这样fatx表里填充了所有的文件盘块以FATid为开头，以-1为结尾
				int n = 0;//fatx[0]里存放的是FAT首项
				while (fatx[n] != -1)
				{

					fseek(fpd, fatx[n] * BLOCK_SIZE, SEEK_SET);//转移指针，写文件
					char c[BLOCK_SIZE];
					memset(c, '\0', BLOCK_SIZE);
					fwrite(c, BLOCK_SIZE, 1, fpd);	//写文件内容
					n++;
				}//全部填充为'\0'
				for (int m = 0; m < 1024; m++)
				{
					fat_copy[m] = SB.FAT[m];
				}
				//删除对应的FAT表内容,置零
				for (int x = SB.FI[i].FileBlockId;; fat_copy[x] = fat_copy[fat_copy[x]])//当不是根节点时
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
				//删除对应文件目录项内容FI[I]，直接在内存中修改
				SB.FI[i].IndexEnable = false;
				memset(SB.FI[i].CreateTime, '\0', sizeof(SB.FI[i].CreateTime));
				memset(SB.FI[i].LastModTime, '\0', sizeof(SB.FI[i].LastModTime));
				cout << "-----------------------------------删除文件：" << Name << "成功" << endl;
			}
			else
			{
				cout << ("对不起，你不能删这个文件夹！\t") << endl;
				return;
			}
		}
	}
	show_dir();
}

void modify(char Name[])
{
	//将修改后的文件进行读取
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
	int ID;//获取此文件的FCB块号
	for (int i = 0; i < FILE_INDEX_NUM; i++)		//搜索索引
	{
		if (SB.FI[i].ParentIndexId == CS.ParentIndexId &&
			SB.FI[i].FileLevel == CS.CurrentLevel && SB.FI[i].IndexEnable == 1 &&
			strcmp(SB.FI[i].FileName, Name) == 0)
		{
			ID = i;
		}
	}
	if (SB.FI[ID].FileLevel != 2)
	{//1)删除对应文件数据区内容
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
		while (fat_copy[FATId] != -1)//当不是根节点时
		{

			fat_copy[FATId] = fat_copy[fat_copy[FATId]];
			j++;
			fatx[j] = fat_copy[FATId];
		}
		//这样fatx表里填充了所有的文件盘块以FATid为开头，以-1为结尾
		int n = 0;//fatx[0]里存放的是FAT首项
		while (fatx[n] != -1)
		{

			fseek(fpd, fatx[n] * BLOCK_SIZE, SEEK_SET);//转移指针，写文件
			char c[BLOCK_SIZE];
			memset(c, '\0', BLOCK_SIZE);
			fwrite(c, BLOCK_SIZE, 1, fpd);	//写文件内容
			n++;
		}//全部填充为'\0'
		for (int m = 0; m < 1024; m++)
		{
			fat_copy[m] = SB.FAT[m];
		}
		//删除对应的FAT表内容,置零
		for (int x = SB.FI[ID].FileBlockId;; fat_copy[x] = fat_copy[fat_copy[x]])//当不是根节点时
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

		//修改文件修改时间，FILELEN
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
		k = get_num(x, y);//所需要的盘块数
		if (l < k)
		{
			cout << ("错误,找不到足够空白的文件块,可能系统已满!") << endl;
			return;
		}
		FCB FE;//文件中保存的首FAT地址

		GetTimeStr(FE.LastModTime);
		FE.FileLen = strlen(a);

		int FATS[1024];
		//当x是0
		if (x == 0)
		{
			fseek(fpd, FATId * BLOCK_SIZE, SEEK_SET);//转移指针，写文件
			fwrite(a, strlen(a), 1, fpd);	//写文件内容

			UpdateFAT(FATId, x, FATS);//由开始的FAT表项
		}
		else if (y == 0)//即有x个盘块
		{
			UpdateFAT(FATId, x, FATS);

			for (int i = 0; i < x; i++)
			{

				fseek(fpd, FATS[i] * BLOCK_SIZE, SEEK_SET);//转移指针，写文件
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
				fwrite(c, strlen(c), 1, fpd);	//写文件内容
			}
		}
		else
		{//x!=0,y!=0
			//即有x+1个盘块
			UpdateFAT(FATId, x + 1, FATS);
			for (int i = 0; i < x + 1; i++)
			{
				fseek(fpd, FATS[i] * BLOCK_SIZE, SEEK_SET);//转移指针，写文件
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
				fwrite(c, strlen(c), 1, fpd);	//写文件内容
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
		cout << ("错误,文件名不存在!") << endl;
		return;
	}
	FT = AFILE;
	FILE *fpr = fopen("C:\\filesys\\read.txt", "w");
	int i;
	for (i = 0; i < FILE_INDEX_NUM; i++)		//搜索索引
	{
		if (SB.FI[i].ParentIndexId == CS.ParentIndexId &&
			SB.FI[i].FileLevel == CS.CurrentLevel && SB.FI[i].IndexEnable == 1 &&
			strcmp(SB.FI[i].FileName, Name) == 0)//找到对应的文件管理项
			if (SB.FI[i].share==1)
			{
				Go_share(SB.FI[i].share_path);
				return;
			}
			else
			{
				//给出FAT表起始项
				//删除对应文件数据区内容
				int fatx[1024];
				int j = 1;
				int x = SB.FI[i].FileBlockId;
				int fat_copy[1024];
				for (int m = 0; m < 1024; m++)
				{
					fat_copy[m] = SB.FAT[m];
				}
				fatx[0] = SB.FI[i].FileBlockId;//这个是首项数
				fatx[1] = SB.FAT[x];

				while (fat_copy[x] != -1)//当不是根节点时
				{
					fat_copy[x] = fat_copy[fat_copy[x]];
					j++;
					fatx[j] = fat_copy[x];
				}//获得fatx即为链表序列
				int y = SB.FI[i].FileLen % DATA_SIZE;
				int n = 0;
				while (fatx[n] != -1)
				{
					if (fatx[n + 1] == -1)
					{
						fseek(fpd, fatx[n] * BLOCK_SIZE, SEEK_SET);//转移指针，写文件
						fread(FC, y, 1, fpd);//将这个文件读到FC中
						fwrite(FC, strlen(FC), 1, fpr);
						n++;
					}
					else
					{
						fseek(fpd, fatx[n] * BLOCK_SIZE, SEEK_SET);//转移指针，写文件
						fread(FC, BLOCK_SIZE, 1, fpd);//将这个文件读到FC中
						fwrite(FC, strlen(FC), 1, fpr);
						n++;
					}

				}//全部填充为'\0'
				//保存修改后的文件
				break;
			}
		
	}

	fclose(fpr);
	cout << "-----------------------------------------文件读取完成" << endl;
}

void rename(char Name[], char Rename[])
{//前是原文件名，后面是目标文件名

	if (CheckFileName(Name))
	{
		cout << ("错误,文件不存在!") << endl;
		return;
	}
	if (!CheckFileName(Rename))
	{
		cout << ("错误,文件名已存在!") << endl;
		return;
	}
	for (int i = 0; i < FILE_INDEX_NUM; i++)		//搜索索引
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
	//软链接
	//获取某个文件的路径，FCB中添加一项即为目标项的地址
	char *a = new char[256];
	for (int i = 0; i < strlen(Path); i++)
	{
		a[i] = CS.CurrentPath[i];
	}

	Go(Path);
	Go(a);
	int i;
	for (i = 0; i < FILE_INDEX_NUM; i++)//对所有的FCB进行遍历查找，检查可用的FCB项
	{
		if (SB.FI[i].IndexEnable == 0)
		{
			break;
		}
		else continue;
	}

	//需要检查有没有空的文件处
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
	int SplitNum = 0;	//分割符出现的次数
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
						CS.ParentIndexId = i;//第几个文件索引块
						CS.CurrentLevel++;	//当前层号增加
						//如果当前路径不是一位的就加上分隔符
						if (strlen(CS.CurrentPath) != 1)
							strcat(CS.CurrentPath, "\\");
						//加上之后再加上路径，就是当前路径了
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
				cout << "位置不可用, 请检查路径是否正确" << endl;
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

	cout << "-------------------------------FAT表输出完成" << endl;
	cout << "---------------------------------地址为：C:\\filesys\\fat.txt" << endl;
}


void logout()
{
	Cd_back();

	cout << "此系统已有两个用户root和cbl" << endl;
	cout << "请选择用户进行登录" << endl;
	userchoose();
}


void userchoose()
{
	while (1)
	{
		cout << "请输入选择的用户名：";
		char c[100];
		scanf("%s", c);

		if (!strcmp(c, "root"))
		{
			while (1)
			{
				cout << "root用户您好，请输入您的密码：";
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
					CS.CurrentLevel = 3;//根目录当前等级是2,最开始是1
					strcat(CS.CurrentPath, "root");//加上分隔符
					CS.ParentIndexId = 1;
					cout << endl;
					cout << ("-------------------------------------------------------------\n");
					cout << "root用户，欢迎进入FAT磁盘文件管理系统！" << endl;
					cout << "当前路径是：" << CS.CurrentPath << endl;
					show_dir();
					break;
				}
				else
				{
					cout << "输入错误，请重新输入" << endl;
				}
			}
			break;
		}
		else if (!strcmp(c, "cbl"))
		{
			while (1)
			{
				cout << "cbl用户您好，请输入您的密码：";
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
					CS.CurrentLevel = 3;//根目录当前等级是2,最开始是1
					strcat(CS.CurrentPath, "cbl");//加上分隔符
					CS.ParentIndexId = 2;
					cout << endl;
					cout << ("-------------------------------------------------------------\n");
					cout << "cbl用户，欢迎进入FAT磁盘文件管理系统！" << endl;
					cout << "当前路径是：" << CS.CurrentPath << endl;
					show_dir();
					break;
				}
				else
				{
					cout << "输入错误，请重新输入" << endl;
				}
			}
			break;
		}
		else
			cout << "输入错误，请重新输入" << endl;
	}
}


void show_att()//展示属性
{
	char Name[256];
	cout << "请输入要查询属性的文件（夹）名:";
	scanf("%s", Name);
	for (int i = 0; i < FILE_INDEX_NUM; i++)		//搜索索引
	{
		if (SB.FI[i].ParentIndexId == CS.ParentIndexId &&
			SB.FI[i].FileLevel == CS.CurrentLevel &&
			strcmp(SB.FI[i].FileName, Name) == 0)
		{
			//读取文件

			cout << "名称：" << SB.FI[i].FileName << endl;
			if (!F.Type)
			{
				cout << "类型：文件" << endl;
				cout << "长度：" << SB.FI[i].FileLen << endl;	//文件元素的长度
			}
			else
			{
				cout << "类型：文件夹" << endl;
			}
			cout << "创建者：" << SB.FI[i].Creator.UserName << endl;
			cout << "创建时间："<<endl;
			
			cout << SB.FI[i].CreateTime[0] << "年" << SB.FI[i].CreateTime[1] << "月" << SB.FI[i].CreateTime[2] <<
				"日" << SB.FI[i].CreateTime[3] << "时" << SB.FI[i].CreateTime[4] << "分"<<SB.FI[i].CreateTime[5] << "秒" << endl;
	
			cout << "最后修改时间："<< endl;
			cout << SB.FI[i].LastModTime[0] << "年" << SB.FI[i].LastModTime[1] << "月" << SB.FI[i].LastModTime[2] <<
				"日" << SB.FI[i].LastModTime[3] << "时" << SB.FI[i].LastModTime[4] << "分" << SB.FI[i].LastModTime[5] << "秒" << endl;
		}
	}
}

//初始化2个用户，一个是管理员
void InitAllUsers()
{
	//初始化预设用户
	strcpy(SB.SU[0].UserName, "root");
	SB.SU[0].Type = ADMIN;//管理员
	strcpy(SB.SU[0].UserPw, "root");
	SB.SU[0].UserRoot = 1;
	strcpy(SB.SU[1].UserName, "cbl");
	SB.SU[1].Type = COMM;
	strcpy(SB.SU[1].UserPw, "cbl");
	SB.SU[1].UserRoot = 2;//用户根节点
}

void Cd_back()//回到用户注册处
{
	CS.CurrentLevel = 2;//根目录当前等级是2,最开始是1
	strcpy(CS.CurrentPath, "\\");//加上分隔符
	CS.ParentIndexId = 0;
}

void back()//回退一级
{
	int SplitNum = 0;
	int i;

	if (CS.CurrentLevel > 2)//如果当前等级大于2
	{
		CS.CurrentLevel--;
		CS.ParentIndexId = SB.FI[CS.ParentIndexId].ParentIndexId;//前推一级
		for (i = strlen(CS.CurrentPath) - 1; i > 0; i--)//倒着来找分割符，进行前推一级，所以把最近的一个路径改掉
		{
			if (CS.CurrentPath[i] == '\\')
			{
				SplitNum++;//分割符加一
				if (SplitNum == 1)	//已过滤掉一个目录名
					break;
			}
		}
		char TempPath[256];		//更改路径，暂存数组
		strcpy(TempPath, CS.CurrentPath);//先把现在的路径复制到TempPath中，再
		memset(CS.CurrentPath, '\0', 256);//将CurrentPath全变0
		if (i == 0)	i++;//如果i=0，即到头了，i=1
		strncpy(CS.CurrentPath, TempPath, i);//将TempPath中前i个给当前路径
		show_path();
		if (CS.CurrentLevel == 2)
		{
			cout << "此系统已有两个用户root和cbl" << endl;
			cout << "请选择用户进行登录" << endl;
			userchoose();
		}
	}
}
//进行目录管理
//回到根目录
void Cd(char Name[])
{
	int SplitNum = 0;	//分割符出现的次数
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
						cout << ("对不起,你无权访问该文件或文件夹!") << endl;
						return;
					}
					if (SB.FI[i].Type == ADIR)
					{
						CS.ParentIndexId = i;//第几个文件索引块
						CS.CurrentLevel++;	//当前层号增加
						//如果当前路径不是一位的就加上分隔符
						if (strlen(CS.CurrentPath) != 1)
							strcat(CS.CurrentPath, "\\");
						//加上之后再加上路径，就是当前路径了
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
				cout << "位置不可用, 请检查路径是否正确" << endl;
			}
		}
	}
}
bool CheckFileName(char Name[])
{
	//检查是否存在重名
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

//更新文件索引，由索引的id，元素名，文件夹还是文件，权限
//对已知id文件的索引进行更新
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

//创建一文件元素,返回文件所在FAT表起始文件块号
void CreateFileElement(User Creator, char FileName[], FileType Type, char Content[], int ParentId)
{

	if (!CheckFileName(FileName))
	{
		cout << ("错误,文件名已存在!") << endl;
		return;
	}

	if (Type == ADIR)
	{
		FCB FE;
		FE.FileLevel = CS.CurrentLevel;		//建立在当前层
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
		FATId = GetBlankFileBlockId();//开始创建时，每个文件配套一个FATId
		if (FATId >= DATA_NUM)
		{
			cout << ("错误,找不到空白的文件块,可能系统已满!") << endl;
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
		k = get_num(x, y);//所需要的盘块数
		if (l < k)
		{
			cout << ("错误,找不到足够空白的文件块,可能系统已满!") << endl;
			return;
		}
		FCB FE;
		FE.FileBlockId = FATId;//文件中保存的首FAT地址
		FE.FileLevel = CS.CurrentLevel;		//建立在当前层
		FE.Type = Type;

		GetTimeStr(FE.CreateTime);
		GetTimeStr(FE.LastModTime);

		strcpy(FE.FileName, FileName);
		FE.Creator = Creator;
		FE.FileLen = strlen(Content);

		int FATS[1024];
		//当x是0
		if (x == 0)
		{
			fseek(fpd, FATId * BLOCK_SIZE, SEEK_SET);//转移指针，写文件
			fwrite(Content, strlen(Content), 1, fpd);	//写文件内容
			//fpd是流指针
			//更新索引
			AddFCB(FATId, FE.FileLevel, FE.FileName, FE.Type, Creator, ParentId, FE.CreateTime, FE.LastModTime, FE.FileLen);

			//更新FAT
			UpdateFAT(FATId, x, FATS);//由开始的FAT表项
		}
		else if (y == 0)//即有x个盘块
		{
			UpdateFAT(FATId, x, FATS);

			for (int i = 0; i < x; i++)
			{

				fseek(fpd, FATS[i] * BLOCK_SIZE, SEEK_SET);//转移指针，写文件
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
				fwrite(c, strlen(c), 1, fpd);	//写文件内容
			}
			AddFCB(FATId, FE.FileLevel, FE.FileName, FE.Type, Creator, ParentId, FE.CreateTime, FE.LastModTime, FE.FileLen);

		}
		else {//x!=0,y!=0
			//即有x+1个盘块
			UpdateFAT(FATId, x + 1, FATS);
			for (int i = 0; i < x + 1; i++)
			{
				fseek(fpd, FATS[i] * BLOCK_SIZE, SEEK_SET);//转移指针，写文件
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
				fwrite(c, strlen(c), 1, fpd);	//写文件内容
			}
			AddFCB(FATId, FE.FileLevel, FE.FileName, FE.Type, Creator, ParentId, FE.CreateTime, FE.LastModTime, FE.FileLen);
		}
	}
}

//添加一文件索引
void AddFCB(int FBId, int Level, char FileName[], FileType Type, User Creator, int ParentId, int  time1[], int time2[], long y)
{
	//先选择空白FCB项处进行查找
	int i;
	for (i = 0; i < FILE_INDEX_NUM; i++)//对所有的FCB进行遍历查找，检查可用的FCB项
	{
		if (SB.FI[i].IndexEnable == 0)
		{
			break;
		}
		else continue;
	}

	//需要检查有没有空的文件处
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
	fwrite(&SB, sizeof(SuperBlock), 1, fpd1);		//写入文件控制块

}


//从FAT表中选择一个空白表项
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


void UpdateFAT(int FATId, int x, int FATS[])//开始盘块号更新
{//先查找所有空的盘块号，然后找前x个即可
	if (x == 0 || x == 1)//表示只有一个FAT表项
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
	}//得到所有空闲的FAT表项数

	if (x == 2)//当是两个盘块时
	{
		SB.FAT[FATId] = FATX[0];//赋值
		SB.FAT[FATX[0]] = -1;//赋值
		FATS[0] = FATId;
		FATS[1] = FATX[0];
		return;
	}
	//x>=3时
	SB.FAT[FATId] = FATX[0];//赋值
	for (int i = 0; i < x - 2; i++)
	{
		SB.FAT[FATX[i]] = FATX[i + 1];
	}
	SB.FAT[FATX[x - 2]] = -1;

	FATS[0] = FATId;//在FATS中按顺序存储文件信息链表项，FAT项
	for (int i = 0; i < x - 1; i++)
	{
		FATS[i + 1] = FATX[i];
	}

}

void Go(char Path[])//给出前往某处的地址
{
	int Pos = 1, j = 0;
	string Str = Path;
	string Sub[10];
	if (!strcmp(Path, "\\"))
	{
		Cd_back();
		return;
	}
	for (int i = 0; i < Str.length(); i++)		//计算目录数、分割符数
	{
		if (Path[i] == '\\')
			j++;
	}
	int i;

	Cd_back();	//回到根目录

	//j是分隔符数
	for (i = 0; i < j; i++)   //分离出目录
	{
		Str = Str.substr(1, Str.length() - 1);//去除最左边/
		Pos = Str.find("\\", 0);//从左侧0索引开始，查找第一个出现分隔符的字符位置,从0最开始查找

		if (Pos != -1)
		{
			Sub[i] = Str.substr(0, Pos);//取str前到分隔符的位置
			//对string进行截取
			Str = Str.substr(Pos);
		}
		else//最后一个
			Sub[i] = Str;//不然就不动了

		
		Cd(const_cast<char *>(Sub[i].c_str()));	//一层一层CD,中间层不显示，返回一个你所指定大小可写内存的成员方法
	}
}

void Go_share(char Path[])//给出前往某处的地址
{
	int Pos = 1, j = 0;
	string Str = Path;
	string Sub[10];
	if (!strcmp(Path, "\\"))
	{
		Cd_back();
		return;
	}
	for (int i = 0; i < Str.length(); i++)		//计算目录数、分割符数
	{
		if (Path[i] == '\\')
			j++;
	}
	int i;

	Cd_back();	//回到根目录

	//j是分隔符数
	for (i = 0; i < j; i++)   //分离出目录
	{
		Str = Str.substr(1, Str.length() - 1);//去除最左边/
		Pos = Str.find("\\", 0);//从左侧0索引开始，查找第一个出现分隔符的字符位置,从0最开始查找

		if (Pos != -1)
		{
			//如果找到了
			Sub[i] = Str.substr(0, Pos);//取str前到分隔符的位置
			Str=Str.substr(Pos);//删除从0开始的pos个字符
		}
		else
			Sub[i] = Str;//不然就不动了

		Cd_1(const_cast<char *>(Sub[i].c_str()));	//一层一层CD,最后一层显示
	}
}

void show_space()//展示已用的FCB和还空的FCB,多少盘块占用
{
	//对于FCB
	int x = 0;
	for (int i = 0; i < FILE_INDEX_NUM; i++)
	{
		if (SB.FI[i].IndexEnable != 0)//找到对应的文件管理项
		{
			x++;
		}
	}
	cout << "FCB已用：" << x << "块（一共100块)" << endl;

	//对于数据盘块
	int y = 0;
	for (int i = 0; i < DATA_NUM; i++)
	{
		if (SB.FAT[i] == 0)
		{
			y++;
		}
	}
	cout << "数据块已用：" << 1024 - y << "块（一共1024块）" << endl;
}


//显示当前目录下所有文件和文件夹
void show_dir()
{
	for (int i = 0; i < FILE_INDEX_NUM; i++)//所有的文件索引进行遍历
	{//如果文件索引的父亲结点和当前的父亲结点相等而且当前层次相同
		if (SB.FI[i].ParentIndexId == CS.ParentIndexId && SB.FI[i].FileLevel == CS.CurrentLevel&&SB.FI[i].IndexEnable != 0)
		{
			cout << "在当前目录下：" << endl;
			if (SB.FI[i].Type == ADIR)//是文件夹
				cout << "有文件夹：" << SB.FI[i].FileName << endl;
			else
				cout << "有文件：" << SB.FI[i].FileName << endl;
		}
	}
}

void Format()//对磁盘空间进行格式化
{
	InitAllUsers();//两个用户初始化
	for (int i = 0; i < FAT_NUM; i++)	//FAT表初始化归零
	{
		SB.FAT[i] = 0;
	}
	//初始化文件索引,根目录,第一层

	for (int i = 0; i < FILE_INDEX_NUM; i++)
	{
		SB.FI[i].IndexEnable = 0;//开始都是空的
		memset(SB.FI[i].CreateTime, '\0', sizeof(SB.FI[i].CreateTime));
		memset(SB.FI[i].FileName, '\0', sizeof(SB.FI[i].FileName));	//元素名
	}

	strcpy(SB.FI[0].FileName, "\\");
	SB.FI[0].ParentIndexId = FILE_INDEX_NUM + 1;//表示为空
	SB.FI[0].FileBlockId = DATA_NUM + 1;
	SB.FI[0].FileLevel = 1;
	SB.FI[0].IndexEnable = 1;
	SB.FI[0].Type = ADIR;//文件夹


	fpd = fopen("C:\\filesys\\filesys.disk", "w+b");
	fseek(fpd, DATA_SIZE, SEEK_CUR);		//分配空间
	fprintf(fpd, "VitualDiskEnd");

	fpd1 = fopen("C:\\filesys\\superblock.disk", "w+b");
	fseek(fpd1, sizeof(SuperBlock), SEEK_CUR);
	fprintf(fpd1, "superblockEnd");

	cout << ("--------------------------------磁盘空间创建成功") << endl;

	Cd_back();//进入根目录
	//初始化文件,建立两个用户文件夹cbl,root,第二层
	CreateFileElement(SB.SU[0], "root", ADIR, "", 0);
	CreateFileElement(SB.SU[1], "cbl", ADIR, "", 0);
	SaveVirtualDisk();
	fclose(fpd);
	fclose(fpd1);
	cout << ("---------------------------------格式化磁盘完成!") << endl;
	LoadVirtualDisk();
}

void GetTimeStr(int YMD[])//获得当前时间，17位char数组
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

void LoadVirtualDisk()//加载虚拟磁盘内容
{
	fpd1 = fopen("C:\\filesys\\superblock.disk", "rb+");
	fpd = fopen("C:\\filesys\\filesys.disk", "rb+");//数据空间
	if (fpd == NULL) {//如果没有即为空，无法加载磁盘
		cout << "无法加载虚拟磁盘文件!" << endl;
		exit(0);
	}
	fseek(fpd1, 0, SEEK_SET);//以SEEK_SET文件开头为基准，推进0，即把fpd定位到开头
	fread(&SB, sizeof(SuperBlock), 1, fpd1);//读一个元素，每个元素 sizeof(SuperBlock)个字节
	//读取直接读取，SuperBlock
	Cd_back();
}


int main()
{
	cout << ("---------Welcome To My Rom System Of File(FAT)---------\n");
	cout << ("\n======================readme：=======================\n");
	cout << ("--------------------------------------------------------------\n");
	cout << ("|| format            :格式化磁盘.                   || \n");
	cout << ("|| create            :创建文件.                     || \n");
	cout << ("|| create_dir        :创建文件夹.                   || \n");
	cout << ("|| delete            :删除文件                      || \n");
	cout << ("|| logout            :注销当前用户.                 || \n");
	cout << ("|| show_att          :显示当前文件/文件夹的属性.    || \n");
	cout << ("|| show_fat          :展示fat表.                    || \n");
	cout << ("|| read              ：读取文件内容.                || \n");
	cout << ("|| cd                ：进入文件夹.                  || \n");
	cout << ("|| back              :后退.                         || \n");
	cout << ("|| rename            :重命名.                       || \n");
	cout << ("|| go                : 输入路径直接前往             || \n");
	cout << ("|| exit              :退出并关闭系统.               || \n");
	cout << ("|| show_dir          ：查看当前目录                 || \n");
	cout << ("|| shared            ：实现软链接                   || \n");
	cout << ("-------------------------------------------------------------\n");

	if (access("C:\\filesys\\filesys.disk", 0)==-1|| access("C:\\filesys\\superblock.disk", 0) == -1)
	{
		cout << "磁盘空间不存在, 程序将通过默认配置并重新格式化磁盘文件" << endl;
		Format();//格式化
	}
	else
	{
		cout << "已创建磁盘空间，请等待磁盘文件加载" << endl;
		LoadVirtualDisk();
		cout << "-----------------------------------磁盘内容(filesys.disk)加载完毕" << endl;
		cout << "-------------------------------------请进行下一步操作" << endl;
	}
	cout << "当前已经完成了FAT磁盘文件管理系统的创建" << endl;
	cout << "数据区大小: " << DATA_SIZE << "字节" << endl;
	cout << "管理区大小: " << sizeof(SuperBlock) << "字节" << endl;
	cout << "FAT表区大小: " << FAT_SIZE << "字节即" << FAT_SIZE / 1024 << "KB" << endl;
	cout << "数据磁盘位置在C:\\filesys\\filesys.disk" << endl;
	cout << "管理磁盘位置在C:\\filesys\\superblock.disk" << endl;
	cout << ("-------------------------------------------------------------\n");
	cout << "此系统已有两个用户root和cbl" << endl;
	cout << "请选择用户进行登录" << endl;

	userchoose();
	while (1)
	{
		cout << "请输入任务指令：" << endl;
		char cmd[1000];
		scanf("%s", cmd);

		if (!strcmp(cmd, "format"))//格式化功能实现
		{
			Format();
			logout();
		}
		else if (!strcmp(cmd, "create_dir"))//创建文件夹
		{
			cout << "请输入要创建文件夹名称：";

			char Name[10];
			scanf("%s", Name);
			create_dir(Name);
		}
		else if (!strcmp(cmd, "delete"))//删除文件
		{
			cout << "请输入要删除的文件名称" << endl;
			char Name[10];
			scanf("%s", Name);
			delet(Name);

		}
		else if (!strcmp(cmd, "back"))//删除文件
		{
			back();
		}
		else if (!strcmp(cmd, "logout"))//注销用户
		{
			cout << ("--------------------------------------------------注销当前用户登录成功\n");
			logout();

		}
		else if (!strcmp(cmd, "create"))//创建文件
		{
			cout << "请输入要创建的文件名称：";
			char Name[10];
			scanf("%s", Name);
			create(Name);
		}
		else if (!strcmp(cmd, "show_att"))//展示文件属性
		{
			show_att();

		}
		else if (!strcmp(cmd, "modify"))//修改文件
		{
			char Name[32];
			cout << "请输入要修改的文件名" << endl;
			scanf("%s", Name);
			if (CheckFileName(Name))
			{
				cout << ("错误,文件名不存在!") << endl;
				continue;
			}
			read(Name);
			char c[3];
			cout << "请确定是否修改完成" << endl;
			scanf("%s", c);
			if (!strcmp(c, "ok"))
			{
				modify(Name);
				cout << "-----------------------------------修改成功" << endl;
			}

		}
		else if (!strcmp(cmd, "read"))//读取文件
		{
			cout << "请输入要读取的文件名称";
			char Name[10];
			scanf("%s", Name);
			read(Name);

		}
		else if (!strcmp(cmd, "show_fat"))
		{
			show_fat();//展示当前系统的fat表

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

			cout << "请输入要进行软链接的源文件路径：" << endl;
			char Path[256];
			scanf("%s", Path);


			cout << "请输入本文件名称：" << endl;
			char Name[32];
			scanf("%s", Name);
			shared(Path, Name);//实现软链接
		}
		else if (!strcmp(cmd, "rename"))
		{
			cout << "请输入要修改的文件名称：";
			char  Name[10];
			scanf("%s", Name);
			if (CheckFileName(Name))
			{
				cout << ("错误,文件名不存在!") << endl;
				continue;
			}
			cout << "请输入要修改为的文件名称：";
			char Rename[10];
			scanf("%s", Rename);
			rename(Name, Rename);
			cout << "---------------------------rename完成" << endl;
		}
		else if (!strcmp(cmd, "go"))
		{
			char Path[256];
			cout << "请输入要前往的文件/文件夹路径";
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
			cout << "请输入当前目录下要前往的文件夹";
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
			cout << "无效指令,请重新输入:" << endl;
	}

	cout << "------------------------------退出成功" << endl;
	cout << ("-------------Thank you for using FAT file system!\n");
	system("pause");
	return 0;
}