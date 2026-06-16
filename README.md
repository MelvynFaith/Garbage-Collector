# SnakeLang Garbage Collector

A compact, educational implementation of **two garbage collection strategies** built on a single shared object model in C. The project demonstrates the core trade-off between **Reference Counting** and **Mark-and-Sweep** collection — including the classic reference-cycle problem and how each strategy handles it.

---

## Overview

Both collectors operate over the same runtime object (`snake_object`) inside a minimal virtual machine (`VM`). The VM maintains a stack of call frames (roots) and a registry of heap objects. Each collector reclaims memory using a different algorithm, allowing a direct, side-by-side comparison.

| Collector | Algorithm | Frees cycles? | Cost model |
|-----------|-----------|:-------------:|------------|
| **Reference Counting** (`rc_*`) | Per-object reference count; eager free at zero | ❌ No | Cost paid on every reference change |
| **Mark-and-Sweep** (`ms_*`) | Tri-color reachability scan from roots | ✅ Yes | Cost paid at collection time (stop-the-world) |

> **Contract:** a given object belongs to exactly **one** collector for its entire lifetime. Reference-counted objects are *not* registered in the VM's object registry; mark-and-sweep objects *must* be. Mixing the two on the same object is a double-free. See the header comment in [`gc.h`](gc.h).

---

## Object Model

`snake_object` is a tagged union supporting five kinds:

| Kind | Payload |
|------|---------|
| `OBJ_INTEGER` | `int` |
| `OBJ_FLOAT` | `double` |
| `OBJ_STRING` | heap-allocated C string |
| `OBJ_VECTOR3` | three child object references (`x`, `y`, `z`) |
| `OBJ_ARRAY` | dynamic array of child object references |

Each object carries both a `ref_count` (used by RC) and an `is_marked` flag (used by MS). `VECTOR3` and `ARRAY` are *container* kinds — collectors recurse into their children.

---

## How Each Collector Works

### Reference Counting (`rc_*`)
- `rc_inc` / `rc_dec` adjust an object's reference count.
- When a count reaches zero, the object is freed immediately, and `rc_dec` cascades into any children.
- Call frames act as roots: `rc_frame_push` increments and records a reference; `rc_pop_frame` decrements every reference the frame held.
- **Limitation:** two objects referencing each other keep their counts above zero forever — a cycle leak. This is demonstrated (and manually cleaned up) in the test suite.

### Mark-and-Sweep (`ms_*`)
- **Mark:** walk every reference in every active frame and color reachable objects.
- **Trace:** follow container children via a gray-object work stack until closure.
- **Sweep:** iterate the VM object registry, free anything unmarked, and reset marks on survivors.
- **Strength:** unreachable cycles are collected correctly, because reachability — not reference count — decides liveness.

---

## Project Layout

```
.
├── snake_object.{c,h}   # Object model: construction, printing, destruction
├── stack.{c,h}          # Generic dynamic array / stack
├── frame.{c,h}          # Call frame holding root references
├── vm.{c,h}             # Virtual machine: frames + object registry
├── gc.{c,h}             # Both garbage collectors (rc_* and ms_*)
├── main.c               # Self-test suite
└── Makefile             # Build, run, and memory-check targets
```

---

## Building & Running

Requires a C11 compiler (`gcc` or `clang`) and `make`.

```sh
make          # build the test binary (./snakelang)
make run      # build and run the test suite
make clean    # remove build artifacts
```

### Memory verification

```sh
make valgrind # full leak check under Valgrind
make asan     # rebuild with AddressSanitizer + UBSan and run
```

---

## Test Suite

`main.c` contains 11 self-tests across three parts:

- **Part 1 — Foundations:** stack operations; object construction and printing for all five kinds.
- **Part 2 — Reference Counting:** primitives, manual inc/dec with shared references, `VECTOR3` and `ARRAY` cascade frees, and the cycle-leak demonstration.
- **Part 3 — Mark-and-Sweep:** basic collection, container reachability through tracing, cycle collection, and multi-frame root preservation.

Verified results:

```
ALL TESTS PASSED

HEAP SUMMARY:
    in use at exit: 0 bytes in 0 blocks
    total heap usage: 119 allocs, 119 frees, 6,302 bytes allocated
All heap blocks were freed -- no leaks are possible
ERROR SUMMARY: 0 errors from 0 contexts
```

---

## License

Educational project. Use freely.
