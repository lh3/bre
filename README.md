***The repo is for discussion only. It is not ready for general uses.***

## Getting Started
```sh
git clone https://github.com/lh3/bre
cd bre && make  # compile examples
make bre.pdf    # compile the spec, requiring pdflatex

echo CCATAAGC | examples/dna6build - > 1.bre              # build BWT for one DNA string
examples/dna6print 1.bre                                  # bre-to-text; new line for sentinel(s)
examples/dna6print 1.bre | examples/dna6import - > 2.bre  # text-to-bre
openssl sha1 1.bre 2.bre                                  # the two files are identical
```

## Introduction

The Binary Run-length Encoding (BRE) format is a simple binary format for
storing [run-length encoded][rle] strings. It aims to ease the interoperability
between tools working with run-length encoded [BWTs][bwt]. This repo gives a
draft specification of BRE and provides a two-file library in C99. It also
comes with a few examples that use the library.

[rle]: https://en.wikipedia.org/wiki/Run-length_encoding
[bwt]: https://en.wikipedia.org/wiki/Burrows%E2%80%93Wheeler_transform
