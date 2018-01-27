//
//  main.cpp
//  MyUnixFileSystem
//
//  Created by zoe lee on 17/01/2018.
//  Copyright © 2018 zoelee. All rights reserved.
//

#include "dataStructure.h"
#include "myUtil.h"

#include <time.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
using namespace std;

MyFileSystem myFileSystem;
Directory* currentDirectory;
Directory* root;
User* currentUser;
User* allUsers;
UserTable* userTable;
SuperBlock* superBlock;
DirectoryTable* directoryTable;
INode* allInodes;
Block* allBlocks;

void initSuperBlock(SuperBlock *superb)
{
    
    // superb.root directory
    root=new Directory();
    
    strcpy(root->fileName,"/");
    root->inodeIndex=0;
  
    
    
    
    // superb.attributes
    superb->size=65536;
    superb->freeBlockNum=46;      //last node saves the info of next block list
    superb->freeInodeNum=16;
    superb->nextFreeBlock=45;     // = block #16 ,because blocks[49]=#0, blocks[45]=#4
                                  // stack-like, used from bottom to top
    
    superb->nextFreeInode=4;      // used from top to bottom
    
    superb->freeInodeNum=20-4;   //first one is root directory . 2nd Users directory 3rd 4th 2user dirs
    
    
    //superb.freeinodes
    
    for (int i=0;i<20;i++)
    {
        superb->freeInodes[i]=i;
    }
    
    // superb.freeblock
    for (int i=0,j=49;i<50;i++,j--)
    {
        superb->freeBlocks[j]=i;
    }
    
    superb->root=*root;
    
    
    
}



void initInode(INode *inode)
{
    
    /*
     
     void initINode(string _mode, string _owner, string _group,int _link,int _size,int _bc,int _vt,int _mt,int_ct,short _id)
     
     */
    
    //     1st node     /
    
    int t=int(time(NULL));
    inode[0].initINode("drwxr-xr-x", "root", "wheel", 1, 0, 0, t, t, t, 0);
    inode[0].directAddress[0]=0;
    inode[0].blockCount=1;
    
    
    
    ////it takes 1 block, so set 0

    
    inode[1].initINode("drwxr-xr-x", "root", "admin", 1,0, 0, t, t, t, 1);
    inode[1].directAddress[0]=1;
    inode[1].blockCount=1;
    
    //     #2 node     takes 1 block #2
    
    
    inode[2].initINode("drwxr-x---", "root", "wheel", 1,0, 0, t, t, t, 2);
    inode[2].directAddress[0]=2;
    inode[2].blockCount=1;
    
    //     4th node     sss parent Users
    
    inode[3].initINode("drwxr-xr-x", "sss", "staff", 1,0, 0, t, t, t, 3);
    inode[3].directAddress[0]=3;
    inode[3].blockCount=1;
    
}

void initBlockChain(Block* blocks)
{
    
    for (int i=0;i<=115;i++) //use $ to show it is a free block
    {
        strcpy(blocks[i].block,"$");
    }
    
    
    //also need to init the block info, to make a blockChain
    
    //#49 has info of #50-#99
    //#99 has info of #100-#116
    //#116 has info of -1 ,meaning next is empty
    Block*block49 =&blocks[49];
    char t_str[512];
    strcpy(t_str,"");
    for (int i=0;i<50;i++)
    {
        char t[10];
        strcat(t_str,itoa(i+50, t, 10));
        strcat(t_str,";");
        
    }
    strcpy(block49->block,"$");
    strcat(block49->block,t_str);
    
    
    Block*block99=&blocks[99];
    strcpy(t_str,"");
    for (int i=0;i<=15;i++)
    {
        char t[10];
        strcat(t_str,itoa(i+100, t, 10));
        strcat(t_str,";");
    }
    strcpy(block99->block,"$");
    strcat(block99->block, t_str);
    
    strcpy(blocks[115].block,"$");
    strcat(blocks[115].block,"-1");
    // -1 means no further blocks
    
    
    
    
}

void initFileSystem(MyFileSystem *fileSystem)
{
    
    initSuperBlock(&(fileSystem->superBlock));
    //fileSystem->superBlock=*superblock;
    INode* firstINodes=new INode[4];//first one is root directory . 2nd Users directory 3rd sss default user directory
    initInode(firstINodes);
    fileSystem->inodes[0]=firstINodes[0];
    fileSystem->inodes[1]=firstINodes[1];
    fileSystem->inodes[2]=firstINodes[2];
    fileSystem->inodes[3]=firstINodes[3];
    initBlockChain(fileSystem->blocks);
    
}



/*
 *  istream& read(char *buffer,int len);
 *
 *　 ostream& write(const char * buffer,int len);
 *
 *  infile.read((char*)&stu[i], sizeof(stu[i]));
 */


void saveFileToDisk(char* file, char* filename,int size)
{
    ofstream f;
    f.open(filename,ios::binary);
    f.write(file,size);
    f.close();
}

void readFileFromDisk(char* file,char* filename,int size)
{
    ifstream is(filename,ios::binary);
    if (!is)
    {
        cout<<"file: "<<filename<<" doesn't exist. \n";
    }
    is.read((char*)file,size);
    is.close();
}


void initAll()
{
    
    MyFileSystem* fileSystem=new MyFileSystem;
    initFileSystem(fileSystem);
    DirectoryTable* dt=new DirectoryTable;
    UserTable*userTable=new UserTable;
    
    //init user info
    userTable->count=2;
    User* userRoot=new User[4];
    userRoot[0].initUser("wheel", "root", "root", "0022");
    userTable->user=&userRoot[0];
    
    //the 2nd user sss
    userRoot[1].initUser("staff", "sss", "zoelee", "0022");
    dt->count=4;
    dt->head=root;
    
    // 4 dirs
    /*Directory* dirs=new Directory[20];
    dirs[0]=*root;
    dirs[1].initDirectory(1, "Users");
    dirs[2].initDirectory(2, "root");
    dirs[3].initDirectory(3, "sss");*/
    
    //distribute blocks for every dirs
    //for block 0, .,0;..,0;Users,1;
    //char [14] name: char[2] inode_id
    char* t_str=new char[512];
    strcpy(t_str,".,");
    strcat(t_str,"0;");
    strcat(t_str,"..,");
    strcat(t_str,"0;");
    strcat(t_str,"Users,");
    strcat(t_str,"1;");
    strcpy(fileSystem->blocks[0].block,t_str);
    
    //for block 1, .,1;..,0;root,2;sss,3;
    strcpy(t_str, ".,1;..,0;root,2;sss,3;");
    strcpy(fileSystem->blocks[1].block,t_str);
    
    //for block 2, .,2;..,1;
    strcpy(t_str,".,2;..,1;");
    strcpy(fileSystem->blocks[2].block,t_str);
    
    //for block 3, .,3;..,1;
    strcpy(t_str, ".,3;..,1;");
    strcpy(fileSystem->blocks[3].block, t_str);
    
    
    saveFileToDisk((char*)(fileSystem),(char*)"fileSystem.txt", sizeof(MyFileSystem));
    saveFileToDisk((char*)(userTable), (char*)"userTable.txt", sizeof(UserTable));
    saveFileToDisk((char*)userRoot, (char*)"users.txt", sizeof(User)*4);
    
}


bool isADirectory(Directory* dir)
{
    if (allInodes[dir->inodeIndex].mode[0]=='d')
    {
        return true;
    }
    else return false;
}

bool isANormalFile(Directory* dir)
{
    if (allInodes[dir->inodeIndex].mode[0]=='-')
    {
        return true;
    }
    else return false;
}



char* getContentOfBlocksOfAFile(Directory *curr)
{
    
    INode currInode=allInodes[curr->inodeIndex];
    int size=currInode.blockCount;
    char *allContents=new char[size*512];
    strcpy(allContents, "");
    
    if (size<=4)                    //direct address
    {
        
        for (int i=0;i<size;i++)
        {
            char *str=allBlocks[currInode.directAddress[i]].block;
            strcat(allContents,str);
        }
        
    }
    else if (size>4 && size <=128)         //single indirect address
    {
        unsigned short blockIndex=currInode.singleIndirect;
        char* content=allBlocks[blockIndex].block;
        vector<short> indirectAddress=getBlockItems(content);
        for (int i=0;i<size;i++)
        {
            char *str=allBlocks[indirectAddress.at(i)].block;
            strcat(allContents,str);
        }
    }
    
    
    if (!isADirectory(curr))
    {
        int fileSize=currInode.size;
        return substr(allContents,0,fileSize);
    }
    
    return allContents;
}


bool isParentUsers(Directory *curr)
{
    bool flag=false;
    char *filename=new char[14];
    vector<pair<char *,short>> items=getItems(getContentOfBlocksOfAFile(curr));
    short parentInodeIndex=0;
    for (int i=0;i<items.size();i++)
    {
        if (strcmp(items.at(i).first,"..")==0)
        {
            parentInodeIndex=items.at(i).second;
            if (parentInodeIndex==1)    //Users inode Index=1
            {
                //parent is Users
                return true;
            }
        }
    }
    
    
    return flag;
}

bool isContainsInAVectorDirectory(vector<Directory>* dirs,char *filename,short id)
{
    bool flag=false;
    for (int i=0;i<dirs->size();i++)
    {
        if (strcmp(dirs->at(i).fileName,filename)==0 && id==dirs->at(i).inodeIndex )   //same ,matched
        {
            return true;
        }
    }
    return flag;
}



void reconstructDirTab(vector<Directory> *dirs,Directory* cur)
{

        INode* currInode=&allInodes[cur->inodeIndex];
        if (isADirectory(cur))    //is a directory
        {
            //unsigned short blockIndex=currInode.in
            //cout<<"curr id "<<currInode.inodeID<<"\tmode\t"<<currInode.mode<<endl;
            char *content=getContentOfBlocksOfAFile(cur);
            vector<pair<char *, short>> items=getItems(content);
            
            //cout<<"content "<<content<<endl;
            //printItemsVector(items);
            
            for (int i=0;i<items.size();i++)
            {
                pair<char *,short> item=items.at(i);
                Directory *now=new Directory();
                now->initDirectory(item.second, item.first);
                if (isContainsInAVectorDirectory(dirs, item.first,item.second))
                {
                    //
                }
                else if(strcmp(item.first,".")!=0 && strcmp(item.first, "..")!=0){                      // needs to add it to dirs, and go recursive find
                    dirs->push_back(*now);
                    
                    //cout<<"item added: \t"<<item.first<<"\tid\t"<<item.second<<endl;
                    reconstructDirTab(dirs, now);
                    
                }
            }
            
            // recover links
            currInode->linkCount=(int)items.size();
            
            // recover size
            currInode->size=(int)strlen(content);
            
            
        }
        else {
            //cout<<"not dir "<<cur->fileName<<"\t inode id\t"<<cur->inodeIndex<<endl;
            //dirs->push_back(*cur);
        }
    
}

