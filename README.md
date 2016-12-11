R Threads - Experimental Support
================================


Design loosely follows Python's [Global Interpreter Lock](https://wiki.python.org/moin/GlobalInterpreterLock).

Check this out:

* start a new thread and execute a R function in its own interpreter
* switch between threads on specific function calls, e.g. `thread_join()`, `thread_print()`, `thread_sleep()`
* finish thread execution
* keep track of `R_PPStackTop`

Missing:

* run an arbitrary C function outside of the Global Interpreter Lock
* synchronize memory allocation

## Example

```r
library(thread)
 
thread_runner <- function (data) {
  thread_print(paste("thread", data, "starting\n"))
  for (i in 1:10) {
    timeout <- as.integer(abs(rnorm(1, 500, 1000)))
    thread_print(paste("thread", data, "iteration", i,
                       "sleeping for", timeout, "\n"))
    thread_sleep(timeout)
  }
  thread_print(paste("thread", data, "exiting\n"))
}
  
message("starting the first thread")
thread1 <- new_thread(thread_runner, 1)
print(ls(threads))
  
message("starting the second thread")
thread2 <- new_thread(thread_runner, 2)
print(ls(threads))
  
message("going to join() both threads")
thread_join(thread1)
thread_join(thread2)
```

Below you can see that output from both threads is interleaved due to randomized timeout. 

```
starting the first thread
[1] "thread_140737231587072"
starting the second thread
[1] "thread_140737223194368" "thread_140737231587072"
going to join() both threads
thread 1 starting
thread 1 iteration 1 sleeping for 144 
thread 2 starting
thread 2 iteration 1 sleeping for 587 
thread 1 iteration 2 sleeping for 761 
thread 2 iteration 2 sleeping for 1327 
thread 1 iteration 3 sleeping for 360 
thread 1 iteration 4 sleeping for 1802 
thread 2 iteration 3 sleeping for 704 
thread 2 iteration 4 sleeping for 463 
thread 1 iteration 5 sleeping for 368 
thread 2 iteration 5 sleeping for 977 
thread 1 iteration 6 sleeping for 261 
thread 1 iteration 7 sleeping for 323 
thread 1 iteration 8 sleeping for 571 
thread 2 iteration 6 sleeping for 509 
thread 2 iteration 7 sleeping for 2521 
thread 1 iteration 9 sleeping for 298 
thread 1 iteration 10 sleeping for 394 
thread 1 exiting
thread 2 iteration 8 sleeping for 966 
thread 2 iteration 9 sleeping for 533 
thread 2 iteration 10 sleeping for 1795 
thread 2 exiting
```