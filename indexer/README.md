# CS50 TSE Indexer - README
## Author: Atziri Enriquez
## GitHub: AtziriEnriquez

## Assumptions
The following assumptions were made during the implementation of the Indexer:

  - The pageDirectory contains files named sequentially (1, 2, 3, ...), without missing numbers.
  - The content of each file in pageDirectory strictly follows the format defined in the project specifications.
  - Due to this assumption, the implementation does not include extensive error checking when reading these files.

I also wanted to note that when testing in testing.sh and using indxcmp, no output means success in copying one index file to another.

## Deviations from Specifications

None. The implementation follows the project specifications as required. For my indexer.c, I do add a function parseArgs() to parse command-line arguments as recommended by CS50 guidelines.

## Known Issues or Failures

None. The Indexer functions correctly as designed.