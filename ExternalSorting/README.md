# External Sorting

Sort a large text file that may not fit in RAM.

## Idea

If the file is bigger than memory, you cannot call `std::sort` on everything at once. External sort does two steps:

1. **Split + sort chunks** — Read as much as fits in the memory budget, sort that chunk in RAM, write it to a temp file (a *run*). Repeat until the input is finished.
2. **Merge** — Open all sorted runs together. Always take the smallest next line among them (min-heap) and write it to the output. Repeat until every run is empty.

```text
big input
   │
   ├─ chunk → sort → tmp/0
   ├─ chunk → sort → tmp/1
   └─ chunk → sort → tmp/2
                        │
                        ▼
              merge (min-heap) → sorted output
```

Each line is one record. Order is **string / dictionary order** (`"10"` before `"2"`), not numeric.

## Build & run

```bash
g++ -std=c++17 -O2 -o external_sort main.cpp
./external_sort [input output mem_kib]
```

| Argument | Default | Meaning |
|----------|---------|---------|
| `input` | `input.txt` | Unsorted text file |
| `output` | `output.txt` | Sorted result |
| `mem_kib` | `512000` | Soft memory budget per chunk (KiB) |

```bash
./external_sort input.txt output.txt 102400
```

If the whole file fits in one chunk, the program writes the output directly and skips the merge.

## Notes

- Needs a writable working directory (`./tmp/`).
- `mem_kib` limits how large each chunk is when building runs; it is not a hard OS RAM lock.
