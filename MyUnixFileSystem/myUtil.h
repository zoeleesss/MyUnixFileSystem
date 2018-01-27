//
//  myUtil.h
//  MyUnixFileSystem
//
//  Created by zoe lee on 17/01/2018.
//  Copyright © 2018 zoelee. All rights reserved.
//

#ifndef myUtil_h
#define myUtil_h

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

template<typename T>
void singleArraryCopy(T* src,T* dst,int size)
{
    for (int i=0;i<size;i++)
    {
        src[i]=dst[i];
    }
}

template<typename T>
void vectorAppend(vector<T>* src,vector<T>*dst)
{
    for (int i=0;i<dst->size();i++)
    {
        src->push_back(dst->at(i));
    }
}


void printVector(vector<char *> *src,int size)
{
    for (int i=0;i<size;i++)
    {
        cout<<src->at(i)<<"\t";
    }
}

void printVector(vector<short> *src,int size)
{
    for (int i=0;i<size;i++)
    {
        cout<<src->at(i)<<"\t";
    }
}


// dst = src/dst  e.g.  dst: root, src: /Users  -> dst=/Users/root
char* reverseStrcat(const char* src,const char* dst)
{
    
    
    if (src==nullptr || dst==nullptr) return " ";
    char* temp=new char[512];
    strcpy(temp,src);
    strcat(temp,"/");
    strcat(temp,dst);
    //cout<<"src \t"<<src<<"\tdst\t"<<dst<<"\ttemp:\t"<<temp<<endl;
    return temp;
}


// e.g. /User/sss/a.txt contains /Users/sss

bool isContains(const char* containedStr,const char* bigStr)
{
    char* s=strstr(bigStr,containedStr);
    
    if (s!=nullptr) return true;
    else return false;
}

/*
string substrTool(string src,int fi,int len)
{
    string s=src;
    return s.substr(fi,len);
}

char* substr(const char* src, int fi,int len)
{
    string s=substrTool(src,fi,len);
    return const_cast<char *>(s.c_str());
}*/


char* substr(const char* src, int fi,int len)
{
    
    const int size=1024*10;
    char str[size];
    strcpy(str,src);
    
    //strcat(str,"/a");
    //len=2;
    //cout<<"  str "<<str<<endl;
    char t[size];
    int start=0;
    
    
    
    //cout<<fi<<"   ss  "<<len<<endl;
    
    for (int i=fi;i<len+fi;start++,i++)
    {
        //cout<<" z  "<<str[i]<<endl;
        
        if (str[i]!='\0')
        {
            //cout<<" s  "<<str[i]<<endl;
            t[start]=str[i];
        }
    }
    t[start]='\0';
    
    //cout<<" t "<<t<<endl;
    char *s=new char[size];
    strcpy(s,t);
    
    //cout<<"  s "<<s;
    

    return s;
}




//  /0;.0;..0;  -> v[0]=/0  v[1]=.0 v[2]=..0
vector<char *> split(const char*src,const char delimiter)
{
    vector<char *> splitedStr;
    char results[512*20];
    strcpy(results,"");
    int count=1;
    for (int i=0;i<strlen(src);i++)
    {
        char t=src[i];
        if (t==delimiter)
        {
            count++;
            char *s=new char[512*20];
            strcpy(s,results);
            splitedStr.push_back(s);
            //cout<<s<<endl;
            strcpy(results,"");
        }else {
            char *tstr=new char[2];
            tstr[0]=t;
            tstr[1]='\0';
            strcat(results,tstr);
        }
        
    }
    
    return splitedStr;
}

void deleteAllVectorCharStar(vector<char *> *items)
{
    for (int i=0;i<items->size();i++)
    {
        char *p=items->at(i);
        delete []p;
    }
}

//  v[0]=/0 v[1]=.0 v[2]=..0  ->  v[0]=</,0>   v[1]=<.,0>   v[2]=<..,0>

