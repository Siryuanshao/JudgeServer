#ifndef COMPARE
#define COMPARE

#define OJ_AC 0
#define OJ_PE 1
#define OJ_WA 2
#define OJ_RE 3
#define OJ_SPJ_ERR 4

int compare(const char* file1, const char* file2);
int compare_spj(const char* spj_exe, const char* file1, const char* file2);

#endif //
