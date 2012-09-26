/*
 *  authtool.c
 *  VirtueDesktops
 *
 *  Created by Tony on 14/06/06.
 *  Copyright 2006 boomBalada! Productions. All rights reserved.
 *
 */
/*
 *  edit by iptton#gmail.com
 * 
 */

#include <Security/AuthorizationTags.h>
//#include <SystemConfiguration/SystemConfiguration.h>
//#include <CoreFoundation/CoreFoundation.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <mach-o/dyld.h>
#include <Security/Authorization.h>
#include <sys/param.h>
#define DEBUG
#if defined(DEBUG)
# define IFDEBUG(code)    code
#else
# define IFDEBUG(code)    /* no-op */
#endif


// Command structure
typedef struct MyAuthorizedCommand
{
  // Arguments to operate on
  char file[MAXPATHLEN];
} MyAuthorizedCommand;


// Exit codes (positive values) and return codes from exec function
enum
{
  kMyAuthorizedCommandInternalError = -1,
  kMyAuthorizedCommandSuccess = 0,
  kMyAuthorizedCommandExecFailed,
  kMyAuthorizedCommandChildError,
  kMyAuthorizedCommandAuthFailed,
  kMyAuthorizedCommandOperationFailed
};

/*

Boolean setProxy(){
    SCDynamicStoreRef store = SCDynamicStoreCreate(kCFAllocatorSystemDefault, CFSTR("rythem"), NULL, NULL);


  SCDynamicStoreRef dynRef=SCDynamicStoreCreate(kCFAllocatorSystemDefault, CFSTR("iked"), NULL, NULL);
  CFDictionaryRef ipv4key = (CFDictionaryRef)SCDynamicStoreCopyValue(dynRef,CFSTR("State:/Network/Global/IPv4"));
  CFStringRef primaryserviceid = (CFStringRef)CFDictionaryGetValue(ipv4key,CFSTR("PrimaryService"));
  //Setup:/Network/Service/ServiceID/Proxies

  CFStringRef primaryservicepath = (CFStringRef)CFStringCreateWithFormat(NULL,NULL,CFSTR("Setup:/Network/Service/%@/Proxies"),primaryserviceid);

  //State:/Network/Service/CB3E440C-8D1B-4752-BBB1-E5F031C8D0C8/Proxies
  CFMutableDictionaryRef newdnskey =  CFDictionaryCreateMutable(NULL, 0, NULL,NULL);
  CFDictionarySetValue(newdnskey,CFSTR("HTTPProxy"),CFSTR("127.9.9.1"));
  CFDictionarySetValue(newdnskey,CFSTR("HTTPSProxy"),CFSTR("127.9.9.1"));
  CFShow(newdnskey);
  if(SCDynamicStoreSetValue(dynRef, primaryserviceid, newdnskey)){
    printf("success.....\n");
  }
  CFRelease(dynRef);
  int n=0;
  bool k = SCDynamicStoreSetValue(store, primaryservicepath , newdnskey);
  CFRelease(store);
  return k;
}
void setDNS(){
  //get current values
  SCDynamicStoreRef dynRef=SCDynamicStoreCreate(kCFAllocatorSystemDefault, CFSTR("iked"), NULL, NULL);
  CFDictionaryRef ipv4key = (CFDictionaryRef)SCDynamicStoreCopyValue(dynRef,CFSTR("State:/Network/Global/IPv4"));
  CFStringRef primaryserviceid = (CFStringRef)CFDictionaryGetValue(ipv4key,CFSTR("PrimaryService"));
  CFStringRef primaryservicepath = (CFStringRef)CFStringCreateWithFormat(NULL,NULL,CFSTR("State:/Network/Service/%@/DNS"),primaryserviceid);
  CFDictionaryRef dnskey = (CFDictionaryRef)SCDynamicStoreCopyValue(dynRef,primaryservicepath);

  //create new values
  CFMutableDictionaryRef newdnskey = CFDictionaryCreateMutableCopy(NULL,0,dnskey);
  CFDictionarySetValue(newdnskey,CFSTR("DomainName"),CFSTR("iptton.com"));

  CFMutableArrayRef dnsserveraddresses = CFArrayCreateMutable(NULL,0,NULL);
  CFArrayAppendValue(dnsserveraddresses, CFSTR("8.8.8.8"));
  CFArrayAppendValue(dnsserveraddresses, CFSTR("4.3.2.2"));
  CFDictionarySetValue(newdnskey, CFSTR("ServerAddresses"), dnsserveraddresses);

  //set values
  bool success = SCDynamicStoreSetValue(dynRef, primaryservicepath, newdnskey);
  if(success){
    printf("set DNS success\n");
  }else{
    printf("set DNS FAIL\n");
  }
  //clean up
  CFRelease(dynRef);
  CFRelease(primaryservicepath);
  CFRelease(dnskey);
  CFRelease(dnsserveraddresses);
  CFRelease(newdnskey);
}
*/

