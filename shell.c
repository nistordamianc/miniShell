#include<stdlib.h>
#include<stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <fcntl.h>
#define BUFSIZE 1000
#define INPBUF 100
#define ARGMAX 10
#define GREEN "\x1b[92m"
#define BLUE "\x1b[94m"
#define DEF "\x1B[0m"
#define CYAN "\x1b[96m"
#define YEL "\x1B[33m"

struct _instr
{
    char * argval[INPBUF];
    int argcount;
};
typedef struct _instr instruction;

char *input;
int exitflag = 0;
char cwd[BUFSIZE];
char* argval[ARGMAX]; // our local argc, argv
int argcount = 0;
void screenfetch();
void getInput();
int function_exit();
void function_pwd(char*, int);
void function_cd(char*);
void function_mkdir(char*);
void function_rmdir(char*);
void function_clear();
void nameFile(struct dirent*, char*);
void function_lsArg(char*);
void function_lsl();
int function_copy(char*,char*);
void function_cp(char*,char*);
void function_cat(char*);
void function_rmr(char*,int);
void function_touch(char*);
void man(char*);

int isDirectory(char*);	

int main(int argc, char* argv[])
{

    function_clear();
    screenfetch();
    function_pwd(cwd,0);

    while(exitflag==0)
    {
		//print user prompt
        printf("%s%s%s ~> %s",CYAN,cwd,DEF,DEF); 
		//call for input-sets argc to the number of args and argva[i] to the argument number
        getInput();
		//
        if(strcmp(argval[0],"exit")==0 || strcmp(argval[0],"z")==0)
        {
			//if we receive the exit signal, make exitflag=1 and exit program
            function_exit();
        }
        else if(strcmp(argval[0],"screenfetch")==0)
        {
			//print a hello message
            screenfetch();
        }
        else if(strcmp(argval[0],"pwd")==0)
        {
			//print the current working directory
            function_pwd(cwd,1);
        }
        else if(strcmp(argval[0],"cd")==0)
        {
			//change the current working directory to the given argument
            char* path = argval[1];
            function_cd(path);
        }
        else if(strcmp(argval[0],"mkdir")==0)
        {
			//create an empty directory with the given name and correct permissions
            char* foldername = argval[1];
            function_mkdir(foldername);
        }
        else if(strcmp(argval[0],"rmdir")==0)
        {
			//remove an empty directory 
            char* foldername = argval[1];
            function_rmdir(foldername);
        }
		 else if(strcmp(argval[0],"rmr")==0)
        {	//recursively remove a folder or remove a file -- given as argument
            char* filename = argval[1];
            function_rmr(filename,0);
        }
        else if(strcmp(argval[0],"clear")==0)
        {	//clear the screen
            function_clear();
        }
		else if(strcmp(argval[0],"cat")==0){
			//print on the screen the contents of a file given as argument
			char* filename=argval[1];
			function_cat(filename);
		}
		else if(strcmp(argval[0],"touch")==0){
			//print on the screen the contents of a file given as argument
			char* filename=argval[1];
			function_touch(filename);
		}
        else if(strcmp(argval[0],"ls")==0)
        {	
			if(argcount==3){
				//if we have an argument
				char* optional = argval[1];
				if(strcmp(optional,"-l")==0)
				{
					//if the argument is -l 
					function_lsl();
				}
				else{
					//if the argument is a directory
					function_lsArg(optional);
				}	
			}
            else 
				//if there are no arguments, list the contents of the current working directory
				function_lsArg(".");
        }
        else if(strcmp(argval[0],"cp")==0)
        {	//copy from the first argument to the second
            char* file1 = argval[1];
            char* file2 = argval[2];
            if(argcount > 2 && strlen(file1) > 0 && strlen(file2) > 0)
            {
				//if the number of arguments is correct
                function_cp(file1,file2);
            }
            else
            {	//else print an error
                printf("+--- Error in cp : insufficient parameters\n");
            }
        }
		else if(strcmp(argval[0],"man")==0)
        {	//man function to display information about how to use a command
            char* command = argval[1];
            man(command);
        }
		else{
			printf("+--- Unknown command!\n");
		}

    }

}

