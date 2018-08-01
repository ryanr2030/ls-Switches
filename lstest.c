#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

#define BUFFERSIZE      4096
#define COPYMODE        0644

void oops(char *, char *);
void do_ls(char[]);
void dostat(char *);
void show_file_info( char *, struct stat *);
void mode_to_letters( int , char [] );
char *uid_to_name( uid_t );
char *gid_to_name( gid_t );
static int compare_size (const void *p, const void *q);
struct stat get_stats(const char* filename);



/* ls2.c
 *	purpose  list contents of directory or directories
 *	action   if no args, use .  else list files in args
 *	note     uses stat and pwd.h and grp.h
 *	BUG: try ls2 /tmp
 *
 *	Molay, chapter 3, p 91-95
 */


void do_lsbyname(char dirname[])
{
	int i,j,k;
	DIR *dir_ptr;
	struct dirent *direntp;
	if((dir_ptr = opendir(dirname)) == NULL)
		{fprintf(stderr, "cannot open %s\n", dirname);}
	else 
	{
		int listingCount = 0;
		int i, j, k;
		int swapInt;
		char swapString[500];
		struct stat info;
		while((direntp = readdir(dir_ptr)) != NULL)
			{listingCount++;}		  
			//Close directory pointer
		closedir(dir_ptr);
			//Declare arrays
		char nameArray[listingCount][500];
		int indexArray[listingCount];
				
		dir_ptr = opendir(dirname);
					//Build the size array
		for (i = 0; i < listingCount; i++)
		{//Start of for
			direntp = readdir(dir_ptr);
			stat(direntp->d_name, &info);
			strcpy(nameArray[i], direntp->d_name);
			indexArray[i] = i;
		}//End of for
		//Close directory pointer
		closedir(dir_ptr);
		for (i = 0; i < listingCount - 1; i++)
		{//Start of outer for
			for (j = 0; j < (listingCount - 1 - i); j++)
			{//Start of inner for
				if (strcmp(nameArray[j], nameArray[j + 1]) > 0)
				{//Start of if
					//Swap timeArray values
					strcpy(swapString, nameArray[j]);
					strcpy(nameArray[j], nameArray[j + 1]);
					strcpy(nameArray[j + 1], swapString);
				}//End of if
			}//End of inner for
		}//End of outer for											
			//Print the sorted array(has to be the nameArray because name is the keyword)
	for(i = 2; i < listingCount; i++)
	{//Start of for
			//Print the file info per the sortedIndex
		{dostat(nameArray[i]);}
	}//End of for
	}//End of else
}


void do_ls( char dirname[] )
/*
 *	list files in directory called dirname
 */
{
	DIR		*dir_ptr;		/* the directory */
	struct dirent	*direntp;		/* each entry	 */

	if ( ( dir_ptr = opendir( dirname ) ) == NULL )
		fprintf(stderr,"ls2: cannot open %s\n", dirname);
	else
	{
		while ( ( direntp = readdir( dir_ptr ) ) != NULL ) {
			if (strcmp(direntp->d_name,".")==0 ||
				strcmp(direntp->d_name,"..")==0) continue;
			dostat( direntp->d_name );
		}
		closedir(dir_ptr);
	}
}


void dostat( char *filename )
{
	struct stat info;
	if ( lstat(filename, &info) == -1 )		/* cannot stat	 */
		perror( filename );			/* say why	 */
	else					/* else show info	 */
		show_file_info( filename, &info );
}


void show_file_info( char *filename, struct stat *info_p )
/*
 * display the info about 'filename'.  The info is stored in struct at *info_p
 */
{
	char	*uid_to_name(), *ctime(), *gid_to_name(), *filemode();
	void	mode_to_letters();
        char    modestr[11];

	mode_to_letters( info_p->st_mode, modestr );

	printf( "%s"    , modestr );
	printf( "%4d "  , (int) info_p->st_nlink);
	printf( "%-8s " , uid_to_name(info_p->st_uid) );
	printf( "%-8s " , gid_to_name(info_p->st_gid) );
	printf( "%8ld " , (long)info_p->st_size);
	printf( "%.12s ", 4+ctime(&info_p->st_mtime));
	printf( "%s\n"  , filename );

}

/*
 * utility functions
 */

/*
 * This function takes a mode value and a char array
 * and puts into the char array the file type and the
 * nine letters that correspond to the bits in mode.
 * NOTE: It does not code setuid, setgid, and sticky
 * codes
 */

void mode_to_letters( int mode, char str[] )
{
    strcpy( str, "----------" );           /* default=no perms */

    if ( S_ISDIR(mode) )  str[0] = 'd';    /* directory?       */
    if ( S_ISCHR(mode) )  str[0] = 'c';    /* char devices     */
    if ( S_ISBLK(mode) )  str[0] = 'b';    /* block device     */

    if ( mode & S_IRUSR ) str[1] = 'r';    /* 3 bits for user  */
    if ( mode & S_IWUSR ) str[2] = 'w';
    if ( mode & S_IXUSR ) str[3] = 'x';

    if ( mode & S_IRGRP ) str[4] = 'r';    /* 3 bits for group */
    if ( mode & S_IWGRP ) str[5] = 'w';
    if ( mode & S_IXGRP ) str[6] = 'x';

    if ( mode & S_IROTH ) str[7] = 'r';    /* 3 bits for other */
    if ( mode & S_IWOTH ) str[8] = 'w';
    if ( mode & S_IXOTH ) str[9] = 'x';
}