void printDirTable()
{
    for (int i=0;i<directoryTable->count;i++)
    {
        //cout<<"dir\t"<<directoryTable->head[i].fileName<<endl;
        printItemsVector(getItems(getContentOfBlocksOfAFile(&(directoryTable->head[i]))));
        
        
    }
    
}



void reconstructDirectoryTable()
{
    
    Directory* _currDir=new Directory();
    _currDir->initDirectory(root->inodeIndex,root->fileName);
    int count=0;
    vector<Directory> dirs;
    dirs.push_back(*root);
    reconstructDirTab(&dirs,_currDir);
    count=(int)dirs.size();
    directoryTable->count=count;
    directoryTable->head=new Directory[count];
    
    for (int i=0;i<dirs.size();i++)
    {
        directoryTable->head[i]=dirs.at(i);
    }
    
}






// return the directory* accroding to inode id
Directory* findDirectoryByInodeId(short id)
{
    Directory *dir=nullptr;
    for (int i=0;i<directoryTable->count;i++)
    {
        if (directoryTable->head[i].inodeIndex==id)
        {
            dir=&directoryTable->head[i];
        }
    }
    return dir;
}

Directory* findDirectoryByFilename(Directory*curr ,char* filename)
{
    Directory*dir=nullptr;
    vector<pair<char*,short>> items=getItems(getContentOfBlocksOfAFile(curr));
    for (int i=0;i<items.size();i++)
    {
        pair<char*,short> item=items.at(i);
        if (strcmp(item.first,filename)==0)
        {
            // found the file
            short id=item.second;
            
            dir=findDirectoryByInodeId(id);
        }
    }
    
    
    return dir;
}

bool isDirectoryEmpty(Directory*curr)
{
    vector<pair<char*,short>>items= getItems(getContentOfBlocksOfAFile(curr));
    if (items.size()>=3)
        return false;
    else return true;
}

char *getAbsolutePathTool(Directory *curr,char *path)
{
    
    
    vector<pair<char *,short>>items= getItems(getContentOfBlocksOfAFile(curr));
    while (1)   //curr==parent-> root, stop
    {
        if (getInodesIdsFromItems(&items,".").at(0)!=getInodesIdsFromItems(&items,"..").at(0))
        {
            //parent inode id
            short id=getInodesIdsFromItems(&items,"..").at(0);
            char *filename=findDirectoryByInodeId(id)->fileName;
            //  parent/path
           
            if (strcmp(filename,"/")==0)    // already is root dir, has /
                path=reverseStrcat("", path);
            else
                path=reverseStrcat(filename, path);
            path=getAbsolutePathTool(findDirectoryByInodeId(id),path);
            break;
        }
        else {
            //path=reverseStrcat("",path);
            break;
        }
    }
    //cout<<"path now : "<<path;
    return path;
    
}

char *getAbsolutePath(Directory *curr)
{
    char *path=new char[512];
    char *currF=curr->fileName;
    strcpy(path,currF);
    return getAbsolutePathTool(curr, path);
}


char *getParentDirectoryName(Directory *curr)
{
    char *path=new char[200];
    vector<pair<char *,short>>items= getItems(getContentOfBlocksOfAFile(curr));
    while (1)   //curr==parent-> root, stop
    {
        if (getInodesIdsFromItems(&items,".").at(0)!=getInodesIdsFromItems(&items,"..").at(0))
        {
            short id=getInodesIdsFromItems(&items,"..").at(0);
            char *filename=findDirectoryByInodeId(id)->fileName;
            //cout<<"path now : "<<filename;
            return filename;
        }
        else {
            reverseStrcat(path, "");
            //cout<<"path now : "<<path;
            return path;
        }
    }
    
}

Directory* getParentDirectory(Directory* curr)
{
    Directory* parent;
    vector<pair<char *,short>>items= getItems(getContentOfBlocksOfAFile(curr));
    short id=getInodesIdsFromItems(&items,"..").at(0);
    parent=findDirectoryByInodeId(id);
    return parent;
}



void umask(char* command)
{
    if (strcmp(command,"umask")==0)
    {
        cout<<currentUser->umask<<endl;
        return;
    }
    
    //012345678
    //umask 022     set umask to 0022
    if (strlen(command)==9 && command[5]==' ')      //umasks022  X
    {
        char* subs=substr(command,6,3);
        if (!isConvertableToNum(subs, 3))           //umask 0as  X
        {
            cout<<command<<": Wrong input\n";
            return;
        }
        
        strcpy(currentUser->umask,"0");
        strcat(currentUser->umask,subs);
        
    }else {
        cout<<command<<": Wrong input\n";
    }
    
}


// authorities check below



bool isHasReadAuthority(Directory* dir)
{
    bool isValid=0;
    if (strcmp(currentUser->userName,allInodes[dir->inodeIndex].owner)!=0 )           // not the owner
    {
        if (strcmp(currentUser->userName,"root")==0)     //root has all authroties
        {
            return true;
        }
        
        
        else if (strcmp(currentUser->userGroup,allInodes[dir->inodeIndex].group)==0 )  //if same group , see group authority
        {
            //owner   group   others
            //0123      456     789
            //drwx      r-x     ---
            char* m=allInodes[dir->inodeIndex].mode;
            //cout<<" m4 "<<m[4];
            if (m[4]=='r')
            {
                return true;
            }
            
        }
        else {
            char* m=allInodes[dir->inodeIndex].mode;
            //not same group , see others authority
            if (m[7]=='r')
            {
                return true;
            }else {
                return false;
            }
            
        }
        
    }
    else                //owner
    {
        char* m=allInodes[dir->inodeIndex].mode;
        
        if (m[1]=='r')
        {
            return true;
        }
        else return false;
    }
    return isValid;
}


bool isHasExecuteAuthority(Directory* dir)
{
    bool isValid=0;
    if (strcmp(currentUser->userName,allInodes[dir->inodeIndex].owner)!=0 )           // not the owner
    {
        if (strcmp(currentUser->userName,"root")==0)     //root has all authroties
        {
            return true;
        }
        
        
        else if (strcmp(currentUser->userGroup,allInodes[dir->inodeIndex].group)==0 )  //if same group , see group authority
        {
            //0123 456789
            //drwx r-x---
            char* m=allInodes[dir->inodeIndex].mode;
            //cout<<" m4 "<<m[4];
            if (m[6]=='x')
            {
                return true;
            }
            
        }
        else {
            char* m=allInodes[dir->inodeIndex].mode;
            //not same group , see others authority
            if (m[9]=='x')
            {
                return true;
            }else {
                return false;
            }
            
        }
        
    }
    else                //owner
    {
        char* m=allInodes[dir->inodeIndex].mode;
        
        if (m[3]=='x')
        {
            return true;
        }
        else return false;
    }

    return isValid;
}

bool isHasWriteAuthority(Directory* dir)
{
    bool isValid=0;
    if (strcmp(currentUser->userName,allInodes[dir->inodeIndex].owner)!=0 )           // not the owner
    {
        if (strcmp(currentUser->userName,"root")==0)     //root has all authroties
        {
            return true;
        }
        
        
        else if (strcmp(currentUser->userGroup,allInodes[dir->inodeIndex].group)==0 )  //if same group , see group authority
        {
            //0123456789
            //drwxrwx---
            char* m=allInodes[dir->inodeIndex].mode;
            //cout<<" m4 "<<m[4];
            if (m[5]=='w')
            {
                return true;
            }
            
        }
        else {
            char* m=allInodes[dir->inodeIndex].mode;
            //not same group , see others authority
            if (m[8]=='w')
            {
                return true;
            }else {
                return false;
            }
            
        }
        
    }
    else                //owner
    {
        char* m=allInodes[dir->inodeIndex].mode;
        if (m[2]=='w')
        {
            return true;
        }
        else return false;
    }

    return isValid;
}

bool isHasChmodAuthority(Directory* dir)
{
    // root or file owner has authority
    
    bool isValid=0;
    if (strcmp(currentUser->userName,allInodes[dir->inodeIndex].owner)!=0 )           // not the owner
    {
        if (strcmp(currentUser->userName,"root")==0)     //root has all authroties
        {
            return true;
        }
        else return false;
    }
    
    else  return true;              //owner
    
    return isValid;
}

bool isHasChgrpAuthority(Directory* dir)
{
    return isHasChmodAuthority(dir);
}

bool isHasChownAuthority(Directory* dir)
{
    return isHasChmodAuthority(dir);
}

// above authority check */

void pwd(){
    
    cout<<getAbsolutePath(currentDirectory)<<endl;
}



void ls(char *command){
    
    
    // if user has read authority of a directory, then able to go ahead
    
    if (!isHasReadAuthority(currentDirectory))
    {
        cout<<command<<": Permission denied\n";
        return;
    }
    
    
    int type=0;
    // if ls
    
    if (strcmp(command,"ls")==0)
    {
        type=1;
    }
    // else if ls -l
    else if (strcmp(command,"ls -l")==0)
    {
        type=2;
    }
    // else if ls -a
    else if (strcmp(command,"ls -a")==0)
    {
        type=3;
    }
    // else if ls -al  or   ls -la
    else if (strcmp(command, "ls -al")==0 || strcmp(command, "ls -la")==0)
    {
        type=4;
    }
    else {cout<<command<<": Wrong Input\n";return;}
    
    vector<pair<char *,short>> items=getItems(getContentOfBlocksOfAFile(currentDirectory));
    for (int i=0;i<items.size();i++)
    {
        pair<char *,short> item=items.at(i);
        INode inode=allInodes[item.second];
        char *filename=item.first;
        if (type==1)
        {
            if (strcmp(substr(filename,0,1),".")==0)   //hidden files
            {}
            else                                            //normal files
                cout<<filename<<"\n";
        }
        else if (type==2)
        {
            time_t t=inode.modifyTime;
            if (strcmp(substr(filename,0,1),".")==0)   //hidden files
            {}
            else                                            //normal files
                cout<<inode.mode<<"\t"<<inode.linkCount<<"\t"<<inode.owner<<"\t"<<inode.group<<"\t"<<inode.size<<"\t"<<ctime(&t)<<"\t"<<filename<<endl;
        }
        else if (type==3)
        {
            
            cout<<filename<<"\n";                      //all files
        }
        else if (type==4)
        {
            time_t t=inode.modifyTime;
                                                            //all files
            cout<<inode.mode<<"\t"<<inode.linkCount<<"\t"<<inode.owner<<"\t"<<inode.group<<"\t"<<inode.size<<"\t"<<ctime(&t)<<"\t"<<filename<<endl;
        }

    }

}



