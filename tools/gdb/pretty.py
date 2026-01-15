# Pretty Printers

# Guidance from https://github.com/epasveer/seer/wiki/Gdb's-Pretty-Print-feature
# NOT READY YET

import gdb

class UnitPrinter:

    # Save a copy of the Location.
    def __init__(self, val):
        #print('GOT HERE 1')
        self.__val = val

    def to_string(self):
        return self.__val['rep_']

    def children(self):
        rep = int(self.to_string())
        print(rep)
        return [
            ("count()", rep)
        ]

    def display_hint(self):
        print('GOT HERE 4')
        return 'string'

def UnitPrinter_func(val):
    t = str(val.type)
    if t.startswith('estd::units::v1::detail::unit<'):
        return UnitPrinter(val)
    return None

print('Registering estd types')

gdb.pretty_printers.append(UnitPrinter_func)
