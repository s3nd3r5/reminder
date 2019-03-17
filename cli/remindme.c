#include <stdio.h>
#include <string.h>
#include <time.h> 
#include <stdlib.h>

#include "../lib/reminder.h"

const char* help = "\n"
"REMINDME\n"
"        remindme - Send yourself reminders at a specific time on one or more devices\n"
"\n"
"SYNOPSIS\n"
"       remindme [-t TIME] [--at TIME] [-i PERIOD] [--in PERIOD] message\n"
"       remindme [-l FLAG] [--list FLAG]\n"
"\n"
"DESCRIPTION\n"
"       remindme queues a message to be sent back to you at a give time or duration.\n"
"\n"
"OPTIONS\n"
"       -t, --at=TIME\n"
"              Sets the time to send the reminder. Time is expected to be in ISO 8601 format in local timezone\n"
"\n"
"      -i, --in=PERIOD\n"
"              Queues the reminder to be sent after a period of time.  Period format is [NUMBER][DHMS].\n"
"\n"
"              NUMBER - Period amount. Must be whole\n"
"              D - in N days\n"
"              H - in N hours\n"
"              M - in N minutes\n"
"              S - in N seconds\n"
"\n"
"       -l, --list=FLAG\n"
"              Lists your reminders, optionally by flag.\n"
"\n"
"EXAMPLES\n"
"              $ remindme --at 2038-01-17T17:00:00 \"Take your laptop home!\"\n"
"              $ remindme --in 30M \"Check if deployment succeeded.\"\n"
"              $ remindeme -l\n"
"                n 1029301391 remember to update manpage!\n"
"              $ remindme --list=m\n"
"                m 1049130139 Don't miss this reminder!\n"
"\n";

void print_help()
{
  printf(help);
}

struct options {
  char* time;
  char* period;
  char* message;
};

int write_reminder(struct reminder* rem)
{
  char* filepath = get_filepath();
  printf("Writing reminder to: %s\n", filepath);
  
  FILE* reminder_file;
  reminder_file = fopen(filepath, "ab+");
  
  if (reminder_file != NULL)
  {
    fprintf(reminder_file, "%c %lld %s\n", rem->flag, rem->time, rem->message);
    fclose(reminder_file);
    free(filepath);
    return 0;
  }
  else
  {
    printf("Unable to write to file: %s", filepath);
    free(filepath);
    return 1;
  }
}


void list(char flag)
{
  char* filepath = get_filepath();
  FILE* reminder_file;
  reminder_file = fopen(filepath, "r");
  if (reminder_file != NULL)
  {
    char line[255];
    while(fgets(line, sizeof(line), reminder_file))
    {
      if (flag == '\0' || line[0] == flag)
      {
        printf("%s", line);
      }
    }
  }
  else
  {
    printf("No reminders :)!");
  }
  fclose(reminder_file);
  free(filepath);
}

int validate_nonmsg_opts(int sflagc, int lflagc, int argc)
{
  if (sflagc > 1 || lflagc > 1 || sflagc + lflagc > 1)
  {
    printf("Invalid number of flags passed in\n");
    return 1;
  }
  if (argc < sflagc * 2 + lflagc) // prg sflag sflagv msg | argc = 4, sflagc * 2 = 4.
  {
    printf("Invalid combination of options and values in arguments\n");
    return 1;
  }
  return 0;
}

int adjusttime(char tmtype)
{
  switch (tmtype)
  {
    case 'D': case 'd':
      return DAYS_SEC;
    case 'M': case 'm':
      return MIN_SEC;
    case 'H': case 'h':
      return HOURS_SEC;
    default:
      return 1;
  } 
}

time_t make_time(struct options* opt)
{
  if (opt->time)
  {
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));
    printf("passed in time: %s \n", opt->time);
    strptime(opt->time, DATE_TIME_FORMAT, &tm);
    return  mktime(&tm);
  }
  else
  {
    int len = strlen(opt->period);
    char tmtype = opt->period[len-1];
   
    
    char cstr[255];
    strcpy(cstr, opt->period); // duplicate string for saftey
    char *nstr = strtok(cstr, "DMHSdmhs"); //tokenize
    int num = atoi(nstr);
    
    int adjust = num * adjusttime(tmtype);
    return time(NULL) + adjust;
  }
}

int main(int argc, char** argv)
{
  if (argc < 2)
  {
    print_help();
    return 0;
  }

  struct options opts = {};
  int sflagc = 0;
  int lflagc = 0;
  for (int i = 1; i < argc; i++)
  {
    // check single args
    char* arg = argv[i];

    // short form arguments 
    if (strcmp(TIME_FLAG, arg) == 0)
    {
      opts.time = argv[i+1];
      ++i;
      ++sflagc;
      continue;
    }
    else if (strcmp(PERIOD_FLAG, arg) == 0)
    {
      opts.period = argv[i+1];
      ++i;
      ++sflagc;
      continue;
    }
    else if (strcmp(LIST_FLAG, arg) == 0)
    {
      if (i+1 < argc)
      {
        char* flags = argv[i+1];
        char flag = flags[0];
        list(flag);
      }
      else
      {
        list('\0');
      }
      return 0;
    }
    else if (strcspn(argv[i], "-") == 0) // long form arguments
    {
      if (strcspn(argv[i], "=") == strlen(argv[i])) 
      {
        printf("Invalid argument: %s\n", argv[i]);
        return 1;
      }

      char* flag = strtok(argv[i], "=");
      char* value = strtok(NULL, "="); // strtok continues previous scan if NULL

      if (strcmp(TIME_FLAG_LONG, flag) == 0)
      {
        opts.time = value;
        ++lflagc;
        continue;
      }
      else if (strcmp(PERIOD_FLAG_LONG, flag) == 0)
      {
        opts.period = value;
        ++lflagc;
        continue;
      }
    }
    
    opts.message = argv[i];
  }
 

  // validate
  int err = validate_nonmsg_opts(sflagc, lflagc, argc); 

  if (err > 0) 
  {
    return err;
  }
  
  if (opts.period && opts.time) 
  {
    printf("You cannot have both period and time set\n");
    return 1;
  }


  // convert variables

  time_t remind_time = make_time(&opts);  
 
  struct reminder rem = { opts.message, remind_time, RFLAG_NEW };

  int write_err = write_reminder(&rem);
  if (write_err > 0)
  {
    return write_err;
  }
  char timestr[25];
  strftime(timestr, sizeof(timestr), DATE_TIME_FORMAT, localtime(&rem.time));
  printf("Reminding you \"%s\" at [%lld] \"%s\"\n", rem.message, rem.time, timestr);
  return 0;
}

