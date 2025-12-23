# CS50 Tiny Search Engine - Crawler

## Author
- **Name:** Atziri Enriquez
- **GitHub Username:** AtziriEnriquez

## Description
This is the **Crawler** module for the Tiny Search Engine (TSE).  
The crawler explores a website starting from a given seed URL, follows internal links up to a specified depth, and stores the crawled webpages in a directory.

## Assumptions
- The `seedURL` is **always internal** and **normalized** before crawling.
- The `pageDirectory` is **created beforehand** and must be **writable**.
- Each crawled page is saved with a **unique document ID** (`1, 2, 3,...`).
- The program **waits 1 second** between each page fetch to avoid server overload.
- The **bag** structure does not guarantee a specific order for crawling.
- The **crawler stops** when no more pages are left in the bag.

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
## Testing
To test run:
```bash
make test
```
- Note that all test output will automatically go to testing.out


