#!/usr/bin/awk -f
BEGIN {
  # srand() set the seed to the current time, usually in seconds and return the
  # current seed. We may want to call this script more than once per second,
  # so use the first command-line parameter to update the seed.
  srand()
  srand(srand() + ARGV[1]);
  printf "%.8X-%.4X-%.4X-%.4X-%.12X\n", rand()*16^8, rand()*16^4, rand()*16^4,
                                        rand()*16^4, rand()*16^12
}
