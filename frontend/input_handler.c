#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../config.h"
#include "../backend/utils.h"
#include "ui.h"

/* ============================================================
   Validated string input
   ============================================================ */
void getValidatedString(const char *prompt, char *dest, int maxlen, int allow_empty) {
    while (1) {
        printf("  %s", prompt);
        fflush(stdout);
        safeInput(dest, maxlen);
        if (allow_empty || !strIsEmpty(dest)) return;
        printf(CLR_RED "  This field cannot be empty. " CLR_RESET);
    }
}

/* ============================================================
   Validated integer
   ============================================================ */
int getValidatedInt(const char *prompt, int min, int max) {
    printf("  %s", prompt);
    fflush(stdout);
    return inputInt(min, max);
}

/* ============================================================
   Validated float
   ============================================================ */
float getValidatedFloat(const char *prompt, float min, float max) {
    printf("  %s", prompt);
    fflush(stdout);
    return inputFloat(min, max);
}

/* ============================================================
   Validated date (YYYY-MM-DD)
   ============================================================ */
void getValidatedDate(const char *prompt, char *dest) {
    char buf[DATE_LEN];
    while (1) {
        printf("  %s", prompt);
        fflush(stdout);
        safeInput(buf, DATE_LEN);
        if (isValidDate(buf)) { strncpy(dest, buf, DATE_LEN - 1); return; }
        printf(CLR_RED "  Invalid date. Use YYYY-MM-DD format. " CLR_RESET);
    }
}

/* ============================================================
   Validated phone
   ============================================================ */
void getValidatedPhone(const char *prompt, char *dest) {
    char buf[PHONE_LEN];
    while (1) {
        printf("  %s", prompt);
        fflush(stdout);
        safeInput(buf, PHONE_LEN);
        if (isValidPhone(buf)) { strncpy(dest, buf, PHONE_LEN - 1); return; }
        printf(CLR_RED "  Invalid phone (7-15 digits). " CLR_RESET);
    }
}
