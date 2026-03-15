#ifndef UTILS_H
#define UTILS_H

#include <time.h>

/* ============================================================
   Utility functions – shared across all backend modules
   ============================================================ */

/* Date / time helpers */
void        getCurrentDate(char *buf);          /* fills buf with YYYY-MM-DD */
int         isValidDate(const char *date);      /* 1=valid, 0=invalid        */
int         compareDates(const char *a, const char *b); /* -1,0,1 */

/* String utilities */
void        strTrimWhitespace(char *s);
int         strIsEmpty(const char *s);
void        strToUpper(char *s);
void        strToLower(char *s);

/* Simple hash for password storage */
unsigned int simpleHash(const char *str);

/* Auto-increment ID helpers */
int         getNextId(const char *filepath, int struct_size);

/* Pause and prompt */
void        pressEnterToContinue(void);

/* Print separator line */
void        printSeparator(int width, char ch);

/* Validate phone (digits only, 7-15 chars) */
int         isValidPhone(const char *phone);

/* Safe string input (no overflow, strips newline) */
void        safeInput(char *dest, int maxlen);

/* Integer input with range validation */
int         inputInt(int min, int max);

/* Float input */
float       inputFloat(float min, float max);

#endif /* UTILS_H */
