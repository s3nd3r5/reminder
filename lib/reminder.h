// Some functions require imports, denoted at the top of each function

const char* BASE_PATH="/usr/local/etc/reminder.d/";

// CLI FLAGS
const char* TIME_FLAG = "-t";
const char* TIME_FLAG_LONG = "--at";
const char* PERIOD_FLAG = "-i";
const char* PERIOD_FLAG_LONG = "--in";
const char* LIST_FLAG = "-l";
const char* LIST_FLAG_LONG = "--list";
// TIME CONSTS
const char* DATE_TIME_FORMAT = "%FT%T%z";
const int MIN_SEC = 60; 
const int HOURS_SEC = 3600;
const int DAYS_SEC = 86400;

//REMINDER FLAGS
const char RFLAG_NEW = 'n'; // a new notification that has yet to be notified
const char RFLAG_MISSED = 'm'; // a notification that was unable to be notified
const char RFLAG_DONE = 'd'; // a notification that was successfully notified
const char RFLAG_MARK_DEL = 'x'; // markes a reminder to be archived

struct reminder {
  char* message;
  time_t time;
  char flag;
};

// this requires <string.h> <stdlib.h>
char* get_filepath()
{
  char *filepath = (char *)malloc(sizeof(char) * 255);
  strcpy(filepath, BASE_PATH);
  strcat(filepath, getenv("USER"));
  strcat(filepath, ".list\0");
  return filepath;
}

