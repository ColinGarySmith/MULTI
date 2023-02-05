/***************************************************************************************/
// Basic multitasking system. Colin Smith (2022) has released this free into the public domain, if you feel like letting me know how terrible it is, or awesome, or have a bug fix,
// my email address is colin.gary.smith@gmail.com, or you can donate me a coffee :)  Basic questions will be answered, but i am not a free technical resource, man's gotta get paid right.
//
// delay without using delay(), so involves non blocking code.
// can either use state machine to keep track of program flow, or scheduled tasking, or chained processes.
// this code does not use delay() at all, and will use the most power as the program is continually checking the timers
// not ideal for battery powered equipment.
// use at your own risk.
// scheduler is called at around 35 thousand times a second if you sprinkle "scheduler();" liberally around any slow code that you have. slow in taking lots of time, but not slow as in using delays. If you need a delay, use fDelay() as it calls scheduler automagically.
// known blocking libs that can slow down/cause jitter in timing:
// - any graphic libraries that write to an LCD/OLED
// - sd card access
// - ethernet/wireless calls.
// - DHT22 and some other enviromental sensors.
// If a registered process is delayed by a blocking call, it will be called immediately upon return.



// note: conditional process starts will be released in ver2 of this code once i can validate its reliability.


// if you schedule a task and it doesn't appear, confirm you haven't registered too many processes.
// you may need to modify MAX_PROCESS.


// Initial variable and defines are here
typedef void (*GeneralFunction) (void);  // this is required to return the pointer to a function. Important note - the called functions do not have a parameter passthrough, it is safe to use global variabls to allow processes to interface with the rest of your code.
// structure that defines a process, also includes some basic telemetry so you can if needed find out how long an individual process took.
struct processList {
  byte state;               // free/stopped/running
  byte runState;           // run once, run always.
  unsigned long interval;
  unsigned long timercheck;
  unsigned long startTime; // stores how long this pid last took to execute.
  unsigned long totalTime; // total time taken so far.
  GeneralFunction address;
};

#define MAX_PROCESS 10
#define STATE_FREEPID 0
#define STATE_STOPPED 1
#define STATE_RUNNING 2
#define RUN_NEVER 0
#define RUN_ONCE 1
#define RUN_ALWAYS 2

#define INVALID_PID 255

processList processes[MAX_PROCESS] = {0}; // with processes at 10, this uses 200 bytes.
/***********************************************************************************************************************************************************************************************************/
// your defines here.
byte ledFlash = 0;

byte motorProcess = 0; // stores the state of the motor/animation progress and if its currently being used.

byte countedTaskCount = 0;
/***********************************************************************************************************************************************************************************************************/
void setup() {
  // put your setup code here
  pinMode(13, OUTPUT); // lets flash the onboard LED twice a second.
  pinMode(2, OUTPUT); // hypothetical motor control on 2 and 3.
  pinMode(3, OUTPUT);

  pinMode(4, INPUT_PULLUP);  // a hypothetical switch is on pin 4, if pin 4 is brought low by the switch, trigger the animation.
  

  // this is an optional tasks that gives a quick rundown on whats happened in the last 1 second. shows cpu time, free time, and currently how many PIDS are registered.
  registerProcess(idleTask, 1000, RUN_ALWAYS);  
  
  
  // schedule the LED flash to occur every 500 milliseconds.
  registerProcess(FlashLED, 500, RUN_ALWAYS);
  
  
  // every 100 milliseconds, check the digital pin inside of stepcheck, and its pushed, trigger the chain of events.
  registerProcess(StepCheck, 100, RUN_ALWAYS);
  
  
  // this process runs 10 times at 900 millisecond intervals, and on run 10, kills itself.
  registerProcess(countedTask, 900, RUN_ALWAYS);


  Serial.begin(9600);
  Serial.println(F("Program start."));
}
/***********************************************************************************************************************************************************************************************************/
void loop() {
  scheduler();
  
  // do not put anything else here unless its very quick. if you have code that needs to run all the time and can not fit into a scheduled process, insert it into the scheduler code below.
  // Otherwise, create a process for it with registerProcess. Avoid using 1ms intervals as if your code in total takes more then a millisecond it would run continously. you can do it, but it must run and exit very very quickly,
  // and not allow other code to run, eventually running into an error. 0 means it gets called instantly, but you must use a RUN_ONCE, otherwise it will loop constantly. The code prevents scheduling a process with RUN_ALWAYS and zero interval.
}
/***********************************************************************************************************************************************************************************************************/
// your code here.

