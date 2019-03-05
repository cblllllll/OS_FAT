#pragma once
#define BLOCK_SIZE 16 //һ��1KB��һ���̿�
#define FILEDATA_SIZE 16
//�ļ�������
//FAT����,FAT���FAT���ܴ�С��Ϊ256���������̿���Ϊ256��һ��1KB��2^8=256����һ�ֽڡ�
#define FAT_SIZE  1024//��FAT����������������һ��������ռһ���̿��С

//�ļ���
//������
#define DATA_NUM 1024//�������̿���
#define DATA_SIZE DATA_NUM*BLOCK_SIZE
#define FAT_NUM DATA_NUM 

//���̿ռ�
#define FILE_INDEX_NUM 100
typedef enum
{
	AFILE,		//�ļ�
	ADIR			//�ļ���
} FileType;		//�ļ�Ԫ������

typedef enum
{
	ADMIN,		//����Ա
	COMM		//һ���û�
} UserType;		//�û�����

typedef enum
{
	CLOSED,		//�ѹر�
	OPENED,		//�Ѵ�
	READING,	//���ڶ�
	WRITING,		//����д
	FOLDER		//�ļ���
} FileStatus;	//�ļ�״̬

typedef struct TUser
{
	char UserName[8];		//�û�����
	char UserPw[4];		//�û�����
	int UserRoot;	//�û����ڵ�
	UserType Type;		//�û����� 
} User;


typedef struct TFileBlock
{
	int FileBlockSize;	//�ļ��������
	long FileBlockAddr;		//�ļ����ַ

} FileBlock;

typedef struct TFile
{
	char FileName[32];	//Ԫ����
	FileType Type;
	long FileLen;	//�ļ�Ԫ�صĳ���
	User Creator;
	int ParentIndexId;		//���ڵ���,FILEINDEX�����νṹ�еĸ��׽�������൱�ڸ������ĸ��ļ�����
	int FileBlockId;	//Ԫ�����ڵ���ʼ������ţ���FAT���б��
	int FileLevel;		//Ԫ�����ڵĲ��
	int IndexEnable;	//�Ƿ���Ч��1��ʾ��Ч���ѱ�ʹ����䣬0��ʾ����ʹ��
	int CreateTime[6];	//����ʱ�䣬���ڸ�ʽ��MM/DD/YY HH:MI:SS
	int LastModTime[6];	//���һ���޸�ʱ��
	int number;//FCB���ļ����еı��
	bool share;
	char share_path[256];
} FCB;		//����Ԫ�ؽṹ

typedef struct TCurrentStatus
{
	User CurrentUser;	//��ǰ�û�
	int ParentIndexId;	//��ǰ��ĸ��ڵ�ID,FILEINDEX
	int CurrentLevel;	//�û������ļ�ϵͳ��
	
	char CurrentPath[256];	//��ǰ·��

} CurrentStatus;		//ϵͳ��ǰ״̬

typedef struct TSuperBlock
{
	int FAT[FAT_NUM];	//�ļ�ϵͳ�е�FAT��

	FCB FI[FILE_INDEX_NUM];		//�ļ�ϵͳ�е��ļ�����

	User SU[2];

} SuperBlock;	//�ļ�ϵͳ�ṹ 7156




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




























