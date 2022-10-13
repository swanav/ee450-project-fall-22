#ifndef __UTILS_H__
#define __UTILS_H__

// void read_csv(const char *filename, int *n, int *d);

FILE* csv_open(const char* file_name);
void csv_close(FILE* fp);

#endif // __UTILS_H__
