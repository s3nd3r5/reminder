#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../lib/reminder.h"

struct remnode
{
  long fileptr;
  struct reminder* reminder;
  struct remnode* next;
};

FILE* reminder_file;

struct reminder * split_line(char* line)
{
  char flag = line[0];

  char cline[255];
  strcpy(cline, line);
  
  strtok(cline, " "); // skip flag
  char * timestr = strtok(NULL, " ");
  time_t line_time = atoi(timestr);
  
  char * message = strtok(NULL, " "); 
  char * cmsg = (char *) malloc(sizeof(char) * 255);
  strcpy(cmsg, message);

  struct reminder * rem = (struct reminder *) malloc(sizeof(struct reminder));
  rem->message = cmsg;
  rem->time = line_time;
  rem->flag = flag;
  return rem;
}

struct remnode * make_remnode(long pos, char* line)
{
  char cline[255];
  strcpy(cline, line);

  struct reminder* rem = split_line(cline);
  struct remnode* node = (struct remnode *)malloc(sizeof(struct remnode));
  node->fileptr = pos;
  node->reminder = rem;
  return node; 
}

// returns the head of the list
void insert_rem(struct remnode * node, struct remnode ** headptr)
{
  struct remnode * curr = *headptr;
  if (curr == NULL)
  {
    *headptr = node;
    printf("inserting node at current position\n");
  }
  else if (difftime(node->reminder->time, curr->reminder->time) < 0)
  {
    node->next = curr;
    *headptr = node;
    printf("inserting node as new head\n");
  }
  else
  {
    printf("moving into list\n");
    insert_rem(node, &curr->next);
  }  
}

void notify(char* message)
{
  printf("Hey! %s\n", message);
}

int do_notify(struct reminder * rem)
{
  time_t now = time(NULL);
  return difftime(rem->time, now) <= 0;
}

void mark_line(struct remnode * node)
{
  // cleanup all the existing notifications
  notify(node->reminder->message);
  fseek(reminder_file, node->fileptr, SEEK_SET);
  fprintf(
      reminder_file,
      "%c %lld %s", 
      RFLAG_DONE, node->reminder->time, node->reminder->message);
  fflush(reminder_file);
}

void traverse_file(long *pos, struct remnode** headptr)
{
  char line[255];
  while(fgets(line, sizeof(line), reminder_file))
  {
    char flag = line[0];
    if (flag == RFLAG_NEW)
    {
      printf("found new line\n");
      struct remnode* node = make_remnode(*pos, line);
      printf("Insertring line: %s", line);
      insert_rem(node, headptr);
      printf("Line inserted\n");
    } 
    *pos = ftell(reminder_file);
  }
}

void int_handler(int code)
{
  printf("Daemon interrupted: %d\n", code);
  if (reminder_file != NULL)
  {
    fclose(reminder_file);
  }
  exit(code);
}

int main(int argc, char* argv[])
{
  printf("Starting daemon process\n");
  signal(SIGINT, int_handler);
  
  char* filepath = get_filepath();
  reminder_file = fopen(filepath, "r+");
  printf("Reading from file: %s\n", filepath);
  
  if (reminder_file != NULL)
  {
    struct remnode * head = NULL;
    // init the scan 
    long pos = ftell(reminder_file);
    printf("Queuing up existing reminders\n");
    traverse_file(&pos, &head); // inital scan of the file
    
    // main daemon loop
    int loop = 1;
    printf("Beginnig Daemon Loop\n");  
    do
    {
      if(head != NULL && do_notify(head->reminder))
      {
        mark_line(head);
        
        struct remnode * tmp = head; 
        
        if (head->next)
        {
          head = head->next;
        }
        else
        {
          head = NULL;
        }
        
        
        // free 
        free(tmp->reminder->message);
        free(tmp->reminder);
        free(tmp); 
      }
      
      // return to where we left off and continue chekc for new reminders 
      fseek(reminder_file, pos, SEEK_SET);
    
      pos = ftell(reminder_file);
      traverse_file(&pos, &head);
      
      printf("sleeping\n");
      sleep(1);
    } while(loop);
    
    // close the file and shutdown
    fclose(reminder_file);
  }
  else
  {
    // no file to tail
    printf("File at path: %s does not exist\n", filepath);
    return 1;
  }
  printf("quitting\n");
  return 0;
}