void man(char *command){
	if(strcmp(command,"cat")==0){
		printf("+--- Function cat. Usage\n cat filename  --------- Display the content of a file on stdout.\n");
	}
	else if(strcmp(command,"pwd")==0){
		printf("+--- Function pwd. Usage\n pwd  --------- Prints the current working directory.\n");
	}
	else if(strcmp(command,"ls")==0){
		printf("+--- Function ls. Usage:\n1. ls directory  --------- Lists the content of the argument directory\n2. ls -l  --------- Lists the content of current working directory with file permissions, owner, size, date created and name\n3. ls   --------- Lists the content of current working directory.\n");
	}
	else if(strcmp(command,"rmr")==0){
		printf("+--- Function rmr. Usage\n rmr file  ---------  Deletes RECURSIVELY the file or folder given as argument.\n");
	}
	else if(strcmp(command,"cp")==0){
		printf("+--- Function cp. Usage\n cp sourceFile targetFile  ---------  Copy recursively from sourceFile to targetFile.\nIf sourceFile is a folder, creates a folder and copies all its content into targetFile.\n");
	}
	else if(strcmp(command,"clear")==0){
		printf("+--- Function clear. Usage\n clear  ---------  Clears the screen.\n");
	}
	else if(strcmp(command,"mkdir")==0){
		printf("+--- Function mkdir. Usage\n mkdir filename  --------- Make an empty folder.\n");
	}
	else if(strcmp(command,"touch")==0){
		printf("+--- Function touch. Usage\n touch filename  --------- Make an empty file.\n");
	}
	else if(strcmp(command,"cd")==0){
		printf("+--- Function cd. Usage\n mkdir path  --------- Change the current working directory to the given path.\n");
	}
}


//take as an argument a char* path - the path of file/folder that you wish to delete
//doesn't return anything
void function_rmr(char *path,int trigger)
{	
	//if the path is not a directory, it is a file-call the remove function to delete it and then exit the function
	if(isDirectory(path)!=1){			
		remove(path);
		return;
	}
	//if this code is reached, it means that the given path is a directory
	//scan the directory and make an array of all its files
	struct dirent **fileList;
	int filesNo = scandir(path,&fileList,0,alphasort);	//filesNo will contain the number of directory entries in the path directory
	//fileListTemp will point to an allocated array of pointers to allocated strings each of which points to an allocated block containing the null-terminated name of a file.
	if(filesNo==2){							//if the filesNo is 2 - this means that we are in an empty folder - 2 because "." is an entry and ".." is another entry				
		rmdir(path);						//delete the empty directory and exit the function
		return;
	}
	else if(filesNo==-1){					//if the scandir function returns -1, then it's an error in reading the folder
		printf("+--- Error in remove\n");
		return;
	}
	//a static member in order to ask just once for 
	if (trigger==0){
		printf("+--- You are trying to delete a folder that contains files. Do you wish to delete the folder and all its files? [y/n]\n");
		size_t buf = 0;
		char *input;
		//read input and store it into input char*
		getline(&input,&buf,stdin);
		if(strcmp(input,"y\n")==0){
			//if the user wishes to delete everything, set the trigger to 1; it is static because we don't want to repeat for every file the question
			trigger=1;
		}
		
	}
	//if the execution reaches this point, it means the user doesnt wish to delete a folder that contains files in it and exit the function
	if (trigger==0){
		printf("+--- Exitting..\n");
		return;
	}
	for(int i=2;i<filesNo;i++){					//iterating through the list of files contained by the folder given as argument
		//if(isDirectory(path)==1){		//daca pathu e directory
			char pathSave[100];			//il salvam in pathSave
			sprintf(pathSave,"%s",path);		//saving the path before calling the function again
			sprintf(path, "%s/%s",path,fileList[i]->d_name);		//copy the name of the ith file pointed by the list of files into the path and call the function again
			function_rmr(path,trigger);			//after this call, if the ith element is a file it will no longer exist and if it is a folder, then it will be empty 
			sprintf(path,"%s",pathSave);
			if(isDirectory(path)==1)	//if the entry from the list was a folder, it is now guaranteed that it's empty and we can call rmdir on an empty directory
				rmdir(path);		//delete the empty directory
	}
}

