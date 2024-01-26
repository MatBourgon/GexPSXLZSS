This program can decompress files stored in Gex 2: Enter the Gecko's BIGFILE.dat, either from pre-extracted files stored on disk, or while they are still stored in the BIGFILE. It can additionally compress files using the same algorithm, creating files that can be decompressed using the same algorithm, while also being more space efficient.

When using the file decompression function, the compressed and uncompressed sizes will be automatically calculated.

When using the in-memory function, it is important to provide the exact size of the compressed block, as we cannot get the size in memory without re-processing the header, and the header is not guaranteed to always exist.

When decompressing a file compressed with this algorithm, the file may end up at most 15 bytes larger than the original file, but only with 0s appended to the end of the file. This is due to how the algorithm works, requiring full instructions to be provided, even if there is nothing at the end of the file to compress.