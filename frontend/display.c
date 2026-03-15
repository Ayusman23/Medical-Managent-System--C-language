#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stddef.h>
#include "../config.h"
#include "../backend/utils.h"

/* Include all modules to count active records for the dashboard */
#include "../backend/patient.h"
#include "../backend/doctor.h"
#include "../backend/appointment.h"
#include "../backend/billing.h"
#include "../backend/ward.h"
#include "../backend/staff.h"
#include "ui.h"

/* ============================================================
   Welcome banner
   ============================================================ */
void showWelcomeBanner(void) {
    CLEAR_SCREEN();
    printf(CLR_CYAN CLR_BOLD);
    printf("\n");
    printf("  ╔══════════════════════════════════════════════════════════════╗\n");
    printf("  ║                                                              ║\n");
    printf("  ║        ██╗  ██╗███╗   ███╗███████╗                          ║\n");
    printf("  ║        ██║  ██║████╗ ████║██╔════╝                          ║\n");
    printf("  ║        ███████║██╔████╔██║███████╗                          ║\n");
    printf("  ║        ██╔══██║██║╚██╔╝██║╚════██║                          ║\n");
    printf("  ║        ██║  ██║██║ ╚═╝ ██║███████║                          ║\n");
    printf("  ║        ╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝                          ║\n");
    printf("  ║                                                              ║\n");
    printf("  ║          HOSPITAL MANAGEMENT SYSTEM  v1.0                   ║\n");
    printf("  ║          Powered by Pure C + Binary File I/O                ║\n");
    printf("  ║                                                              ║\n");
    printf("  ╚══════════════════════════════════════════════════════════════╝\n");
    printf(CLR_RESET);

    char today[DATE_LEN];
    getCurrentDate(today);
    printf(CLR_YELLOW "  Date: %s\n\n" CLR_RESET, today);
}

/* Helper macros for reading just the is_active flag dynamically */
static int countActiveRecords(const char *filepath, size_t struct_size, size_t active_offset) {
    FILE *f = fopen(filepath, "rb");
    if (!f) return 0;
    int count = 0;
    char *buf = malloc(struct_size);
    if (!buf) { fclose(f); return 0; }
    while (fread(buf, struct_size, 1, f) == 1) {
        /* is_active is typically an int at the end of the struct */
        int is_active = *(int*)(buf + active_offset);
        if (is_active) count++;
    }
    free(buf);
    fclose(f);
    return count;
}

/* ============================================================
   Dashboard (real-time stats summary)
   ============================================================ */
void showDashboard(void) {
    /* Use the generic counter with offsetof to safely check active status */
    int c_pat  = countActiveRecords(DB_PATIENTS,     sizeof(Patient),     offsetof(Patient, is_active));
    int c_doc  = countActiveRecords(DB_DOCTORS,      sizeof(Doctor),      offsetof(Doctor, is_active));
    int c_appt = countActiveRecords(DB_APPOINTMENTS, sizeof(Appointment), offsetof(Appointment, is_active));
    int c_bill = countActiveRecords(DB_BILLING,      sizeof(Bill),        offsetof(Bill, is_active));
    int c_ward = countActiveRecords(DB_WARDS,        sizeof(Ward),        offsetof(Ward, is_active));
    int c_stf  = countActiveRecords(DB_STAFF,        sizeof(Staff),       offsetof(Staff, is_active));

    printf(CLR_BOLD CLR_CYAN "\n  ┌──────────────────────────────────────────┐\n");
    printf("  │       SYSTEM DASHBOARD (Overview)        │\n");
    printf("  └──────────────────────────────────────────┘\n" CLR_RESET);
    printf("  %-18s  %s\n", "Module", "Active Records");
    printSeparator(45, '-');
    printf("  %-18s  " CLR_GREEN "%d\n" CLR_RESET, "Patients",     c_pat);
    printf("  %-18s  " CLR_GREEN "%d\n" CLR_RESET, "Doctors",      c_doc);
    printf("  %-18s  " CLR_GREEN "%d\n" CLR_RESET, "Appointments", c_appt);
    printf("  %-18s  " CLR_GREEN "%d\n" CLR_RESET, "Bills",        c_bill);
    printf("  %-18s  " CLR_GREEN "%d\n" CLR_RESET, "Wards",        c_ward);
    printf("  %-18s  " CLR_GREEN "%d\n" CLR_RESET, "Staff",        c_stf);
    printSeparator(45, '-');
    printf(CLR_YELLOW "\n  System operates with real-time binary synchronization.\n" CLR_RESET);
    pressEnterToContinue();
}

/* ============================================================
   About screen
   ============================================================ */
void showAbout(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_MAGENTA);
    printSeparator(60, '=');
    printf("  HOSPITAL MANAGEMENT SYSTEM  v1.0\n");
    printSeparator(60, '=');
    printf(CLR_RESET);
    printf("  Language  : Pure C (C99 standard)\n");
    printf("  Storage   : Binary File I/O (fread/fwrite)\n");
    printf("  Build     : GNU Make + GCC\n\n");
    printf("  Modules:\n");
    printf("    ✓ Authentication (role-based: Admin / Staff)\n");
    printf("    ✓ Patient Management (CRUD + soft delete)\n");
    printf("    ✓ Doctor Management (CRUD)\n");
    printf("    ✓ Appointment Scheduling\n");
    printf("    ✓ Billing & Invoice Generation\n");
    printf("    ✓ Ward/Bed Management\n");
    printf("    ✓ Staff Management\n\n");
    printf("  Database files stored in: ./database/\n\n");
    printSeparator(60, '=');
    pressEnterToContinue();
}