int runSelfByPriviledge(char *path_to_self){
  AuthorizationRef authorizationRef;
  AuthorizationItem right = { kAuthorizationRightExecute, 0, NULL, 0 };
  AuthorizationRights rightSet = { 1, &right };
  OSStatus status;
  AuthorizationFlags flags = kAuthorizationFlagDefaults | kAuthorizationFlagPreAuthorize | kAuthorizationFlagInteractionAllowed | kAuthorizationFlagExtendRights;
  
  
  /* Create a new authorization reference which will later be passed to the tool. */
  
  status = AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, &authorizationRef);
  
  if (status != errAuthorizationSuccess)
  {
    IFDEBUG(fprintf(stderr, "Failed to create the authref: %ld.\n", status));
    return kMyAuthorizedCommandInternalError;
  }
  
  /* This shows how AuthorizationCopyRights() can be used in order to pre-authorize the user before attempting to perform the privileged operation.  Pre-authorization is optional but can be useful in certain situations.  For example, in the Installer application, the user is asked to pre-authorize before configuring the installation because it would be a waste of time to let the user proceed through the entire installation setup, only to be denied at the final stage because they weren't the administrator. */
  
  status = AuthorizationCopyRights(authorizationRef, &rightSet, kAuthorizationEmptyEnvironment, flags, NULL);
  
  if (status == errAuthorizationSuccess)
  {

    int status;
    int pid;
    FILE *commPipe = NULL;
    char *arguments[] = { "--self-repair", NULL };
    char buffer[1024];
    
    /* Set our own stdin and stderr to be the communication channel with ourself. */
    
    IFDEBUG(fprintf(stderr, "Tool about to self-exec through AuthorizationExecuteWithPrivileges.\n");)
    if (AuthorizationExecuteWithPrivileges(authorizationRef, path_to_self, kAuthorizationFlagDefaults, arguments, &commPipe))
      return (kMyAuthorizedCommandInternalError);

    /* Flush any remaining output. */
    fflush(commPipe);
    
    /* Close the communication pipe to let the child know we are done. */
    fclose(commPipe);
    
    /* Wait for the child of AuthorizationExecuteWithPrivileges to exit. */
    pid = wait(&status);
    fprintf(stderr,"child thread done %d %d \n",pid,status);
    if (pid == -1 /*|| ! WIFEXITED(status)*/)
      return kMyAuthorizedCommandInternalError;
    fprintf(stderr,"child thread done success\n");
    /* Exit with the same exit code as the child spawned by AuthorizationExecuteWithPrivileges() */
    //exit(WEXITSTATUS(status));
    return kMyAuthorizedCommandSuccess;
  }

  return 1;
}

