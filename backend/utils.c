#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "../config.h"
#include "utils.h"

/* ============================================================
   Date / time helpers
   ============================================================ */

void getCurrentDate(char *buf) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buf, DATE_LEN, "%Y-%m-%d", tm_info);
}

/* Basic date validation: YYYY-MM-DD */
int isValidDate(const char *date) {
    if (!date || strlen(date) != 10) return 0;
    if (date[4] != '-' || date[7] != '-') return 0;
    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) continue;
        if (!isdigit((unsigned char)date[i])) return 0;
    }
    int year  = atoi(date);
    int month = atoi(date + 5);
    int day   = atoi(date + 8);
    if (month < 1 || month > 12) return 0;
    if (day   < 1 || day   > 31) return 0;
    if (year  < 1900 || year > 2100) return 0;
    return 1;
}

int compareDates(const char *a, const char *b) {
    return strcmp(a, b); /* lexicographic works for YYYY-MM-DD */
}

/* ============================================================
   String utilities
   ============================================================ */

void strTrimWhitespace(char *s) {
    if (!s) return;
    /* Trim leading */
    int start = 0;
    while (s[start] && isspace((unsigned char)s[start])) start++;
    if (start > 0) memmove(s, s + start, strlen(s) - start + 1);
    /* Trim trailing */
    int len = (int)strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[--len] = '\0';
    }
}

int strIsEmpty(const char *s) {
    if (!s) return 1;
    while (*s) {
        if (!isspace((unsigned char)*s)) return 0;
        s++;
    }
    return 1;
}

void strToUpper(char *s) {
    for (; *s; s++) *s = (char)toupper((unsigned char)*s);
}

void strToLower(char *s) {
    for (; *s; s++) *s = (char)tolower((unsigned char)*s);
}

/* ============================================================
   Password hash (simple djb2)
   ============================================================ */

unsigned int simpleHash(const char *str) {
    unsigned int hash = 5381;
    int c;
    while ((c = (unsigned char)*str++) != 0)
        hash = ((hash << 5) + hash) + c;
    return hash;
}

/* ============================================================
   Auto-increment ID
   Returns (record_count + 1) based on file size / struct_size.
   ============================================================ */

int getNextId(const char *filepath, int struct_size) {
    FILE *f = fopen(filepath, "rb");
    if (!f) return 1;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fclose(f);
    return (int)(sz / struct_size) + 1;
}

/* ============================================================
   UI helpers
   ============================================================ */

void pressEnterToContinue(void) {
    printf("\n  Press ENTER to continue...");
    fflush(stdout);
    /* consume any leftover newline then wait */
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void printSeparator(int width, char ch) {
    for (int i = 0; i < width; i++) putchar(ch);
    putchar('\n');
}

/* ============================================================
   Validation helpers
   ============================================================ */

int isValidPhone(const char *phone) {
    int len = (int)strlen(phone);
    if (len < 7 || len > 15) return 0;
    for (int i = 0; i < len; i++) {
        if (!isdigit((unsigned char)phone[i])) return 0;
    }
    return 1;
}

/* ============================================================
   Safe input (reads line, no overflow, strips newline)
   ============================================================ */

void safeInput(char *dest, int maxlen) {
    if (!fgets(dest, maxlen, stdin)) {
        dest[0] = '\0';
        return;
    }
    /* strip newline */
    size_t len = strlen(dest);
    if (len > 0 && dest[len - 1] == '\n') {
        dest[len - 1] = '\0';
    } else {
        /* flush leftover input */
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }
    strTrimWhitespace(dest);
}

/* ============================================================
   Integer input with range
   ============================================================ */

int inputInt(int min, int max) {
    char buf[32];
    int value;
    while (1) {
        safeInput(buf, sizeof(buf));
        if (sscanf(buf, "%d", &value) == 1 && value >= min && value <= max)
            return value;
        printf("  " CLR_RED "Invalid input. Enter a number between %d and %d: " CLR_RESET, min, max);
        fflush(stdout);
    }
}

/* ============================================================
   Float input with range
   ============================================================ */

float inputFloat(float min, float max) {
    char buf[32];
    float value;
    while (1) {
        safeInput(buf, sizeof(buf));
        if (sscanf(buf, "%f", &value) == 1 && value >= min && value <= max)
            return value;
        printf("  " CLR_RED "Invalid input. Enter a value between %.2f and %.2f: " CLR_RESET, min, max);
        fflush(stdout);
    }
}
