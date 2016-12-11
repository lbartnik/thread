#' @export
run_c_printing_example <- function ()
{
  thread_runner <- function (data)
  {
    thread_benchmark(10, 1)
  }

  message("starting the first thread")
  thread1 <- new_thread(thread_runner, list())
  print(ls(threads))

  message("starting the second thread")
  thread2 <- new_thread(thread_runner, list())
  print(ls(threads))

  message("going to join() both threads")
  thread_join(thread1)
  thread_join(thread2)
}


#' @export
run_r_printing_example <- function ()
{
  thread_runner <- function (data)
  {
    thread_print(paste("thread", data, "starting\n"))

    for (i in 1:10) {
      timeout <- as.integer(abs(rnorm(1, 500, 1000)))
      thread_print(paste("thread", data, "iteration", i, "sleeping for",
                         timeout, "\n"))
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
}



#' @export
run_c_computing_example <- function ()
{
  thread_runner <- function (args) {
    thread_print(paste("thread", args$id, "computing from", args$from,
                       "to", args$to, "\n"))
    thread_sum(args$data, args$from, args$to) 
  }
  
  message('generating data')
  data <- as.numeric(seq(1e9))
  nthr <- 4

  time1 <- system.time({
    message('starting threads')
    threads <- lapply(seq(nthr), function(i) {
      from <- length(data)/nthr * (i-1)
      to   <- from + length(data)/nthr - 1
      new_thread(thread_runner, list(data = data, from = from, to = to, id = i))
    })
    
    message('main: going to join all threads')
    lapply(threads, thread_join)
    
    message('main: going to claim results')
    result1 <- sum(unlist(lapply(threads, thread_result)))
  })
  
  time2 <- system.time({
    result2 <- sum(data)
  })
  
  print(result1)
  print(result2)
  print(time1)
  print(time2)
}


#' @export
dump_threads <- function ()
{
  eapply(threads, function (x) {
    str(x$main_data)
    str(as.list(x$thread_env))
  })
  
  invisible()
}
