#pragma once
#define BLOCK_SIZE 16 //一个1KB，一个盘块
#define FILEDATA_SIZE 16
//文件管理区
//FAT表区,FAT表项，FAT区总大小即为256，数据区盘块数为256个一个1KB，2^8=256正好一字节。
#define FAT_SIZE  1024//和FAT表项数数据区块数一样，但是占一个盘块大小

//文件区
//数据区
#define DATA_NUM 1024//数据区盘块数
#define DATA_SIZE DATA_NUM*BLOCK_SIZE
#define FAT_NUM DATA_NUM 

//磁盘空间
#define FILE_INDEX_NUM 100
typedef enum
{
	AFILE,		//文件
	ADIR			//文件夹
} FileType;		//文件元素类型

typedef enum
{
	ADMIN,		//管理员
	COMM		//一般用户
} UserType;		//用户类型

typedef enum
{
	CLOSED,		//已关闭
	OPENED,		//已打开
	READING,	//正在读
	WRITING,		//正在写
	FOLDER		//文件夹
} FileStatus;	//文件状态

typedef struct TUser
{
	char UserName[8];		//用户名称
	char UserPw[4];		//用户密码
	int UserRoot;	//用户根节点
	UserType Type;		//用户类型 
} User;


typedef struct TFileBlock
{
	int FileBlockSize;	//文件块的容量
	long FileBlockAddr;		//文件块地址

} FileBlock;

typedef struct TFile
{
	char FileName[32];	//元素名
	FileType Type;
	long FileLen;	//文件元素的长度
	User Creator;
	int ParentIndexId;		//父节点名,FILEINDEX，树形结构中的父亲结点数，相当于告诉在哪个文件夹下
	int FileBlockId;	//元素所在的起始物理块编号，在FAT表中编号
	int FileLevel;		//元素所在的层次
	int IndexEnable;	//是否有效，1表示有效，已被使用填充，0表示不能使用
	int CreateTime[6];	//创建时间，日期格式：MM/DD/YY HH:MI:SS
	int LastModTime[6];	//最后一次修改时间
	int number;//FCB在文件块中的编号
	bool share;
	char share_path[256];
} FCB;		//索引元素结构

typedef struct TCurrentStatus
{
	User CurrentUser;	//当前用户
	int ParentIndexId;	//当前层的父节点ID,FILEINDEX
	int CurrentLevel;	//用户所在文件系统层
	
	char CurrentPath[256];	//当前路径

} CurrentStatus;		//系统当前状态

typedef struct TSuperBlock
{
	int FAT[FAT_NUM];	//文件系统中的FAT表

	FCB FI[FILE_INDEX_NUM];		//文件系统中的文件索引

	User SU[2];

} SuperBlock;	//文件系统结构 7156




void AddFCB(int FBId, int Level, char FileName[], FileType Type, User Creator, int ParentId, int  time1[], int time2[], long y);
void back();
void Cd(char Name[]);

void Cd_1(char Name[]);
void Cd_back();
bool CheckFileName(char Name[]);
void create(char Name[]);
void create_dir(char Name[]);
void CreateFileElement(User Creator, char FileName[], FileType Type, char Content[], int ParentId);
void delet(char Name[]);
void exit();
void Format();
int get_num(int x, int y);
int GetBlankFileBlockId();
void GetTimeStr(int YMD[]);
void Go(char Path[]);
void Go_share(char Path[]);
void InitAllUsers();
void LoadVirtualDisk();
void logout();
void modify(char Name[]);
void read(char Name[]);
void rename(char Name[], char Rename[]);
void SaveVirtualDisk();
void shared(char Path[], char Name[]);
void show_att();
void show_dir();
void show_fat();
void show_path();
void show_space();
void UpdateFAT(int FATId, int x, int FATS[]);
void UpdateFCB(int FIId, int time[], long y);
void userchoose();




