// following are examples of the 3 ways to use the scheduling system.
// example one is for a flashing LED. Example two is to show a process that is chained, so can be triggered as a chain of events. Example three is a task that runs a few times and is then freed.
void FlashLED( void )
{ // ledflash is a global variable, and thus can be used by other code to determine the status of the LED
  if (ledFlash == 0)
  {
    ledFlash = 1;

  }
  else
  {
    ledFlash = 0;

  }
  digitalWrite(13, ledFlash);
}

// this example shows a chained process list for long processes, for example animating leds or motors. It is triggered by a digital low on pin 4.
// being chained you can schedule the next step at different times as required. since the time is ALSO a variable, you can also change the timing based on your program flow.
void StepCheck(void)
{ // this gets called every 10 milliseconds to check the input status of pin 4.

  if ((digitalRead(4) == 0) && (motorProcess == 0))  // motorprocess is checked here so that the chain doesn't restart if the button is pushed again.
  {
    motorProcess = 1; // will stop the above from being called again until these chained routines are all finished.
    registerProcess(Step1, 0, RUN_ONCE); // when returned, immediately start the animation. to delay the start of the animation, just change the 0 to something else.
  }
}

void Step1(void)
{
  // set motors, etc, for example if there is a motor control on pin2, lets turn it on. pin2 will be motor on or off and pin 3 is motor direction.
  Serial.println(F("Step 1 is in progress."));
  digitalWrite(2, 1);
  digitalWrite(3, 0);
  registerProcess(Step2, 500, RUN_ONCE); // next step will be called in 500 milliseconds. this process( Step1(); ) will be automatically deregistered as soon as it returnes to the scheduler.
}

void Step2(void)
{
  Serial.println(F("Step 2 is in progress."));
  digitalWrite(2, 1);
  digitalWrite(3, 1); // reverse motor direction
  registerProcess(Step3, 700, RUN_ONCE); // next step will be called in 700 milliseconds.
}


void Step3(void)
{
  Serial.println(F("Step 3 is in progress."));
  digitalWrite(2, 1);
  digitalWrite(3, 0); // reverse it again
  registerProcess(Step4, 1000, RUN_ONCE); // next step will be called in 1 second.
}

void Step4(void)
{ // shows using the scheduler safe version of the delay.
  Serial.println(F("Step 4a is in progress."));
  digitalWrite(2, 0);  // turn the motor off.
  digitalWrite(3, 0);
  fDelay(100);
  Serial.println(F("Step 4b is in progress."));
  digitalWrite(2, 1);  // turn the motor on.
  fDelay(200);
  Serial.println(F("Step 4c is in progress."));
  digitalWrite(2, 1);  // turn the motor off again.
  // the animation has finished.
  // shut down the chain.
  motorProcess = 0; // this will allow StepStart() to restart it again if digitalRead is still low.


  // since motorProcess is also a global variable, you can reference it in other code so it knows if the motor is currently in animation mode.
  // you can use use another global variable to keep a track of what step you are in and trigger/set other code to run.
  // using switch statements in a single "Step() procedure will also work. increment your motorProcess number, and do different operations.
}



