#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../config.h"
#include "doctor.h"
#include "utils.h"

static int loadDoctors(Doctor *arr, int maxn) {
    FILE *f = fopen(DB_DOCTORS, "rb");
    if (!f) return 0;
    int n = 0;
    Doctor d;
    while (n < maxn && fread(&d, sizeof(Doctor), 1, f) == 1)
        if (d.is_active) arr[n++] = d;
    fclose(f);
    return n;
}

static void saveAllDoctors(Doctor *arr, int n) {
    FILE *f = fopen(DB_DOCTORS, "wb");
    if (!f) { perror("Cannot open doctors.dat"); return; }
    fwrite(arr, sizeof(Doctor), n, f);
    fclose(f);
}

static void appendDoctor(const Doctor *d) {
    FILE *f = fopen(DB_DOCTORS, "ab");
    if (!f) { perror("Cannot open doctors.dat"); return; }
    fwrite(d, sizeof(Doctor), 1, f);
    fclose(f);
}

static void printDoctorHeader(void) {
    printf(CLR_CYAN);
    printSeparator(100, '=');
    printf("  %-6s  %-25s  %-22s  %-14s  %-20s\n",
           "ID","Name","Specialization","Phone","Available Days");
    printSeparator(100, '-');
    printf(CLR_RESET);
}

static void printDoctorRow(const Doctor *d) {
    printf("  %-6d  %-25s  %-22s  %-14s  %-20s\n",
           d->doctor_id, d->name, d->specialization, d->phone, d->available_days);
}

void addDoctor(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_GREEN "\n  ╔══════════════════════════════╗\n");
    printf("  ║     ADD NEW DOCTOR           ║\n");
    printf("  ╚══════════════════════════════╝\n" CLR_RESET);

    Doctor d;
    memset(&d, 0, sizeof(Doctor));

    {
        FILE *f = fopen(DB_DOCTORS, "rb");
        int max_id = 0;
        if (f) {
            Doctor tmp;
            while (fread(&tmp, sizeof(Doctor), 1, f) == 1)
                if (tmp.doctor_id > max_id) max_id = tmp.doctor_id;
            fclose(f);
        }
        d.doctor_id = max_id + 1;
    }
    printf(CLR_YELLOW "  Doctor ID: %d (auto-assigned)\n" CLR_RESET, d.doctor_id);

    printf("  Full Name            : "); fflush(stdout); safeInput(d.name, NAME_LEN);
    if (strIsEmpty(d.name)) { printf(CLR_RED "  Name cannot be empty.\n" CLR_RESET); pressEnterToContinue(); return; }

    printf("  Specialization       : "); fflush(stdout); safeInput(d.specialization, SPEC_LEN);
    if (strIsEmpty(d.specialization)) strncpy(d.specialization, "General", SPEC_LEN - 1);

    do {
        printf("  Phone                : "); fflush(stdout); safeInput(d.phone, PHONE_LEN);
        if (!isValidPhone(d.phone)) printf(CLR_RED "  Invalid phone. " CLR_RESET);
    } while (!isValidPhone(d.phone));

    printf("  Available Days (e.g. Mon,Wed,Fri): "); fflush(stdout); safeInput(d.available_days, DAYS_LEN);
    if (strIsEmpty(d.available_days)) strncpy(d.available_days, "Mon-Fri", DAYS_LEN - 1);

    d.is_active = 1;
    appendDoctor(&d);
    printf(CLR_GREEN "\n  ✓ Dr. %s added with ID %d.\n" CLR_RESET, d.name, d.doctor_id);
    pressEnterToContinue();
}

void searchDoctor(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_CYAN "\n  SEARCH DOCTOR\n" CLR_RESET);
    printf("  1. By ID\n  2. By Name\n  3. By Specialization\n  Choice: ");
    fflush(stdout);
    int choice = inputInt(1, 3);

    Doctor buf[MAX_DOCTORS];
    int n = loadDoctors(buf, MAX_DOCTORS);
    int found = 0;
    printDoctorHeader();

    if (choice == 1) {
        printf("  Enter Doctor ID: "); fflush(stdout);
        int id = inputInt(1, 999999);
        for (int i = 0; i < n; i++)
            if (buf[i].doctor_id == id) { printDoctorRow(&buf[i]); found = 1; }
    } else if (choice == 2) {
        char kw[NAME_LEN]; printf("  Name keyword: "); fflush(stdout); safeInput(kw, NAME_LEN); strToLower(kw);
        for (int i = 0; i < n; i++) {
            char ln[NAME_LEN]; strncpy(ln, buf[i].name, NAME_LEN - 1); strToLower(ln);
            if (strstr(ln, kw)) { printDoctorRow(&buf[i]); found = 1; }
        }
    } else {
        char kw[SPEC_LEN]; printf("  Specialization keyword: "); fflush(stdout); safeInput(kw, SPEC_LEN); strToLower(kw);
        for (int i = 0; i < n; i++) {
            char ls[SPEC_LEN]; strncpy(ls, buf[i].specialization, SPEC_LEN - 1); strToLower(ls);
            if (strstr(ls, kw)) { printDoctorRow(&buf[i]); found = 1; }
        }
    }
    printSeparator(100, '=');
    if (!found) printf(CLR_RED "  No matching doctors found.\n" CLR_RESET);
    pressEnterToContinue();
}

