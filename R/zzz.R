zzz <- new.env()
zzz$prompt <- " "

.onLoad <- function (libname, pkgname) {
  zzz$prompt <- options(prompt = "[threaded] > ")
}

.onUnload <- function (libpath) {
  options(prompt = zzz$prompt)
}