// this shows the example of the task scheduling that when a threshold is reached, kills itself permanently.
void countedTask( void )
{
  delay(10); // this will show up as "CPU time" in the idle task thats called once a second.
  Serial.print(F("Counted task num:"));
  Serial.println(countedTaskCount);
  countedTaskCount++;
  if (countedTaskCount > 10)
  {
    Serial.println(F("Counted task has ended and is about to be freed."));
    deregisterProcess(returnPID(countedTask));  // this process (countedTask() will now kill itself so wont be called again and the PID slot is freed.
  }
}



// below here is the scheduler/process control code.
/***********************************************************************************************************************************************************************************************************/
// Co-operative scheduler code starts here. do not modify this unless you know what you are doing.
// you might either end up with processes called over and over again, or not running at all.
// this needs to be called as often as possible in any code that takes a long time or is waiting for something to happen.
void scheduler(void)
{

  // any code that needs to be called as often as possible, put here.
  // for example, a triggered process, but its better to just use registerProcess and register a process that does the checking for you.


  // checks the pid list.
  // if any of their timers have expired, and the process is not RUNNING
  // call it if its timer has expired.

  unsigned long current = millis();
  // current time stamp.

  // normally all future timestamps are POSITIVE.
  // if the test fails run the process.
  unsigned long tc;
  unsigned long interval;

  for ( byte PID = 0; PID < MAX_PROCESS; PID++)
  {

    if (processes[PID].runState != STATE_FREEPID)  //  does this entry have a process associated with it?
    { //yes.
      if (processes[PID].state == STATE_STOPPED) // first check to see if its not already running, if it is this will fail.
      { // now see if it needs to be run.
        tc = processes[PID].timercheck;
        interval = processes[PID].interval;
        if ((signed long)(tc - current) < 0)
        {
          if (processes[PID].address != 0) // if its a valid address, (0) is the reset vector, dont process that or the uc will reboot.
          {
            processes[PID].timercheck = tc + interval; // set time in the future.
            lock(PID); // lock it so if you call scheduler INSIDE the process you are about to run, it doesn't just rerun the same thing again.
            processes[PID].address();
            unlock(PID); // unlocked.
          }

        }
      }
    }
  }
}

/***************************************************************************************/
// if you NEED a delay in your code, for example to wait for a sensor to respond, use this instead.
void fDelay(unsigned long interval)
{
  // scheduler friendly version of the delay
  unsigned long start = millis();           // start: timestamp

  for (;;) {
    scheduler();
    unsigned long now = millis();         // now: timestamp
    unsigned long elapsed = now - start;  // elapsed: duration
    if (elapsed >= interval) {                   // comparing durations to see if the inverval has expired, and if it has return.
      return;
    }
  }
}

/***************************************************************************************/
// a form of mutexing, this prevents a copy of a process from being called again and again if its already running
// fork protection to stop an individual process from spiralling out of control
// processes are refered to internally by a PID - program id.
void lock( byte pid)
{
  processes[pid].state = STATE_RUNNING;
  processes[pid].startTime = millis();
}
/***************************************************************************************/
// frees a pid if it was RUN_ONCE, and stores some basic info on how long that process was active for.
// if its only a run once process, the PID is freed so it can be used for another process.
void unlock( byte pid)
{

  // there is a possibility that a process has already suicided.
  // if thats the case just return, no used doing anything else as its now free for reallocation.
  if (processes[pid].runState == STATE_FREEPID) return;


  processes[pid].state = STATE_STOPPED;
  unsigned long current = millis();
  unsigned long start = processes[pid].startTime;
  unsigned long total = current - start; // this provides a record of how long a process took to run.

  processes[pid].totalTime = processes[pid].totalTime + total; // count up how long that process took to run.
  // if its stopped, check to see if its a runonce. if it is, free it's PID and process list entry.
  if (processes[pid].runState == RUN_ONCE)
  {
    deregisterProcess(pid);
  }
}

