#' @export
run_example <- function ()
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

