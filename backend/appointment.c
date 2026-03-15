#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../config.h"
#include "appointment.h"
#include "patient.h"
#include "doctor.h"
#include "utils.h"

/* ============================================================
   Internal helpers
   ============================================================ */
static int loadAllRawAppts(Appointment *arr, int maxn) {
    FILE *f = fopen(DB_APPOINTMENTS, "rb");
    if (!f) return 0;
    int n = 0;
    while (n < maxn && fread(&arr[n], sizeof(Appointment), 1, f) == 1) n++;
    fclose(f);
    return n;
}

static void saveRawAppts(Appointment *arr, int n) {
    FILE *f = fopen(DB_APPOINTMENTS, "wb");
    if (!f) { perror("Cannot open appointments.dat"); return; }
    fwrite(arr, sizeof(Appointment), n, f);
    fclose(f);
}

static int loadAppts(Appointment *arr, int maxn) {
    FILE *f = fopen(DB_APPOINTMENTS, "rb");
    if (!f) return 0;
    int n = 0;
    Appointment a;
    while (n < maxn && fread(&a, sizeof(Appointment), 1, f) == 1)
        if (a.is_active) arr[n++] = a;
    fclose(f);
    return n;
}

static void printApptHeader(void) {
    printf(CLR_CYAN);
    printSeparator(105, '=');
    printf("  %-6s  %-10s  %-25s  %-10s  %-25s  %-12s  %-10s  %-12s\n",
           "ApptID","PatientID","Patient Name","DoctorID","Doctor Name","Date","Time","Status");
    printSeparator(105, '-');
    printf(CLR_RESET);
}

static void printApptRow(const Appointment *a) {
    Patient  p; Doctor d;
    char pname[NAME_LEN] = "-", dname[NAME_LEN] = "-";
    if (getPatientById(a->patient_id, &p)) strncpy(pname, p.name, NAME_LEN - 1);
    if (getDoctorById(a->doctor_id,  &d)) strncpy(dname, d.name, NAME_LEN - 1);

    const char *color = CLR_RESET;
    if (strcmp(a->status, "Cancelled")  == 0) color = CLR_RED;
    if (strcmp(a->status, "Completed")  == 0) color = CLR_GREEN;
    if (strcmp(a->status, "Scheduled")  == 0) color = CLR_YELLOW;

    printf("  %-6d  %-10d  %-25s  %-10d  %-25s  %-12s  %-10s  %s%-12s%s\n",
           a->appt_id, a->patient_id, pname, a->doctor_id, dname,
           a->date, a->time_slot, color, a->status, CLR_RESET);
}

/* ============================================================
   bookAppointment
   ============================================================ */
void bookAppointment(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_GREEN
           "\n  ╔══════════════════════════════════╗\n"
           "  ║     BOOK APPOINTMENT             ║\n"
           "  ╚══════════════════════════════════╝\n" CLR_RESET);

    Appointment a;
    memset(&a, 0, sizeof(Appointment));

    {
        FILE *f = fopen(DB_APPOINTMENTS, "rb");
        int max_id = 0;
        if (f) {
            Appointment tmp;
            while (fread(&tmp, sizeof(Appointment), 1, f) == 1)
                if (tmp.appt_id > max_id) max_id = tmp.appt_id;
            fclose(f);
        }
        a.appt_id = max_id + 1;
    }

    printf("  Patient ID    : "); fflush(stdout); a.patient_id = inputInt(1, 999999);
    Patient p;
    if (!getPatientById(a.patient_id, &p)) {
        printf(CLR_RED "  ✗ Patient ID %d does not exist.\n" CLR_RESET, a.patient_id);
        pressEnterToContinue(); return;
    }
    printf("  Patient       : %s  (Blood: %s)\n", p.name, p.blood_group);

    printf("  Doctor ID     : "); fflush(stdout); a.doctor_id = inputInt(1, 999999);
    Doctor d;
    if (!getDoctorById(a.doctor_id, &d)) {
        printf(CLR_RED "  ✗ Doctor ID %d does not exist.\n" CLR_RESET, a.doctor_id);
        pressEnterToContinue(); return;
    }
    printf("  Doctor        : Dr. %s  [%s]  Available: %s\n",
           d.name, d.specialization, d.available_days);

    char dstr[DATE_LEN];
    do {
        printf("  Date [YYYY-MM-DD]: "); fflush(stdout); safeInput(dstr, DATE_LEN);
        if (!isValidDate(dstr)) printf(CLR_RED "  Invalid date. " CLR_RESET);
    } while (!isValidDate(dstr));
    strncpy(a.date, dstr, DATE_LEN - 1);

    printf("  Time Slot [HH:MM] (e.g. 09:00): "); fflush(stdout);
    safeInput(a.time_slot, TIME_LEN);
    if (strIsEmpty(a.time_slot)) strncpy(a.time_slot, "09:00", TIME_LEN - 1);

    strncpy(a.status, "Scheduled", STATUS_LEN - 1);
    a.is_active = 1;

    FILE *f = fopen(DB_APPOINTMENTS, "ab");
    if (!f) { perror("Cannot open appointments.dat"); pressEnterToContinue(); return; }
    fwrite(&a, sizeof(Appointment), 1, f);
    fclose(f);

    printf(CLR_GREEN
           "\n  ╔══════════════════════════════════╗\n"
           "  ║  ✓ Appointment Booked!           ║\n"
           "  ╚══════════════════════════════════╝\n" CLR_RESET);
    printf("  Appt #%d  |  %s  |  Dr. %s  |  %s  %s\n",
           a.appt_id, p.name, d.name, a.date, a.time_slot);
    pressEnterToContinue();
}