int setpac(char *serviceName,char *pac){

        //int comms[2] = {};
        //pipe(comms);
        pid_t pid;
        int status;
        int childStatus = 0;
        int retcode;
        switch(pid = fork())
        {
          case 0: /* Child */
          {            
            char *const envp[] = { NULL };

            //dup2(comms[1], 1);
            //close(comms[0]);
            //close(comms[1]);
            //fprintf(stderr,"is child %s\n",path_to_self);
            if (pac == 0){
              retcode = execle("/usr/sbin/networksetup", "/usr/sbin/networksetup","-setautoproxystate",serviceName,"off",NULL,envp);
            }else{
              retcode = execle("/usr/sbin/networksetup", "/usr/sbin/networksetup","-setautoproxyurl",serviceName,pac,NULL,envp);
            }
            wait(&status);
            fprintf(stderr,"child exited..%d \n",retcode);
            _exit(0);
          }
          case -1: /* an error occured */
            //close(comms[0]);
            //close(comms[1]);
            return kMyAuthorizedCommandInternalError;
          default: /* Parent */
            break;
        }
        printf("isparent");
        /* Parent */
        /* Don't abort the program if write fails. */
        //signal(SIGPIPE, SIG_IGN);
        
        /* Wait for the tool to return */
        int wp = waitpid(pid, &childStatus, 0);
        return 0;

}
int main(int argc, char* const argv[])
{
  OSStatus status;
  AuthorizationRef auth;
  
  uint32_t path_to_self_size = 0;
  char *path_to_self = NULL;
  
  printf("called authtool..\n");
  path_to_self_size = MAXPATHLEN;
  if (! (path_to_self = (char*)malloc(path_to_self_size))){
    fprintf(stderr,"cannot alloc for path_to_self %s %s\n",argv[0],argv[1]);
    exit(kMyAuthorizedCommandInternalError);
  }
  if (_NSGetExecutablePath(path_to_self, &path_to_self_size) == -1)
  {
    /* Try again with actual size */
    if (! (path_to_self = (char*)realloc(path_to_self, path_to_self_size + 1)))
      exit(kMyAuthorizedCommandInternalError);
    if (_NSGetExecutablePath(path_to_self, &path_to_self_size) != 0)
      exit(kMyAuthorizedCommandInternalError);
  }

  if(argc < 2){
    fprintf(stderr,"need arguments\n");
    return 1;
  }
  fprintf(stderr,"called:%s\n",argv[1]);

  /* If we are not running as root we need to self-repair. */
  if (geteuid() != 0)
  {
      if(kMyAuthorizedCommandSuccess == runSelfByPriviledge(path_to_self)){
        fprintf(stderr,"relaunching\n");
        execv(path_to_self,argv);
        //system("/Users/pxz/Documents/cproject/authtool/authtool --setwebproxy");
        return 0;
      }else{
        fprintf(stderr,"fail");
        return 1;
      }
  }

  if (path_to_self)
    free(path_to_self);

  if (argc >= 2 && !strcmp(argv[1], "--self-repair"))
  {
    /*  Self repair code.  We ran ourselves using AuthorizationExecuteWithPrivileges()
    so we need to make ourselves setuid root to avoid the need for this the next time around. */
    
    struct stat st;
    int fd_tool;
    
    /* Recover the passed in AuthorizationRef. */
    if (AuthorizationCopyPrivilegedReference(&auth, kAuthorizationFlagDefaults))
      exit(kMyAuthorizedCommandInternalError);
    
    /* Open tool exclusively, so noone can change it while we bless it */
    fd_tool = open(path_to_self, O_NONBLOCK|O_RDONLY|O_EXLOCK, 0);
    
    if (fd_tool == -1)
    {
      IFDEBUG(fprintf(stderr, "Exclusive open while repairing tool failed: %d.\n", errno);)
      exit(kMyAuthorizedCommandInternalError);
    }
    
    if (fstat(fd_tool, &st))
      exit(kMyAuthorizedCommandInternalError);
    
    if (st.st_uid != 0)
      fchown(fd_tool, 0, st.st_gid);
    
    /* Disable group and world writability and make setuid root. */
    fchmod(fd_tool, (st.st_mode & (~(S_IWGRP|S_IWOTH))) | S_ISUID);
    
    close(fd_tool);
    
    IFDEBUG(fprintf(stderr, "Tool self-repair done.\n");)
    return 0;
      
  }

  if(!strcmp(argv[1],"--setpac")){
    printf("--setpac %s\n",argv[2],argv[3]);
    return setpac(argv[2],argv[3]);

  }else if(!strcmp(argv[1],"--disablepac")){
    printf("--disablepac %s\n",argv[2]);
    return setpac(argv[2],0);
  }
}