vector<pair<char *, short>> processDirectoryItems(vector<char *> *src)
{
    vector<pair<char *, short>> items;
    
    for (int i=0;i<src->size();i++)
    {
        //cout<<"s\t"<<src->size()<<"\n";
        char *curr=src->at(i);
        char digits[3]="";
        char name[14]="";
        char *filename=new char[14];
        short index=0;
        bool isMetComma=false;                  //   characters,digits  use comma as inner-delimiter
        for (int j=0;j<strlen(curr);j++)
        {
            
            if (curr[j]!=',' && !isMetComma)                               //character
            {
                char t_c[2];
                t_c[0]=curr[j];
                t_c[1]='\0';
                strcat(name,t_c);
            }
            else
            {
                if (curr[j]==',')
                {
                    isMetComma=true;
                }
                if (curr[j]<=57 && curr[j]>=48 && isMetComma)                //digit
                {
                    char t_digit[2];
                    t_digit[0]=curr[j];
                    t_digit[1]='\0';
                    strcat(digits,t_digit);
                }
            }
            
        }
        strcpy(filename, name);
        index=atoi(digits);
        pair<char *,short> item;
        item.first=filename;
        item.second=index;
        items.push_back(item);
        
        
    }
    
    return items;
}

vector<short> processBlockItems(vector<char *> *src)
{
    vector<short> items;
    for (int i=0;i<src->size();i++)
    {
        items.push_back(atoi(src->at(i)));
    }
    
    return items;
}

/**
 **       Simple Interface for users, combine split func + processDirectoryItems func.
 **
 */

vector<pair<char *, short>> getItems(char *src)
{
    
    
    vector<char *> strs=split(src,';');
    
    vector<pair<char *, short>> t=processDirectoryItems(&strs);
    //printVector(&strs, strs.size());
    return t;
}

vector<short> getBlockItems(char *src)
{
    // bug could happen here because it starts with #1 not #1
    // bug when indirect address 10;11;12;13;14;15;
    // bug fix: content changed to +10;11;12;13;14;15;
    
    
    char *validInfo=substr(src,1,strlen(src)-1);
    vector<char *> strs=split(validInfo,';');
    vector<short> t=processBlockItems(&strs);
    
    deleteAllVectorCharStar(&strs);
    
    return t;
    
}

void printItemsVector(vector<pair<char *,short>> src)
{
    
    for (int i=0;i<src.size();i++)
    {
        pair<char *,short> item=src.at(i);
        cout<<"filename "<<item.first<<"\tinodeID\t"<<item.second<<endl;
    }
    
}

vector<short> getInodesIdsFromItems(vector<pair<char *,short>> *items,char *target)
{
    vector<short> ids;
    for (int i=0;i<items->size();i++)
    {
        pair<char *,short> item=items->at(i);
        if (strcmp(item.first,target)==0)
        {
            ids.push_back(item.second);
        }
    }
    return ids;
}





/**
 **             if all files exist , return @true;
 **                 else             return @false;
 */


bool isExistFiles()
{
    bool flag=1;
    string filenames[3]={"fileSystem.txt","userTable.txt","users.txt"};
    
    for (int i=0;i<3;i++)
    {
        
        ifstream f;
        f.open(filenames[i],ios::binary);
        if (!f)
        {
            flag=0;
            break;
        }
        else{
            f.close();
        }
    }
    return flag;
    
}

char* itoa(int num,char*str,int radix)
{/*索引表*/
    char index[]="0123456789ABCDEF";
    unsigned unum;/*中间变量*/
    int i=0,j,k;
    /*确定unum的值*/
    if(radix==10 &&num<0)/*十进制负数*/
    {
        unum=(unsigned)-num;
        str[i++]='-';
    }
    else unum=(unsigned)num;/*其他情况*/
    /*转换*/
    do{
        str[i++]=index[unum%(unsigned)radix];
        unum/=radix;
    }while(unum);
    str[i]='\0';
    /*逆序*/
    if(str[0]=='-')k=1;/*十进制负数*/
    else k=0;
    char temp;
    for(j=k;j<=(i-1)/2;j++)
    {
        temp=str[j];
        str[j]=str[i-1+k-j];
        str[i-1+k-j]=temp;
    }
    return str;
}

