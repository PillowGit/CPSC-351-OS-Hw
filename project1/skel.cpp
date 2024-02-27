#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

/* The pipe for parent-to-child communications */
int parentToChildPipe[2];

/* The pipe for the child-to-parent communication */
int childToParentPipe[2];

/* The read end of the pipe */
#define READ_END 0

/* The write end of the pipe */
#define WRITE_END 1

/* The maximum size of the array of hash programs */
#define HASH_PROG_ARRAY_SIZE 6

/* The maximum length of the hash value */
#define HASH_VALUE_LENGTH 1000

/* The maximum length of the file name */
#define MAX_FILE_NAME_LENGTH 1000

/* The array of names of hash programs */
const string hashProgs[] = {"md5sum",    "sha1sum",   "sha224sum",
                            "sha256sum", "sha384sum", "sha512sum"};

string fileName;

/**
 * The function called by a child
 * @param hashProgName - the name of the hash program
 */
void computeHash(const string &hashProgName) {

  /* The hash value buffer */
  char hashValue[HASH_VALUE_LENGTH];

  /* Reset the value buffer */
  memset(hashValue, (char)NULL, HASH_VALUE_LENGTH);

  /* The received file name string */
  char fileNameRecv[MAX_FILE_NAME_LENGTH];

  /* Fill the buffer with 0's */
  memset(fileNameRecv, (char)NULL, MAX_FILE_NAME_LENGTH);

  /** TODO: Now, lets read a message from the parent **/
  // tldr; We are reading the file name from the parent process through the
  // pipe. This is honestly a really weird process if you've never seen it done,
  // so read the over TODO #2 in parentFunc for more info. The exact steps are
  // explained there, and we are doing the same thing here, just with a
  // different buffer and pipe.
  ssize_t readAmount =
      read(parentToChildPipe[READ_END], fileNameRecv, MAX_FILE_NAME_LENGTH);
  if (readAmount < 0) {
    perror("read");
    exit(-1);
  }
  fileNameRecv[readAmount] = '\0';

  /* Glue together a command line <PROGRAM NAME>.
   * For example, sha512sum fileName.
   */
  string cmdLine(hashProgName);
  cmdLine += " ";
  cmdLine += fileNameRecv;

  /* TODO: Open the pipe to the program (specified in cmdLine)
   * Once again, there's a lot going on so I'll explain things as they occur:
   * 1. We are using the popen() function to open a pipe to the hash function we
   * want to use
   * 2. If the pipe fails to open, we print an error message and exit the
   * program.
   * 3. We read the hash value from the pipe and store it in the hashValue. If
   * it returns a value less than 0, we print an error message and exit the
   * program.
   * 4. We close the pipe and check if it closed properly. If it didn't, we
   * again print an error message and exit the program.
   * 5. Finally, we ensure the hash value is properly terminated with a null
   * string terminator
   */
  FILE *hashOutput = popen(cmdLine.c_str(), "r");
  if (!hashOutput) {
    perror("popen");
    exit(-1);
  }
  if (fread(hashValue, sizeof(char), sizeof(char) * HASH_VALUE_LENGTH,
            hashOutput) < 0) {
    perror("fread");
    exit(-1);
  }
  if (pclose(hashOutput) < 0) {
    perror("pclose");
    exit(-1);
  }
  hashValue[strlen(hashValue) - 1] = '\0';

  /* TODO: Send a string to the parent
   * tldr; we are sending the hash value to the parent process through the pipe.
   * Read first todo in parentFunc for more info. The steps are the same, just
   * sending the hash value instead of the file name.
   */
  write(childToParentPipe[WRITE_END], hashValue, strlen(hashValue) + 1);

  /* The child terminates */
  exit(0);
}

