# Pretty Printers

# Guidance from https://github.com/epasveer/seer/wiki/Gdb's-Pretty-Print-feature
# NOT READY YET

import gdb

class UnitPrinter:

    # Save a copy of the Location.
    def __init__(self, val):
        self.__val = val

    def to_string(self):
        return "XYZ"

def UnitPrinter_func(val):
    t = str(val.type)
    if t.startswith('estd::units::detail::v1::unit'):
        return UnitPrinter(val)
    return None

print('Registering estd types')
# Add the function to gdb's list of PrettyPrinter functions.
gdb.pretty_printers.append(UnitPrinter_func)