//copy the sourceFile to the targetFile	-- checks if it exists and asks the user if replace it, else it creates it and writes to it
//called by function_cp(char*,char*) in order to copy every file from a folder
void file_to_file(char* sourceFile,char * targetFile){
	FILE *f1,*f2;
	struct stat t1,t2;			//stat structure which holds information about a file
	size_t buf = 0;
	//try to open sourceFile in readonly mode
    f1 = fopen(sourceFile,"r");
	
    if(f1 == NULL)
    {
		//if fopen fails, exit from the function
        printf("+--- Can not open %s.\n",sourceFile);
        return;
    }
	//try to open the second file to see if it exists--readonly
    f2 = fopen(targetFile,"r");
		if(f2)
		{
			// if file exists
			// sourceFile must be more recently updated
			stat(sourceFile, &t1);				//t1 holds now information about the first file
			stat(targetFile, &t2);				//t2 holds now information about the second file
			if(difftime(t1.st_mtime,t2.st_mtime) < 0)		//if targetFile is more recent that sourceFile
			{
				printf("+--- Error in cp:\"%s\" exists and is more recently updated than \"%s\". Do you wish to overwrite it? [y/n]\n",targetFile,sourceFile);
				//print an error message and ask for a decision to replace or not the second file with the first one
				char *input;
				getline(&input,&buf,stdin);		//get input
				if(strcmp(input,"y\n")==0){
					f2 = fopen(targetFile,"w+");		//if execution reaches this point, targetFile will be overwritten by sourceFile -- it is now opened in write mode
				}
				else{
					//if the users doesnt want to replace the second file by the first
					printf("+--- Exitting copy.\n");
					return;
				}
			}

			
		}	
	f2 = fopen(targetFile,"ab+"); // create the file if it doesn't exist
    fclose(f2);

    f2 = fopen(targetFile,"w+");	//open the file as write only
    if(f2 == NULL)
    {	
		//if the file could not be opened, print a message and exit
        printf("+--- Can not open %s.\n",targetFile);
        fclose(f1);
        return;
    }

    if(open(sourceFile,O_RDONLY)<0 || open(targetFile,O_WRONLY)<0)
    {
		//try to open first file as readonly or the second as write only--prints an error if the user doesnt have permission to read first file OR write the second file
        printf("+--- Error in cp access \n");
        return;
    }
    char cp;
    while((cp=getc(f1))!=EOF)		//make cp hold one character from file one until the end of sourceFile
    {
        putc(cp,f2);				//copy the character held by cp into targetFile
    }
    fclose(f1);						//close the files
    fclose(f2);
}


//first it checks if the first argument is a directory in order to see if it should copy a folder or a file
void function_cp(char* source, char* target)
{	
	//
	if(isDirectory(source)==1){
		//if the source file is a folder, the final path in which the file will be created is target/source 		
		char finalPath[100];
		sprintf(finalPath,"%s/%s",target,source);
		//try to open the final path to see if the folder exists
		DIR* dir = opendir(finalPath);
		if (dir)
		{	//if it exists, just close it and continue 
			closedir(dir);
		}
		else{
			//if it doesnt exist, create it and continue
			function_mkdir(finalPath);
		}
		struct dirent **fileList;		//make a list of all the files contained in the source folder
		int filesNo = scandir(source,&fileList,0,alphasort);
		for(int i=2;i<filesNo;i++){			
			//iterate through all the files contained in the source folder
			char *iterator;
			iterator=fileList[i]->d_name;
			char targetName[100],sourceName[100];
			//build the sourceName and the targetName of the files
			sprintf(targetName,"%s/%s",finalPath,iterator);		//destinatie
			sprintf(sourceName,"%s/%s",source,iterator);
			if(!isDirectory(iterator)){			
				//if the file is a folder, call file_to_file function 
				file_to_file(sourceName,targetName);
				
			}
			else{
				function_mkdir(target);
				function_cp(sourceName,targetName);
			}
			
	}
   }
   else{
	   file_to_file(source,target);
   }
    
}
void function_touch(char *filename){
	FILE *f;
	f=fopen(filename,"r");
	if(f){
		printf("+--- Error in touch, file already exists\n");
		return;
	}
	fopen(filename,"ab+");
}

//prints at the standard output the contents of a file
void function_cat(char *file)
{	
	//open the file 
	FILE *fp=fopen(file,"r");
	if (fp==NULL){
		//if the file is not opened successfully, exit 
		printf("+--- Error in cat, wrong filename\n");
		return;
	}
	//delcare and initialize the linelength as 256
	int LINELEN=256;
	char line[LINELEN];	
	while(fgets(line, LINELEN, fp))
    {
		//get 256 bytes from the file and print them at standard output
		fputs(line,stdout);
    }
}

//check if a file is a directory
int isDirectory( char *path)		//daca intoarce 1 e tru
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);

}
/*get input containing spaces and tabs and store it in argval*/
void getInput()
{
    fflush(stdout); // clear all previous buffers if any
    input = NULL;
    size_t buf = 0;
	//read input
    getline(&input,&buf,stdin);
    argcount = 0;
	//if the string is not null and we don't have more that 10 arguments
    while((argval[argcount] = strsep(&input, " \t\n")) != NULL && argcount < ARGMAX-1)
    {
		//valid input --- arguments separated by tab or space
        if(sizeof(argval[argcount])==0)
        {
            free(argval[argcount]);
        }
        else argcount++;

    }
    free(input);
}