Directory* testAvaiableRedirectForCD(char* command)
{
    
    // type 1 cd, absolute
    
    // type 2 cd, relative
    
    // type 3 mv/cp    without cd.
    
    Directory *t=nullptr;
    bool match=true;
    char* p;
    
    // absolute path    cd /..
    int type=0;
    //cout<<"sub"<<substr(command,3,1)<<endl;
    if (strcmp(substr(command,3,1),"/")==0)
    {
        type=1;
    }
    else type=2;
    
    if (type==1)
    {
        t=root;
        p=strtok(substr(command, 4, strlen(command)-4),"/");
        //cout<<"pp \t"<<p<<endl;
        
    }else if (type==2)
    {
        // relative path    cd ../
        t=currentDirectory;
        p=strtok(substr(command, 3, strlen(command)-3),"/");
    }
    else {
        cout<<command<<": Wrong input\n";
        return t;
    }
    
    //cout<<" p1 "<<p;
    
    
    while (p)
    {
        //cout<<"\np "<<p;
        //cout<<"\nt "<<t->fileName;
        if (strcmp("..",p)==0)
        {
            t=getParentDirectory(t);
            p=strtok(NULL, "/");
            continue;
        }
        
        // p is current dirname
        
        vector<pair<char *,short>> items=getItems(getContentOfBlocksOfAFile(t));
        
        for (int i=0;i<items.size();i++)
        {
            pair<char* ,short> item=items.at(i);
            char *filename=item.first;
            Directory* dir=findDirectoryByInodeId(item.second);
            if (strcmp(filename,p)==0)    //same name
            {
                if (isADirectory(dir))
                {
                    
                    if (isHasExecuteAuthority(dir)) //has authority to execute -> cd directory
                    {
                        t=dir;          //cd dir
                        break;
                    }
                    else
                    {
                        cout<<command<<": Permission denied\n";
                        return nullptr;
                    }
          
                }else {
                    cout<<command<<": Not a directory\n";
                    return nullptr;
                }
                break;
 
            }
            else if (i==items.size()-1 )    //last one still not match
            {
                //cout<<"not match "<<filename<<endl;
                match=false;
                cout<<command<<": No such file or directory\n";
                return nullptr;
            }
            
        }
        
        p=strtok(NULL,"/");
    }
    
    return t;
    
    
}



void cd(char *command)
{
    
    // if cd        user directory
    if (strcmp("cd",command)==0)
    {
        //user dir
        
        for (int i=0;i<directoryTable->count;i++)
        {
            // cd /Users/user
            Directory* curr=&directoryTable->head[i];
            if (isADirectory(curr))                     //cause it will crash if it is not a directory ,just a file
            {
                char *parentName=getParentDirectoryName(curr);
                //cout<<"parent "<<parentName<<endl;
                if (strcmp(parentName,"Users")==0 && strcmp(curr->fileName, currentUser->userName)==0)
                {
                    currentDirectory=&(directoryTable->head[i]);
                }
            }
        }
        
    }
    else if (strcmp("cd ..",command)==0)
    {
        currentDirectory=getParentDirectory(currentDirectory);
        //cout<<" new d: "<<currentDirectory->fileName<<endl;
    }else if (strcmp("cd .",command)==0)
    {
        ;
    }
    else if (strcmp("cd /", command)==0)
    {
        currentDirectory=root;
    }// if cd /....  absolute directory
    
    else if (strcmp(substr(command,0,4),"cd /")==0 && strlen(command)>=5)
    {
        
        Directory* d=testAvaiableRedirectForCD(command);
        if (d!=nullptr )
        {
            currentDirectory=d;
        }
        
        
        // if   cd ....   relative directory
    }
    else if (strcmp(substr(command,0,3),"cd ")==0 && strlen(command)>=4 && strcmp(substr(command,2,2),"  ")!=0)
    {
        
        Directory* d=testAvaiableRedirectForCD(command);
        if (d!=nullptr )
        {
            currentDirectory=d;
        }
        
    }else
    {
        cout<<command<<": Wrong input\n";
    }

}

void clear()
{
    system("clear");
}

void printDirectoryTable()
{
    for (int i=0;i<directoryTable->count;i++)
    {
        cout<<i<<"\t"<<directoryTable->head[i].fileName<<endl;
    }
    
}


//locate file or dir
/*
void locate(char *command)
{
    printDirectoryTable();
    char* filename=substr(command,7,strlen(command)-7);
    for (int i=0;i<20;i++)
    {
        
        if (allInodes[i].createTime!=0)
        {
            // see if the inode is used, if already used go on.
            Directory* dir=findDirectoryByInodeId(i);
            //cout<<dir->inodeIndex<<endl;
            
            if (strcmp(dir->fileName,filename)==0)  //found
            {
                if (isADirectory(dir))
                {
                    cout<<getAbsolutePath(dir)<<endl;
                }
                else
                {
                    Directory*parent=getParentDirectory(dir);
                    char *path=getAbsolutePath(parent);
                    strcat(path, "/");
                    strcat(path,dir->fileName);
                    cout<<path<<endl;
                }
            }

        }
    }
}*/


void locateTool(vector<Directory> *dirs,Directory* cur,char *filename)
{
    
    INode currInode=allInodes[cur->inodeIndex];
    if (isADirectory(cur))    //is a directory
    {
        //unsigned short blockIndex=currInode.in
        //cout<<"curr id "<<currInode.inodeID<<"\tmode\t"<<currInode.mode<<endl;
        char *content=getContentOfBlocksOfAFile(cur);
        vector<pair<char *, short>> items=getItems(content);
        
        //cout<<"content "<<content<<endl;
        //printItemsVector(items);
        
        for (int i=0;i<items.size();i++)
        {
            pair<char *,short> item=items.at(i);
            Directory *now=new Directory();
            now->initDirectory(item.second, item.first);
            if (isContainsInAVectorDirectory(dirs, item.first,item.second))
            {
                //
            }
            else if(strcmp(item.first,".")!=0 && strcmp(item.first, "..")!=0){   // needs to add it to dirs, and go recursive find
                
                if (strcmp(item.first,filename)==0)
                {
                    // found file
                    Directory*fileDir=findDirectoryByFilename(cur, filename);
                    if (isADirectory(fileDir))
                    {
                        cout<<getAbsolutePath(fileDir)<<endl;
                    }
                    else {
                        // not a directory , just a file. so it has no parent.. then need to use curr.path+/filename
                        char *path=getAbsolutePath(cur);
                        strcat(path, "/");
                        strcat(path,filename);
                        cout<<path<<endl;
                    }
                    
                }
                dirs->push_back(*now);
                
                //cout<<"item added: \t"<<item.first<<"\tid\t"<<item.second<<endl;
                locateTool(dirs, now,filename);
                
            }
        }
    }
    else {
        //cout<<"not dir "<<cur->fileName<<"\t inode id\t"<<cur->inodeIndex<<endl;
        //dirs->push_back(*cur);
    }
    
}


void locate(char *command)
{
    char* filename=substr(command,7,strlen(command)-7);
    
    Directory* _currDir=new Directory();
    _currDir->initDirectory(root->inodeIndex,root->fileName);
    int count=0;
    vector<Directory> dirs;
    dirs.push_back(*root);
    locateTool(&dirs,_currDir,filename);
    
}

/** update version  **/


bool isExistsSameFilename(Directory *dir,const char* filename)
{
    vector<pair<char *,short>> items=getItems(getContentOfBlocksOfAFile(dir));
    for (int i=0;i<items.size();i++)
    {
        // doesnt allow same filename in the same folder
        char *existedFilename=items.at(i).first;
        if (  strcmp(existedFilename,filename)==0)
        {
            return true;
        }
    }
    
    return false;
}

short getAvailableBlockSize()
{
    short size=0;
    for (int i=0;i<=127-12;i++)
    {
        
        if (strcmp(substr(allBlocks[i].block,0,1),"$")==0) //free
        {
            size++;
        }
    }
    
    return size;
}

short getAvailableInodeSize()
{
    short size=0;
    for (int i=0;i<20;i++)
    {
        if (allInodes[i].modifyTime!=0)
        {
            size++;
        }
    }
    return size;
}

// return pyhiscal address

//superBlock->nextFreeBlock    ->    array address
//array address                ->    array content
//array content                ->    physical address

vector<short> allocateBlock(int size)
{
    vector<short> blocks;
    int count=0;
    
    // size <= all available blocks size
    if (getAvailableBlockSize()<size)
    {
        return blocks;
    }
    if (getAvailableInodeSize()<1)
    {
        return blocks;
    }
    
    while (count<size)
    {
    if (superBlock->freeBlockNum>1) //if >1,it means it has more free blocks in the stack
    {
        //pop a block and use it
        blocks.push_back(superBlock->freeBlocks[superBlock->nextFreeBlock]);
        superBlock->nextFreeBlock--;
        superBlock->freeBlockNum--;
        //allocated one block
        count++;
    }
    else if (superBlock->freeBlockNum==1){   //if ==1, it means we need to reload other blocks to stack
        //fetch content of next blocks
        Block* infoBlock=&allBlocks[superBlock->freeBlocks[superBlock->nextFreeBlock]];
        short blockIndexPhysical=superBlock->freeBlocks[superBlock->nextFreeBlock];
        vector<short> nextBlocks=getBlockItems(allBlocks[superBlock->freeBlocks[superBlock->nextFreeBlock]].block);
        
        //adjust superBlock's stack, stack pointer , freeBlocksNum
        
        
        for (int i=0;i<nextBlocks.size();i++)
        {
            superBlock->freeBlocks[nextBlocks.size()-1-i]=nextBlocks.at(i);
        }
        superBlock->freeBlockNum=nextBlocks.size();
        
         // also need to empty this block. and this block is also free
        //strcpy(infoBlock->block,"$");
        
        if (nextBlocks.size()==0)
            superBlock->nextFreeBlock=0;
        else
        {
            
            //pop the block and use it
            blocks.push_back(blockIndexPhysical);
            superBlock->nextFreeBlock--;
            
            //allocated one block
            count++;
            
            superBlock->nextFreeBlock=nextBlocks.size()-1;
            
            
        }
        
        continue;
        
    }
    
   
    }
    /*for (int i=0;i<blocks.size();i++)
    {
        cout<<"n\t"<<blocks.at(i);
    }*/
    
    
    return blocks;
}

