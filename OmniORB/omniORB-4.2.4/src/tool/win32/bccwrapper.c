/*
 * bccwrapper.c - wrapper around Borland C++'s "bcc32" program to perform
 * GNU-WIN32 file name translation (from unix-like to dos-like names).
 *
 * Compile this program with "bcc bccwrapper.c advapi32.lib".
 */

#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <string.h>

#define MAX_MOUNTS 256

void GetMounts(void);
char *TranslateFileName(char *in, int offset);

int main(int argc, char **argv)
{
  int rc;
  int i;

  GetMounts();

  argv[0] = "bcc32";

  printf("bcc32");

  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {

      case 'I':
	argv[i] = TranslateFileName(argv[i],2);
	break;

      case 'o':
	argv[i] = TranslateFileName(argv[i],2);
	break;

      default:
	break;

      }

    } else {

      /* just a file argument */

      int len = strlen(argv[i]);

      argv[i] = TranslateFileName(argv[i],0);
    }

    printf(" %s", argv[i]);
  }

  printf("\n");

  rc = spawnvp(_P_WAIT,argv[0],argv);
  if (rc == -1) {
    fprintf(stderr,"%s: ",argv[0]);
    perror("");
    return 1;
  }
  return rc;
}


char *dos[MAX_MOUNTS];
char *unix[MAX_MOUNTS];
int nmounts;

char *TranslateFileName(char *in, int offset)
{
  int i;
  char *out = NULL;

  for (i = 0; i < nmounts; i++) {
    if (strncmp(unix[i], &in[offset], strlen(unix[i])) == 0) {
      out = malloc(strlen(in) - strlen(unix[i]) + strlen(dos[i]) + 1);
      strncpy(out, in, offset);
      strcpy(out + offset, dos[i]);
      strcat(out, &in[offset + strlen(unix[i])]);
      break;
    }
  }

  if (!out) {
    out = malloc(strlen(in) + 1);
    strcpy(out, in);
  }

  for (i = offset; i < strlen(out); i++) {
    if (out[i] == '/') {
      out[i] = '\\';
    }
  }

  return out;
}

void GetMounts(void)
{
  HKEY hkey;
  LONG rc;
  int len;
  char key[256];

  for (nmounts = 0; nmounts < MAX_MOUNTS; nmounts++) {
    sprintf(key,
	    "Software\\Cygnus Solutions\\CYGWIN.DLL setup\\b15.0\\mounts\\%02X"
	    ,nmounts);

    if (RegOpenKeyEx(HKEY_CURRENT_USER,
		     key, 0, KEY_READ, &hkey) != ERROR_SUCCESS) {
      if (nmounts == 0) {
	printf("Couldn't find registry key '%s' - RegOpenKey error %d\n",
	       key, GetLastError());
	exit(1);
      }
      break;
    }

    if (RegQueryValueEx(hkey,"unix",NULL,NULL,NULL,&len) != ERROR_SUCCESS) {
      printf("RegQueryValueEx failed - error %d\n",GetLastError());
      exit(1);
    }
    unix[nmounts] = (char *)malloc(len+1);
    RegQueryValueEx(hkey, "unix", NULL, NULL, unix[nmounts], &len);

    if (RegQueryValueEx(hkey,"native",NULL,NULL,NULL,&len) != ERROR_SUCCESS) {
      printf("RegQueryValueEx failed - error %d\n",GetLastError());
      exit(1);
    }
    if (strcmp(unix[nmounts], "/") == 0) {
      dos[nmounts] = (char *)malloc(len+2);
      RegQueryValueEx(hkey, "native", NULL, NULL, dos[nmounts], &len);
      dos[nmounts][len-1] = '\\';
      dos[nmounts][len] = 0;
    } else {
      dos[nmounts] = (char *)malloc(len+1);
      RegQueryValueEx(hkey, "native", NULL, NULL, dos[nmounts], &len);
    }
  }
}