#include	<pwd.h>

char *uid_to_name( uid_t uid )
/*
 *	returns pointer to username associated with uid, uses getpw()
 */
{
	struct	passwd *getpwuid(), *pw_ptr;
	static  char numstr[10];

	if ( ( pw_ptr = getpwuid( uid ) ) == NULL ){
		sprintf(numstr,"%d", uid);
		return numstr;
	}
	else
		return pw_ptr->pw_name ;
}

#include	<grp.h>

char *gid_to_name( gid_t gid )
/*
 *	returns pointer to group number gid. used getgrgid(3)
 */
{
	struct group *getgrgid(), *grp_ptr;
	static  char numstr[10];

	if ( ( grp_ptr = getgrgid(gid) ) == NULL ){
		sprintf(numstr,"%d", gid);
		return numstr;
	}
	else
		return grp_ptr->gr_name;
}


struct stat get_stats(const char* filename)
{
    struct stat sb;

    if (lstat(filename, &sb) < 0)
    {   
        perror(filename);
    }

    return sb;
}

void display_stats(char* dir, char* filename)
{
    struct stat sb = get_stats(filename);
	show_file_info( filename, &sb );   
}


static int cmp_time(const void* p1, const void* p2)
{
    const char* str1 = *(const char**)p1;
    const char* str2 = *(const char**)p2;

    time_t time1 = get_stats(str1).st_mtime;
    time_t time2 = get_stats(str2).st_mtime;

    return time1 > time2;
}

static int cmp_size(const void* p1, const void* p2)
{
    const char* str1 = *(const char**)p1;
    const char* str2 = *(const char**)p2;

    long int size1 = get_stats(str1).st_size;
    long int size2 = get_stats(str2).st_size;

    return size1 > size2;
}


void do_ls1( char dirname[], char* opt )
/*
 *	list files in directory called dirname
 */
{
	DIR		*dir_ptr;		/* the directory */
	struct dirent	*direntp;		/* each entry	 */
	long curr_alloc_amt = 30000;
    	char** dir_arr = malloc(curr_alloc_amt * sizeof(char*));

    	long int count = 0;

	if ( ( dir_ptr = opendir( dirname ) ) == NULL )
		fprintf(stderr,"ls2: cannot open %s\n", dirname);
	else
	{
		while ( ( direntp = readdir( dir_ptr ) ) != NULL ) {
			if (strcmp(direntp->d_name,".")==0 ||
				strcmp(direntp->d_name,"..")==0) continue;
			dir_arr[count] = direntp->d_name;
           		count++;
		}
		if (strcmp(opt, "-t")==0)
   		{
        		qsort(dir_arr, count, sizeof(char*), cmp_time);
    		}
    		else if (strcmp(opt, "-s")==0)
    		{
        		qsort(dir_arr, count, sizeof(char*), cmp_size);
    		}

    		for (long int i = 0; i < count; ++i)
    		{
       			display_stats(direntp, dir_arr[i]);
    		}

    		closedir(dir_ptr);
   	}
}


void oops(char *s1, char *s2)
{
        fprintf(stderr,"Error: %s ", s1);
        perror(s2);
        exit(1);
}

int main(int argc, char ** argv) {
	if (argc==1)
		do_ls(".");
	if(argc==2){
		if((strcmp(argv[1], "-t") == 0) || (strcmp(argv[1], "-s") == 0))
			do_ls1(".",argv[1]);
		if ((strcmp(argv[1], "-l") == 0))
			do_lsbyname(".");	
	}
	
	if(argc==4){
	if(strcmp(argv[1], "cp") == 0)
	{
		int in_fd, out_fd, n_chars;
		char buf[BUFFERSIZE];
		if ( argc != 4 ){
                	fprintf( stderr, "usage: %s source destination\n", *argv);
                	exit(1);
        	}
		char *src = argv[2];
		char *dest = argv[3];

		if (strcmp(src,dest)==0)
			printf("File already exists!\n");

				/* open files	*/
	
        	if ( (in_fd=open(argv[2], O_RDONLY)) == -1 )
               		oops("Cannot open ", argv[2]);

       		if ( (out_fd=creat( argv[3], COPYMODE)) == -1 )
               		oops( "Cannot creat", argv[3]);
		
					/* copy files	*/

        	while ( (n_chars = read(in_fd , buf, BUFFERSIZE)) > 0 )
               		if ( write( out_fd, buf, n_chars ) != n_chars )
                        	oops("Write error to ", argv[3]);
							
		if ( n_chars == -1 )
			oops("Read error from ", argv[2]);

						/* close files	*/

       		if ( close(in_fd) == -1 || close(out_fd) == -1 )
                	oops("Error closing files","");
	}
    }
}
