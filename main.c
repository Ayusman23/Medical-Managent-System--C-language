/*
 * Hospital Management System
 * main.c — Entry point
 *
 * Build:  make
 * Run:    ./hms          (Linux/macOS)
 *         hms.exe        (Windows)
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
  #include <windows.h>
  /* Older MinGW SDKs may not define this flag */
  #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
    #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
  #endif
#endif

#include "config.h"
#include "backend/utils.h"
#include "frontend/ui.h"

/* ============================================================
   Enable ANSI colours on Windows 10+ (Virtual Terminal)
   ============================================================ */
static void enableWindowsAnsi(void) {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif
}

/* ============================================================
   Ensure all database files (and their directory) exist
   ============================================================ */
static void initDatabaseFiles(void) {
    const char *files[] = {
        DB_PATIENTS, DB_DOCTORS, DB_APPOINTMENTS,
        DB_BILLING,  DB_WARDS,   DB_STAFF, DB_USERS
    };
    int n = (int)(sizeof(files) / sizeof(files[0]));

    /* Create database directory if needed */
#ifdef _WIN32
    system("if not exist database mkdir database");
#else
    system("mkdir -p database");
#endif

    for (int i = 0; i < n; i++) {
        /* Open in "ab" so the file is created if it doesn't exist */
        FILE *f = fopen(files[i], "ab");
        if (f) fclose(f);
    }
}

/* ============================================================
   Program entry
   ============================================================ */
int main(void) {
    enableWindowsAnsi();
    initDatabaseFiles();

    /* Application loop: show login, then main menu, then logout */
    while (1) {
        showWelcomeBanner();

        int role = ROLE_STAFF;
        int logged_in = showLoginScreen(&role);

        if (!logged_in) {
            /* User typed 'exit' at login */
            CLEAR_SCREEN();
            printf(CLR_CYAN CLR_BOLD "\n  Thank you for using HMS. Goodbye!\n\n" CLR_RESET);
            break;
        }

        showMainMenu(role);
        /* After logout we loop back to login screen */
    }

    return 0;
}
