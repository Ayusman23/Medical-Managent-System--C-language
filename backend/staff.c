#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../config.h"
#include "staff.h"
#include "utils.h"

static int loadStaff(Staff *arr, int maxn) {
    FILE *f = fopen(DB_STAFF, "rb");
    if (!f) return 0;
    int n = 0;
    Staff s;
    while (n < maxn && fread(&s, sizeof(Staff), 1, f) == 1)
        if (s.is_active) arr[n++] = s;
    fclose(f);
    return n;
}

static void saveAllStaff(Staff *arr, int n) {
    FILE *f = fopen(DB_STAFF, "wb");
    if (!f) { perror("Cannot open staff.dat"); return; }
    fwrite(arr, sizeof(Staff), n, f);
    fclose(f);
}

static void appendStaff(const Staff *s) {
    FILE *f = fopen(DB_STAFF, "ab");
    if (!f) { perror("Cannot open staff.dat"); return; }
    fwrite(s, sizeof(Staff), 1, f);
    fclose(f);
}

static void printStaffHeader(void) {
    printf(CLR_CYAN);
    printSeparator(95, '=');
    printf("  %-6s  %-22s  %-18s  %-18s  %-13s  %-10s\n",
           "ID","Name","Role","Department","Phone","Salary($)");
    printSeparator(95, '-');
    printf(CLR_RESET);
}

static void printStaffRow(const Staff *s) {
    printf("  %-6d  %-22s  %-18s  %-18s  %-13s  %-10.2f\n",
           s->staff_id, s->name, s->role, s->department, s->phone, s->salary);
}

void addStaff(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_GREEN "\n  ╔══════════════════════════════╗\n");
    printf("  ║     ADD NEW STAFF            ║\n");
    printf("  ╚══════════════════════════════╝\n" CLR_RESET);

    Staff s;
    memset(&s, 0, sizeof(Staff));

    {
        FILE *f = fopen(DB_STAFF, "rb");
        int max_id = 0;
        if (f) {
            Staff tmp;
            while (fread(&tmp, sizeof(Staff), 1, f) == 1)
                if (tmp.staff_id > max_id) max_id = tmp.staff_id;
            fclose(f);
        }
        s.staff_id = max_id + 1;
    }
    printf(CLR_YELLOW "  Staff ID: %d (auto-assigned)\n" CLR_RESET, s.staff_id);

    printf("  Full Name   : "); fflush(stdout); safeInput(s.name, NAME_LEN);
    if (strIsEmpty(s.name)) { printf(CLR_RED "  Name cannot be empty.\n" CLR_RESET); pressEnterToContinue(); return; }

    printf("  Role        : "); fflush(stdout); safeInput(s.role, ROLE_LEN);
    if (strIsEmpty(s.role)) strncpy(s.role, "Support Staff", ROLE_LEN - 1);

    printf("  Department  : "); fflush(stdout); safeInput(s.department, DEPT_LEN);
    if (strIsEmpty(s.department)) strncpy(s.department, "General", DEPT_LEN - 1);

    do {
        printf("  Phone       : "); fflush(stdout); safeInput(s.phone, PHONE_LEN);
        if (!isValidPhone(s.phone)) printf(CLR_RED "  Invalid phone. " CLR_RESET);
    } while (!isValidPhone(s.phone));

    printf("  Salary ($)  : "); fflush(stdout); s.salary = inputFloat(0, 1000000);
    s.is_active = 1;

    appendStaff(&s);
    printf(CLR_GREEN "  ✓ Staff '%s' added with ID %d.\n" CLR_RESET, s.name, s.staff_id);
    pressEnterToContinue();
}

void updateStaff(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_YELLOW "\n  UPDATE STAFF\n" CLR_RESET);
    printf("  Enter Staff ID: "); fflush(stdout);
    int id = inputInt(1, 999999);

    Staff buf[MAX_STAFF]; int n = loadStaff(buf, MAX_STAFF);
    int idx = -1;
    for (int i = 0; i < n; i++) if (buf[i].staff_id == id) { idx = i; break; }
    if (idx < 0) { printf(CLR_RED "  Staff ID %d not found.\n" CLR_RESET, id); pressEnterToContinue(); return; }

    Staff *st = &buf[idx];
    printf("  Updating %s (ENTER to keep current)\n", st->name);
    char tmp[ADDR_LEN];

    printf("  Name [%s]: ",       st->name);       fflush(stdout); safeInput(tmp, NAME_LEN); if (!strIsEmpty(tmp)) strncpy(st->name, tmp, NAME_LEN - 1);
    printf("  Role [%s]: ",       st->role);       fflush(stdout); safeInput(tmp, ROLE_LEN); if (!strIsEmpty(tmp)) strncpy(st->role, tmp, ROLE_LEN - 1);
    printf("  Department [%s]: ", st->department); fflush(stdout); safeInput(tmp, DEPT_LEN); if (!strIsEmpty(tmp)) strncpy(st->department, tmp, DEPT_LEN - 1);
    printf("  Phone [%s]: ",      st->phone);      fflush(stdout); safeInput(tmp, PHONE_LEN);
    if (!strIsEmpty(tmp)) { if (isValidPhone(tmp)) strncpy(st->phone, tmp, PHONE_LEN - 1); else printf(CLR_RED "  Invalid, keeping old.\n" CLR_RESET); }
    printf("  Salary [%.2f]: ",   st->salary);     fflush(stdout); safeInput(tmp, 20);
    if (!strIsEmpty(tmp)) { float v = atof(tmp); if (v >= 0) st->salary = v; }

    saveAllStaff(buf, n);
    printf(CLR_GREEN "  ✓ Staff %d updated.\n" CLR_RESET, id);
    pressEnterToContinue();
}