/***************************************************************************************/
// used to schedule a process to be run in the future at a predetermined interval, or, if its a runonce process, will run it in current Millis()+ interval.
void registerProcess( GeneralFunction add, unsigned long interval, byte runstate)
{ // single run process runs after a delay, a normal process runs as soon as its registered.
  byte found = 0;
  if (runstate==RUN_ALWAYS)
  {
    if(interval==0) interval=1; // this prevents infinite loops and overruns if you call a process to RUN_ALWAYS with no interval. You can still break your code if something takes longer then 1millisecond, but this will help reduce infinite loops.
                                // you can get around this if the process suicides when it runs, but if thats the case, just use RUN_ONCE with an interval of 0 and it will be automatically cleared after its processed.
  }
  for (byte freepid = 0; freepid < MAX_PROCESS; freepid++)
  {
    if (processes[freepid].runState == STATE_FREEPID) // find a free one.
    {
      found = 1;
      processes[freepid].runState = runstate;
      processes[freepid].address = add;
      processes[freepid].state = STATE_STOPPED;
      processes[freepid].interval = interval;
      processes[freepid].startTime = 0;
      processes[freepid].timercheck = millis(); // start time is the current millis
      if (runstate == RUN_ONCE)
      {
        // schedule this to run after the interval, otherwise it will run as soon as its registered and scheduler() is called again.
        processes[freepid].timercheck = interval + millis(); // start this process later
      }
      break;
    }
  }
  if (!found) {
    Serial.println(F("There are no more process slots to be found. Considering increasing MAX_PROCESS, or re-organising your code to use a state machine and a global variable that increments each time is run."));
  }
}

/***************************************************************************************/
// this returns the PID of a function if its registered.
byte returnPID(GeneralFunction add)
{ // since all functions reside PROM, their addresses do not change.
  byte found = INVALID_PID;
  for (byte findpid = 0; findpid < MAX_PROCESS; findpid++)
  {

    if (processes[findpid].address == add)
    {
      found = findpid;
      break;
    }
  }
  return found;
}
/***************************************************************************************/
// clears all entries of a particular pid so its now free to be assigned again by registerprocess.
void deregisterProcess(byte PID)
{ // clears the entry from the process list using PID.
  if (PID == INVALID_PID) return; // pid is invalid, so cant be used as an entry in the process list. INVALID_PID is 255, so don't use it as an index into an array, it will overwrite other heap data.
  processes[PID].runState = STATE_FREEPID;
  processes[PID].address = 0;
  processes[PID].state = RUN_NEVER;
  processes[PID].interval = 0;
  processes[PID].timercheck = 0;
}

/***************************************************************************************/
void callPID(byte PID)
{ // allows you to call a process that scheduled prematurely.
  lock(PID); // lock it 
  processes[PID].address();
  unlock(PID); // unlocked.

}

/***************************************************************************************/
void idleTask(void)
{
  // can be asked to run every second.
  // calculates processor idle time.
  // add up all pid times this last second.
  unsigned long idletime = 0;
  unsigned long cpuTime = 0;
  int numpid = 0;
  for (byte PIDcheck = 0; PIDcheck < MAX_PROCESS; PIDcheck++)
  {
    if (processes[PIDcheck].runState != STATE_FREEPID) numpid++;
    if (processes[PIDcheck].state == STATE_STOPPED) // first check to see if its not running, if its running there is no way to see what its done in the last 1000 milliseconds.
    { // its not running
      cpuTime = cpuTime + processes[PIDcheck].totalTime;
      processes[PIDcheck].totalTime = 0;
    }
    if (processes[PIDcheck].state == STATE_RUNNING)
    {
      cpuTime = cpuTime + processes[PIDcheck].totalTime;

    }

    processes[PIDcheck].totalTime=0; // zero out all counters.
  }
  if (cpuTime > 1000) cpuTime = 1000; // something is taking a long long time, so basically, its a second.
  Serial.print(F("#Number of active processes (including idle task): "));
  Serial.println(numpid);
  Serial.print(F("#CPU Time: "));
  Serial.print(cpuTime);
  Serial.println(" ms");
  Serial.print(F("#Free Time: "));
  Serial.print(1000 - cpuTime);
  Serial.println(" ms");
}
