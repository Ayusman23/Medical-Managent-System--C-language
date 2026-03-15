#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../config.h"
#include "patient.h"
#include "utils.h"

/* ============================================================
   Internal: load ALL active patient records into array
   ============================================================ */
static int loadPatients(Patient *arr, int maxn) {
    FILE *f = fopen(DB_PATIENTS, "rb");
    if (!f) return 0;
    int n = 0;
    Patient p;
    while (n < maxn && fread(&p, sizeof(Patient), 1, f) == 1)
        if (p.is_active) arr[n++] = p;
    fclose(f);
    return n;
}

/* ============================================================
   Internal: overwrite entire file with raw array (all records,
   including soft-deleted ones) — preserves DB positions
   ============================================================ */
static int loadAllRaw(Patient *arr, int maxn) {
    FILE *f = fopen(DB_PATIENTS, "rb");
    if (!f) return 0;
    int n = 0;
    while (n < maxn && fread(&arr[n], sizeof(Patient), 1, f) == 1) n++;
    fclose(f);
    return n;
}

static void saveRaw(Patient *arr, int n) {
    FILE *f = fopen(DB_PATIENTS, "wb");
    if (!f) { perror("Cannot write patients.dat"); return; }
    fwrite(arr, sizeof(Patient), n, f);
    fclose(f);
}

static void saveAllPatients(Patient *arr, int n) {
    /* arr contains only active records; build new file from scratch */
    FILE *f = fopen(DB_PATIENTS, "wb");
    if (!f) { perror("Cannot open patients.dat"); return; }
    fwrite(arr, sizeof(Patient), n, f);
    fclose(f);
}

static void appendPatient(const Patient *p) {
    FILE *f = fopen(DB_PATIENTS, "ab");
    if (!f) { perror("Cannot open patients.dat"); return; }
    fwrite(p, sizeof(Patient), 1, f);
    fclose(f);
}

/* ============================================================
   Print table header / row
   ============================================================ */
static void printPatientHeader(void) {
    printf(CLR_CYAN);
    printSeparator(115, '=');
    printf("  %-6s  %-25s  %-4s  %-8s  %-6s  %-15s  %-12s  %-6s\n",
           "ID","Name","Age","Gender","Blood","Phone","Admitted","Ward");
    printSeparator(115, '-');
    printf(CLR_RESET);
}

static void printPatientRow(const Patient *p) {
    const char *ward = (p->ward_no > 0) ? "" : "-";
    if (p->ward_no > 0)
        printf("  %-6d  %-25s  %-4d  %-8s  %-6s  %-15s  %-12s  %-6d\n",
               p->patient_id, p->name, p->age, p->gender,
               p->blood_group, p->phone, p->admitted_date, p->ward_no);
    else
        printf("  %-6d  %-25s  %-4d  %-8s  %-6s  %-15s  %-12s  %-6s\n",
               p->patient_id, p->name, p->age, p->gender,
               p->blood_group, p->phone, p->admitted_date, ward);
}

/* ============================================================
   addPatient
   ============================================================ */
