# Common Module - Page Directory Handling

## Author
- **Name:** Atziri Enriquez
- **GitHub Username:** AtziriEnriquez

## Description
This module provides functionality to **initialize a page directory** for the Tiny Search Engine and **save crawled webpages** to that directory.

## Assumptions
- The **page directory must be writable** before calling `pagedir_init()`.
- Webpages are **saved with a unique document ID** (starting from `1`).
- The `pagedir_save()` function **does not check for duplicates**.

## Deviations from Specs
- No major deviations at this time.

## Compilation & Execution
First run:
```bash
make clean
```
Then run:
```bash
make
```