void parentFunc(const string &hashProgName) {

  /* I am the parent */

  /** TODO: close the unused ends of two pipes. **/
  // tldr; The parent has no use for the some of the components of the pipe. We
  // are using the close() function to make sure that this causes no problems.
  // For more info read the 2nd tldr in the loop in main. The pipe ends are not
  // the same, but the idea is reflected simliarly.
  close(parentToChildPipe[READ_END]);
  close(childToParentPipe[WRITE_END]);

  /* The buffer to hold the string received from the child */
  char hashValue[HASH_VALUE_LENGTH];

  /* Reset the hash buffer */
  memset(hashValue, (char)NULL, HASH_VALUE_LENGTH);

  /* TODO: Send the string to the child
   * tldr; we are sending the file name to the child process through the pipe.
   * to do this, we're using the write() function that comes with pipes. It's
   * arguments are:
   * (int pipe_end, const void *buf, size_t count)
   * Translating from C to sane English, the pipe_end is which to write to
   * (either the read end or the write end). buf is the string we are writing,
   * called a "string buffer". Finally, count is the length of the string. We
   * have + 1 so it includes the string null terminator '\0'.
   */
  write(parentToChildPipe[WRITE_END], fileName.c_str(), fileName.length() + 1);

  /* TODO: Read the string sent by the child
   * tldr; we are reading the hash value from the child process through the
   * pipe. Here's the steps in the order they occur:
   * 1. Call the read() function on our pipe, which returns the number of bytes
   * we read from the child, along with putting it into the buffer hashValue
   * that was made for us above to store the hashValue result from the child.
   * 2. Ensure that we read the result from the child properly. We do this by
   * checking the number of bytes we were able to read from the pipe. If this is
   * less than one (typically -1 indicating an error) then we print an error
   * message and exit the program.
   * 3. If all went well, we add a null terminator to the end of the string to
   * ensure string safety and finally print the hash value.
   */
  ssize_t readAmount =
      read(childToParentPipe[READ_END], hashValue, HASH_VALUE_LENGTH);
  if (readAmount < 0) {
    perror("read");
    exit(-1);
  }
  hashValue[readAmount] = '\0';

  /* Print the hash value */
  fprintf(stdout, "%s HASH VALUE: %s\n", hashProgName.c_str(), hashValue);
  fflush(stdout);
}

int main(int argc, char **argv) {

  /* Check for errors */
  if (argc < 2) {
    fprintf(stderr, "USAGE: %s <FILE NAME>\n", argv[0]);
    exit(-1);
  }

  /* Save the name of the file */
  fileName = argv[1];

  /* The process id */
  pid_t pid;

  /* Run a program for each type of hashing algorithm hash algorithm */
  for (int hashAlgNum = 0; hashAlgNum < HASH_PROG_ARRAY_SIZE; ++hashAlgNum) {

    /** TODO: create two pipes **/
    // tldr; we are using a function called pipe() to convert a int[2] into a
    // pipe. This allows communication. Of course this is C, so there's big if
    // statements to make sure that we properly handle a pipe not opening. But
    // that's whats happening here.
    if (pipe(parentToChildPipe) < 0 || pipe(childToParentPipe) < 0) {
      perror("pipe");
      exit(-1);
    }

    /* Fork a child process and save the id */
    if ((pid = fork()) < 0) {
      perror("fork");
      exit(-1);
    }
    /* I am a child */
    else if (pid == 0)

    {
      /** TODO: close the unused ends of two pipes **/
      // tldr; The child has no use for the some of the components of the pipe.
      // For example, it needs to read from parent and write to parent, but it
      // does not need to read from child or write to child. This code is to
      // close those unused ends of the pipe if we are the child process.
      close(parentToChildPipe[WRITE_END]);
      close(childToParentPipe[READ_END]);

      /* Compute the hash */
      computeHash(hashProgs[hashAlgNum]);
    }

    else {

      parentFunc(hashProgs[hashAlgNum]);

      /* Wait for the child to terminate */
      if (wait(NULL) < 0) {
        perror("wait");
        exit(-1);
      }
    }
  }

  return 0;
}
