# TSV parser {#tsvboot}

Mini-library for parsing the config.

### File format
The bootloader config is represented by a structure based on TSV (tab-separated values). The first column of a row holds the key, and all following columns in that row are values for that key. For readability, the file may be separated by blank lines. No other entries are allowed in the file. The parser correctly handles both LF and CRLF as line endings. Keys and values in the file cannot contain the characters: `\t`, `\n`, `\r`, `\0`.

### Internal representation
Given a pointer to the file buffer and its length, `tsv_init` converts the source file into a library-friendly format according to the following rules:

	\t      -> \0
	\n...\n -> \n
	\n      -> \0\n
	\r      -> [discarded]
	EOF     -> \n\r

The function allocates a new buffer for subsequent work with the data, freeing the old one after copying.

### Working with data
The library implements several functions for reading and interpreting data (see [tsv.c](tsv_8c.html)). The key/index lookup function (note: value indices start at 1, while column numbers start at 0) takes the start of the buffer as its base; the others take a pointer to the value it found. Instead of copying data, the string functions return pointers directly into the internal buffer. The number parsing function walks the string from the end (least significant digits) to replace division of the coefficent with multiplication.
