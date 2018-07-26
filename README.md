# Art-Gallery-Log

## Overview
This log describes the state of an art gallery. It is created and updated through the use of logappend.c. Information can be retrieved through logread.c. It keeps track of the guests and employees who have entered and left the gallery, as well as what person is in what room at any given time. Command line arguments are used to specify information to write onto the log, as well as information in the desired output.

## Usage 


### logappend
*Invocation:* 
* `logappend -T <timestamp> -K <token> (-E <employee-name> | -G <guest-name>) (-A | -L) [-R <room-id>] <log>`

*Conditions:*
  * Appends data to log at specified time using token
  * Log created if it does not exist, otherwise existing log simply appended
  * If data to be appended inconsistent with current state of log or conflicting arguments, prints “invalid” and leaves log unchanged
  * Command line arguments may appear in any order

*Arguments:*

* `-T <timestamp>`
  * Time of event, formatted as number of seconds since gallery opened
  * non-negative integer from 1 to 1,073,741,823
  * Time for appended events must always increase from last

* `-K <token>`
  * Token used to authenticate log
  * Arbitrary-sized string of values (a-z, A-Z, and 0-9) chars
  * All append call tokens must match with token used to create log

* `-E <employee-name>`
  * Alphabetic chars (a-z, A-Z), no spaces, case-sensitive
  * Employees/guests may have same names

* `-G <guest-name>`
  * Alphabetic chars (a-z, A-Z), no spaces, case-sensitive
  * Employees/guests may have same names

* `-A` (Event Arrival)
  * Can be used with -E, -G, and -R
  * Shows arrival of employee/guest to gallery
  * If -R used, arrival to specific room
  * Nobody may enter room without first entering gallery
  * Nobody may enter room without first leaving prev room

* `-L` (Event Leave)
  * Same idea as first 3 bullet points for -A
  * Nobody should leave gallery without first leaving last room entered
  * Nobody should leave room without first entering it

* `-R <room-id>`
  * Room ID for event is non-negative integer from 0 to 1,073,741,823
  * Leading zeros are dropped (eg. 003 = 3)
  * List of rooms not available
  * If no room ID, event for whole gallery

* `<log>`
  * Path to the file containing the event log
  * Alphanumeric chars, underscores, slashes
  * If invalid path or other error, fail


### logread

*Invocation:* 
* `logread -K <token> -S <log>`
* `logread -K <token> -R (-E <name> | -G <name>) <log>`

*Conditions:*
  * Person considered in gallery if they enter and have not left
  * Same deal as logappend for errors
  * If logread given person who doesn’t exist in log, give empty output
  * If logread cannot validate that an entry in log was created with logappend using valid token, logread prints “integrity violation” and fails
  * Lists are comma-separated

*Arguments:*

`-K <token>`
  * Token used to authenticate log
  * Arbitrary-sized string of values (a-z, A-Z, and 0-9) chars
  * All append call tokens must match with token used to create log

`-S` (prints log to stdout)
  * State printed on at least two lines separated by \n
  * First: list of employees in gallery
  * Second: list of guests in gallery
  * More: Room-by-room info (ID, then colon, then space, then list of guests and employees). All in ascending lexicographic order.

`-R` (list of rooms entered by person)
  * Output list of rooms in chronological order

`-E/-G` must be specified; that person’s visited rooms IDs printed as list
  * If employee/guest DNE, nothing printed

`-E <employee-name>`
  * Alphabetic chars (a-z, A-Z), no spaces, case-sensitive
  * Employees/guests may have same names

`-G <guest-name>`
  * Alphabetic chars (a-z, A-Z), no spaces, case-sensitive
  * Employees/guests may have same names

`<log>`
  * Path to the file containing the event log
  * Alphanumeric chars, underscores, slashes
  * If invalid path or other error, fail
  
## Sample Usage

### Input ###

```
./logappend -T 1 -K secret -A -E Fred log1  
./logappend -T 2 -K secret -A -G Charlotte log1  
./logappend -T 3 -K secret -A -E Fred -R 1 log1   
./logappend -T 4 -K secret -A -G Charlotte -R 1 log1  
./logread -K secret -S log1  
./logappend -T 5 -K secret -L -E Fred -R 1 log1  
./logappend -T 6 -K secret -A -E Fred -R 2 log1  
./logappend -T 7 -K secret -L -E Fred -R 2 log1  
./logappend -T 8 -K secret -A -E Fred -R 3 log1  
./logappend -T 9 -K secret -L -E Fred -R 3 log1  
./logappend -T 10 -K secret -A -E Fred -R 1 log1  
./logread -K secret -R -E Fred log1
cat log1
```

### Output ###

`./logread -K secret -S log1`:

Fred  
Charlotte  
1: Fred,Charlotte

`./logread -K secret -R -E Fred log1`:

1,2,3,1

`cat log1`

Art Gallery Log  
Key: secret 
 
Time: 1 &nbsp;&nbsp;Employee: Fred &nbsp;&nbsp;Action: Arrive  
Time: 2 &nbsp;&nbsp;Guest: Charlotte &nbsp;&nbsp;Action: Arrive  
Time: 3 &nbsp;&nbsp;Employee: Fred &nbsp;&nbsp;Action: Arrive &nbsp;&nbsp;Room: 1  
Time: 4 &nbsp;&nbsp;Guest: Charlotte &nbsp;&nbsp;Action: Arrive &nbsp;&nbsp;Room: 1  
Time: 5 &nbsp;&nbsp;Employee: Fred &nbsp;&nbsp;Action: Leave &nbsp;&nbsp;Room: 1  
Time: 6 &nbsp;&nbsp;Employee: Fred &nbsp;&nbsp;Action: Arrive &nbsp;&nbsp;Room: 2  
Time: 7 &nbsp;&nbsp;Employee: Fred &nbsp;&nbsp;Action: Leave &nbsp;&nbsp;Room: 2  
Time: 8 &nbsp;&nbsp;Employee: Fred &nbsp;&nbsp;Action: Arrive &nbsp;&nbsp;Room: 3  
Time: 9 &nbsp;&nbsp;Employee: Fred &nbsp;&nbsp;Action: Leave &nbsp;&nbsp;Room: 3  
Time: 10 &nbsp;&nbsp;Employee: Fred &nbsp;&nbsp;Action: Arrive &nbsp;&nbsp;Room: 1  
