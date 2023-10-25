# Platforms

## Overview

There is minimal platform abstraction.  It isn't this library's function but
sometimes (like during chrono) it's convenient to do it and have it in estdlib

Also since I use Espressif chips a lot, there's some version-query helpers for
them

## ESP32 / esp-idf

### Observations: 03MAY22 esp-idf v4.4.1

It appears that the inbuilt `std::char_traits<char>::pos_type` resolves to fpos, which is
pretty standard stuff.  What is mysterious and does not seem right is that this fpos
results in incredible stack bloat, particularly with our streambuf and ios code.

My best guess here is that there never was great care into avoiding copying `pos_type` around
because I always used to think it was an integer type.  Probably somewhere I should be doing
a `const pos_type&` instead of a `pos_type`.  

In the meantime, to rememdy this I've freshened `estd::char_traits` and provided a feature flag 
FEATURE_ESTD_CHARTRAITS to force usage of `estd` version instead of `std` version
