# estd::variadic namespace

c++17 really brought us something excellent with fold expressions.
If those aren't available to you, this namespace is for you.

Inspired by C# LINQ approach (which itself seems inspired by Python generators)
the `estd::variadic` namespace has many compile-time goodies for list processing
and manipulation.

For example:

```c++
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
    constexpr bool operator()(variadic::value<I, T, v>, T param) const
    {
        return param == v;
    }
};

using values = variadic::values<int, 0, 7, 77, 777>;

// index = 2 (position of 77), compile-time 'values' visited at runtime
int key = 77;
int index = values::visit(identify_value_functor{}, key);
```

## Lists

The meat and potatoes of this namespace are compile time lists.
They start life very similar to `integer_sequence` and `tuple`, then
grow into their own behaviors.

### `variadic::values`

Similar to `integer_sequence`, with differences being:

* Need not be numeric types at all
* Visitation, projection and selection availability

### `variadic::types`

Similar to `tuple`, with differences being:

* No specific expectation of runtime instantiation
* Visitation, projection and selection availability

## Visitation

Lists offer a visit capability.  This is expressly a runtime behavior.

For `variadic::types` visitation, two visitation possibilities are possible:

* Default: functor accepting `variadic::type<I, T>`
* Tuple-like: functor accepting `variadic::instance<I, T>`

## Selectors

A compile time select applies to lists, similar to a SQL where clause.

## Projectors

A compile time re-interpretation of lists is available, morphing each element
into a different element.
