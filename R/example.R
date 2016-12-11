#' @example
run_example <- function ()
{
  thread_runner <- function (data)
  {
    thread_benchmark()
  }

  message("starting the first thread")
  thread1 <- new_thread(thread_runner, list())
  str(threads)

  message("starting the second thread")
  thread2 <- new_thread(thread_runner, list())
  str(threads)

  thread_join(thread1)
  thread_join(thread2)
}

