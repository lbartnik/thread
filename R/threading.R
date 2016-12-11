

#' Here is where are threads are rooted. This way garbage collector
#' won't claim their memory.
threads <- new.env()


#' Creates a new thread object and starts its main routine.
#' 
#' @param fun Main thread routine.
#' @param data User data passed to \code{fun}.
#' @return The identifier of the newly created thread.
#' 
#' @export
#' 
new_thread <- function (fun, data)
{
  # TODO make sure that thread does not inherit from globalenv()
  meta_env <- new.env(parent = baseenv())

  meta_env$main_routine <- fun
  meta_env$main_data    <- data
  meta_env$thread_env   <- new.env(parent = meta_env)

  # this creates a new thread in C
  handle <- create_new_thread(fun, data, meta_env$thread_env)
  meta_env$handle <- handle

  # store this thread's meta-data under its ID
  assign(paste0('thread_', handle), meta_env, envir = threads)
  
  handle
}


create_new_thread <- function (fun, data, env)
{
  stopifnot(is.function(fun), is.environment(env))
  .Call("C_create_new_thread", fun, data, env)
}


#' @export
thread_join <- function (handle)
{
  .Call("C_thread_join", handle)
}

#' @export
thread_yield <- function ()
{
  .Call("thread_yield")
}


#' @export
thread_benchmark <- function (n, timeout)
{
  .Call("C_thread_benchmark", as.integer(n), as.integer(timeout))
}
