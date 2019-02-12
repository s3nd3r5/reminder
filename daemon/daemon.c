#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../lib/reminder.h"

struct remnode
{
  long fileptr;
  struct reminder* reminder;
  struct remnode* prev;
  struct remnode* next;
};

FILE* reminder_file = NULL;

struct reminder * split_line(char* line)
{
  char flag = line[0];

  char cline[255];
  strcpy(cline, line);
  
  char* tok;
  
  // Two toks to get the [F] [TS] [MSG]
  strtok(cline, " ");
  tok = strtok(NULL, " ");

  time_t line_time = atoi(tok);
  struct reminder *rem = malloc(sizeof(struct reminder));
  rem->message = cline;
  rem->time = line_time;
  rem->flag = flag;
  return rem;
}

struct remnode * make_remnode(long pos, char* line)
{
  char cline[255];
  strcpy(cline, line);

  struct reminder* rem = split_line(cline);
  struct remnode * node = malloc(sizeof(struct remnode));
  node->fileptr = pos;
  node->reminder = rem;

  return node; 
}

// returns the head of the list
struct remnode * insert_rem(struct remnode * node, struct remnode * head)
{
  struct remnode * curr = head;
  do
  {
    if (difftime(curr->reminder->time, curr->reminder->time) >= 0)
    {
      node->next = curr;
      return node;
    }
  } while (curr->next != NULL);

  curr->next = node;
  return head;
}

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
      "%c %llf %s", 
      RFLAG_DONE, node->reminder->time, node->reminder->message);
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
 
  struct remnode * head;

  if (reminder_file != NULL)
  {
    char line[255];
    // scan the file
    long pos = ftell(reminder_file);
    while(fgets(line, sizeof(line), reminder_file))
    {
      char flag = line[0];
      if (flag == RFLAG_NEW)
      {
        
        struct remnode* node = make_remnode(pos, line);
        if (head == NULL)
        {
          head = node;
        }
        else
        {
          insert_rem(node, head);
        }
      }
      pos = ftell(reminder_file);
    }
    // main daemon loop
    long tailstart = pos;
    int loop = 1;
    do
    { 
    
      struct remnode* curr = head;
      while(curr != NULL && do_notify(curr->reminder))
      {
        mark_line(curr);
        
        // capture current in tmp object to free
        struct remnode* tmp = curr;
       
        // move to next
        curr = curr->next;
      
        // free 
        free(tmp->reminder->message);
        free(tmp->reminder);
        free(tmp);
      }
      //FIXME - tail file
    } while(loop);

    // close the file and shutdown
    fclose(reminder_file);
  }
  else
  {
    // no file to tail
    printf("File at path: %s does not exist", filepath);
    return 1;
  }
  printf("quitting");
  return 0;
}