//print the name of a file based on its type -- used by ls,lsArgs,lsl
void nameFile(struct dirent* name,char* followup)
{
    if(name->d_type == DT_REG)          // regular file
    {
        printf("%s%s%s",BLUE, name->d_name, followup);
    }
    else if(name->d_type == DT_DIR)    // a directory
    {
        printf("%s%s/%s",GREEN, name->d_name, followup);
    }
    else                              // unknown file types
    {
        printf("%s%s%s",CYAN, name->d_name, followup);
    }
}


//list the content of the current working directory with details
void function_lsl()
{
    int i=0;
    struct dirent **listr;																	//an aray containing all the files					
    struct stat details;																	//details-uses stat lib that contains details about a file
	char timer[14];																			//char * with 14 dimension
    int listn = scandir(".",&listr,0,alphasort);			
    if(listn > 0)
    {				
        for ( i = 2; i < listn; i++)														//incepi cu 2 pt ca primele doua sunt . si ..
        {
		if(stat(listr[i]->d_name,&details)==0)												//daca e fisier											
            {
                // example - -rwxrwxr-x 1 user user  8872 Jun 26 10:19 a.out
                // owner permissions - group permissions - other permissions
                // links associated - owner name - group name
                // file size (bytes) - time modified - name

                // owner permissions - group permissions - other permissions	
                printf("%s%1s",DEF,(S_ISDIR(details.st_mode)) ? "d" : "-");					//def e culoarea
                printf("%s%1s",DEF,(details.st_mode & S_IRUSR) ? "r" : "-");				//like an inline if - 
                printf("%s%1s",DEF,(details.st_mode & S_IWUSR) ? "w" : "-");
                printf("%s%1s",DEF,(details.st_mode & S_IXUSR) ? "x" : "-");
                printf("%s%1s",DEF,(details.st_mode & S_IRGRP) ? "r" : "-");
                printf("%s%1s",DEF,(details.st_mode & S_IWGRP) ? "w" : "-");
                printf("%s%1s",DEF,(details.st_mode & S_IXGRP) ? "x" : "-");
                printf("%s%1s",DEF,(details.st_mode & S_IROTH) ? "r" : "-");
                printf("%s%1s",DEF,(details.st_mode & S_IWOTH) ? "w" : "-");
                printf("%s%1s ",DEF,(details.st_mode & S_IXOTH) ? "x" : "-");
                // links associated - owner name - group name
                printf("%s ",(getpwuid(details.st_uid))->pw_name);							//user name
                // file size (bytes) - time modified - name
                printf("%5lld ",(unsigned long long)details.st_size);
                strftime (timer,14,"%h %d %H:%M",localtime(&details.st_mtime));
                printf("%s ",timer);
                nameFile(listr[i],"\n");
            }
        }
        printf("%s+--- Total %d files.\n",YEL,listn-2);
    }
    else
    {
            printf("+--- Empty directory.\n" );
    }
}


//list the content of a folder
void function_lsArg(char *folder)
{
    int i=0;
    struct dirent **listr;
    int listn = scandir(folder, &listr, 0, alphasort);
    if (listn >= 0)
    {
        
        for(i = 2; i < listn; i++ )
        {
			nameFile(listr[i],"    ");
            if(i%8==0) printf("\n");
        }
		printf("\n%s+--- Total %d files.\n",YEL,listn-2);
    }
    else
    {
        printf ("+--- Error in ls \n");
    }

}


//clear the screen
void function_clear()
{
    const char* blank = "\e[1;1H\e[2J";
    write(STDOUT_FILENO,blank,12);
}

/* remove folder */
void function_rmdir(char* name)
{
    int statrm = rmdir(name);
    if(statrm==-1)
    {
        printf("+--- Error in rmdir \n");
    }
}


//create an empty folder
void function_mkdir(char* name)
{
    int stat = mkdir(name, 0777);// all appropriate permissions
    if(stat==-1)
    {
        printf("+--- Error in mkdir \n");
    }
}


//change directory
void function_cd(char* path)
{
    int ret = chdir(path);
    if(ret==0) // path could be changed if cd successful
    {
        function_pwd(cwd,0);
    }
    else printf("+--- Error in cd \n");
}


// set exit flag to 1 in order to stop getting input
int function_exit()
{
    exitflag = 1;
    return 0; 
}

//Implement pwd function in shell 
void function_pwd(char* cwdstr,int command)
{
    char temp[BUFSIZE];
    char* path=getcwd(temp, sizeof(temp));
    if(path != NULL)
    {
        strcpy(cwdstr,temp);
        if(command==1)  // check if pwd is to be printed
        {
            printf("%s\n",cwdstr);
        }
    }
    else printf("+--- Error in getcwd() \n");

}

//prints a hello message
void screenfetch()
{
    printf("%s","+--- Hello, user!\n");
}