void addPatient(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_GREEN
           "\n  ╔══════════════════════════════════╗\n"
           "  ║     REGISTER NEW PATIENT         ║\n"
           "  ╚══════════════════════════════════╝\n" CLR_RESET);

    Patient p;
    memset(&p, 0, sizeof(Patient));

    /* Auto-increment ID */
    {
        FILE *f = fopen(DB_PATIENTS, "rb");
        int max_id = 0;
        if (f) {
            Patient tmp;
            while (fread(&tmp, sizeof(Patient), 1, f) == 1)
                if (tmp.patient_id > max_id) max_id = tmp.patient_id;
            fclose(f);
        }
        p.patient_id = max_id + 1;
    }
    printf(CLR_YELLOW "  Patient ID : %d  (auto-assigned)\n\n" CLR_RESET, p.patient_id);

    printf("  Full Name       : "); fflush(stdout);
    safeInput(p.name, NAME_LEN);
    if (strIsEmpty(p.name)) {
        printf(CLR_RED "  Name cannot be empty.\n" CLR_RESET);
        pressEnterToContinue(); return;
    }

    printf("  Age  [1-120]    : "); fflush(stdout);
    p.age = inputInt(1, 120);

    printf("  Gender (M/F/O)  : "); fflush(stdout);
    safeInput(p.gender, GENDER_LEN);
    strToUpper(p.gender);
    if (strIsEmpty(p.gender)) strncpy(p.gender, "M", GENDER_LEN - 1);

    printf("  Blood Group     : "); fflush(stdout);
    safeInput(p.blood_group, BLOODGRP_LEN);
    strToUpper(p.blood_group);
    if (strIsEmpty(p.blood_group)) strncpy(p.blood_group, "O+", BLOODGRP_LEN - 1);

    do {
        printf("  Phone (digits)  : "); fflush(stdout);
        safeInput(p.phone, PHONE_LEN);
        if (!isValidPhone(p.phone))
            printf(CLR_RED "  Invalid phone number. " CLR_RESET);
    } while (!isValidPhone(p.phone));

    printf("  Address         : "); fflush(stdout);
    safeInput(p.address, ADDR_LEN);

    /* Admission date */
    char today[DATE_LEN];
    getCurrentDate(today);
    printf("  Admission Date  [YYYY-MM-DD, blank = today %s]: ", today);
    fflush(stdout);
    char tmp_date[DATE_LEN];
    safeInput(tmp_date, DATE_LEN);
    if (strIsEmpty(tmp_date)) {
        strncpy(p.admitted_date, today, DATE_LEN - 1);
    } else {
        while (!isValidDate(tmp_date)) {
            printf(CLR_RED "  Invalid. Enter YYYY-MM-DD: " CLR_RESET);
            safeInput(tmp_date, DATE_LEN);
        }
        strncpy(p.admitted_date, tmp_date, DATE_LEN - 1);
    }

    p.ward_no   = 0;   /* assigned separately via Ward module */
    p.is_active = 1;

    appendPatient(&p);

    printf(CLR_GREEN
           "\n  ╔══════════════════════════════════╗\n"
           "  ║  ✓ Patient Registered!           ║\n"
           "  ╚══════════════════════════════════╝\n" CLR_RESET);
    printf("  ID: %d  |  Name: %s\n", p.patient_id, p.name);
    printf("  Tip: Use Ward Management to assign a bed.\n");
    pressEnterToContinue();
}

/* ============================================================
   searchPatient
   ============================================================ */
void searchPatient(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_CYAN "\n  SEARCH PATIENT\n" CLR_RESET);
    printf("  1. By Patient ID\n"
           "  2. By Name (partial)\n"
           "  3. By Phone\n"
           "  4. By Blood Group\n"
           "  Choice: "); fflush(stdout);
    int choice = inputInt(1, 4);

    Patient buf[MAX_PATIENTS];
    int n = loadPatients(buf, MAX_PATIENTS);
    int found = 0;
    printPatientHeader();

    char kw[ADDR_LEN];

    if (choice == 1) {
        printf("  Enter Patient ID: "); fflush(stdout);
        int id = inputInt(1, 999999);
        for (int i = 0; i < n; i++)
            if (buf[i].patient_id == id) { printPatientRow(&buf[i]); found = 1; }

    } else if (choice == 2) {
        printf("  Name keyword: "); fflush(stdout); safeInput(kw, NAME_LEN); strToLower(kw);
        for (int i = 0; i < n; i++) {
            char ln[NAME_LEN]; strncpy(ln, buf[i].name, NAME_LEN - 1); ln[NAME_LEN-1]='\0'; strToLower(ln);
            if (strstr(ln, kw)) { printPatientRow(&buf[i]); found = 1; }
        }
    } else if (choice == 3) {
        printf("  Phone: "); fflush(stdout); safeInput(kw, PHONE_LEN);
        for (int i = 0; i < n; i++)
            if (strstr(buf[i].phone, kw)) { printPatientRow(&buf[i]); found = 1; }

    } else {
        printf("  Blood Group: "); fflush(stdout); safeInput(kw, BLOODGRP_LEN); strToUpper(kw);
        for (int i = 0; i < n; i++)
            if (strcmp(buf[i].blood_group, kw) == 0) { printPatientRow(&buf[i]); found = 1; }
    }

    printSeparator(115, '=');
    if (!found) printf(CLR_RED "  No matching patients found.\n" CLR_RESET);
    else        printf(CLR_YELLOW "  Found %d match(es).\n" CLR_RESET, found);
    pressEnterToContinue();
}

