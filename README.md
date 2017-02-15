# Thread-scheduler
Thread scheduling simulator for a single processor system.
</br></br>
Scheduler uses a Round Robin algorithm implemented using a priority queue.</br>
A semaphore is associated to each simulated thread in order to mimic preemption.

The state in which any thread is found within the simulation is one of the following:</br>
<b>NEW</b>        - threads that are being created, any new thread that has not yet been put in the thread queue</br>
<b>READY</b>      - threads in the thread queue waiting to be scheduled</br>
<b>RUNNING</b>    - one single thread in the system at a given moment, the one with the unlocked semaphore</br>
<b>WAITING</b>    - threads found in the IO waiting structure (the list vector)</br>
<b>TERMINATED</b> - any thread that finishes running its handler function</br>

In order for the simulator to work, the initial forked thread is assigned a negative priority. This makes sure that it is always at the end of the priority queue and is scheduled after all of the other threads have finished running.

A global thread structure list is maintained in order to keep track of all the active threads in the system. When reaching the TERMINATED state, a thread is deleted from the list.
