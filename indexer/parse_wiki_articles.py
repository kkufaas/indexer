# Author: Morten Grønnesby <morten.gronnesby@uit.no>

import os
import sys
import fnmatch
import shutil
import re

def clean_filename(fname):
    """Only keep ascii characters from the argument"""
    return "".join(c for c in fname if ord(c) < 128)


def get_html_files(indir, outdir, max_files):
    """Recursively find .html files and copy these to the outdir arguement"""

    if max_files > 0 and get_html_files.counter >= max_files:
        return

    root_path = os.path.abspath(indir)
    current_files = os.listdir(root_path)

    for fname in current_files:
        fpath = os.path.join(root_path, fname)
        
        # If it is a file, copy it to outdir
        if os.path.isfile(fpath):
            if fpath.endswith(".html"):
                shutil.copy(fpath, os.path.join(outdir, clean_filename(fname)))
                get_html_files.counter += 1
                print("Copying files {0}".format(get_html_files.counter), end="\r")
        
        # If it is a directory, descend down
        elif os.path.isdir(fpath):
            get_html_files(fpath, outdir, max_files)


if __name__ == "__main__":

    if len(sys.argv) < 4:
        print("usage: python parse_wiki_articles.py <input_dir> <output_dir> <numfiles>")
        exit(1)

    indir = sys.argv[1]
    outdir = sys.argv[2]
    numfiles = int(sys.argv[3])

    if not os.path.isdir(indir):
        print("Input {0} is not a directory".format(indir))
    elif not os.path.isdir(outdir):
        print("Output {0} is not a directory".format(outdir))
    

    get_html_files.counter = 0
    get_html_files(indir, outdir, numfiles)