/* ============================================================
   updatePatient
   ============================================================ */
void updatePatient(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_YELLOW "\n  UPDATE PATIENT RECORD\n" CLR_RESET);
    printf("  Enter Patient ID: "); fflush(stdout);
    int id = inputInt(1, 999999);

    /* Load raw so positions are preserved */
    Patient all[MAX_PATIENTS]; int total = loadAllRaw(all, MAX_PATIENTS);
    int idx = -1;
    for (int i = 0; i < total; i++)
        if (all[i].patient_id == id && all[i].is_active) { idx = i; break; }

    if (idx < 0) {
        printf(CLR_RED "  Patient ID %d not found.\n" CLR_RESET, id);
        pressEnterToContinue(); return;
    }

    Patient *p = &all[idx];
    printf("\n  Updating: %s (press ENTER to keep current value)\n\n", p->name);
    char tmp[ADDR_LEN];

    printf("  Name      [%s]: ", p->name);       fflush(stdout); safeInput(tmp, NAME_LEN);
    if (!strIsEmpty(tmp)) strncpy(p->name, tmp, NAME_LEN - 1);

    printf("  Age       [%d]: ", p->age);         fflush(stdout); safeInput(tmp, 8);
    if (!strIsEmpty(tmp)) { int v = atoi(tmp); if (v >= 1 && v <= 120) p->age = v; }

    printf("  Gender    [%s]: ", p->gender);      fflush(stdout); safeInput(tmp, GENDER_LEN);
    if (!strIsEmpty(tmp)) { strncpy(p->gender, tmp, GENDER_LEN - 1); strToUpper(p->gender); }

    printf("  BloodGrp  [%s]: ", p->blood_group); fflush(stdout); safeInput(tmp, BLOODGRP_LEN);
    if (!strIsEmpty(tmp)) { strncpy(p->blood_group, tmp, BLOODGRP_LEN - 1); strToUpper(p->blood_group); }

    printf("  Phone     [%s]: ", p->phone);       fflush(stdout); safeInput(tmp, PHONE_LEN);
    if (!strIsEmpty(tmp)) {
        if (isValidPhone(tmp)) strncpy(p->phone, tmp, PHONE_LEN - 1);
        else printf(CLR_RED "  Invalid phone, keeping old.\n" CLR_RESET);
    }

    printf("  Address   [%s]: ", p->address);     fflush(stdout); safeInput(tmp, ADDR_LEN);
    if (!strIsEmpty(tmp)) strncpy(p->address, tmp, ADDR_LEN - 1);

    saveRaw(all, total);
    printf(CLR_GREEN "\n  ✓ Patient %d record updated.\n" CLR_RESET, id);
    pressEnterToContinue();
}

/* ============================================================
   deletePatient  (soft delete)
   ============================================================ */
void deletePatient(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_RED "\n  DELETE PATIENT\n" CLR_RESET);
    printf("  Enter Patient ID: "); fflush(stdout);
    int id = inputInt(1, 999999);

    Patient all[MAX_PATIENTS]; int total = loadAllRaw(all, MAX_PATIENTS);
    int found = 0;
    for (int i = 0; i < total; i++) {
        if (all[i].patient_id == id && all[i].is_active) {
            printf("  Are you sure you want to delete '%s'? (y/n): ", all[i].name);
            char c[4]; safeInput(c, 4);
            if (c[0] == 'y' || c[0] == 'Y') {
                all[i].is_active = 0;
                all[i].ward_no   = 0;   /* clear ward reference */
                found = 1;
                printf(CLR_GREEN "  ✓ Patient %d (%s) deleted.\n" CLR_RESET,
                       all[i].patient_id, all[i].name);
            } else {
                printf(CLR_YELLOW "  Deletion cancelled.\n" CLR_RESET);
            }
            break;
        }
    }
    if (!found) printf(CLR_RED "  Patient ID %d not found.\n" CLR_RESET, id);

    saveRaw(all, total);
    pressEnterToContinue();
}

/* ============================================================
   listAllPatients
   ============================================================ */
