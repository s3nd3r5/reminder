reminder
========

reminder.d is a daemon that monitors the /usr/local/etc/reminder.d/ dir
remindme is the CLI used at the user level to manage the contents within the /usr/etc/reminder.d/$USER.list 
reminder-server syncs the reminders across other configured daemons

daemon
======

The actual daemon is setup with the /usr/etc/reminder.d/$USER.config where the server information is configured.
The daemon listens to the $USER.list for any changes and syncs it with the server.
It also listens to the server for any changes for that $USER and syncs it locally. 

daemon depends on:
  libnotify-dev

reminders
=========

Depending on the configuration reminder.d will notify the user when/if the date is past.

cli
===

The user can run commands via the cli to add new reminders, list upcoming reminders, remove, and update reminders

server
======

The server exists solely as a file-syncer across users. 

Keys are produced on the users local machines and placed manually on the server. 
These keys are used for authenticating requests between the clients and the servers.
A single key exists per $USER so that key is shared on all of the users machines. 

$USER.list
==========

Your reminders will exist in the $USER.list file.

The format of this file is [flag] [epochsecond] [reminder]
  The [epochsecond] will be used to verify if/when the [reminder] should be sent out. 
  The [flag] will denote the state of the reminder.


dev notes
=========

Create and add user to reminder group
chown -R :reminder /usr/local/etc/remidner.d
chmod 770 (or higher) /usr/local/etc/reminder.d

