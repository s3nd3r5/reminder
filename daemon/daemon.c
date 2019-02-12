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
struct remnode * insert_rem(struct remnode * node, struct remnode * head)
{
  // node is new head
  if (difftime(head->reminder->time, node->reminder->time) >= 0)
  {
    node->next = head;
    return node; // node will be new head
  }
  struct remnode * prev = NULL;
  struct remnode * curr = head;
  while(curr->next != NULL)
  {
    prev = curr;
    curr = curr->next;

    if (difftime(curr->reminder->time, node->reminder->time) >= 0)
    {
      prev->next = node;
      node->next = curr;
      return head; 
    }
  }
  curr->next = node;
  return head;
}

void notify(char* message)
{
  printf("Hey! %s\n", message);
}

int do_notify(struct reminder * rem)
{
  time_t now = time(NULL);
  return difftime(rem->time, now);
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
  struct remnode * head;
  unsigned int length = 0;
  if (reminder_file != NULL)
  {
    char line[255];
    // scan the file
    long pos = ftell(reminder_file);
    printf("Queuing up existing reminders\n");
    while(fgets(line, sizeof(line), reminder_file))
    {
      char flag = line[0];
      if (flag == RFLAG_NEW)
      {
        
        struct remnode* node = make_remnode(pos, line);
        if (head == NULL)
        {
          printf("insert head: %d %s", length, line);
          head = node;
        }
        else
        {
          insert_rem(node, head);
          printf("insert node: %d %s", length, line);
        }
       ++length; 
      } 
      pos = ftell(reminder_file);
    }
    printf("%u reminders queued\n", length); 
    // main daemon loop
    long tailstart = pos;
    int loop = 1;
    printf("Beginnig Daemon Loop\n");  
    do
    { 
      if(head != NULL && do_notify(head->reminder))
      {
        mark_line(head);
         
        // capture current in tmp object to free
        struct remnode* tmp = head;
         
        // move to next
        head = head->next;
        // free 
        free(tmp->reminder->message);
        free(tmp->reminder);
        free(tmp); 
      }
      printf("sleeping\n");
      sleep(1);
      //FIXME - tail file
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
