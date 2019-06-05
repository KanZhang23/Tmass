/*******************************************************************************
*									       *
* publish.c -- Routines for histoscope to find programs to monitor, and for    *
*	       programs to announce their presence to histoscope tools	       *
*									       *
* Copyright (c) 1991, 1993 Universities Research Association, Inc.	       *
* All rights reserved.							       *
* 									       *
* This material resulted from work developed under a Government Contract and   *
* is subject to the following license:  The Government retains a paid-up,      *
* nonexclusive, irrevocable worldwide license to reproduce, prepare derivative *
* works, perform publicly and display publicly by or for the Government,       *
* including the right to distribute to other Government contractors.  Neither  *
* the United States nor the United States Department of Energy, nor any of     *
* their employees, makes any warrenty, express or implied, or assumes any      *
* legal liability or responsibility for the accuracy, completeness, or         *
* usefulness of any information, apparatus, product, or process disclosed, or  *
* represents that its use would not infringe privately owned rights.           *
*                                        				       *
* Fermilab Nirvana GUI Library						       *
* May 20, 1992								       *
*									       *
* Written by Mark Edel							       *
*									       *
* Mangled by Joy Kyriakopulos for VMS port  2/5/93			       *
*									       *
*******************************************************************************/

#ifndef USE_DIRENT
#define USE_DIRENT
#endif

#ifdef VXWORKS
#include "vxWorks.h"
#endif /*VXWORKS*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef VMS
#include "vmsscandir.h"
#include <unixlib.h>
#include <starlet.h>
#include <ssdef.h>
#include <lib$routines.h>
#include <jpidef.h>
#include <descrip.h>
#include <processes.h>
#include "../util/VMSparam.h"
#include "../util/VMSUtils.h"
#else
#ifndef VXWORKS
#include <netdb.h>
#include <pwd.h>
#include <sys/param.h>
#ifdef USE_DIRENT
#include <dirent.h>
#else
#include <sys/dir.h>
#endif /* USE_DIRENT */
#endif /*VXWORKS*/
#include <unistd.h>
#endif /*VMS*/
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include "hsTypes.h"
#include "publish.h"
#ifdef SOLARIS
#include "scandir.h"
#endif /*SOLARIS*/
#ifdef VMS_CLIENT
#include "../histo_api/clientXDRsocket1.h"
#define gethostname hs_gethostname
#define gethostbyname hs_gethostbyname
#else
#ifdef VMS
#include <mul/netdb.h>
#endif /*VMS*/
#endif /*VMS_CLIENT*/
#ifdef VXWORKS
#include "remLib.h"
#include "hostLib.h"
#include "taskLib.h"
#include "vxscandir.h"
#define MAXPATHLEN FILENAME_MAX
#define MAXHOSTNAMELEN 64
#endif /*VXWORKS*/

#define False 0
#define True 1
#define WHITESPACE " \t\n"
#define REMOTE_LS_BUF_SIZE 20000 /* buffer for output from remote ls cmd  */

/* The following value is written at the beginning of the files histoscope
   clients leave in the /tmp directory.   These files announce the presence
   of the client for histoscope tools which may want to connect with them.
   the purpose is to positively identify files created by histoscope clients
   before deleting them in the cleanup operation. */
#define FILE_MATCH_KEY	123454321 

#ifdef VMS
#define TMP_PREFIX 23
static char IDFile[L_tmpnam + TMP_PREFIX] = "HISTO_PROCESS_DIR:HISTO";
#else
#ifdef VXWORKS
static char IDFile[MAXHOSTNAMELEN+14];
#else
char *IDFile = NULL;	   /* Name of file left in the /tmp directory so
			   histoscope process can find our socket #	*/
#endif /*VXWORKS*/
#endif /*VMS*/

#ifndef VXWORKS
static int listLocalIDFiles(idFileData **list, char *errorMessage);
static int listRemoteIDFiles(char *remoteHostName, idFileData **list,
			     char *errorMessage, char *userName, char *passwd);
static int readRemoteIDFile(char *node, char *name, idFileData *fileData, 
			    int unixFlag, char *userName, char *passwd);
static int rmtListCmd(char *command, char *buf, int *nRead, char *errorMessage);
#endif /*VXWORKS*/

static int readIDFile(const char *name, char *tmpDir, idFileData *fileData);

#if defined(VMS) || defined(VXWORKS) || defined (SOLARIS)      /* no scandir */
static int selectFileForDelete(char *dir);
#ifndef VXWORKS
static int selectFileForList(char *dir);
#endif /*VXWORKS*/
#else
#ifdef USE_DIRENT
static int selectFileForDelete(const struct dirent *dir);
static int selectFileForList(const struct dirent *dir);
#else
static int selectFileForDelete(const struct direct *dir);
static int selectFileForList(const struct direct *dir);
#endif /* USE_DIRENT */
#endif /* VMS || VXWORKS || SOLARIS */

#ifdef VMS
static int procAlive(int pID);
static struct dsc$descriptor_s *histoTmpnam(char *tmpName);
static char *makeVMSrshCmd(char *command, char *userName, char *passwd,
			   char *remoteHostName, char *execStr);
#else
#ifndef VXWORKS
static void makeUNIXrshCmd(char *command, char *userName, char *remoteHostName,
			   char *execStr);
#endif /*VXWORKS*/
#endif /*VMS*/

