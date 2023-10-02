# Namespace Convention

```
The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL
NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "NOT RECOMMENDED",
"MAY", and "OPTIONAL" in this document are to be interpreted as
described in BCP 14 [RFC2119] [RFC8174] when, and only when, they
appear in all capitals, as shown here.
```

## detail

Developer SHOULD use this namespace when it contains a better fit than regular `estd` namespace

This namespace takes a page out of GCC/C++ examples and is used
to isolate helper classes and functions away from main namespace.

Very similar to `internal`, but strongly minimizes breaking changes

## internal

Developer MAY use `internal` namespace, with the caveat that breaking changes are expected
here.

Developer SHOULD prefer `detail` or `estd` namespace if desired functionality is found there.

Oftentimes this namespace is used not only to isolate more wiggly
code, but also to keep main namespace less cluttered.

## experimental

Developer SHOULD NOT use `experimental` namespace

Experimental namespaces come with no gauruntees.  Proceed with
caution!
