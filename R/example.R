#' @export
run_example <- function ()
{
  thread_runner <- function (data)
  {
    thread_benchmark()
  }

  message("starting the first thread")
  thread1 <- new_thread(thread_runner, list())
  ls(threads)

  message("starting the second thread")
  thread2 <- new_thread(thread_runner, list())
  ls(threads)

  thread_join(thread1)
  thread_join(thread2)
  
  thread_yield()
}

