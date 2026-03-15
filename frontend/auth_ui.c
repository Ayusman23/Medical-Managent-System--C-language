#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../config.h"
#include "../backend/utils.h"
#include "ui.h"

/* ============================================================
   User record for authentication
   ============================================================ */
typedef struct {
    char         username[USERNAME_LEN];
    unsigned int pass_hash;
    int          role;    /* ROLE_ADMIN or ROLE_STAFF */
    int          is_active;
} UserRecord;

/* ============================================================
   Helper: append user to users.dat
   ============================================================ */
static void appendUser(const UserRecord *u) {
    FILE *f = fopen(DB_USERS, "ab");
    if (!f) { perror("Cannot open users.dat"); return; }
    fwrite(u, sizeof(UserRecord), 1, f);
    fclose(f);
}

/* ============================================================
   Helper: seed default admin if database is empty
   ============================================================ */
static void seedDefaultAdmin(void) {
    FILE *f = fopen(DB_USERS, "rb");
    if (f) {
        /* Check if at least one record exists */
        UserRecord tmp;
        int n = (int)fread(&tmp, sizeof(UserRecord), 1, f);
        fclose(f);
        if (n == 1) return; /* already has users */
    }
    /* Create default admin */
    UserRecord admin;
    memset(&admin, 0, sizeof(UserRecord));
    strncpy(admin.username, "admin", USERNAME_LEN - 1);
    admin.pass_hash = simpleHash("admin123");
    admin.role      = ROLE_ADMIN;
    admin.is_active = 1;
    appendUser(&admin);

    /* Create default staff */
    UserRecord staff;
    memset(&staff, 0, sizeof(UserRecord));
    strncpy(staff.username, "staff", USERNAME_LEN - 1);
    staff.pass_hash = simpleHash("staff123");
    staff.role      = ROLE_STAFF;
    staff.is_active = 1;
    appendUser(&staff);

    printf(CLR_YELLOW "  [INFO] Default accounts created:\n");
    printf("         Admin  -> username: admin   password: admin123\n");
    printf("         Staff  -> username: staff   password: staff123\n" CLR_RESET);
    pressEnterToContinue();
}

/* ============================================================
   showLoginScreen
   Returns 1 on success, 0 on user choosing to exit.
   Sets *role_out to ROLE_ADMIN or ROLE_STAFF.
   ============================================================ */
int showLoginScreen(int *role_out) {
    seedDefaultAdmin();

    while (1) {
        CLEAR_SCREEN();
        printf(CLR_BOLD CLR_CYAN);
        printf("\n  ╔════════════════════════════════════╗\n");
        printf("  ║        HMS  —  LOGIN               ║\n");
        printf("  ╚════════════════════════════════════╝\n" CLR_RESET);
        printf(CLR_YELLOW "  [Tip] Default: admin/admin123 or staff/staff123\n\n" CLR_RESET);

        char uname[USERNAME_LEN];
        char pword[PASS_LEN];

        printf("  Username (or 'exit'): "); fflush(stdout);
        safeInput(uname, USERNAME_LEN);
        if (strncmp(uname, "exit", USERNAME_LEN) == 0) return 0;

        printf("  Password            : "); fflush(stdout);
        /* On Windows/Linux: password is echoed (no hidden input without ncurses) */
        safeInput(pword, PASS_LEN);

        unsigned int entered_hash = simpleHash(pword);

        FILE *f = fopen(DB_USERS, "rb");
        if (!f) {
            printf(CLR_RED "  Cannot open user database.\n" CLR_RESET);
            pressEnterToContinue();
            continue;
        }

        int found = 0;
        UserRecord u;
        while (fread(&u, sizeof(UserRecord), 1, f) == 1) {
            if (u.is_active &&
                strncmp(u.username, uname, USERNAME_LEN) == 0 &&
                u.pass_hash == entered_hash) {
                found = 1;
                *role_out = u.role;
                break;
            }
        }
        fclose(f);

        if (found) {
            printf(CLR_GREEN "\n  ✓ Login successful! Welcome, %s.\n" CLR_RESET, uname);
            printf("  Role: %s\n", (*role_out == ROLE_ADMIN) ? "Administrator" : "Staff");
            pressEnterToContinue();
            return 1;
        } else {
            printf(CLR_RED "\n  ✗ Invalid username or password. Try again.\n" CLR_RESET);
            pressEnterToContinue();
        }
    }
}

/* ============================================================
   showRegistrationScreen  (Admin only — called from menu)
   ============================================================ */
void showRegistrationScreen(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_GREEN "\n  ╔══════════════════════════════════╗\n");
    printf("  ║     REGISTER NEW USER           ║\n");
    printf("  ╚══════════════════════════════════╝\n" CLR_RESET);

    UserRecord u;
    memset(&u, 0, sizeof(UserRecord));

    printf("  Username (max %d chars): ", USERNAME_LEN - 1); fflush(stdout);
    safeInput(u.username, USERNAME_LEN);
    if (strIsEmpty(u.username)) {
        printf(CLR_RED "  Username cannot be empty.\n" CLR_RESET);
        pressEnterToContinue(); return;
    }

    /* Check for duplicates */
    FILE *f = fopen(DB_USERS, "rb");
    if (f) {
        UserRecord tmp;
        while (fread(&tmp, sizeof(UserRecord), 1, f) == 1) {
            if (tmp.is_active && strncmp(tmp.username, u.username, USERNAME_LEN) == 0) {
                fclose(f);
                printf(CLR_RED "  Username '%s' already exists.\n" CLR_RESET, u.username);
                pressEnterToContinue(); return;
            }
        }
        fclose(f);
    }

    char pword[PASS_LEN];
    printf("  Password              : "); fflush(stdout); safeInput(pword, PASS_LEN);
    if (strIsEmpty(pword)) { printf(CLR_RED "  Password cannot be empty.\n" CLR_RESET); pressEnterToContinue(); return; }
    u.pass_hash = simpleHash(pword);

    printf("  Role (1=Admin, 2=Staff): "); fflush(stdout);
    u.role = inputInt(1, 2);
    u.is_active = 1;

    appendUser(&u);
    printf(CLR_GREEN "  ✓ User '%s' registered as %s.\n" CLR_RESET,
           u.username, u.role == ROLE_ADMIN ? "Admin" : "Staff");
    pressEnterToContinue();
}
