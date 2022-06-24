# Namespace Convention

## internal

Internal namespaces are considered production useable, with the
important caveat that breaking changes are more readily expected
here.  Be mindful about depending on `internal` APIs.

Oftentimes this namespace is used not only to isolate more wiggly
code, but also to keep main namespace less cluttered.

## experimental

Experimental namespaces come with no gauruntees.  Proceed with
caution!

## detail

This namespace takes a page out of GCC/C++ examples and is used
to isolate helper classes and functions away from main namespace.

Very similar to `internal`, but less subject to breaking changes