void deleteFileFromParentBlock(Directory*parent, char *filename)
{
    vector<pair<char *,short>>items=getItems(getContentOfBlocksOfAFile(parent));
    int size=items.size();
    for (int i=0;i<size;i++)
    {
        pair<char *,short> item=items.at(i);
        if (strcmp(item.first,filename)==0)
        {
            items.erase(items.begin()+i);
            char *content=putVectorItemsToCharStar(&items);
            Block* block=&allBlocks[allInodes[parent->inodeIndex].directAddress[0]];
            //cout<<"content:\t"<<content<<endl;
            strcpy(block->block,content);
            return;
        }
    }

    // also need to update the modify time
    
    int currTime=int(time(NULL));
    INode* inode=&allInodes[parent->inodeIndex];
    inode->modifyTime=currTime;
    

    
}

bool deallocateBlock(INode *inode)
{
    
    int size=inode->blockCount;
    int count=0;
    vector<short> blockAddressVector;
    if (size>4)                     //indirect address
    {
        //dont push 4 direct address cause save all address in singleAddress
        /*blockAddressVector.push_back(inode->directAddress[0]);
        blockAddressVector.push_back(inode->directAddress[1]);
        blockAddressVector.push_back(inode->directAddress[2]);
        blockAddressVector.push_back(inode->directAddress[3]);*/
        
        int indirectBlockAddress=inode->singleIndirect;
        char *content=allBlocks[indirectBlockAddress].block;
        vector<short> blockItems=getBlockItems(content);
        vectorAppend(&blockAddressVector,&blockItems);
        
        // but dont forget to deallocate the block that contains the info of address
        blockAddressVector.push_back(inode->singleIndirect);
        size++;
        
    }
    else {
        for (int i=0;i<size;i++)
        {
            blockAddressVector.push_back(inode->directAddress[i]);
        }
    }
    
//    cout<<"\n\n-------\n\n";
//    printVector(&blockAddressVector, blockAddressVector.size());
//    cout<<"\n\n-------\n\n";
    
    while (count<size)
    {
        int blockIndex=blockAddressVector.at(count);
        Block* block=&allBlocks[blockIndex];
        
        if (superBlock->freeBlockNum<50)    //  if <50, means you can simply push the block to stack
        {
            // adjust superBlock's stack(push new block), stack pointer(pointer++), freeBlockNum++
            superBlock->nextFreeBlock++;
            superBlock->freeBlocks[superBlock->nextFreeBlock]=blockIndex;
            superBlock->freeBlockNum++;
            
            strcpy(allBlocks[blockIndex].block, "$");   //set to free state
            
            count++;
        }
        else if (superBlock->freeBlockNum==50)
        {
//            cout<<"index=0: \t"<<allBlocks[superBlock->freeBlocks[49]].block<<endl;
            
            // if ==50 ,means we need to save the info of free 50 blocks to the new block and push new block to stack
            // adjust superBlock's stack (new block only), stack pointer (point to new block), freeBlockNum( =1)
            char *str=new char[512];
            strcpy(str, "");
            
            // to make 1;2;3;... to record the info of physical address of free blocks
            for (int i=0;i<50;i++)
            {
                char t[10];
                
                strcat(str, itoa(superBlock->freeBlocks[50-i-1], t, 10));
                strcat(str, ";");
            }
//            cout<<"block1 "<<blockIndex<<endl;
            strcpy(block->block,"$");
            // means empty
            strcat(block->block, str);
//            cout<<" str : "<<str<<endl;
 //           cout<<"block2 "<<block->block<<endl;
            
            // it is free with info
            count++;
            
            superBlock->freeBlocks[0]=blockIndex;
            superBlock->nextFreeBlock=0;
            superBlock->freeBlockNum=1;
            
            continue;
        }
        else return false;
    }
    
    return true;
}


void top()
{
    // superblock->inodes info
    char *freeInodeInfo=new char[200];
    char *busyInodeInfo=new char[200];
    char *freeBlockInfo=new char[500];
    char *busyBlockInfo=new char[500];
    strcpy(freeInodeInfo,"");
    strcpy(busyInodeInfo, "");
    strcpy(freeBlockInfo, "");
    strcpy(busyBlockInfo, "");
    
    //inodes info
    for (int i=0;i<20;i++)
    {
        if (allInodes[i].modifyTime!=0)        //busy
        {
            char t[10];
            itoa(i,t,10);
            strcat(busyInodeInfo,t);
            strcat(busyInodeInfo, ", ");
        }else {                                 //free
            char t[10];
            itoa(i,t,10);
            strcat(freeInodeInfo,t);
            strcat(freeInodeInfo, ", ");
        }
        
    }
    
    //blocks info
    
    for (int i=0;i<=127-12;i++)
    {
//        cout<<"i "<<i<<"\t"<<substr(allBlocks[i].block,0,strlen(allBlocks[i].block))<<endl;
        if (strcmp(substr(allBlocks[i].block,0,1),"$")==0) //free
        {
            char t[10];
            itoa(i,t,10);
            strcat(freeBlockInfo,t);
            strcat(freeBlockInfo, ", ");

        }
        else {                                //busy
         
            char t[10];
            itoa(i,t,10);
            strcat(busyBlockInfo,t);
            strcat(busyBlockInfo, ", ");
        }
    }
    // print info
    cout<<"Free Inodes Info:\n"<<substr(freeInodeInfo,0,strlen(freeInodeInfo)-2)<<endl;
    cout<<"Busy Inodes Info:\n"<<substr(busyInodeInfo,0,strlen(busyInodeInfo)-2)<<endl;
    cout<<"Free Blocks Info:\n"<<substr(freeBlockInfo,0,strlen(freeBlockInfo)-2)<<endl;
    cout<<"Busy Blocks Info:\n"<<substr(busyBlockInfo,0,strlen(busyBlockInfo)-2)<<endl;
    
    delete[] freeInodeInfo;
    delete[] busyInodeInfo;
    delete[] freeBlockInfo;
    delete[] busyBlockInfo;
    
}

void addContentToDirectoryBlock(Directory* dir, char *filename,short id)
{
    
    // filename,id;   add to block
    char t[512*20];
    char t_str[10];
    strcpy(t,filename);
    strcat(t, ",");
    strcat(t, itoa(id, t_str, 10));
    strcat(t, ";");
    Block* block=&allBlocks[allInodes[dir->inodeIndex].directAddress[0]];
    strcat(block->block, t);
    
    // also need to update the modify time
    
    int currTime=int(time(NULL));
    INode* inode=&allInodes[dir->inodeIndex];
    inode->modifyTime=currTime;
    
    
}


bool isSafeToAllocate(int blockSize,int inodeSize=0)
{
    //blocks info
    
    int freeBlockSize=0,freeInodeSize=superBlock->freeInodeNum;
    for (int i=0;i<=127-12;i++)
    {
        if (strcmp(substr(allBlocks[i].block,0,1),"$")==0) //free
        {
            freeBlockSize++;
        }
    }
    
    //if block size >4 means need another block
    if (blockSize>4)    blockSize++;
    
    if (freeBlockSize>=blockSize && freeInodeSize>=inodeSize)
    {
        return true;
    }
    else return false;
    
}


short allocateInode()
{
    short id=-1;
    
    if (superBlock->freeInodeNum<=0)
    {
        
        return -1;
    }
    //id=superBlock->nextFreeInode;
    //superBlock->nextFreeInode++;
    for (int i=0;i<20;i++)
    {
        if (allInodes[i].linkCount==0)
        {
            id=i;
            break;
        }
    }
    
    superBlock->freeInodeNum--;
    
    
    return id;
}



void deallocateInode(INode* inode)
{
    
    inode->initINode("", "", "", 0, 0, 0, 0, 0, 0, inode->inodeID);
    for (int i=0;i<4;i++)
        inode->directAddress[i]=0;
    inode->singleIndirect=0;
    
    // set up superBlock's inode stack
    superBlock->freeInodeNum++;
    //superBlock->nextFreeInode--;
    //superBlock->freeInodes[superBlock->nextFreeInode]=inode->inodeID;
    
    
}



void mkdir(char *command)
{
    // problem 0. filename out of boundary problem 1 . same filename in same dir. problem 2 no write authroity . problem 3 no inode space. problem 4 no block space.
    
    // procedure 1. allocate block space. procedure 2. allocate inode space. procedure 3. set up inode info. procedure 4. add .,.. to block.
    // procedure 5. add filename to parent.block
    
    
    //solve p0:
    char* filename=substr(command, 6, strlen(command)-6);
    if (strlen(filename)>=14)
    {
        cout<<command<<": File name too long\n";
        return;
    }
        
    //solve p1:
    if (strcmp(substr(command,5,2),"  ")==0 &&strlen(command)>=7)
    {
        cout<<command<<": Wrong input\n";
        return;
    }else {
        
        //cout<<"cmd "<<command<<endl;
        
       

        if (isExistsSameFilename(currentDirectory, filename))
        {
            // p1 solved
            cout<<command<<": File exists\n";
            return;
            
            
        }
        
        //solve p2
        
        if (!isHasWriteAuthority(currentDirectory))
        {
            cout<<command<<": Permission denied\n";
            return;
        }
        // p2 solved
        
        
        if (!isSafeToAllocate(1,1))
        {
            cout<<command<<": Not enough space in volume!\n";
            return;
        }
        
        // procedure 1
        vector<short> blocks= allocateBlock(1);
        
        
            //problem 3 and 4 solved here.
        
        
        // !!!!!        block 7 .,9;..,3;  block 9  .,9;..,3;
        
        
        
        //above has already allocated space of block
        //now we need to allocate an inode  (pyhsical address of blocks in `blocks`)
        
        // procedure 2
        /*unsigned short nexti=superBlock->freeInodes[superBlock->nextFreeInode];
        INode* newInode=&(allInodes[nexti]);
        superBlock->nextFreeInode++;
        superBlock->freeInodeNum--;*/
        
        
        unsigned short nexti=allocateInode();
        
        
        
        INode* newInode=&(allInodes[nexti]);

        // procedure 3
        char* mode=new char[12];
        strcpy(mode,"d");
        char* umask=currentUser->umask;
        setDirectoryModeByUmask(umask, mode);
        
        int t=int(time(NULL));
        newInode->initINode(mode, currentUser->userName, currentUser->userGroup, 1, 0, 1, t, t, t, nexti);
        newInode->directAddress[0]=blocks.at(0);
        
        
        //procedure 4
        
        // .,4;..,3; add to new block
        
        Block* newBlock=&(allBlocks[blocks.at(0)]);
        char *blockContent=new char[100];
        strcpy(blockContent,".,");
        char t_str[10];
        strcat(blockContent,itoa(newInode->inodeID, t_str, 10));
        strcat(blockContent,";..,");
        strcat(blockContent, itoa(currentDirectory->inodeIndex, t_str, 10));
        strcat(blockContent, ";");
        strcpy(newBlock->block,blockContent);
        
        //procedure 5
        // .,3;..,1;filename,4;  add to parent block
        
        addContentToDirectoryBlock(currentDirectory, filename, nexti);

        //dont forget to reconstruct directory table
        reconstructDirectoryTable();
        
    }
}

