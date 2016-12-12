context("native call")

test_that("native function can be called", {
  ret <- native_call("C_sample_call", 42)
  expect_equal(ret, 42)
})


test_that("native call in a separate thread", {
  thread_runner <- function (data) {
    native_call("C_sample_call", 42)
  }
  
  th <- new_thread(thread_runner, list())
  thread_join(th)
  rc <- thread_result(th)
  
  expect_equal(rc, 42)
})
