# Overview

This is specifically where specializations/impls go

## Augmented Spec

We add a few API to streambuf:

### streamsize xin_avail()

Optional feeder for `showmanyc`, required feeder for `sgetc`.  
Reveals the number of characters immediately available in the get area buffer.  Typically used in scenarios where
we *always* know if more characters are available or not.  Implies one single monolithic buffer.

Often produces result as if `egptr() - gptr()`

### char_type xsgetc()

An underlying call to feed `sgetc`.  More raw and does no buffer management.  If character isn't immediately available
to the impl, behavior is undefined 