void rmdir(char *command)
{
    // problem 1, check if file exists. Problem2, check if it is a directory. problem 3, check if user has write authroity.
    // problem 4, check if directory has children files (not empty)
    
    
    // procedure 1,deallocate this block. procedure 2,deallocate this inode. procedure 3, update parent's block content (delete filename,id;)
    // procedure 4,reconstruct directory table.
    
    
    char* filename=substr(command, 6, strlen(command)-6);
    if (!isExistsSameFilename(currentDirectory, filename))
    {
        // p1 solved
        cout<<command<<": No such file or directory\n";
        return;
    }
    
    Directory* dir=findDirectoryByFilename(currentDirectory,filename);
    if (!isADirectory(dir))
    {
        cout<<command<<": Not a directory\n";
        return;
    }
    
    // needs the w authority of the file to be deleted and w authority of current directory
    
    if (!isHasWriteAuthority(dir) || !isHasWriteAuthority(currentDirectory))
    {
        cout<<command<<": Permission denied\n";
        return;
    }
    if (!isDirectoryEmpty(dir))
    {
        cout<<command<<": Directory not empty\n";
        return;
    }
    
    //problems solved. then go on procedures
    
    //before p1, we need to save the parent directory
    Directory*parent = getParentDirectory(dir);
    //procedure 1
    INode* inode=&allInodes[dir->inodeIndex];
    deallocateBlock(inode);
    //procedure 2
    /*inode->initINode("", "", "", 0, 0, 0, 0, 0, 0, inode->inodeID);
    inode->directAddress[0]=0;
     
    // set up superBlock's inode stack
    superBlock->freeInodeNum++;
    superBlock->nextFreeInode--;
    superBlock->freeInodes[superBlock->nextFreeInode]=inode->inodeID;*/
    deallocateInode(inode);
    
    //procedure 3
    deleteFileFromParentBlock(parent, filename);
    
    //procedure 4
    reconstructDirectoryTable();
    
}

string getInputContent()
{
    string str="";
    
    string t="";
    int first=1;
    
    while (t!=":wq")
    {
        if (first!=1)
            str=str+"\n";
        else first=0;
        
        getline(cin,t);
        if (t!=":wq")
        {
            str=str+t;
            
            //cout<<"\n\n"<<t<<"\n"<<endl;
        }
        if (t==":q")
        {
            str=t;
            return str;
        }
    }
    str=str+"\0";
    
    //cout<<"\n\n------\n"<<str<<"\n----\n\n";
    return str;
}

void assignContentToBlocks(const char *content,vector<short> blocks)
{
    int size=blocks.size();
    for (int i=0;i<size;i++)
    {
        char* parti=substr(content, 511*i, 511);
        short blockIndex=blocks.at(i);
        strcpy(allBlocks[blockIndex].block,parti);
    }
    
}

//change inode attribute accroding to new blocks allocated
void assignBlocksToInode(vector<short>* blocks,INode* inode )
{
    inode->blockCount=blocks->size();
    Block* oldBlock=&allBlocks[inode->directAddress[0]];
    short oldBlockIndex=inode->directAddress[0];
    if (blocks->size()>4)                     //indirect address
    {
        //push 4 direct address
        inode->directAddress[0]=blocks->at(0);
        inode->directAddress[1]=blocks->at(1);
        inode->directAddress[2]=blocks->at(2);
        inode->directAddress[3]=blocks->at(3);
        
        //old block as address storage to store new blocks' addresses
        inode->singleIndirect=oldBlockIndex;
        
        //format : 2;54;32;75;
        char *formatedContent=putBlockVectorItemsToCharStar(blocks);
        //save to its content
        //format : +2;54;32;75;
        strcpy(oldBlock->block,"+");
        strcat(oldBlock->block,formatedContent);
    }
    else {
        for (int i=0;i<blocks->size();i++)
        {
            inode->directAddress[i]=blocks->at(i);
        }
    }
    
    //change modify time
    int t=int(time(NULL));
    inode->modifyTime=t;
    
    
    
}
/*
short allocateInode()
{
    short id;
    
    if (superBlock->freeInodeNum<=0)
    {
        
        return -1;
    }
    id=superBlock->nextFreeInode;
    superBlock->nextFreeInode++;
    superBlock->freeInodeNum--;
    
    
    return id;
}

void deallocateInode(INode* inode)
{
    
    inode->initINode("", "", "", 0, 0, 0, 0, 0, 0, inode->inodeID);
    for (int i=0;i<4;i++)
        inode->directAddress[i]=0;
    inode->singleIndirect=0;
    
    // set up superBlock's inode stack
    superBlock->freeInodeNum++;
    superBlock->nextFreeInode--;
    superBlock->freeInodes[superBlock->nextFreeInode]=inode->inodeID;
    
    
}
*/


void vi(char *command)
{
    
    //problem1. check if filename exists && is a normal file && haswrite authority.
    //if exists ,then need to change the file
    
    // if filename doesnt exist ,then create a normal file. allocate inode. allocate blocks probably need to give it more blocks .
    //      dont forget to add to parent directory block
    
    char *filename=substr(command, 3, strlen(command)-3);
    
    // if filename is too long
    if (strlen(filename)>=14)
    {
        cout<<command<<": File name too long\n";
        return;
    }
    
    if (isExistsSameFilename(currentDirectory, filename) )
    {
        //authroity to write the file ..                            ( dont need the authority to write & read the directory )
        if (!isHasWriteAuthority(findDirectoryByFilename(currentDirectory, filename)) || !isHasReadAuthority(findDirectoryByFilename(currentDirectory, filename)))
        {
            cout<<command<<": Permission denied\n";
            return;
        }
        
        
        
        if (isANormalFile(findDirectoryByFilename(currentDirectory, filename)))
        {
            // is a normal file,then go ahead
            cout<<"Input Content:\t(enter :wq save and quit. enter :q to quit (it won't save) )\n";
            string content=getInputContent();
            if (content==":q")
            {
                return;
            }
            
            int size=content.size();
            int blockSize=size/511+1;
            
            INode*inode= &allInodes[findDirectoryByFilename(currentDirectory, filename)->inodeIndex];
            
            // allocate blocks space
            vector<short> blocks= allocateBlock(blockSize);
            
            if (blocks.size()==0)
            {
              
                cout<<command<<": Not enough space in volume!\n";
                return;
            }
            //deallocate old block if blockSize<=4
            if (blockSize<=4)
                deallocateBlock(inode);
            else
            {
                deallocateBlock(inode);
                vector<short>blockToSaveAddress= allocateBlock(1);
                inode->blockCount=1;
                inode->directAddress[0]=blockToSaveAddress.at(0);
            }
            // else it will save the address of other blocks.
            
            
            // content add to new blocks

            const char *char_content=content.c_str();
            assignContentToBlocks(char_content, blocks);
            
            //change inode attributes including size and modify time
            assignBlocksToInode(&blocks,inode);
            inode->size=size;

            // also need to update the modify time

            int currTime=int(time(NULL));
            inode->modifyTime=currTime;
            

            
            return;
            
        }
        
        // not a normal file, return;
        cout<<command<<": Is not a normal file\n";
        return;
    }
    

    
    // file doesnt exist , need to create a file
    
    // the authority to write the directory
    if ( !isHasWriteAuthority(currentDirectory))
    {
        cout<<command<<": Permission denied\n";
        return;
    }
    
    
    //get content
    cout<<"Input Content:\t(enter :wq save and quit. enter :q to quit (it won't save) )\n";
    string content=getInputContent();
    
    if (content==":q")
    {
        return;
    }
    
    int size=content.size();
    int blockSize=size/511+1;
    
    
    //before allocation ,check if it has enough space
    if (!isSafeToAllocate(blockSize,1))
    {
        cout<<command<<": Not enough space in volume!\n";
        return;
    }
    
    
    //allocate an inode
    short inodeIndex=allocateInode();
    if (inodeIndex==-1)
    {
        cout<<command<<": Not enough space\n";
        return;
    }
    
    
    // allocate blocks space
    vector<short> blocks= allocateBlock(blockSize);
    if (blocks.size()==0)
    {
    
        cout<<command<<": Not enough space in volume!\n";
        return;
    }

    //inode *
    INode* newInode=&allInodes[inodeIndex];
    
    
    //cout<<"content size is "<<size<<endl;
    
    //cout<<"allocate inode id "<<inodeIndex<<endl;
    
    
    
    //set mode by umask
    char* mode=new char[12];
    strcpy(mode,"-");
    setFileModeByUmask(currentUser->umask, mode);
    
    //init inode
    int t=int(time(NULL));
    newInode->initINode(mode, currentUser->userName, currentUser->userGroup, 1, size, blockSize, t, t, t, inodeIndex);
    
    // notice that if block size >4 , need to use another block to save address!!!
    if (blockSize>4)
    {
        vector<short> saveAddressBlock=allocateBlock(1);
        short address=saveAddressBlock.at(0);
        newInode->singleIndirect=address;
        newInode->directAddress[0]=address;
        
        //cout<<"indirect address saved in block id "<<saveAddressBlock.at(0)<<endl;
    }
    
    

    const char *char_content=content.c_str();
    //save content to blocks
    assignContentToBlocks(char_content, blocks);
    
    //cout<<"allocate blocks ids (real phsical address )\n";
    //printVector(&blocks, blocks.size());
    
    //cout<<endl;
    //change inode attributes
    assignBlocksToInode(&blocks,newInode);
    
    //dont forget to add this new info to parent directory
    addContentToDirectoryBlock(currentDirectory, filename, inodeIndex);
    
    //reconstruct directory table
    reconstructDirectoryTable();
    
}

