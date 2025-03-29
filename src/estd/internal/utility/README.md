Unusually, some of these files define things in the `std` namespace.  In particular:

* declval.h
* forward.h

DEBT: Document reasoning for that, but short story is certain limited "core" c++/std abilities we like to keep the std prefix on, even if they aren't actually present (AVR)