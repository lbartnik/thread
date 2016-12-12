context("native call")

test_that("native function can be called", {
  ret <- native_call("C_sample_call", 1)
  expect_equal(ret, 1)
})