void rm(char *command)
{
    
    char *filename=substr(command, 3, strlen(command)-3);
    
    if (!isExistsSameFilename(currentDirectory, filename))
    {
        cout<<command<<": File not exist\n";
        return;
    }
    
    if (isADirectory(findDirectoryByFilename(currentDirectory, filename)))
    {
        cout<<command<<": Is a directory\n";
        return;
    }
    
    //authroity to write the file .. and the authority to write the directory
    if (!isHasWriteAuthority(findDirectoryByFilename(currentDirectory, filename)) || !isHasWriteAuthority(currentDirectory))
    {
        cout<<command<<": Permission denied\n";
        return;
    }
    
    Directory* curr=findDirectoryByFilename(currentDirectory, filename);

    INode* inode=&allInodes[curr->inodeIndex];
    
    // if linkCount=1 , then able to go ,otherwise will not delete inode or blocks only delete filename.
    
    if (inode->linkCount>1)
    {
        inode->linkCount--;
        // delete the filename
        deleteFileFromParentBlock(currentDirectory, filename);
        reconstructDirectoryTable();
        return;
    }
    
    
    //deallocate blocks associated with inode
    deallocateBlock(inode);
    
    //deallocate inode
    deallocateInode(inode);
    
    
    // delete info from parent
    deleteFileFromParentBlock(currentDirectory, filename);
    
    //dont forget to refresh diectory table
    reconstructDirectoryTable();
    
    
    
}

void cat(char *command)
{
    // output the content of a file
    // check if a file/dir exists, check if it isnot a dir, check if user has read authroity, output content
    
    char *filename=substr(command, 4, strlen(command)-4);
    
    if (!isExistsSameFilename(currentDirectory, filename))
    {
        cout<<command<<": File not exist\n";
        return;
    }
    
    if (isADirectory(findDirectoryByFilename(currentDirectory, filename)))
    {
        cout<<command<<": Is a directory\n";
        return;
    }
    
    if (!isHasReadAuthority(findDirectoryByFilename(currentDirectory, filename)))
    {
        cout<<command<<": Permission denied\n";
        return;
    }
    Directory* curr=findDirectoryByFilename(currentDirectory, filename);
    
    char *content=getContentOfBlocksOfAFile(curr);
    
    
    cout<<content<<endl;
    
}


void passwd()
{
    
    cout<<"Changing password for "<<currentUser->userName<<".\n";
    cout<<"Old Password:";
    
    string oldPassword;
    string newPassword;
    string retypePassword;
    getline(cin, oldPassword);
    cout<<"\nNew Password:";
    getline(cin,  newPassword);
    cout<<"\nRetype New Password:";
    getline(cin,  retypePassword);
    
    
    // wrong old password
    if (strcmp(oldPassword.c_str(),currentUser->userPassword)!=0)
    {
        cout<<"\npasswd: authentication token failure\n";
        return;
    }
    
    // old password is right, but two new passwords dont match
    if (newPassword!=retypePassword)
    {
        cout<<"\npasswd: try again\n";
        return;
    }
    
    // old password is right and two new password match. too long tho
    
    if (newPassword.length()>=20)
    {
        cout<<"\npasswd: password too long\n";
        return;
    }
    
    // old password is right and two new password match : change password now!
    strcpy(currentUser->userPassword,newPassword.c_str());
    
    
    
}


void mv(char *command)
{
    
    char *files=substr(command, 3, strlen(command)-3);
    strcat(files," ");

    vector<char *> fileVector=split(files,' ');

    //printVector(&fileVector, fileVector.size());
    if (fileVector.size()!=2)
    {
        //input format is wrong
        cout<<command<<": Wrong input\n";
        return;
    }
    
    char* oldFilename=fileVector.at(0);
    char* newFilename=fileVector.at(1);
    
    if (!isExistsSameFilename(currentDirectory, oldFilename))
    {
        cout<<command<<": Old file not exist\n";
        return;
    }
    
    if (isExistsSameFilename(currentDirectory, newFilename))
    {
        cout<<command<<": New filename exist\n";
        return;
    }
    
    
    /*if (isADirectory(findDirectoryByFilename(currentDirectory, oldFilename)))
    {
        cout<<command<<": Is a directory\n";
        return;
    }*/
    // needs to have w authority of current Directory and w authority of the file itself
    if (!isHasWriteAuthority(findDirectoryByFilename(currentDirectory, oldFilename)) || !isHasWriteAuthority(currentDirectory))
    {
        cout<<command<<": Permission denied\n";
        return;
    }
    
    if (strlen(newFilename)>=14)
    {
        cout<<command<<": File name too long\n";
        return;
    }
    
    Directory* curr=findDirectoryByFilename(currentDirectory, oldFilename);
    
    strcpy(curr->fileName,newFilename);
    
    // change the filename and change corresponding info of directory
    deleteFileFromParentBlock(currentDirectory, oldFilename);
    addContentToDirectoryBlock(currentDirectory, newFilename, curr->inodeIndex);
    
}

void cp(char *command)
{
    
    char *files=substr(command, 3, strlen(command)-3);
    strcat(files," ");
    
    vector<char *> fileVector=split(files,' ');
    
    //printVector(&fileVector, fileVector.size());
    if (fileVector.size()!=2)
    {
        //input format is wrong
        cout<<command<<": Wrong input\n";
        return;
    }
    
    char* oldFilename=fileVector.at(0);
    char* newFilename=fileVector.at(1);
    
    if (!isExistsSameFilename(currentDirectory, oldFilename))
    {
        cout<<command<<": Old file not exist\n";
        return;
    }
    
    if (isExistsSameFilename(currentDirectory, newFilename))
    {
        cout<<command<<": New filename exist\n";
        return;
    }
    
    
    if (isADirectory(findDirectoryByFilename(currentDirectory, oldFilename)))
    {
        cout<<command<<": Is a directory\n";
        return;
    }
    // needs to have w authority of current Directory and r authority of the file itself
    if (!isHasReadAuthority(findDirectoryByFilename(currentDirectory, oldFilename)) || !isHasWriteAuthority(currentDirectory))
    {
        cout<<command<<": Permission denied\n";
        return;
    }
    
    if (strlen(newFilename)>=14)
    {
        cout<<command<<": File name too long\n";
        return;
    }
    
    Directory* curr=findDirectoryByFilename(currentDirectory, oldFilename);
    
    
    // need to create a file
    
    //get content
    char *content=getContentOfBlocksOfAFile(curr);
    int size=strlen(content);
    int blockSize=size/511+1;
    
    
    //before allocation ,check if it has enough space
    if (!isSafeToAllocate(blockSize,1))
    {
        cout<<command<<": Not enough space in volume!\n";
        return;
    }
    
    
    //allocate an inode
    short inodeIndex=allocateInode();
    if (inodeIndex==-1)
    {
        cout<<command<<": Not enough space\n";
        return;
    }
    
    
    // allocate blocks space
    vector<short> blocks= allocateBlock(blockSize);
    if (blocks.size()==0)
    {
        
        cout<<command<<": Not enough space in volume!\n";
        return;
    }
    
    //inode *
    INode* newInode=&allInodes[inodeIndex];
    
    
//    cout<<"content size is "<<size<<endl;
    
//    cout<<"allocate inode id "<<inodeIndex<<endl;
    
    
    
    //set mode by umask
    char* mode=new char[12];
    strcpy(mode,"-");
    setFileModeByUmask(currentUser->umask, mode);
    
    //init inode
    int t=int(time(NULL));
    newInode->initINode(mode, currentUser->userName, currentUser->userGroup, 1, size, blockSize, t, t, t, inodeIndex);
    
    // notice that if block size >4 , need to use another block to save address!!!
    if (blockSize>4)
    {
        vector<short> saveAddressBlock=allocateBlock(1);
        short address=saveAddressBlock.at(0);
        newInode->singleIndirect=address;
        newInode->directAddress[0]=address;
        
//        cout<<"indirect address saved in block id "<<saveAddressBlock.at(0)<<endl;
    }
    
    
    
    const char *char_content=content;
    //save content to blocks
    assignContentToBlocks(char_content, blocks);
    
//    cout<<"allocate blocks ids (real phsical address )\n";
//    printVector(&blocks, blocks.size());
    
//    cout<<endl;
    //change inode attributes
    assignBlocksToInode(&blocks,newInode);
    
    //dont forget to add this new info to parent directory
    addContentToDirectoryBlock(currentDirectory, newFilename, inodeIndex);
    
    //reconstruct directory table
    reconstructDirectoryTable();
    
}

vector<short> findAllInodesUsingSameBlocks(INode* inode)
{
    vector<short> allInodesIndexes;
 
    for (int i=0;i<20;i++)
    {
        INode* currInode=&allInodes[i];
        
        int count=0;
        
        //if info of blocks are the same ,then push to the vector
        if (inode->singleIndirect==currInode->singleIndirect)
        {
            count++;
        }
        if (inode->doubleIndirect==currInode->doubleIndirect)
        {
            count++;
        }
        for (int j=0;j<4;j++)
        {
        
            if (inode->directAddress[j]==currInode->directAddress[j])
            {
                count++;
            }
        
        }
        
        if (count==6)
        {
            // match here
            allInodesIndexes.push_back(i);
        }
        
    }
    
    return allInodesIndexes;
}

void linkPlusOne(short inodeIndex)
{
    INode* inode=&allInodes[inodeIndex];
    inode->linkCount++;
}

void allLinksPlusOne(vector<short> inodeIndexes)
{
    
    for (int i=0;i<inodeIndexes.size();i++)
    {
        INode* inode=&allInodes[inodeIndexes.at(i)];
        inode->linkCount++;
    }
    
}

void ln(char *command)
{
    
    
    
    char *files=substr(command, 3, strlen(command)-3);
    strcat(files," ");
    
    vector<char *> fileVector=split(files,' ');
    
    //printVector(&fileVector, fileVector.size());
    if (fileVector.size()!=2)
    {
        //input format is wrong
        cout<<command<<": Wrong input\n";
        return;
    }
    
    char* oldFilename=fileVector.at(0);
    char* newFilename=fileVector.at(1);
    
    if (!isExistsSameFilename(currentDirectory, oldFilename))
    {
        cout<<command<<": Old file not exist\n";
        return;
    }
    
    if (isExistsSameFilename(currentDirectory, newFilename))
    {
        cout<<command<<": New filename exist\n";
        return;
    }
    
    if (isADirectory(findDirectoryByFilename(currentDirectory, oldFilename)))
    {
        cout<<command<<": Is a directory\n";
        return;
    }
    // needs to have w authority of current Directory and r authority of the file itself
    if (!isHasReadAuthority(findDirectoryByFilename(currentDirectory, oldFilename)) || !isHasWriteAuthority(currentDirectory))
    {
        cout<<command<<": Permission denied\n";
        return;
    }
    
    if (strlen(newFilename)>=14)
    {
        cout<<command<<": File name too long\n";
        return;
    }
    
    // if it is a hard link ,which means the one inode with several filenames
    
    Directory* curr=findDirectoryByFilename(currentDirectory, oldFilename);
    
    // so add the new filename to directory's block
    addContentToDirectoryBlock(currentDirectory, newFilename, curr->inodeIndex);
    
    reconstructDirectoryTable();
    
    // also need to increase the link number
    
    allInodes[curr->inodeIndex].linkCount++;
    
    
}

