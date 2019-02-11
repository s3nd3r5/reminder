#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../lib/reminder.h"

FILE* reminder_file;

void notify(char* line)
{
  char cline[255];
  strcpy(cline, line);
  char *tok;

  strtok(cline, " ");
  strtok(NULL, " ");
  tok = strtok(NULL, " ");
  
  printf("Hey! %s", tok);
}

void mark_line(char* line, char flag)
{
  line[0] = flag;
}

int do_notify(char* line)
{
  char cline[255];
  strcpy(cline, line);
  
  char* tok;
  
  // Two toks to get the [F] [TS] [MSG]
  strtok(cline, " ");
  tok = strtok(NULL, " ");

  time_t line_time = atoi(tok);
  time_t now = time(NULL);
  return difftime(line_time, now);
}

void int_handler(int code)
{
  printf("Daemon interrupted: %d", code);
  if (reminder_file != NULL)
  {
    fclose(reminder_file);
  }
}

int main(int argc, char* argv[])
{
  signal(SIGINT, int_handler);
  
  char* filepath = get_filepath();
  reminder_file = fopen(filepath, "r+");

  if (reminder_file != NULL)
  {
    char line[255];
    for (;;)
    {
      // scan the file
      long pos = ftell(reminder_file);
      while(fgets(line, sizeof(line), reminder_file))
      {
        char flag = line[0];
        if (flag == RFLAG_NEW)
        {
          // check timestamp
          if (do_notify(line) <= 0)
          {
            // cleanup all the existing notifications
            notify(line);
            mark_line(line, RFLAG_DONE);
            fseek(reminder_file, pos, SEEK_SET);
            fprintf(reminder_file, "%s", line);
          }
        }
        pos = ftell(reminder_file);
      }
      fseek(reminder_file, 0, SEEK_SET);
    }
    // close the file and shutdown
    fclose(reminder_file);
  }
  else
  {
    // no file to tail
    printf("File at path: %s does not exist", filepath);
    return 1;
  }

  return 0;
}
