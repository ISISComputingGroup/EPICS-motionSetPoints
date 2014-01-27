#ifndef SETPOINT_H
#define SETPOINT_H 1
/* C functions for motion setpoint lookup */

void checkLoadFile(const char* env_fname);
void loadDefFile(const char* env_fname);
int name2posn(const char *name, const char* env_fname);
int posn2name(double x, double tol, const char* env_fname);
int setFilter(const char *name, const char *value, const char* env_fname);
int checkFilter(const char *name, const char *filter);
double currentPosn(int bFirst, const char* env_fname);
int getFilterOut(char *target, const char *name);
int getPosnName(char *target, int isRBV, const char* env_fname);
int getPositions(char *target, int elem_size, const char* env_fname);

#endif