void chmod(char *command)
{
    // chmod u+x filename, u-r, a-w,o+w ..
    
    char *info=substr(command,6,strlen(command)-6);
    strcat(info," ");
    vector<char *> strs=split(info,' ');
    
    char *mode=strs.at(0);
    
    if (strs.size()!=2 && strlen(mode)!=3)
    {
        cout<<command<<": Wrong input\n";
        return;
    }
    if (mode[1]!='+' && mode[1]!='-')
    {
        cout<<command<<": Wrong input\n";
        return;
    }
    
    char *filename=strs.at(1);
    
    if (!isExistsSameFilename(currentDirectory, filename))
    {
        cout<<command<<": File not exist\n";
        return;
    }
    
    Directory* curr=findDirectoryByFilename(currentDirectory, filename);
    
    INode* inode=&allInodes[curr->inodeIndex];
    
    //cout<<"name: "<<curr->fileName<<endl;
    
    if (!isHasChmodAuthority(curr))
    {
        cout<<command<<": Permission denied\n";
        return;
    }
    
    //cout<<"0"<<mode[0]<<"\t1\t"<<mode[1]<<"\t2\t"<<mode[2]<<endl;
    
    //          012
    // mode:    u+x , g, o , a
    
    //          user group other
    //oldMode:  0 123 456 789
    //          d rwx r-x r-x
    char* oldMode=inode->mode;
    
    if (mode[1]=='+')
    {
        if (mode[0]=='u')
        {
            if (mode[2]=='r')
            {
                //u+r
                oldMode[1]='r';
            }
            if (mode[2]=='w')
            {
                //u+w
                oldMode[2]='w';
            }
            if (mode[2]=='x')
            {
                //u+x
                oldMode[3]='x';
            }
        }
        if (mode[0]=='g')
        {
            if (mode[2]=='r')
            {
                //g+r
                oldMode[4]='r';
            }
            if (mode[2]=='w')
            {
                //g+w
                oldMode[5]='w';
            }
            if (mode[2]=='x')
            {
                //g+x
                oldMode[6]='x';
            }

        }
        if (mode[0]=='o')
        {
            if (mode[2]=='r')
            {
                //o+r
                oldMode[7]='r';
            }
            if (mode[2]=='w')
            {
                //o+w
                oldMode[8]='w';
            }
            if (mode[2]=='x')
            {
                //o+x
                oldMode[9]='x';
            }

        }
        if (mode[0]=='a')
        {
            if (mode[2]=='r')
            {
                //a+r
                oldMode[1]='r';
                oldMode[4]='r';
                oldMode[7]='r';
            }
            if (mode[2]=='w')
            {
                //a+w
                oldMode[2]='w';
                oldMode[5]='w';
                oldMode[8]='w';
            }
            if (mode[2]=='x')
            {
                //a+x
                oldMode[3]='x';
                oldMode[6]='x';
                oldMode[9]='x';
            }

        }
    }
    else if (mode[1]=='-')
    {
        if (mode[0]=='u')
        {
            if (mode[2]=='r')
            {
                //u-r
                oldMode[1]='-';
            }
            if (mode[2]=='w')
            {
                //u-w
                oldMode[2]='-';
            }
            if (mode[2]=='x')
            {
                //u-x
                oldMode[3]='-';
            }
        }
        if (mode[0]=='g')
        {
            if (mode[2]=='r')
            {
                //g-r
                oldMode[4]='-';
            }
            if (mode[2]=='w')
            {
                //g-w
                oldMode[5]='-';
            }
            if (mode[2]=='x')
            {
                //g-x
                oldMode[6]='-';
            }
            
        }
        if (mode[0]=='o')
        {
            if (mode[2]=='r')
            {
                //o-r
                oldMode[7]='-';
            }
            if (mode[2]=='w')
            {
                //o-w
                oldMode[8]='-';
            }
            if (mode[2]=='x')
            {
                //o-x
                oldMode[9]='-';
            }
            
        }
        if (mode[0]=='a')
        {
            if (mode[2]=='r')
            {
                //a-r
                oldMode[1]='-';
                oldMode[4]='-';
                oldMode[7]='-';
            }
            if (mode[2]=='w')
            {
                //a-w
                oldMode[2]='-';
                oldMode[5]='-';
                oldMode[8]='-';
            }
            if (mode[2]=='x')
            {
                //a-x
                oldMode[3]='-';
                
                oldMode[6]='-';
                oldMode[9]='-';
                
            }
            
        }
    }
    //cout<<"mode"<<oldMode<<endl;
    strcpy(inode->mode,oldMode);

}

void chown(char *command)
{
    // chown newowner filename
    
    char *info=substr(command,6,strlen(command)-6);
    strcat(info," ");
    vector<char *> strs=split(info,' ');
    
    
    if (strs.size()!=2 )
    {
        cout<<command<<": Wrong input\n";
        return;
    }
   
    char *newOwner=strs.at(0);
    char *filename=strs.at(1);
    
    if (!isExistsSameFilename(currentDirectory, filename))
    {
        cout<<command<<": File not exist\n";
        return;
    }
    
    Directory* curr=findDirectoryByFilename(currentDirectory, filename);
    
    INode* inode=&allInodes[curr->inodeIndex];
    
    //cout<<"name: "<<curr->fileName<<endl;
    
    if (!isHasChownAuthority(curr))
    {
        cout<<command<<": Permission denied\n";
        return;
    }

    strcpy(inode->owner,newOwner);
    
    
}

void chgrp(char *command)
{
    
    // chgrp newgroup filename
    
    char *info=substr(command,6,strlen(command)-6);
    strcat(info," ");
    vector<char *> strs=split(info,' ');
    
    
    if (strs.size()!=2 )
    {
        cout<<command<<": Wrong input\n";
        return;
    }
    
    char *newGroup=strs.at(0);
    char *filename=strs.at(1);
    
    if (!isExistsSameFilename(currentDirectory, filename))
    {
        cout<<command<<": File not exist\n";
        return;
    }
    
    Directory* curr=findDirectoryByFilename(currentDirectory, filename);
    
    INode* inode=&allInodes[curr->inodeIndex];
    
    //cout<<"name: "<<curr->fileName<<endl;
    
    if (!isHasChgrpAuthority(curr))
    {
        cout<<command<<": Permission denied\n";
        return;
    }
    
    strcpy(inode->group,newGroup);

    
}

void help(char* command)
{
    if (strcmp(command,"help")==0)
    {
        cout<<"GNU myBash, version 1.0-release (x86_64-apple-darwin16)\n";
        cout<<"These shell commands are defined internally.  Type `help' to see this list.\n";
        cout<<"Type `help name' to find out more about the function `name'.\n\n";
        cout<<"cd\nexit\nhelp\npwd\nls\npasswd\nclear\nmkdir\nrmdir\nlocate\ntop\numask\nvi\ncat\nrm\nmv\ncp\nln\nchmod\nchown\nchgrp\n\n";
        cout<<"copyright©️ 李业芃 2018\n\n";
        return;
        
    }
    
    strcpy(command,substr(command,5,strlen(command)-5));
    
    if (strcmp(command,"exit")==0)
    {
        cout<<"EXIT:\n";
        cout<<"Format:\texit\n";
        cout<<"Function:\texit the shell and save modifications\n";
        return;
    }
    else if ( strcmp(command,"cd")==0)
    {
        cout<<"CD:\n";
        cout<<"Format:\tcd\t[absolute path]/[relative path]\n";
        cout<<"Example:\tcd\t/Users/sss\n";
        cout<<"Function:\tcd\t[absolute path]/[relative path]: go to a directory by absolute path or relative path\n";
        return;
    }
    else if ( strcmp(command,"help")==0)
    {
        cout<<"HELP:\n";
        cout<<"Format:\thelp\t[command name]\n";
        cout<<"Example:\thelp\thelp\n";
        cout<<"Function:\thelp\t:show all commands\nhelp\t[command name]\t: show the details of a command\n";
        return;
    }
    else if (strcmp(command, "pwd")==0 )
    {
        cout<<"PWD:\n";
        cout<<"Format:\tpwd\n";
        cout<<"Function:\tpwd\t:show current directory path(absolute path).\n";
        return;
        
    }else if (strcmp(command,"ls")==0)
    {
        cout<<"LS:\n";
        cout<<"Format:\tls / ls -l / ls -a / ls -al /ls -la\n";
        cout<<"Functions:\n";
        cout<<"ls\t:show all filenames (will not show hidden files)\n";
        cout<<"ls -a\t:show all filenames (will show hidden files)\n";
        cout<<"ls -l\t:show details of current directory (will not show hidden files)\n";
        cout<<"ls -al\t:show details of current directory (will show hidden files)\n";
        return;

    }else if (strcmp("clear",command)==0)
    {
        cout<<"CLEAR:\n";
        cout<<"Format:\tclear\n";
        cout<<"Function:\tclear the screen window\n";
        return;
    }
    else if (strcmp("passwd",command)==0)
    {
        cout<<"PASSWD:\n";
        cout<<"Format:\tpasswd\n";
        cout<<"Function:\tchange user passowrd\n";
        return;
    }
    else if (strcmp(command,"mkdir")==0)
    {
        cout<<"MKDIR:\n";
        cout<<"Format:\tmkdir [filename]\n";
        cout<<"Example:\tmkdir testdir\n";
        cout<<"Function:\tmake a new directory\n";
        return;
    }
    else if (strcmp(command,"locate")==0)
    {
        cout<<"LOCATE:\n";
        cout<<"Format:\tlocate [filename]\n";
        cout<<"Example:\tlocate testdir\n";
        cout<<"Function:\tfind all absolute paths of files whose name are [filename]\n";
        return;
        
    }else if (strcmp("top",command)==0)
    {
        cout<<"TOP:\n";
        cout<<"Format:\ttop\n";
        cout<<"Function:\tshow info of real pyhsical inodes and physical blocks ,free&busy blocks,free&busy inodes \n";
        return;
    }
    else if (strcmp(command,"umask")==0)
    {
        cout<<"UMASK:\n";
        cout<<"Format:\tumask / umask [mode]\n";
        cout<<"Example:\tumask 022\n";
        cout<<"Functions:\numask:\t show current user mask\n";
        cout<<"umask 022:\tset user mask to 022\n";
        cout<<"ps:\t umask is used to set the default authority of created files/directories. mode is 3 digits 000-777 \n";
        return;
    }else if (strcmp(command,"rmdir")==0)
    {
        cout<<"RMDIR:\n";
        cout<<"Format:\trmdir [name]\n";
        cout<<"Example:\trmdir testdir\n";
        cout<<"Function:\tdelete an empty directory.\n";
        return;
    }
    else if (strcmp(command,"vi")==0)
    {
        cout<<"VI:\n";
        cout<<"Format:\tvi [filename]\n";
        cout<<"Example:\tvi testfile\n";
        cout<<"Function:\tedit a file or create a file if the file doesn't exist.\n";
        return;
    }
    else if (strcmp(command,"cat")==0)
    {
        cout<<"CAT:\n";
        cout<<"Format:\tcat [filename]\n";
        cout<<"Example:\tcat testfile\n";
        cout<<"Function:\toutput the content of a file\n";
        return;
    }
    else if (strcmp(command,"rm")==0)
    {
        cout<<"RM:\n";
        cout<<"Format:\trm [filename]\n";
        cout<<"Example:\trm testfile\n";
        cout<<"Function:\tremove a existed file\n";
        return;
    }
    else if (strcmp(command,"mv")==0)
    {
        cout<<"MV:\n";
        cout<<"Format:\tmv [filename] [newfilename]\n";
        cout<<"Example:\tmv oldfile newfile\n";
        cout<<"Function:\tchange the filename\n";
        return;
    }
    else if (strcmp(command,"cp")==0)
    {
        cout<<"CP:\n";
        cout<<"Format:\tcp [filename] [filenamecopy]\n";
        cout<<"Example:\tcp oldfile newfile\n";
        cout<<"Function:\tcopy a file\n";
        return;
    }
    else if (strcmp(command,"ln")==0)
    {
        cout<<"LN:\n";
        cout<<"Format:\tln [filename] [filenamelink]\n";
        cout<<"Example:\tln oldfile newfile\n";
        cout<<"Function:\tcreate a hard link of a file. (won't allocate blocks or inodes)\n";
        return;
    }
    else if (strcmp(command,"chmod")==0)
    {
        cout<<"CHMOD:\n";
        cout<<"Format:\tchmod (u|g|o|a)(+|-)(r|w|x) [filename]\n";
        cout<<"Example:\tchmod u+x testfile\n";
        cout<<"Function:\tchange the authroity mode of a file.\n";
        cout<<"ps.:\t u means user, g means group, o means others, a means all\n+means give authroity\t-means withdraw authority\n";
        cout<<"r means read authority\tw mean write authority\tx means execute authority\n";
        return;
    }
    else if (strcmp(command,"chown")==0)
    {
        cout<<"CHOWN:\n";
        cout<<"Format:\tchown [username] [filename]\n";
        cout<<"Example:\tchown root testfile\n";
        cout<<"Function:\tchange the file's owner\n";
        return;
    }
    else if (strcmp(substr(command,0,5),"chgrp")==0)
    {
        cout<<"CHGRP:\n";
        cout<<"Format:\tchgrp [groupname] [filename]\n";
        cout<<"Example:\tchgrp wheel testfile\n";
        cout<<"Function:\tchange the file's group\n";
        return;
    }

    
}