void updateDoctor(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_YELLOW "\n  UPDATE DOCTOR\n" CLR_RESET);
    printf("  Enter Doctor ID: "); fflush(stdout);
    int id = inputInt(1, 999999);

    Doctor buf[MAX_DOCTORS];
    int n = loadDoctors(buf, MAX_DOCTORS);
    int idx = -1;
    for (int i = 0; i < n; i++) if (buf[i].doctor_id == id) { idx = i; break; }
    if (idx < 0) { printf(CLR_RED "  Doctor ID %d not found.\n" CLR_RESET, id); pressEnterToContinue(); return; }

    Doctor *d = &buf[idx];
    printf("  Updating Dr. %s (press ENTER to keep current)\n", d->name);
    char tmp[ADDR_LEN];

    printf("  Name [%s]: ", d->name); fflush(stdout); safeInput(tmp, NAME_LEN); if (!strIsEmpty(tmp)) strncpy(d->name, tmp, NAME_LEN - 1);
    printf("  Specialization [%s]: ", d->specialization); fflush(stdout); safeInput(tmp, SPEC_LEN); if (!strIsEmpty(tmp)) strncpy(d->specialization, tmp, SPEC_LEN - 1);
    printf("  Phone [%s]: ", d->phone); fflush(stdout); safeInput(tmp, PHONE_LEN);
    if (!strIsEmpty(tmp)) { if (isValidPhone(tmp)) strncpy(d->phone, tmp, PHONE_LEN - 1); else printf(CLR_RED "  Invalid phone, keeping old.\n" CLR_RESET); }
    printf("  Available Days [%s]: ", d->available_days); fflush(stdout); safeInput(tmp, DAYS_LEN); if (!strIsEmpty(tmp)) strncpy(d->available_days, tmp, DAYS_LEN - 1);

    saveAllDoctors(buf, n);
    printf(CLR_GREEN "\n  ✓ Doctor %d updated.\n" CLR_RESET, id);
    pressEnterToContinue();
}

void deleteDoctor(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_RED "\n  DELETE DOCTOR\n" CLR_RESET);
    printf("  Enter Doctor ID: "); fflush(stdout);
    int id = inputInt(1, 999999);

    FILE *f = fopen(DB_DOCTORS, "rb");
    if (!f) { printf(CLR_RED "  No doctor database.\n" CLR_RESET); pressEnterToContinue(); return; }
    Doctor all[MAX_DOCTORS]; int total = 0;
    while (total < MAX_DOCTORS && fread(&all[total], sizeof(Doctor), 1, f) == 1) total++;
    fclose(f);

    int found = 0;
    for (int i = 0; i < total; i++) {
        if (all[i].doctor_id == id && all[i].is_active) {
            printf("  Delete Dr. %s? (y/n): ", all[i].name);
            char c[4]; safeInput(c, 4);
            if (c[0] == 'y' || c[0] == 'Y') { all[i].is_active = 0; found = 1; }
            break;
        }
    }
    if (!found) { printf(CLR_RED "  Doctor ID %d not found.\n" CLR_RESET, id); pressEnterToContinue(); return; }

    f = fopen(DB_DOCTORS, "wb");
    fwrite(all, sizeof(Doctor), total, f);
    fclose(f);
    printf(CLR_GREEN "  ✓ Doctor %d deleted.\n" CLR_RESET, id);
    pressEnterToContinue();
}

void listAllDoctors(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_CYAN "\n  ALL DOCTORS\n" CLR_RESET);
    Doctor buf[MAX_DOCTORS];
    int n = loadDoctors(buf, MAX_DOCTORS);
    if (n == 0) { printf(CLR_YELLOW "  No doctor records found.\n" CLR_RESET); pressEnterToContinue(); return; }
    printDoctorHeader();
    for (int i = 0; i < n; i++) printDoctorRow(&buf[i]);
    printSeparator(100, '=');
    printf(CLR_YELLOW "  Total: %d doctor(s)\n" CLR_RESET, n);
    pressEnterToContinue();
}

int doctorExists(int doctor_id) {
    FILE *f = fopen(DB_DOCTORS, "rb");
    if (!f) return 0;
    Doctor d;
    while (fread(&d, sizeof(Doctor), 1, f) == 1)
        if (d.doctor_id == doctor_id && d.is_active) { fclose(f); return 1; }
    fclose(f);
    return 0;
}

int getDoctorById(int doctor_id, Doctor *out) {
    FILE *f = fopen(DB_DOCTORS, "rb");
    if (!f) return 0;
    Doctor d;
    while (fread(&d, sizeof(Doctor), 1, f) == 1) {
        if (d.doctor_id == doctor_id && d.is_active) { *out = d; fclose(f); return 1; }
    }
    fclose(f);
    return 0;
}
