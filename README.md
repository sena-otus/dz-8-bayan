# dz-8-bayan
dz-8-bayan. search dups on filesystem

Usage: bayan -h
   or: bayan [-d <dir1> [-d <dir2> [...]]]  [-x <direx1> [ -x <direx2> [...]]] [-r <0|1>] [-m <minsize>] [-b <blocksize>] [--hash <hash>]


Allowed options:
  -h [ --help ]          produce help message
  -d [ --dirs ] arg      path to scan, default is working directory
  -x [ --exclude ] arg   directory name to exclude
  -r [ --recursive ] arg 1: scan recursively (default), 0: no recursion
  -m [ --minsize ] arg   min file size, must be positive (default 1)
  -w [ --wildcard ] arg  regex for filenames (default .*)
  -b [ --blocksize ] arg blocksize, must be positive  (default 512)
  --hash arg             hash to use (default is boosthash)

Supported hash algo: boosthash crc64 md5 sha1
