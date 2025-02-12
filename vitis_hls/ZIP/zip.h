#ifndef ZIP_H
#define ZIP_H


const unsigned MAX_SIZE = 4096;

// Define implementation type
typedef float ZIP_T;

void zip_add_top(ZIP_T A [MAX_SIZE], ZIP_T B [MAX_SIZE], ZIP_T C [MAX_SIZE], int size);
void zip_sub_top(ZIP_T A [MAX_SIZE], ZIP_T B [MAX_SIZE], ZIP_T C [MAX_SIZE], int size);
void zip_mult_top(ZIP_T A [MAX_SIZE], ZIP_T B [MAX_SIZE], ZIP_T C [MAX_SIZE], int size);
void zip_div_top(ZIP_T A [MAX_SIZE], ZIP_T B [MAX_SIZE], ZIP_T C [MAX_SIZE], int size);

#endif