/*
** CreateIDFile
**
** Creates a file in the /tmp directory (Unix) or the HISTO_PROCESS_DIR
** directory (VMS) of the local machine to announce
** that this process is available for inspection by Histoscope tools.
** The number of the socket that Histoscope can connect to, as well as
** the identity string that the user supplied in the hsInitialize routine,
** must be passed in, so they can be communicated to Histoscope.  The
** argument publString should be MAX_ID_FILE_SIZE in length and will be
** set by CreateIDFile to the publish string written to the /tmp file.
*/
int CreateIDFile(int socketNum, const char *inpString, char *publString)
{
    char identStringBuf[HS_MAX_IDENT_LENGTH];
    const char* identString = inpString;
    time_t clock;
#ifdef VXWORKS
char userName[MAX_USER_NAME_LEN];
#else
    char *userName;
    struct passwd *passwdEntry;
#endif /*VXWORKS*/
    char *timeString;
    FILE *fp;
    char node_name[MAXHOSTNAMELEN+1];
#ifdef VMS
    char delStr[MAXPATHLEN];
    short retLen;
    int jpiStat, stat;
    struct getJPIdescriptor {
	short bufLength;
	short itemCode;
	int  *bufAddr;
	int  *retLenAddr;
	int  *endList;
    } getJPID;
#endif /*VMS*/
#ifdef VMS_CLIENT
    char *hp_name;			/* for hs_gethostbyname */
#else
#ifndef VXWORKS
    struct hostent *hp;                 /* for gethostbyname */
#endif /*VXWORKS*/
#endif /*VMS_CLIENT*/
    
    /* Make up a name for the file of information for histoscope */
#ifdef VMS
    tmpnam(IDFile + TMP_PREFIX);	/* name returned in place */
    strcpy(delStr, "HISTO_PROCESS_DIR:HISTO");
    strcat(delStr, IDFile + TMP_PREFIX);
    strcat(delStr, ".");
    
    /* delete all files of same name */
    while ((stat = remove(delStr)) == 0)	/* 0 signifies success */
    	{}
#else
#ifdef VXWORKS
    if (gethostname(node_name, MAXHOSTNAMELEN+1) < 0) {
    	fprintf(stderr,"hs_initialize:  Error getting VXWORKS host name\n");
    	return False;
    }
    strcpy(IDFile, "/tmp/histoAAA");
    strcat(IDFile, node_name);
#else
    {   
	/* igv: replace original "tempnam" with secure code */
        /* IDFile = tempnam("/tmp", "histo"); */
	int fd;
	char *tmpdir, *default_tmpdir = "/tmp";
	tmpdir = getenv("TMPDIR");
	if (tmpdir == NULL)
	    tmpdir = default_tmpdir;
	IDFile = malloc(strlen(tmpdir) + 16);
	if (IDFile == NULL)
	{
	    fprintf(stderr, "Fatal error: out of memory. Exiting.\n");
	    exit(EXIT_FAILURE);
	}
	strcpy(IDFile, tmpdir);
	strcat(IDFile, "/");
	strcat(IDFile, "histoXXXXXX");
	fd = mkstemp(IDFile);
	if (fd < 0)
	{
	    fprintf(stderr, "Fatal error: failed to open a temporary file. Exiting.\n");
	    exit(EXIT_FAILURE);
	}
	fchmod(fd, 0600);
	close(fd);
    }
#endif /*VXWORKS*/
#endif /*VMS*/

    /* Create the file */
#if defined(VXWORKS) && defined(COM_DEBUG)
    printf("in CreateIDFile:  IDFile = %s, calling fopen\n", IDFile);
#endif
    if ((fp = fopen(IDFile, "w")) == NULL) {
#ifdef VMS
	perror("Couldn't write to HISTO_PROCESS_DIR.  Can't communicate with Histoscope");
#else
    	perror("couldn't write to /tmp.  Can't communicate with Histoscope");
#ifndef VXWORKS
	free(IDFile);
	IDFile = NULL;
#endif /*VXWORKS*/
#endif /*VMS*/
        return False;
    }
    
#ifndef VXWORKS
    /* Make the file owner/group/world readable/writeable */
    /* igv: comment that out -- obvious security problem  */
    /* chmod(IDFile, 0666); */
#endif /*VXWORKS*/

#ifdef VMS
    /* Get the user name on VMS */
    /* (for some reason getenv("USER") does not work properly here, */
    /*      so do the following instead:) */
    userName = malloc(13);		/* 12 plus 1 for ending null */
    getJPID.bufLength  = 12;		/* (max) size of user name */
    getJPID.itemCode   = JPI$_USERNAME;
    getJPID.bufAddr    = userName;
    getJPID.retLenAddr = &retLen;
    getJPID.endList    = 0;
    if ((jpiStat = sys$getjpiw(1,0,0,&getJPID,0,0,0)) != SS$_NORMAL) {
	fprintf(stderr, "CreateIDFile: Error calling GETJPI.  Status = %d\n",
	    jpiStat);
	return False;
    }
    userName[retLen] = '\0';
/*    printf("userName = %s, length = %d\n", userName, retLen);  */
#else    
#ifdef VXWORKS
#ifdef COM_DEBUG
    printf("Calling remCurIdGet\n");
#endif
    remCurIdGet(userName, NULL);		/* get current user name */
#ifdef COM_DEBUG
    /* printf("Calling hostTblInit\n"); */ 
#endif
    /* hostTblInit(); */ /* in startup instead */ /* for gethostname below */
#else
    /* Get the user name and truncate it to our maximum length.  Unix requres
       this wierd sequence of getlogin followed by getpwuid because getlogin
       only works if a terminal is attached & there can be more than one
       name associated with a uid (really?).  Both calls return a pointer
       to a static area.  We truncate the string to our maximum length by
       writing a null into the static area, which is slightly sleazy */
    userName = getlogin();
    if (userName == NULL || userName[0] == '\0') {
    	passwdEntry = getpwuid(getuid());
    	userName = passwdEntry->pw_name;
    }
    if (strlen(userName) >= MAX_USER_NAME_LEN)
    	userName[MAX_USER_NAME_LEN-1] = '\0';
#endif /*VXWORKS*/
#endif /*VMS*/

    /* Put the current time in a string with format: Tue 11:45 PM */
    time(&clock);
    timeString = ctime(&clock);
    strncpy(&timeString[4], &timeString[11], 5);
    timeString[9] = '\0';

    /* get OFFICIAL local host name */
#if defined(VXWORKS) && defined(COM_DEBUG)
    printf("calling gethostname\n");
#endif
    if (gethostname(node_name, MAXHOSTNAMELEN+1) < 0) {
    	fprintf(stderr,"hs_initialize:  Error getting host name\n");
#if !defined(VXWORKS) && !defined(VMS)
	free(IDFile);
	IDFile = NULL;
#endif /*!VXWORKS && !VMS*/
    	return False;
    }
#ifndef VXWORKS
#ifdef VMS_CLIENT
    hp_name = hs_gethostbyname(node_name);
    if (hp_name == NULL || hp_name[0] == '\0') {
#else
    hp = gethostbyname(node_name);
    if (hp == 0) {
#endif /*VMS_CLIENT*/
	fprintf(stderr,"hs_initialize:  Error getting local hostname\n");
#if !defined(VXWORKS) && !defined(VMS)
	free(IDFile);
	IDFile = NULL;
#endif /*!VXWORKS && !VMS*/
        return False;
    }
#ifdef VMS_CLIENT
    strcpy(node_name, hp_name);
#else
    strcpy(node_name, hp->h_name);
#endif /*VMS_CLIENT*/
#endif /*VXWORKS*/
    if (strlen(node_name) > MAX_NODE_LEN) {
    	fprintf(stderr,"hs_initialize:  Error - hostname > MAX_NODE_LEN (%s)\n",
    		node_name);
    	node_name[0] = '\0';
    }

    /* Write all of the information to the file in /tmp and close it */
#ifdef VXWORKS
#ifdef COM_DEBUG
    printf("node_name = %s\nCalling taskShowInit\n", node_name);
#endif

    if (strlen(inpString) >= HS_MAX_IDENT_LENGTH-1)
    {
        strncpy(identStringBuf, inpString, HS_MAX_IDENT_LENGTH-2);
        identStringBuf[HS_MAX_IDENT_LENGTH-1] = 0;
        identString = identStringBuf;
    }

    /* On VxWorks, we don't use the pid, because /tmp will be on a separate
       Unix machine, and we don't want histo-scope in listLocalIDFiles to 
       check that the pid exists on its machine.  So instead, use 0, which 
       makes histoscope think the process does exist on its machine. */ 
    /*taskShowInit();*/
    sprintf(publString, "%d %d %d %s \"%s\" \"%s\" %s", FILE_MATCH_KEY, 
    	     0, /*taskIdSelf(),*/	/* taskIdSelf returns pid on VxWorks */
    	    socketNum, userName, identString, timeString, node_name);
#else
    sprintf(publString, "%d %d %d %s \"%s\" \"%s\" %s", FILE_MATCH_KEY, getpid(),
    	    socketNum, userName, identString, timeString, node_name);
#endif /*VXWORKS*/
    fprintf(fp, "%s", publString);
    fclose(fp);
    return True;
}

/*
** RemoveIDFile
**
** Removes the file created by CreateIDFile.  Should be called when the
** histoscope client program is about to terminate, to delete the file
** advertising the program's presence to Histoscope tools.
*/
void RemoveIDFile(void)
{
    if (IDFile != NULL && IDFile[0] != '\0') {
    	remove(IDFile);
#if !defined(VXWORKS) && !defined(VMS)
	free(IDFile);
	IDFile = NULL;
#else
	IDFile[0] = '\0';
#endif /*!VXWORKS && !VMS*/
    }
}

/*
** CleanupOldIDFiles
**
** Our method of announcing port numbers relies on creating files.  Because
** Unix can not gurantee that a program will be allowed clean up after itself,
** we do the next best thing, clean up the next time the program is run.
** CleanupOldIDFiles Looks through all of the files in the /tmp directory
** that it can read, checking if they were created by CreateIDFile above.
** It then tests if their process ids still correspond to active processes.
** If not, it removes the files.  On VMS we do a similar thing, looking in
** HISTO_PROCESS_DIR instead of /tmp.
*/
void CleanupOldIDFiles(void)
{
#if defined(VMS) || defined(VXWORKS) || defined(SOLARIS)
    char **nameList;
#else
#ifdef USE_DIRENT
    struct dirent **nameList = NULL;
#else
    struct direct **nameList = NULL;
#endif /* USE_DIRENT */
    char fullName[MAXPATHLEN];
#endif /*VMS || VXWORKS || SOLARIS */
    int nNames, i;
    char *tmpDir;
       
    /* Scan the /tmp directory for histoXXXX files and check them for
       correspondance with live processes.   On VMS, Solaris, and VxWorks,
       scandir doesn't exist, so look for scandir.c in /nirvana/histo_util */
#ifdef VMS
    nNames = scandir("HISTO_PROCESS_DIR:HISTO*.", &nameList, 
			selectFileForDelete, NULL);
#else
#ifdef VXWORKS
    tmpDir = "/tmp";
#else
    tmpDir = getenv("TMPDIR");
    if (tmpDir == 0)
    	tmpDir = "/tmp";
#endif /*VXWORKS*/
    nNames = scandir(tmpDir, &nameList, selectFileForDelete, NULL);
#endif /*VMS*/
    if (nNames == -1)
    	return;		/* just give up if any errors occur */
    
    for (i=0; i<nNames; i++) {
#if defined(VMS) || defined(VXWORKS) || defined(SOLARIS)
    	remove(nameList[i]);
#else
    	sprintf(fullName, "%s/%s", tmpDir, nameList[i]->d_name);
    	remove(fullName);
#endif /*VMS || VXWORKS || SOLARIS*/
    }
    /* Free the memory allocated by scandir */
    for (i=0; i<nNames; i++)
    	free(nameList[i]);
    if (nameList != NULL)
    	free(nameList);
}

#ifndef VXWORKS
/*
** ListIDFiles
**
** Creates a list of the contents of all active id files in the /tmp (Unix)
** or HISTO_PROCESS_DIR (VMS)
** directory of a local or remote machine.  Returns a -1 if an error occurs
** reading the directory and deposits error message text in the string
** errorMessage.  Otherwise, returns the number of files in the list.
** The parameter "list", returns an array of idFileData structures that
** contain all of the information read from the id files.  The caller
** is responsible for freeing the list.  If no histoscope id files are
** found, or there is an error, no memory is allocated.  The caller
** must supply a string of length MAX_LIST_ERROR_LEN for returned
** error messages.
*/
int ListIDFiles(char *remoteHostName, idFileData **list, char *errorMessage,
	char *userName, char *passwd)
{
#ifdef VMS
    /* On VMS, allow specification of a username or password to utilize rsh */
    if (remoteHostName != NULL && *remoteHostName != '\0')
    	return listRemoteIDFiles(remoteHostName, list, errorMessage, userName,
		passwd);
    else if (passwd != NULL && passwd[0] != '\0' || userName!= NULL && 
    		userName[0] != '\0')
    	return listRemoteIDFiles("0", list, errorMessage, userName, passwd);
    else 
    	return listLocalIDFiles(list, errorMessage);
#else
    /* on Unix, commPanel insures a local search doesn't have a username (or
       password) */
    if (remoteHostName == NULL || *remoteHostName == '\0')
    	return listLocalIDFiles(list, errorMessage);
    else
    	return listRemoteIDFiles(remoteHostName, list, errorMessage, userName,
		passwd);
#endif /*VMS*/
}

/* listLocalIDFiles - finds and reads the histoscope id files in the    */
/*		      /tmp directory (Unix) or HISTO_PROCESS_DIR (VMS)  */
/*		      of the local machine				*/
static int listLocalIDFiles(idFileData **list, char *errorMessage)
{
    int nNames, i;
#ifdef VMS
    char **nameList;
	    
    /* Use VMS's LIB$FILESCAN to find histoscope id files */
    /* This will return full VMS file specifications      */
    nNames = VMSFileScan("HISTO_PROCESS_DIR:HISTO*.*", &nameList,
    			 selectFileForList, NOT_ERR_FNF);
    	
    /* return if no files were found, or errors occured */ 
    if (nNames == -1) {
    	strcpy(errorMessage, "Error scanning HISTO_PROCESS_DIR directory");
    	*list = NULL;
    	return -1;
    } else if (nNames == 0) {
    	*list = NULL;
    	return 0;
    }
    
    /* allocate space for the returned list in array form */
    *list = (idFileData *)malloc(sizeof(idFileData) * nNames);
    
    /* read each of the files depositing the data in the returned list */
    for (i=0; i<nNames; i++)
    	readIDFile(nameList[i], "HISTO_PROCESS_DIR:", &(*list)[i]); 
    						/* errors already chkd */
    
    /* free the memory allocated by VMSFileScan */
    for (i=0; i<nNames; i++)
    	free(nameList[i]);
    free(nameList);
    VMSFileScanDone();				/* also FAB and RAB memory */
    
    return nNames;

#else
    char *tmpDir;
#ifdef SOLARIS
    char **nameList;
    char filename[MAXPATHLEN];
#else
#ifdef USE_DIRENT
    struct dirent **nameList;
#else
    struct direct **nameList;
#endif /* USE_DIRENT */
#endif /* SOLARIS */
   
    /* Scan the /tmp directory for histoXXXX files and check them for
       correspondance with live processes.  If the system has a TMPDIR
       variable, use it, because that was where the files were put by
       tempnam.  Remote access, however, won't work with a TMPDIR variable. */
    tmpDir = getenv("TMPDIR");
    if (tmpDir == 0)
    	tmpDir = "/tmp";
    nNames = scandir(tmpDir, &nameList, selectFileForList, NULL);
    
    /* return if no files were found, or errors occured */ 
    if (nNames == -1) {
    	strcpy(errorMessage, "Error scanning /tmp directory");
    	*list = NULL;
    	return -1;
    } else if (nNames == 0) {
    	*list = NULL;
    	return 0;
    }
    
    /* allocate memory for the list */
    *list = (idFileData *)malloc(sizeof(idFileData) * nNames);
    
    /* read each of the files depositing the data in the returned list */
    for (i=0; i<nNames; i++)
#ifdef SOLARIS
    {
    	char *tmpStr;
    	/*sprintf(filename, "%s/%s", tmpDir, nameList[i]);*/
    	tmpStr = strrchr(nameList[i], '/');	     /* find last '/' */
    	strcpy(filename, tmpStr != NULL ? tmpStr+1 : nameList[i]);
    	readIDFile(filename, tmpDir, &(*list)[i]);   /* errors already chkd */
    }
#else
    	readIDFile(nameList[i]->d_name, tmpDir, &(*list)[i]); 
    						     /* errors already chkd */
#endif /* SOLARIS */    
    /* Free the memory allocated by scandir */
    for (i=0; i<nNames; i++)
    	free(nameList[i]);
    free(nameList);

    return nNames;

#endif /*VMS*/
}

/* finds and reads the histoscope id files in the
   /tmp directory of a remote machine */
static int listRemoteIDFiles(char *remoteHostName, idFileData **list,
			     char *errorMessage, char *userName, char *passwd)
{
    int i, nRead, filenameLen, nFiles = 0, unixFlag, errFlg;
    char command[MAXHOSTNAMELEN+112], buf[REMOTE_LS_BUF_SIZE], *p, *q;
    char filename[MAXPATHLEN];
    struct listElem {
    	idFileData data;
    	struct listElem *next;
    } *fileData, *f, *fileDataList = NULL;

    /* Compose an rsh command to execute a directory listing on the remote
       machine.  Assume the remote machine is the same operating system as
       this one:  i.e. if this is VMS, assume VMS.  If this is unix, 
       assume unix.	*/
#ifdef VMS
    /* compose a Multinet rsh command to execute DIR on remote VMS machine */
    unixFlag = 0;
    makeVMSrshCmd(command, userName, passwd, remoteHostName, 
		    "DIR/NOTRAIL HISTO_PROCESS_DIR:");
#else
    /* compose a unix rsh command to execute ls on the remote machine */
    unixFlag = -1;
    makeUNIXrshCmd(command, userName, remoteHostName, "ls /tmp 2>&1");
#endif /*VMS*/
    /* execute the command */
    if (rmtListCmd(command, buf, &nRead, errorMessage))
    	return -1;				/* error		      */
#ifdef VMS
    /* Check if it was a unix system instead of a VMS system */
    if (buf[0] == '\0' || (p = strstr(buf, "Command not found")) 
    		!= NULL) {
	unixFlag = -1;
	makeVMSrshCmd(command, userName, passwd, remoteHostName, "ls /tmp");
#else
    /* Check if it was a VMS system instead of a UNIX system */
    if (strstr(buf, "%DCL-") != NULL) {
	unixFlag = 0;
	makeUNIXrshCmd(command, userName, remoteHostName,
			"'DIR/NOTRAIL HISTO_PROCESS_DIR:'");
#endif /*VMS*/
    	if (rmtListCmd(command, buf, &nRead, errorMessage)) 
    	    return -1;				/* error		      */
    }
    if (unixFlag)
    	p = buf;
    else {
    	/* make p point to last occurrence of a Directory output so skip any 
    	   VMS login output */
    	for (p = buf-1, errFlg = -1; (q = strstr(p+1, "Directory ")) != NULL; p = q)
    	    errFlg = 0;				/* req at least 1 occurrence  */
	if (errFlg) {
	    if (strstr(buf, "-W-NOFILES") != NULL)
		return 0;			/* No files found	      */
	    else {
		strncpy(errorMessage, buf, MAX_LIST_ERROR_LEN);
		return -1;			/* Error: No directory output */
	    }
	}
	else
	    p = strchr(p, '\n');		/* point to end of line       */
    }
    
    /* read file names from remote ls (or DIR) command.  Since ls has different
       argument names on different types of machines, it was safer to
       just issue the command without any arguments and parse the output
       to get the file names, whether they're arranged in columns or not */
    while (p-buf < nRead) {
    	p += strspn(p, WHITESPACE);
    	filenameLen = strcspn(p, WHITESPACE);
    	if (p-buf == nRead)
    	    break;
	if (unixFlag) {
	    strncpy(filename, p, filenameLen);	    /* for Unix systems */
	    filename[filenameLen] = '\0';
	}
	else {
	    strcpy(filename, "HISTO_PROCESS_DIR:"); /* for VMS systems  */
	    strncat(filename, p, filenameLen);
    	    filename[filenameLen+18] = '\0';
	}
    	p += filenameLen;
	fileData = (struct listElem *)malloc(sizeof(struct listElem));
	if (readRemoteIDFile(remoteHostName, filename, &fileData->data,
		unixFlag, userName, passwd)) {
	    fileData->next = fileDataList;
	    fileDataList = fileData;
	    nFiles++;
	} else {
	    free(fileData);
	}
    }
    	
    /* allocate space for the returned list in array form */
    *list = (idFileData *)malloc(sizeof(idFileData) * nFiles);
    
    /* copy the file data from the linked list to the returned array */
    for (i=0, f=fileDataList; i<nFiles && f!=NULL; i++, f=f->next)
    	(*list)[i] = f->data;
    
    /* free the memory allocated for the linked list */
    while (fileDataList != NULL) {
    	f = fileDataList->next;
    	free(fileDataList);
    	fileDataList = f;
    }
    return nFiles;
}
#endif /*VXWORKS*/

/* a selection routine for scandir which selects dead histoscope id files.
   Dead, in this case, means that they no longer represent running processes */
#if defined(VMS) || defined(VXWORKS) || defined(SOLARIS)
static int selectFileForDelete(char *dir)
#else
#ifdef USE_DIRENT
static int selectFileForDelete(const struct dirent *dir)
#else
static int selectFileForDelete(const struct direct *dir)
#endif /*USE_DIRENT*/
#endif /*VMS || VXWORKS || SOLARIS */
{
    idFileData fileData;
    char *tmpDir;
    
    /* check that the file is ours and read the pid and socket number */
#ifdef VMS
    if (!readIDFile(dir, "", &fileData))
#else
#ifdef VXWORKS
    tmpDir = "/tmp";
#else
    tmpDir = getenv("TMPDIR");
    if (tmpDir == 0)
    	tmpDir = "/tmp";
#endif /*VXWORKS*/
#if defined(VXWORKS) || defined(SOLARIS)
    if (!readIDFile(dir, tmpDir, &fileData))
#else
    if (!readIDFile(dir->d_name, tmpDir, &fileData))
#endif /* VXWORKS || SOLARIS */
#endif /* VMS */
    	return False;
    
    /* reject files which represent processes that are still alive.  This
       is done differently on VMS and UNIX. */
#ifdef VMS
    if (procAlive(fileData.pid) == 1)
	return False;
#else
    /* UNIX: use the kill system call with a signal argument of zero, which
       just checks if the pid is ok, without sending any signals.	*/
    if (kill(fileData.pid, 0) == 0) {
#ifdef linux
	errno = 0;	/* linux scandir returns error if sel rtn sets errno */
#endif /*linux*/
    	return False;	/* process is still alive, reject the file */
    }
    if (errno == EPERM)	{
#ifdef linux
	errno = 0;	/* linux scandir returns error if sel rtn sets errno */
#endif /*linux*/
    	return False;	/* process is alive, just no permission to signal */
    }
#endif /*VMS*/
#ifdef linux
	errno = 0;	/* linux scandir returns error if sel rtn sets errno */
#endif /*linux*/
    return True;
}

#ifndef VXWORKS
/* A selection routine for scandir which selects live histoscope id files.
   Live, in this case, means that they represent active processes */
#if defined(VMS) || defined(SOLARIS)
static int selectFileForList(char *dir)
#else
#ifdef USE_DIRENT
static int selectFileForList(const struct dirent *dir)
#else
static int selectFileForList(const struct direct *dir)
#endif /*USE_DIRENT*/
#endif /*VMS || SOLARIS*/
{
    idFileData fileData;
    char *tmpDir;
    
    /* check that the file is ours and read the pid and socket number */

#ifdef VMS
    if (!readIDFile(dir, "", &fileData))
#else
    tmpDir = getenv("TMPDIR");
    if (tmpDir == 0)
    	tmpDir = "/tmp";
#if defined(SOLARIS)
    if (!readIDFile(dir, tmpDir, &fileData))
#else
    if (!readIDFile(dir->d_name, tmpDir, &fileData))
#endif /*SOLARIS*/
#endif /*VMS*/
    	return False;
    
    /* accept files which represent processes that are still alive.  This
       is done differently on VMS and UNIX. */
#ifdef VMS
    if (procAlive(fileData.pid) == 1)
	return True;
#else
    /* UNIX: use the kill system call with a signal argument of zero, which
       just checks if the pid is ok, without sending any signals.	*/
    if (kill(fileData.pid, 0) == 0)
    	return True;	/* process is still alive, accept the file */
    if (errno == EPERM)	
    	return True;	/* process is alive, just no permission to signal */
#endif /*VMS*/
    return False;
}
#endif /*VXWORKS*/

/* read a histoscope id file and copy its contents into the structure
   fileData.  Return False if the file is not an id file, or can't be read */
static int readIDFile(const char *name, char *tmpDir, idFileData *fileData)
{
    FILE *fp;
    char fullName[MAXPATHLEN];
    int nRead, key;
    
    /* reject files that don't begin with the histo prefix */
#ifdef VMS
    if (strncmp("HISTO", name, 5) && (strstr(name, "]HISTO") == NULL) &&
		(strstr(name, "HISTO_PROCESS_DIR:HISTO") == NULL) )
#else
    if (strncmp("histo", name, 5))
#endif /*VMS*/
    	return False;
    
#ifdef VMS
    fp = fopen(name, "r");
#else
    /* prepend "/tmp/" (or TMPDIR) to make the name a full path to the file */
    if (tmpDir[0] != '\0')
        sprintf(fullName, "%s/%s", tmpDir, name);
    else
    	strcpy(fullName, name);
    	
    /* open the file to read the process id */
    fp = fopen(fullName, "r");
#endif /*VMS*/
    if (fp == NULL)
    	return False;	/* reject files we can't open */
    
    /* read in the match key, the pid and the socket number and close file */
    fileData->node[0] = '\0';		/* allow upwards compatability */
    nRead = fscanf(fp, "%d %d %d %s \"%[^\"]\" \"%[^\"]\" %s", &key,
    		   &fileData->pid, &fileData->socketNum, fileData->userName,
    		   fileData->idString, fileData->startTime, fileData->node);
    fclose(fp);
    if (nRead != 6 && nRead != 7)	/* allow V1 files too */
    	return False;	/* reject files not in correct format */

    /* reject files which don't match key (therefore not created by us) */
    if (key != FILE_MATCH_KEY)
    	return False;
    
    return True;
}

#ifndef VXWORKS
/* read a histoscope id file from a remote machine, and copy its contents
   into the structure fileData.  Return False if the file is not an id file,
   or can't be read.  Unlike readIDFile above, readRemoteIDFile does not
   verify that the process is still alive */
   
static int readRemoteIDFile(char *node, char *name, idFileData *fileData,
		int unixFlag, char *userName, char *passwd)
{
    char buf[MAX_ID_FILE_SIZE], command[MAXHOSTNAMELEN+MAXPATHLEN+65];
    char *p, fileMatchStr[65], execStr[MAXPATHLEN+8];
    FILE *pipe;
    int key, nRead;
#ifdef VMS
    int spawnFlags = 2;				/* NOCLISYM */
    int spawn_sts;
    struct dsc$descriptor_s *cmdDesc;
    struct dsc$descriptor_s *dirOutFile;
    char dirOF[MAXPATHLEN+1];
#endif /*VMS*/
    
    /* reject files that don't begin with the histo prefix */
    /* and compose an rsh command to execute ls on the remote machine */
    if (unixFlag) {				/* target system is unix */
    	if (strncmp("histo", name, 5))
    	    return False;
	sprintf(execStr, "cat /tmp/%s", name);
#ifdef VMS
	makeVMSrshCmd(command, userName, passwd, node, execStr);
#else
	makeUNIXrshCmd(command, userName, node, execStr);
#endif /*VMS*/
    }
    else {					/* target system is VMS */
	if (strncmp("HISTO", name, 5) && (strstr(name, "]HISTO") == NULL) &&
		(strstr(name, "HISTO_PROCESS_DIR:HISTO") == NULL) )
    	    return False;
    	else {
#ifdef VMS
	    if (strncmp(name, "HISTO", 5))
		sprintf(execStr, "TYPE HISTO_PROCESS_DIR:%s", name);
	    else
		sprintf(execStr, "TYPE %s", name);
	    makeVMSrshCmd(command, userName, passwd, node, execStr);
#else
	    /* for VMS from unix systems */
	    if (strstr(name, "HISTO_PROCESS_DIR:HISTO") == NULL)
		sprintf(execStr, "'TYPE HISTO_PROCESS_DIR:%s'", name);
	    else
    		sprintf(execStr, "'TYPE %s'", name); 
	    makeUNIXrshCmd(command, userName, node, execStr);
#endif /*VMS*/
    	}
    }
    
    /* execute the command */
#ifdef VMS

    cmdDesc = NulStrToDesc(command);		/* build command descriptor */
    dirOutFile = histoTmpnam(dirOF);		/* get temporary filename   */
    spawn_sts = lib$spawn(cmdDesc, 0, dirOutFile, &spawnFlags, 0,0,0,0,0,0,0,0);
    if (spawn_sts != SS$_NORMAL) {
	fprintf(stderr, "Error return from lib$spawn: %d", spawn_sts);
	return False;
    }
    FreeStrDesc(dirOutFile);
    FreeStrDesc(cmdDesc);
    pipe = fopen(dirOF, "r");

#else
    pipe = popen(command,"r");
#endif /*VMS*/

    if (pipe == NULL) {
	fprintf(stderr, "Failed executing popen\n"); 
	return False;
    }
    
    /* read output from remote ls command then close pipe */
    nRead = fread(buf, sizeof(char), MAX_ID_FILE_SIZE-1, pipe);
    if (!feof(pipe)) {
#ifdef VMS
	remove(dirOF);
#endif /*VMS*/
    	return False; /* file is larger than a histoscope file */
    }
    if (ferror(pipe)) {
    	fprintf(stderr, "Error reading from remote machine\n");
#ifdef VMS
	remove(dirOF);
#endif /*VMS*/
    	return False;
    }
    buf[nRead] = '\0';
#ifdef VMS
    remove(dirOF);
    if (fclose(pipe))
#else
    if (pclose(pipe))
#endif /*VMS*/
	return False;  /* couldn't cat the remote file */
    
    /* read in the match key, the pid and the socket number... */
    if (unixFlag)				/* unix */
    	p = buf;
    else {					/* VMS, skip login stuff */
	sprintf(fileMatchStr, "%d", FILE_MATCH_KEY);
    	p = strstr(buf, fileMatchStr);
	if (p == NULL)
	    return False;
    }
    
    fileData->node[0] = '\0';		/* allow upward compatability from V1 */
    nRead = sscanf(p, "%d %d %d %s \"%[^\"]\" \"%[^\"]\" %s", &key,
    		   &fileData->pid, &fileData->socketNum, fileData->userName,
    		   fileData->idString, fileData->startTime, fileData->node);
    if (nRead != 6 && nRead != 7)	/* allow V1 files too */
    	return False;	/* reject files not in correct format */

    /* reject files which don't match key (therefore not created by us) */
    if (key != FILE_MATCH_KEY)
    	return False;
    
    return True;
}

static int rmtListCmd(char *command, char *buf, int *nRead, char *errorMessage)
{
    FILE *pipe;
    
#ifdef VMS
    int spawnFlags = 2;			/* NOCLISYM */
    int spawn_sts;
    struct dsc$descriptor_s *cmdDesc;
    struct dsc$descriptor_s *dirOutFile;
    char dirOF[MAXPATHLEN+1];

    cmdDesc = NulStrToDesc(command);		/* build command descriptor */
    dirOutFile = histoTmpnam(dirOF);		/* get temporary filename   */
    spawn_sts = lib$spawn(cmdDesc, 0, dirOutFile, &spawnFlags, 0,0,0,0,0,0,0,0);
    if (spawn_sts != SS$_NORMAL) {
	fprintf(stderr, "Error return from lib$spawn: %d\n%s\n", spawn_sts,
		strerror(EVMSERR, spawn_sts));
	strcpy(errorMessage, "Error spawning rsh command\n");
	strcat(errorMessage, strerror(EVMSERR, spawn_sts));
	return -1;
    }
    FreeStrDesc(dirOutFile);
    FreeStrDesc(cmdDesc);
    pipe = fopen(dirOF, "r");
#else
    pipe = popen(command,"r");
#endif /*VMS*/

    if (pipe == NULL) {
	strcpy(errorMessage, strerror(errno)); 
	return -1;
    }
    
    /* read output from remote ls command then close pipe */
    *nRead = fread(buf, sizeof(char), REMOTE_LS_BUF_SIZE-1, pipe);
    if (!feof(pipe)) {
    	strcpy(errorMessage, "Buffer overflow reading from remote machine");
    	return -1;
    }
    if (ferror(pipe)) {
    	strcpy(errorMessage, "Error reading output from remote machine");
    	return -1;
    }
    /* if the rsh command terminated unsuccessfully, get
       the error message from the output stream of the command */
    buf[*nRead] = '\0';
#ifdef VMS
    remove(dirOF);
    if (fclose(pipe)) {
#else
    if (pclose(pipe)) {
#endif /*VMS*/
    	strncpy(errorMessage, buf, MAX_LIST_ERROR_LEN);
	return -1;
    }
    return 0;
}

#ifdef VMS
/* compose a Multinet rsh command to execute on a VMS machine */
static char *makeVMSrshCmd(char *command, char *userName, char *passwd,
			   char *remoteHostName, char *execStr)
{
    strcpy(command, "rsh");
    if (userName != NULL && userName[0] != '\0') {
	strcat(command, "/username=");
	strcat(command, userName);
    }
    if (passwd != NULL && passwd[0] != '\0') {
	strcat(command, "/password=");
	strcat(command, passwd);
    }
    strcat(command, " ");
    strcat(command, remoteHostName);
    strcat(command, " \"");
    strcat(command, execStr);
    strcat(command, "\"");
#ifdef COM_DEBUG
    printf("VMS rsh command = %s\n", command);
#endif /*COM_DEBUG*/
}

#else /*VMS*/
/* Compose an rsh command to execute on a unix machine */
/* Note: Unix does not allow a password to be specified      */
static void makeUNIXrshCmd(char *command, char *userName, char *remoteHostName,
			   char *execStr)
{
    strcpy(command, "ssh ");
    strcat(command, remoteHostName);
    if (userName != NULL && userName[0] != '\0') {
	strcat(command, " -l ");
	strcat(command, userName);
    }
    strcat(command, " ");
    strcat(command, execStr);
#ifdef COM_DEBUG
    printf("UNIX rsh command = %s\n", command);
#endif
}
#endif /*VMS*/

#ifdef VMS
/* procAlive: see if a process (identified by pID) is still alive on VMS.
   Returns:  1 - process exists
	     0 - process does not exist
	    -1 - error getting process info
*/
static int procAlive(int pID)
{
    int jpiStat;
    short retLen;
    char userName[13];			/* 12 plus 1 for ending null */
    struct getJPIdescriptor {
	short bufLength;
	short itemCode;
	int  *bufAddr;
	int  *retLenAddr;
	int  *endList;
    } getJPID;

    getJPID.bufLength  = 12;		/* (max) size of user name */
    getJPID.itemCode   = JPI$_USERNAME;
    getJPID.bufAddr    = userName;
    getJPID.retLenAddr = &retLen;
    getJPID.endList    = 0;
    jpiStat = sys$getjpiw(1,&pID,0,&getJPID,0,0,0);
    if (jpiStat == SS$_NORMAL || jpiStat == SS$_NOPRIV 
		|| jpiStat == SS$_SUSPENDED)
	return 1;			/* process exists	  */
    if (jpiStat == SS$_NONEXPR)
	return 0;			/* process does not exist */
    fprintf(stderr, "histoscope: Error calling GETJPI.  Status = %d\n",
	    jpiStat);
    return -1;				/* error		  */
}


/*
** Construct a temporary VMS filename and return a VMS descriptor of it.
** This descriptor must later be free'd by FreeStrDesc.  If the argument
** is non-null, the null-terminated filename will be copied into the string
** address passed.
*/
static struct dsc$descriptor_s *histoTmpnam(char *tmpName)
{
    char *name;
    char nam[MAXPATHLEN+1];

    if (tmpName == NULL)
	name = nam;
    else
	name = tmpName;
    strcpy(name, "HISTO");
    tmpnam(name+5);				/* name returned in place */
    strcat(name, ".TMP");
    return NulStrToDesc(name);
}

#endif /*VMS*/
#endif /*VXWORKS*/
