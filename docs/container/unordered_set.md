# Overview

Document v0.1

# Scope

This document descibes behavior of `unordered_set`

# Architecture

`unordered_set` freely moves/swaps items within itself around, unlike
`unordered_map` which maintains pointer sanctity of active items.

This means that GC things `unordered_map` gets up to such as gc bit, empty vs null
items, etc. are not applicable here.