/* ============================================================
   cancelAppointment
   ============================================================ */
void cancelAppointment(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_RED "\n  CANCEL / UPDATE APPOINTMENT STATUS\n" CLR_RESET);
    printf("  Enter Appointment ID: "); fflush(stdout);
    int id = inputInt(1, 999999);

    Appointment all[MAX_APPOINTMENTS]; int total = loadAllRawAppts(all, MAX_APPOINTMENTS);
    int found = 0;
    for (int i = 0; i < total; i++) {
        if (all[i].appt_id == id && all[i].is_active) {
            printf("  Appointment #%d  Status: %s\n", id, all[i].status);
            printf("  1. Mark as Cancelled\n  2. Mark as Completed\n  0. Back\n  Choice: ");
            fflush(stdout);
            int ch = inputInt(0, 2);
            if (ch == 1) { strncpy(all[i].status, "Cancelled",  STATUS_LEN -1); printf(CLR_RED  "  ✓ Marked Cancelled.\n" CLR_RESET); }
            if (ch == 2) { strncpy(all[i].status, "Completed",  STATUS_LEN -1); printf(CLR_GREEN "  ✓ Marked Completed.\n" CLR_RESET); }
            found = 1; break;
        }
    }
    if (!found) printf(CLR_RED "  Appointment ID %d not found.\n" CLR_RESET, id);
    saveRawAppts(all, total);
    pressEnterToContinue();
}

/* ============================================================
   viewAppointmentsByDoctor
   ============================================================ */
void viewAppointmentsByDoctor(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_CYAN "\n  APPOINTMENTS BY DOCTOR\n" CLR_RESET);
    printf("  Enter Doctor ID (0 = all): "); fflush(stdout);
    int did = inputInt(0, 999999);

    Appointment buf[MAX_APPOINTMENTS];
    int n = loadAppts(buf, MAX_APPOINTMENTS);
    int found = 0;
    printApptHeader();
    for (int i = 0; i < n; i++) {
        if (did == 0 || buf[i].doctor_id == did) { printApptRow(&buf[i]); found++; }
    }
    printSeparator(105, '=');
    if (!found) printf(CLR_RED "  No appointments found.\n" CLR_RESET);
    else        printf(CLR_YELLOW "  %d appointment(s).\n" CLR_RESET, found);
    pressEnterToContinue();
}

/* ============================================================
   viewAppointmentsByPatient
   ============================================================ */
void viewAppointmentsByPatient(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_CYAN "\n  APPOINTMENTS BY PATIENT\n" CLR_RESET);
    printf("  Enter Patient ID: "); fflush(stdout);
    int pid = inputInt(1, 999999);

    Appointment buf[MAX_APPOINTMENTS];
    int n = loadAppts(buf, MAX_APPOINTMENTS);
    int found = 0;
    printApptHeader();
    for (int i = 0; i < n; i++)
        if (buf[i].patient_id == pid) { printApptRow(&buf[i]); found++; }
    printSeparator(105, '=');
    if (!found) printf(CLR_RED "  No appointments for Patient %d.\n" CLR_RESET, pid);
    else        printf(CLR_YELLOW "  %d appointment(s).\n" CLR_RESET, found);
    pressEnterToContinue();
}

/* ============================================================
   listAllAppointments
   ============================================================ */
void listAllAppointments(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_CYAN "\n  ALL APPOINTMENTS\n" CLR_RESET);
    Appointment buf[MAX_APPOINTMENTS];
    int n = loadAppts(buf, MAX_APPOINTMENTS);
    if (n == 0) { printf(CLR_YELLOW "  No appointment records.\n" CLR_RESET); pressEnterToContinue(); return; }
    printApptHeader();
    int sched = 0, canc = 0, comp = 0;
    for (int i = 0; i < n; i++) {
        printApptRow(&buf[i]);
        if (strcmp(buf[i].status, "Scheduled") == 0) sched++;
        if (strcmp(buf[i].status, "Cancelled") == 0) canc++;
        if (strcmp(buf[i].status, "Completed") == 0) comp++;
    }
    printSeparator(105, '=');
    printf(CLR_YELLOW "  Total: %d  |  " CLR_YELLOW "Scheduled: %d  |  "
                      CLR_GREEN "Completed: %d  |  " CLR_RED "Cancelled: %d\n" CLR_RESET,
           n, sched, comp, canc);
    pressEnterToContinue();
}

/* ============================================================
   viewTodayAppointments — useful for daily workflow
   ============================================================ */
void viewTodayAppointments(void) {
    CLEAR_SCREEN();
    char today[DATE_LEN];
    getCurrentDate(today);
    printf(CLR_BOLD CLR_GREEN "\n  TODAY'S APPOINTMENTS  [%s]\n" CLR_RESET, today);

    Appointment buf[MAX_APPOINTMENTS];
    int n = loadAppts(buf, MAX_APPOINTMENTS);
    int found = 0;
    printApptHeader();
    for (int i = 0; i < n; i++) {
        if (strcmp(buf[i].date, today) == 0) { printApptRow(&buf[i]); found++; }
    }
    printSeparator(105, '=');
    if (!found) printf(CLR_YELLOW "  No appointments scheduled for today.\n" CLR_RESET);
    else        printf(CLR_YELLOW "  %d appointment(s) today.\n" CLR_RESET, found);
    pressEnterToContinue();
}