void listAllPatients(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_CYAN "\n  ALL PATIENTS\n" CLR_RESET);
    Patient buf[MAX_PATIENTS];
    int n = loadPatients(buf, MAX_PATIENTS);
    if (n == 0) { printf(CLR_YELLOW "  No patient records found.\n" CLR_RESET); pressEnterToContinue(); return; }
    printPatientHeader();
    for (int i = 0; i < n; i++) printPatientRow(&buf[i]);
    printSeparator(115, '=');
    printf(CLR_YELLOW "  Total: %d active patient(s)\n" CLR_RESET, n);
    pressEnterToContinue();
}

/* ============================================================
   viewAdmittedPatients  — patients with ward_no > 0
   ============================================================ */
void viewAdmittedPatients(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_GREEN "\n  CURRENTLY ADMITTED PATIENTS\n" CLR_RESET);
    Patient buf[MAX_PATIENTS];
    int n = loadPatients(buf, MAX_PATIENTS);
    int found = 0;
    printPatientHeader();
    for (int i = 0; i < n; i++) {
        if (buf[i].ward_no > 0) { printPatientRow(&buf[i]); found++; }
    }
    printSeparator(115, '=');
    if (!found) printf(CLR_YELLOW "  No patients currently admitted to any ward.\n" CLR_RESET);
    else        printf(CLR_YELLOW "  Total admitted: %d\n" CLR_RESET, found);
    pressEnterToContinue();
}

/* ============================================================
   dischargePatient  — reset ward_no = 0, confirm with billing
   ============================================================ */
void dischargePatient(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_RED "\n  DISCHARGE PATIENT\n" CLR_RESET);
    printf("  Enter Patient ID to discharge: "); fflush(stdout);
    int id = inputInt(1, 999999);

    Patient all[MAX_PATIENTS]; int total = loadAllRaw(all, MAX_PATIENTS);
    int idx = -1;
    for (int i = 0; i < total; i++)
        if (all[i].patient_id == id && all[i].is_active) { idx = i; break; }

    if (idx < 0) {
        printf(CLR_RED "  Patient %d not found.\n" CLR_RESET, id);
        pressEnterToContinue(); return;
    }

    printf("  Patient: %s  |  Ward: %d\n", all[idx].name, all[idx].ward_no);
    if (all[idx].ward_no == 0) {
        printf(CLR_YELLOW "  Patient is not currently admitted to any ward.\n" CLR_RESET);
        pressEnterToContinue(); return;
    }

    printf("  Confirm discharge? (y/n): "); fflush(stdout);
    char c[4]; safeInput(c, 4);
    if (c[0] != 'y' && c[0] != 'Y') {
        printf(CLR_YELLOW "  Cancelled.\n" CLR_RESET); pressEnterToContinue(); return;
    }

    int old_ward = all[idx].ward_no;
    all[idx].ward_no = 0;
    saveRaw(all, total);

    printf(CLR_GREEN "  ✓ Patient %d discharged from Ward %d.\n" CLR_RESET, id, old_ward);
    printf(CLR_YELLOW "  Reminder: Generate a final bill via the Billing module.\n" CLR_RESET);
    pressEnterToContinue();
}

/* ============================================================
   updatePatientWardNo  — called by ward.c when admitting/discharging
   Returns 1 on success, 0 if patient not found
   ============================================================ */
int updatePatientWardNo(int patient_id, int ward_no) {
    Patient all[MAX_PATIENTS]; int total = loadAllRaw(all, MAX_PATIENTS);
    for (int i = 0; i < total; i++) {
        if (all[i].patient_id == patient_id && all[i].is_active) {
            all[i].ward_no = ward_no;
            saveRaw(all, total);
            return 1;
        }
    }
    return 0;
}

/* ============================================================
   Helpers used by other modules
   ============================================================ */
int patientExists(int patient_id) {
    FILE *f = fopen(DB_PATIENTS, "rb");
    if (!f) return 0;
    Patient p;
    while (fread(&p, sizeof(Patient), 1, f) == 1)
        if (p.patient_id == patient_id && p.is_active) { fclose(f); return 1; }
    fclose(f);
    return 0;
}

int getPatientById(int patient_id, Patient *out) {
    FILE *f = fopen(DB_PATIENTS, "rb");
    if (!f) return 0;
    Patient p;
    while (fread(&p, sizeof(Patient), 1, f) == 1) {
        if (p.patient_id == patient_id && p.is_active) { *out = p; fclose(f); return 1; }
    }
    fclose(f);
    return 0;
}
