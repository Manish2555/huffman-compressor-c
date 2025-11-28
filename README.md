# Huffman File Compressor in C

A command-line file compression tool based on the **Huffman Coding** algorithm. This project demonstrates low-level systems programming concepts including bit manipulation, memory management, and binary file I/O.

## 🚀 Features
- **Lossless Compression:** Reduces file size without losing data.
- **Custom Header:** Implements a binary header for portability.
- **Bitwise Operations:** Packs variable-length codes into compact bytes.
- **Memory Safety:** Optimized with AddressSanitizer to ensure 0 memory leaks.

## 🛠️ Usage
```bash
make
./huffman -c input.txt output.huff
./huffman -d output.huff restored.txt