void printConsoleInfo(const char* username)
{
    
    char* pathName=getAbsolutePath(currentDirectory);
    // cout<<"debug: "<<pathName<<endl;
    char* pathContains=new char[200];
    strcpy(pathContains,"/Users/");
    char* path=new char[200];
    strcpy(path,"");
    strcat(pathContains,username);
    
    // if pathname contains /Users/username... then ouput ~...
    
    // test /Users/sss/a
    // test /bin/s
    int len_contains_str=(int)strlen(pathContains);
    bool ic=isContains(pathContains, pathName);
    char* catedStr=substr(pathName, len_contains_str, (int)strlen(pathName)-len_contains_str);
    
    //cout<<" ic "<<ic<<endl;
    
    // cout<<" c "<<catedStr<<endl;
    
    if (ic)
    {
        strcpy(path,"~");
        strcat(path,catedStr);
    }
    
    
    // else output pathname
    
    else strcpy(path,pathName);
    
    if (strcmp(username,"root")!=0)
        cout<<"["<<username<<"@computer "<<path<<"]$";
    else
        cout<<"["<<username<<"@computer "<<pathName<<"]#";
}


void function_distrubute()
{
    
    cout<<"********Welcome To mini-UnixFileSystem*********\n";
    int isFound=0;
    User* user;
    char* username=new char[20];
    char* password=new char[20];
    string un;
    string pw;
    while(isFound==0){
        cout<<"Login: Please enter your username:\n";
        //cin>>username;
        getline(cin,un);
        strcpy(username,un.c_str());
        cout<<"Password:\n";
        //cin>>password;
        getline(cin,pw);
        strcpy(password, pw.c_str());
        
        int u_len=userTable->count;
        
        for (int i=0;i<u_len;i++)
        {
            // cout<<" name "<<users[i].userName<<endl;
            // cout<<" name "<<users[i].userPassword<<endl;
            if (strcmp(username,allUsers[i].userName)==0)          //same username
            {
                //cout<<" name "<<username<<endl;
                
                if (strcmp(password, allUsers[i].userPassword)==0) //same password
                {
                    // found user
                    isFound=1;
                    user=&allUsers[i];
                    currentUser=user;
                    //cout<<" d l "<<d_len<<endl;
                    for (int j=0;j<directoryTable->count;j++)
                    {
                        
                        //cout<<"d i f "<<dirs[j].fileName<<" p f "<<dirs[j].parentDir->fileName<<endl;
                        //char *parentF=getParentDirectoryName(&(directoryTable->head[j]));
                        if (strcmp((directoryTable->head[j]).fileName, username)==0
                            && isParentUsers(&directoryTable->head[j]))
                            
                            //cd  /Users/username
                            {
                                currentDirectory=&directoryTable->head[j];
                                break;
                            }
                    }
                    
                    
                    
                    break;
                }
                else if (i==u_len-1) {cout<<"Wrong username or password\n";break;}
            }
            else if (i==u_len-1) {cout<<"Wrong username or password\n";break;}
        }
    }
    
    while (1)
    {
        char* command=new char[200];
        printConsoleInfo(username);
        string aa;
        getline(cin,aa);
        strcpy(command,aa.c_str());
        
        //refresh int_type datas
        
        int u_len=userTable->count;
        int d_len=directoryTable->count;
        
        //execute commands
        if (strcmp(command,"exit")==0)
        {
            cout<<"Logout\n";
            cout<<"Saving session...\n";
            cout<<"...copying shared history...\n";
            cout<<"...saving history...truncating history files...\n";
            cout<<"...completed.\n";
            cout<<"Deleting expired sessions... completed\n";
            return;
        }
        else if ( strcmp(substr(command,0,2),"cd")==0)
        {
            cd(command);
        }
        else if ( strcmp(substr(command,0,4),"help")==0)
        {
            help(command);
        }
        else if (strcmp(command, "pwd")==0 )
        {
            pwd();
        }else if (strcmp(substr(command,0,2),"ls")==0)
        {
            ls(command);
        }else if (strcmp("clear",command)==0)
        {
            clear();
        }
        else if (strcmp("passwd",command)==0)
        {
            passwd();
        }
        else if (strcmp(substr(command,0,5),"mkdir")==0)
        {
            mkdir(command);
        }
        else if (strcmp(substr(command,0,6),"locate")==0)
        {
            locate(command);
        }else if (strcmp("top",command)==0)
        {
            top();
        }
        else if (strlen(command)<=9 && strcmp(substr(command,0,5),"umask")==0)
        {
            umask(command);
        }else if (strcmp(substr(command,0,5),"rmdir")==0)
        {
            rmdir(command);
        }
        else if (strcmp(substr(command,0,2),"vi")==0)
        {
            vi(command);
        }
        else if (strcmp(substr(command,0,3),"cat")==0)
        {
            cat(command);
        }
        else if (strcmp(substr(command,0,2),"rm")==0)
        {
            rm(command);
        }
        else if (strcmp(substr(command,0,2),"mv")==0)
        {
            mv(command);
        }
        else if (strcmp(substr(command,0,2),"cp")==0)
        {
            cp(command);
        }
        else if (strcmp(substr(command,0,2),"ln")==0)
        {
            ln(command);
        }
        else if (strcmp(substr(command,0,5),"chmod")==0)
        {
            chmod(command);
        }
        else if (strcmp(substr(command,0,5),"chown")==0)
        {
            chown(command);
        }
        else if (strcmp(substr(command,0,5),"chgrp")==0)
        {
            chgrp(command);
        }
        else {
            cout<<"-bash: "<<command<<": command not found\n";
        }
        
    }
    
}


int main(int argc, const char * argv[]) {

   

    
    char* t_str=new char[512];
    strcpy(t_str,"/");
    strcat(t_str,"0;");
    strcat(t_str,".filename");
    strcat(t_str,"12;");
    strcat(t_str,"..");
    strcat(t_str,"0;");
    
    Block block;
    strcpy(block.block,t_str);
    delete[] t_str;
    
    //vector<pair<char *, short>> t=getItems(block.block);
    //initAll();
    //cout << block.block;
    
    
    
    
    if (isExistFiles())
    {
        //initAll();
    }
    else {
        initAll();
    }
    
        
        //read filesystem from disk to mem
        
        
        readFileFromDisk((char*)(&myFileSystem), (char*)"fileSystem.txt", sizeof(MyFileSystem));
        superBlock=&myFileSystem.superBlock;
        INode* inodes=myFileSystem.inodes;
        Block* blocks=myFileSystem.blocks;
        
        
        allInodes=inodes;
        allBlocks=blocks;
        
        
        //read user info from disk to mem
        int len;
        userTable=new UserTable;
        readFileFromDisk((char*)userTable, (char*)"userTable.txt", sizeof(UserTable));
        len=userTable->count;
        allUsers =new User[len];
        readFileFromDisk((char*)allUsers, (char*)"users.txt", sizeof(User)*len);
        userTable->user=allUsers;
        
        Directory r;
        r.initDirectory(0, "/");
        superBlock->root=r;
        root=&superBlock->root;
        
        
        directoryTable=new DirectoryTable();
        reconstructDirectoryTable();
        
        
    
        //save changes to disk
        
       
        
        
        function_distrubute();
        
        
        saveFileToDisk((char*)(&myFileSystem),(char*)"fileSystem.txt", sizeof(MyFileSystem));
        saveFileToDisk((char*)(userTable), (char*)"userTable.txt", sizeof(UserTable));
        saveFileToDisk((char*)allUsers, (char*)"users.txt", sizeof(User)*len);
 

    
    
    return 0;
}











