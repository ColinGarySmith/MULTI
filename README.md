# MULTI
Basic scheduling system for Arduino.

This is a super basic sketch that emulates a cooperative multitasking operating system.

The arduino is incapable of SMP, it has only got 1 core, and with limited ram, and no memory mapping or protected mode, it is not possible to run any kind of 
modern operating system on it.

This sketch relies on the fact that very very few things need to run instantly, and to humans, if a task is delayed by 50ms its basically undetectable.

Loop() only contains one entry, that is "scheduler();"

This needs to be called all the time to keep track of what needs to be run next and maintains the list of currently active "processes". It also makes sure that the 
same process can not be called multiple times. Its a basic form of fork protection. 

In user code, if anything takes a significant amount of time (100ms or more) calling scheduler(); throughout is recommended so that other processes have time to be 
checked and run if needed.

Avoid using "delay();" in anything as it will stop scheduler from being called, i have added a scheduler friendly delay called fDelay(); that does the same thing, but 
also keeps calling scheduler. 

The only other issues can be caused by blocking calls. Writing to LCDs, sd cards, some sensors etc that can take some time will block the execution checks, so try to 
keep operations as modular as possible and call scheduler(); between all operations. 

The system allows for "processes" to be called instantly and run only once, or called on a regular basis. They can also chain, with proc1 being called RUN_ONCE 
setting another proc2 with RUN_ONCE etc etc and large chains of processes can be queued up to the "PROCESS_LIMIT".

Feel free to suggest changes, submit bug fixes etc.

There is also a 1 second process that can be disabled if not needed that provides execution details on your processes, if you have a lot of them and sometimes 
everything slows down or stops responding, the output on the serial console can tell you what processes are using up all your time.

On a basic test sketch, the scheduler gets called around 25 thousand times a second, so to lower power usage on mobile systems you can put a "delay(1);" underneath 
the scheduler calling in loop(). This will slow down the checks to only 1000 times a second. 

Other various power saving systems can be utilised if needed, and im currently working on an actual "sleep" power saving system that will put most of the arduino to 
sleep for the amount of time before the next process is due to be called.

The prototype is currently saving a fair amount of power, however on wake its breaking the serial console so more work is needed.
