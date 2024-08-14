# estd::variadic namespace

c++17 really brought us something excellent with fold expressions.
If those aren't available to you, this namespace is for you.

Inspired by C# LINQ approach (which itself seems inspired by Python generators)
the `estd::variadic` namespace has many compile-time goodies for list processing
and manipulation.

For example:

```c+++
using types = variadic::types<monostate, float, char*>;

using selected = types::selector<internal::is_same_selector<float> >::selected;
using first = selected::first;

// what position in 'types' float was found
// first::type will be 'float'
constexpr int index = first::index;
```

Additionally, one can runtime evaluate using a visitor pattern:

```c++
struct identify_value_functor
{
    template <size_t I, class T, T v>
    constexpr bool operator()(variadic::value<I, T, v>, T&& param) const
    {
        return param == v;
    }
};

using values = variadic::values<int, 0, 7, 77, 777>;

// index = 2 (position of 77), compile-time 'values' visited at runtime
int index = values::visit(identify_value_functor{}, 77);
```

