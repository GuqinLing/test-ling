#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <assert.h>
#include <fcntl.h>

ssize_t typedef (*unlinkat_func_t)(int fd,const char *pathname,int flag);
ssize_t typedef (*rmdir_func_t)(const char *pathname);

const char cneos_path_modified[] = "/mnt/cf/modified";
const char cneos_path_update[] = "/mnt/cf/update";

#define ns_plen (sizeof(cneos_path_modified)-1)
#define ut_plen (sizeof(cneos_path_update)-1)

int rmdir(const char *pathname)
{
	static rmdir_func_t cneos_old_rmdir = NULL;
    char r_path[PATH_MAX+1] = {'\0'};
    realpath(pathname,r_path);
	if(strstr(r_path,cneos_path_modified) == r_path) 
		if(r_path[ns_plen]=='\0'|| r_path[ns_plen]=='/')
			return 0;
	if(strstr(r_path,cneos_path_update) == r_path) 
		if(r_path[ut_plen]=='\0'|| r_path[ut_plen]=='/')
			return 0;
	cneos_old_rmdir = dlsym(RTLD_NEXT,"rmdir");
	return cneos_old_rmdir(pathname);
}

int unlinkat(int fd,const char *pathname,int flag)
{
	static unlinkat_func_t cneos_old_unlinkat = NULL;
	cneos_old_unlinkat = dlsym(RTLD_NEXT,"unlinkat"); 
	struct stat link_path;
    int ret = 0;
    char buf[50] = {'\0'};
	char file_path[512] = {'\0'};
	char r_path[PATH_MAX+1] = {'\0'};
	snprintf(buf,sizeof(buf), "/proc/self/fd/%d", fd); //注；"/"目录下fd值为-100
	if(fd >0)
	{
		ret=readlink(buf,file_path,sizeof(file_path)-1);   //从fd获取路径名
		strcat(file_path,"/");
	}	
	else if(pathname[0]!='/')  //如果路径是相对路径
	{
		getcwd(file_path,sizeof(file_path));
		strcat(file_path,"/");
	}
	strcat(file_path,pathname);
    lstat(file_path,&link_path);
    if(!S_ISLNK(link_path.st_model))
        realpath(file_path,r_path); //从pathname获取实际路径  */
	if(strlen(r_path)>=ns_plen || strlen(r_path)>=ut_plen)
	{
		if(strstr(r_path,cneos_path_modified)==r_path) //如果路径的前缀是cneos_path_modified
		{
			if(r_path[ns_plen]=='\0'||r_path[ns_plen]=='/'){		
				char merge_path[1024]="/";
				if(strstr(r_path,"/mnt/cf/modified/nsfocus/")==r_path){   //如果路径前缀是"/mnt/cf/modified/nsfocus/"
					strcat(merge_path,"opt/nsfocus/");
					strcat(merge_path,r_path+strlen("/mnt/cf/modified/nsfocus/"));
					int rt = cneos_old_unlinkat(fd,merge_path,flag);
					if(rt == 0 && flag == AT_REMOVEDIR )  //如果对象是目录，则改变属性为文件
						flag=0;
                    if(rt != 0 && flag == AT_REMOVEDIR )
                        return 0;
				}else{
					if(strstr(r_path,"/mnt/cf/modified/root/")==r_path){
						strcat(merge_path,r_path+strlen("/mnt/cf/modified/root/"));
						cneos_old_unlinkat(fd,merge_path,flag);
					}	
				}					
			}				
		}
		if(strstr(r_path,cneos_path_update)==r_path) //如果路径的前缀是cneos_path_update
		{
			if(r_path[ut_plen]=='\0'||r_path[ut_plen]=='/'){
				char merge_path[1024]="/";
				char modify_path[1024]="/";
				if(strstr(r_path,"/mnt/cf/update/nsfocus/")==r_path){
					strcat(modify_path,"mnt/cf/modified/nsfocus/");
					strcat(modify_path,r_path+strlen("/mnt/cf/update/nsfocus/"));
					int rt = 0;
					DIR *dir_rt ;
					if(flag==AT_REMOVEDIR )
						dir_rt = opendir(modify_path);
					if(flag==0)
						rt = access(modify_path,F_OK);
					if( rt != 0 || dir_rt == NULL ){  //判断modified下目录或者文件是否存在	
						strcat(merge_path,"opt/nsfocus/");
						strcat(merge_path,r_path+strlen("/mnt/cf/update/nsfocus/"));
						cneos_old_unlinkat(fd,merge_path,flag);
                        cneos_old_unlinkat(fd,pathname,flag);
                        return cneos_old_unlinkat(fd,modify_path,0);
					}
				}else{
					if(strstr(r_path,"/mnt/cf/updateroot/")==r_path){
						strcat(merge_path,r_path+strlen("/mnt/cf/update/root/"));
						cneos_old_unlinkat(fd,merge_path,flag);
					}	
				}					
			}					
		}
	}
	return cneos_old_unlinkat(fd,pathname,flag);	
}