void removeStaff(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_RED "\n  REMOVE STAFF\n" CLR_RESET);
    printf("  Enter Staff ID: "); fflush(stdout);
    int id = inputInt(1, 999999);

    FILE *f = fopen(DB_STAFF, "rb");
    if (!f) { printf(CLR_RED "  No staff database.\n" CLR_RESET); pressEnterToContinue(); return; }
    Staff all[MAX_STAFF]; int total = 0;
    while (total < MAX_STAFF && fread(&all[total], sizeof(Staff), 1, f) == 1) total++;
    fclose(f);

    int found = 0;
    for (int i = 0; i < total; i++) {
        if (all[i].staff_id == id && all[i].is_active) {
            printf("  Remove '%s'? (y/n): ", all[i].name);
            char c[4]; safeInput(c, 4);
            if (c[0] == 'y' || c[0] == 'Y') { all[i].is_active = 0; found = 1; }
            break;
        }
    }
    if (!found) { printf(CLR_RED "  Staff ID %d not found.\n" CLR_RESET, id); pressEnterToContinue(); return; }

    f = fopen(DB_STAFF, "wb");
    fwrite(all, sizeof(Staff), total, f);
    fclose(f);
    printf(CLR_GREEN "  ✓ Staff %d removed.\n" CLR_RESET, id);
    pressEnterToContinue();
}

void listStaff(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_CYAN "\n  ALL STAFF\n" CLR_RESET);
    Staff buf[MAX_STAFF]; int n = loadStaff(buf, MAX_STAFF);
    if (n == 0) { printf(CLR_YELLOW "  No staff records.\n" CLR_RESET); pressEnterToContinue(); return; }
    printStaffHeader();
    float total_salary = 0;
    for (int i = 0; i < n; i++) { printStaffRow(&buf[i]); total_salary += buf[i].salary; }
    printSeparator(95, '=');
    printf(CLR_YELLOW "  Staff: %d  |  Total Payroll: $%.2f\n" CLR_RESET, n, total_salary);
    pressEnterToContinue();
}

void searchStaff(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_CYAN "\n  SEARCH STAFF\n" CLR_RESET);
    printf("  1. By ID\n  2. By Name\n  3. By Department\n  Choice: "); fflush(stdout);
    int choice = inputInt(1, 3);

    Staff buf[MAX_STAFF]; int n = loadStaff(buf, MAX_STAFF);
    int found = 0;
    printStaffHeader();

    if (choice == 1) {
        printf("  Staff ID: "); fflush(stdout); int id = inputInt(1, 999999);
        for (int i = 0; i < n; i++) if (buf[i].staff_id == id) { printStaffRow(&buf[i]); found = 1; }
    } else if (choice == 2) {
        char kw[NAME_LEN]; printf("  Name: "); fflush(stdout); safeInput(kw, NAME_LEN); strToLower(kw);
        for (int i = 0; i < n; i++) {
            char ln[NAME_LEN]; strncpy(ln, buf[i].name, NAME_LEN - 1); strToLower(ln);
            if (strstr(ln, kw)) { printStaffRow(&buf[i]); found = 1; }
        }
    } else {
        char kw[DEPT_LEN]; printf("  Department: "); fflush(stdout); safeInput(kw, DEPT_LEN); strToLower(kw);
        for (int i = 0; i < n; i++) {
            char ld[DEPT_LEN]; strncpy(ld, buf[i].department, DEPT_LEN - 1); strToLower(ld);
            if (strstr(ld, kw)) { printStaffRow(&buf[i]); found = 1; }
        }
    }
    printSeparator(95, '=');
    if (!found) printf(CLR_RED "  No matching staff found.\n" CLR_RESET);
    pressEnterToContinue();
}