/*** umask part 
 *                  umask    directory   file
 *
 *                  022         755      644
 *                  027         750      640
 *                  002         775      664
 *                  006         771      660
 *                  007         770      660
 *
 **/


void setDirectoryModeByUmask(char* umask,char* mode)
{
    // for directory
    
    
    //          0123
    //normal is 0022
    
    for (int i=1;i<4;i++)
    {
        //0123456789
        //drwxr--r--
        
        // 1 ,2 ,4 ,5 , 6, 7
        //-x -w -r -rx -rw -rwx
        if (umask[i]=='1')
        {
            strcat(mode,"rw-");
        }else if (umask[i]=='2')
        {
            strcat(mode,"r-x");
            //cout<<" mx "<<mode[3*i];
        }
        else if (umask[i]=='3')
        {
            strcat(mode,"r--");
        }
        else if (umask[i]=='4')
        {
            strcat(mode,"-wx");
        }else if (umask[i]=='5')
        {
            strcat(mode,"-w-");
        }else if (umask[i]=='6')    //w,r = '-'
        {
            strcat(mode,"--x");
        }else if (umask[i]=='7')    // r,w,x = '-'
        {
            strcat(mode,"---");
        }else
        {
            strcat(mode,"rwx");
        }
        
    }
    //cout<<"mode "<<mode<<endl;
    
}

void setFileModeByUmask(char* umask,char* mode)
{
    // for directory
    
    
    //          0123
    //normal is 0022
    
    for (int i=1;i<4;i++)
    {
        //0123456789
        //drwxr--r--
        
        // 1 ,2 ,4 ,5 , 6, 7
        //-x -w -r -rx -rw -rwx
        if (umask[i]=='1')
        {
            strcat(mode,"rw-");
        }else if (umask[i]=='2')
        {
            strcat(mode,"r--");
            //cout<<" mx "<<mode[3*i];
        }else if (umask[i]=='3')
        {
            strcat(mode,"r--");
        }
        else if (umask[i]=='4')
        {
            strcat(mode,"-w-");
        }else if (umask[i]=='5')
        {
            strcat(mode,"-w-");     //1
        }else if (umask[i]=='6')
        {
            strcat(mode,"---");     //
        }else if (umask[i]=='7')
        {
            strcat(mode,"---");
        }else
        {
            strcat(mode,"rw-");
        }
        
    }
    //cout<<"mode "<<mode<<endl;
    
}


/**
 *          End of Umask
 *
 **/



bool isConvertableToNum(char *str,int size)
{
    int flag=0;
    for (int i=0;i<size;i++)
    {
        if (str[i]>='0' && str[i]<='7')
        {
            flag++;
        }
        
    }
    return flag==size;
}


char * putVectorItemsToCharStar(vector<pair<char *,short>>* items)
{
    char *str=new char[512];
    strcpy(str,"");
    for (int i=0;i<items->size();i++)
    {
        pair<char *,short> item=items->at(i);
        strcat(str,item.first);
        strcat(str,",");
        char t[10];
        strcat(str,itoa(item.second,t,10));
        strcat(str,";");
        
    }
    return str;
}

char *putBlockVectorItemsToCharStar(vector<short>*items)
{
    // 42;13;7;53;33;
    char *str=new char[512];
    strcpy(str,"");
    for (int i=0;i<items->size();i++)
    {
        short item=items->at(i);
        char t[10];
        strcat(str,itoa(item,t,10));
        strcat(str,";");
    }
    return str;
}




#endif /* myUtil_h */
