#
# This file is distributed under the MIT License. See LICENSE.md for details.
#

import argparse
import sys

from .log import set_verbose
from .merge_dynamic import merge_dynamic


def main():
  parser = argparse.ArgumentParser(
    description="Merge the dynamic portions of the translate ELF with the one from the host ELF."
  )
  parser.add_argument("to_extend",
                      metavar="TO_EXTEND",
                      help="The ELF to extend.")
  parser.add_argument("source",
                      metavar="SOURCE",
                      help="The original ELF.")
  parser.add_argument("output",
                      metavar="OUTPUT",
                      nargs="?",
                      default="-",
                      help="The output ELF.")
  parser.add_argument("--verbose",
                      action="store_true",
                      help="Print debug information and warnings.")
  parser.add_argument("--base",
                      metavar="ADDRESS",
                      default="0x400000",
                      help="The base address where dynamic object have been loaded.")
  parser.add_argument("--merge-load-segments",
                      action="store_true",
                      help="Merge the LOADed segments from the source ELF into the output ELF.")

  args = parser.parse_args()
  set_verbose(args.verbose)

  base = int(args.base, base=0)

  if args.output == "-":
    output_file = sys.stdout
  else:
    output_file = open(args.output, "wb")

  with output_file, open(args.source, "rb") as source_file, open(args.to_extend, "rb") as to_extend_file:
    retcode = merge_dynamic(to_extend_file,
                            source_file,
                            output_file,
                            base=base,
                            merge_load_segments=args.merge_load_segments)

  return retcode


if __name__ == "__main__":
  retcode = main()
  sys.exit(retcode)
