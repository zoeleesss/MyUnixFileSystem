//
//  dataStructure.h
//  MyUnixFileSystem
//
//  Created by zoe lee on 17/01/2018.
//  Copyright © 2018 zoelee. All rights reserved.
//

#ifndef dataStructure_h
#define dataStructure_h



#include <string>
using namespace std;

struct User     //65B
{
    char userGroup[20];
    char userName[20];
    char userPassword[20];
    char umask[5];
    void initUser(string _group, string _name, string _pwd, string _umask)
    {
        strcpy(userGroup , _group.c_str());
        strcpy(userName, _name.c_str());
        strcpy(userPassword, _pwd.c_str());
        strcpy(umask,_umask.c_str());
    }
};

struct UserTable    //16B       (Deconstruction takes 4B)
{
    int count;
    User * user;
    ~UserTable(){ if (user != NULL &&count != 0){ delete[] user; } }
};


//目录项 用16字节表示，其中14字节为文件名，2字节为I节点号

struct Directory    // 16B
{
    unsigned short inodeIndex;    // points to INode
    char fileName[14];          //filename
    void initDirectory(unsigned short _index, char* _name){ inodeIndex = _index; strcpy(fileName, _name); }
    
};



struct DirectoryTable   //16B   (Deconstruction takes 4B)
{
    int count;                  //how many files in the directory
    Directory* head;            //first directory
    DirectoryTable(){ count = 0; head = NULL ; }
    ~DirectoryTable(){ if (head != NULL&&count!=0)delete[] head; }
};


struct  Block         // 512B
{
    char block[512];
};


//下一组空闲盘块 大小为一个block
struct BlockIndexTable   //512B
{
    unsigned short nextBlocks[50];
    char fill[412];
};


//d  目录文件   - 普通文件      l 链接文件
//  drwxrwxrwx       777     user group all


struct INode        //256B
{
    char mode[12];                  //文件类型及访问权限标志
    char owner[20];                 // 文件拥有者的用户标识
    char group[20];                 // 文件拥有者的组标识
    unsigned short inodeID;         //the node id
    unsigned int linkCount;         //硬连接计数 =0 时需要删除文件
    unsigned int size;              // 文件大小
    unsigned int blockCount;        // the number of block it takes up
    unsigned int visitTime;         // 文件最近一次访问时间
    unsigned int modifyTime;	    // 文件最近一次修改时间
    unsigned int createTime;	    // 文件创建时间
    unsigned int directAddress[4];  //直接寻址          //4*512B =2KB
    unsigned int singleIndirect;    //一级寻址          //at least 512/4*512=64KB, so it is enough for this fileSystem
    unsigned int doubleIndirect;    //二级寻址
    char fillTo256B[150];			//保证iNode区 iNode连续存放
    
    void initINode(string _mode, string _owner, string _group,int _link,int _size,int _bc,int _vt,int _mt,int _ct,short _id)
    {
        strcpy(mode , _mode.c_str());
        strcpy(owner, _owner.c_str());
        strcpy(group, _group.c_str());
        linkCount=_link;
        size=_size;
        blockCount=_bc;
        visitTime=_vt;
        modifyTime=_mt;
        createTime=_ct;
        inodeID=_id;
        
        for (int i=0;i<4;i++)
        {
            directAddress[i]=0;
        }
        singleIndirect=0;
        doubleIndirect=0;
    }
    
};



struct SuperBlock     //  512B
{
    unsigned int size;                  //the size of the disk
    unsigned short freeInodes[20];      //空闲INode栈
    unsigned short freeBlocks[50];      //空闲block栈
    unsigned short nextFreeBlock;		//the pointer of the next free block in the stack
    unsigned short nextFreeInode;       //the next free inode in the stack
    unsigned short freeInodeNum;		//the totally number of the free inode in the disk
    unsigned short freeBlockNum;		//the totally number of the free block in the disk
    Directory root;						// root directory
    char fillTo256B[342];               //填充至512byte
};




struct MyFileSystem     // 64KB
{
    Block boot;                 // #0
    SuperBlock superBlock;      // #1
    INode inodes[20];           // #2-#11        10 blocks   20 inodes
    Block blocks[116];          // #12-#127     116 blocks
};

























#endif /* dataStructure_h */
