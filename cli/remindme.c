#include <stdio.h>
#include <string.h>
#include <time.h> 
#include <stdlib.h>

const char* TIME_FLAG = "-t";
const char* TIME_FLAG_LONG = "--at";
const char* PERIOD_FLAG = "-i";
const char* PERIOD_FLAG_LONG = "--in";

const char* DATE_TIME_FORMAT = "%FT%T%z";

const int MIN_SEC = 60; 
const int HOURS_SEC = MIN_SEC * 60;
const int DAYS_SEC = HOURS_SEC * 24;

const char* help = ""
"    REMINDME\n"
"      remindme - Send yourself remidners at a specific time on one or more devices\n"
"\n"
"    SYNOPSIS\n"
"      remindme [-t TIME] [--at TIME] [-i PERIOD] [--in PERIOD] message\n"
"\n"
"    OPTIONS\n"
"      -t, --at=TIME\n"
"        Sets the time the reminder will be set at.\n"
"        ISO 8601 Format\n"
"\n"
"      -i, --in=PERIOD\n"
"        Queues the reminder to be sent after a period of time.\n"
"        Period format is [NUMBER][DHSEC]\n"
"\n"
"        NUMBER - 0-9+\n"
"        D - in N days\n"
"        H - in N hours\n"
"        M - in N minute\n"
"        S - in N seconds\n"
"\n"
"   EXAMPLES\n"
"     $ remindme --at \"2038-01-17T17:00:00+01:00\" \"Take your laptop home!\"\n"
"     $ remindme --in 30M \"Check if the deployment succeeded.\"\n"
"     $ remindme -i 10S \"Beep!\"\n"
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
  struct tm tm;
  if (opt->time)
  {
    strptime(opt->time, DATE_TIME_FORMAT, &tm);
    return mktime(&tm);
  }
  else
  {
    int len = strlen(opt->period);
    char tmtype = opt->period[len-1];
    
    char* nstr = strtok(opt->period, "DMHSdmhs");
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
  }

  struct options opts = {};
  int sflagc = 0;
  int lflagc = 0;
  for (int i = 1; i < argc-1; i++)
  {
    // check single args
    char* arg = argv[i];

    // short form arguments 
    if (strcmp(TIME_FLAG, arg) == 0)
    {
      opts.time = argv[i+1];
      ++i;
      ++sflagc;
    }
    else if (strcmp(PERIOD_FLAG, arg) == 0)
    {
      opts.period = argv[i+1];
      ++i;
      ++sflagc;
    }
    else // long form arguments
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
      }
      else if (strcmp(PERIOD_FLAG_LONG, flag) == 0)
      {
        opts.period = value;
        ++lflagc;
      }
    }
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

  opts.message = argv[argc-1];

  // convert variables

  time_t remind_time = make_time(&opts);  
  
  printf("Reminding you \"%s\" at [%d] \"%s\b\"", opts.message, &remind_time, asctime(localtime(&remind_time)));

  return 0;
}
