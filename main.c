#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ================================
// PART 1: DATA STRUCTURES
// ================================

typedef struct Node {
    unsigned char character;
    unsigned freq;
    struct Node *left, *right;
} Node;

typedef struct MinHeap {
    unsigned size;
    unsigned capacity;
    Node **array;
} MinHeap;

char *codes[256]; 

// ================================
// PART 2: HEAP HELPERS
// ================================

Node* createNode(unsigned char character, unsigned freq) {
    Node* temp = (Node*)malloc(sizeof(Node));
    temp->left = temp->right = NULL;
    temp->character = character;
    temp->freq = freq;
    return temp;
}

MinHeap* createMinHeap(unsigned capacity) {
    MinHeap* minHeap = (MinHeap*)malloc(sizeof(MinHeap));
    minHeap->size = 0;
    minHeap->capacity = capacity;
    minHeap->array = (Node**)malloc(minHeap->capacity * sizeof(Node*));
    return minHeap;
}

void swapMinHeapNode(Node** a, Node** b) {
    Node* t = *a;
    *a = *b;
    *b = t;
}

void minHeapify(MinHeap* minHeap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < minHeap->size && minHeap->array[left]->freq < minHeap->array[smallest]->freq)
        smallest = left;
    if (right < minHeap->size && minHeap->array[right]->freq < minHeap->array[smallest]->freq)
        smallest = right;

    if (smallest != idx) {
        swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
    }
}

Node* extractMin(MinHeap* minHeap) {
    Node* temp = minHeap->array[0];
    minHeap->array[0] = minHeap->array[minHeap->size - 1];
    --minHeap->size;
    minHeapify(minHeap, 0);
    return temp;
}

void insertMinHeap(MinHeap* minHeap, Node* minHeapNode) {
    ++minHeap->size;
    int i = minHeap->size - 1;
    while (i && minHeapNode->freq < minHeap->array[(i - 1) / 2]->freq) {
        minHeap->array[i] = minHeap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    minHeap->array[i] = minHeapNode;
}

// ================================
// PART 3: BUILD TREE
// ================================

void storeCodes(Node* root, char* str) {
    if (!root) return;
    if (!root->left && !root->right) {
        codes[root->character] = strdup(str);
    }
    char leftStr[256], rightStr[256];
    strcpy(leftStr, str); strcat(leftStr, "0");
    storeCodes(root->left, leftStr);
    
    strcpy(rightStr, str); strcat(rightStr, "1");
    storeCodes(root->right, rightStr);
}

Node* buildHuffmanTree(unsigned *freq) {
    MinHeap* minHeap = createMinHeap(256);
    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) insertMinHeap(minHeap, createNode((unsigned char)i, freq[i]));
    }
    while (minHeap->size != 1) {
        Node *left = extractMin(minHeap);
        Node *right = extractMin(minHeap);
        Node *top = createNode('$', left->freq + right->freq);
        top->left = left;
        top->right = right;
        insertMinHeap(minHeap, top);
    }
    return extractMin(minHeap);
}

// ================================
// PART 4: COMPRESSION (With Header)
// ================================

unsigned char buffer = 0;
int bitCount = 0;

void writeBit(int bit, FILE *out) {
    if (bit == 1) buffer |= (1 << (7 - bitCount));
    bitCount++;
    if (bitCount == 8) {
        fwrite(&buffer, 1, 1, out);
        buffer = 0; bitCount = 0;
    }
}

void flushBuffer(FILE *out) {
    if (bitCount > 0) fwrite(&buffer, 1, 1, out);
}

void compressFile(const char *inputFile, const char *outputFile, unsigned *freq) {
    FILE *in = fopen(inputFile, "rb");
    FILE *out = fopen(outputFile, "wb");

    // HEADER: Write the frequency table (Key to decoding)
    // We write the count of total characters first (to know when to stop)
    unsigned totalChars = 0;
    for(int i=0; i<256; i++) totalChars += freq[i];
    fwrite(&totalChars, sizeof(unsigned), 1, out);

    // Then write the full frequency table (256 integers)
    fwrite(freq, sizeof(unsigned), 256, out);

    // BODY: Write the compressed bits
    unsigned char ch;
    while (fread(&ch, 1, 1, in) == 1) {
        char *code = codes[ch];
        for (int i = 0; code[i] != '\0'; i++) {
            writeBit(code[i] - '0', out);
        }
    }
    flushBuffer(out);
    fclose(in);
    fclose(out);
}

// ================================
// PART 5: DECOMPRESSION (NEW!)
// ================================

void decompressFile(const char *inputFile, const char *outputFile) {
    FILE *in = fopen(inputFile, "rb");
    FILE *out = fopen(outputFile, "wb");

    // 1. READ HEADER
    unsigned totalChars;
    fread(&totalChars, sizeof(unsigned), 1, in);

    unsigned freq[256];
    fread(freq, sizeof(unsigned), 256, in);

    // 2. REBUILD TREE
    Node* root = buildHuffmanTree(freq);
    Node* current = root;

    // 3. READ BITS & TRAVERSE TREE
    unsigned char ch;
    unsigned count = 0;
    while (fread(&ch, 1, 1, in) == 1 && count < totalChars) {
        // Process each bit in the byte
        for (int i = 7; i >= 0; i--) {
            int bit = (ch >> i) & 1;

            if (bit == 0) current = current->left;
            else current = current->right;

            // Found a leaf node?
            if (!current->left && !current->right) {
                fputc(current->character, out);
                current = root; // Reset to top
                count++;
                if (count == totalChars) break; // Stop if we have all chars
            }
        }
    }
    fclose(in);
    fclose(out);
}

// ================================
// PART 6: MAIN
// ================================

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage:\n");
        printf("  Compress:   ./huffman -c input.txt output.huff\n");
        printf("  Decompress: ./huffman -d output.huff restored.txt\n");
        return 1;
    }

    if (strcmp(argv[1], "-c") == 0) {
        printf("--- Compressing ---\n");
        unsigned freq[256] = {0};
        
        // 1. Analyze File
        FILE *f = fopen(argv[2], "rb");
        if (!f) { printf("File not found!\n"); return 1; }
        unsigned char buf;
        while (fread(&buf, 1, 1, f) == 1) freq[buf]++;
        fclose(f);

        // 2. Build Map
        Node* root = buildHuffmanTree(freq);
        char str[256] = "";
        storeCodes(root, str);

        // 3. Save
        compressFile(argv[2], argv[3], freq);
       // --- Calculate Statistics ---
        long origSize = ftell(fopen(argv[2], "rb"));
        long compSize = ftell(fopen(argv[3], "rb"));
        float percent = 100.0 - ((float)compSize / origSize * 100.0);
        
        printf("\n-----------------------------------\n");
        printf("SUCCESS! File Compressed.\n");
        printf("Original Size:   %ld bytes\n", origSize);
        printf("Compressed Size: %ld bytes\n", compSize);
        printf("Space Saved:     %.2f%%\n", percent);
        printf("-----------------------------------\n");

    } else if (strcmp(argv[1], "-d") == 0) {
        printf("--- Decompressing ---\n");
        decompressFile(argv[2], argv[3]);
        printf("Decompressed %s -> %s\n", argv[2], argv[3]);
    }

    return 0;
}
