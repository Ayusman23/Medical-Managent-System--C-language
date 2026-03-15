#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../config.h"
#include "ward.h"
#include "patient.h"     /* updatePatientWardNo, patientExists, getPatientById */
#include "utils.h"

/* ============================================================
   Internal helpers
   ============================================================ */
static int loadWards(Ward *arr, int maxn) {
    FILE *f = fopen(DB_WARDS, "rb");
    if (!f) return 0;
    int n = 0;
    Ward w;
    while (n < maxn && fread(&w, sizeof(Ward), 1, f) == 1)
        if (w.is_active) arr[n++] = w;
    fclose(f);
    return n;
}

/* Load all wards including inactive ones (preserves file positions) */
static int loadAllRawWards(Ward *arr, int maxn) {
    FILE *f = fopen(DB_WARDS, "rb");
    if (!f) return 0;
    int n = 0;
    while (n < maxn && fread(&arr[n], sizeof(Ward), 1, f) == 1) n++;
    fclose(f);
    return n;
}

static void saveRawWards(Ward *arr, int n) {
    FILE *f = fopen(DB_WARDS, "wb");
    if (!f) { perror("Cannot open wards.dat"); return; }
    fwrite(arr, sizeof(Ward), n, f);
    fclose(f);
}

static void saveAllWards(Ward *arr, int n) {
    FILE *f = fopen(DB_WARDS, "wb");
    if (!f) { perror("Cannot open wards.dat"); return; }
    fwrite(arr, sizeof(Ward), n, f);
    fclose(f);
}

static void appendWard(const Ward *w) {
    FILE *f = fopen(DB_WARDS, "ab");
    if (!f) { perror("Cannot open wards.dat"); return; }
    fwrite(w, sizeof(Ward), 1, f);
    fclose(f);
}

/* ============================================================
   addWard — create a new ward with a type and bed capacity
   ============================================================ */
void addWard(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_GREEN
           "\n  ╔══════════════════════════════════╗\n"
           "  ║     CREATE NEW WARD              ║\n"
           "  ╚══════════════════════════════════╝\n" CLR_RESET);

    Ward w;
    memset(&w, 0, sizeof(Ward));

    /* Auto-assign ward number */
    {
        FILE *f = fopen(DB_WARDS, "rb");
        int max_no = 0;
        if (f) {
            Ward tmp;
            while (fread(&tmp, sizeof(Ward), 1, f) == 1)
                if (tmp.ward_no > max_no) max_no = tmp.ward_no;
            fclose(f);
        }
        w.ward_no = max_no + 1;
    }
    printf(CLR_YELLOW "  Ward No: %d  (auto-assigned)\n\n" CLR_RESET, w.ward_no);

    printf("  Ward Types: General | ICU | ICCU | Pediatric | Maternity | Burns | Oncology\n");
    printf("  Ward Type : "); fflush(stdout);
    safeInput(w.ward_type, WTYPE_LEN);
    if (strIsEmpty(w.ward_type)) strncpy(w.ward_type, "General", WTYPE_LEN - 1);

    printf("  Total Beds (1-%d): ", MAX_BEDS_PER_WARD); fflush(stdout);
    w.total_beds    = inputInt(1, MAX_BEDS_PER_WARD);
    w.occupied_beds = 0;
    w.is_active     = 1;

    appendWard(&w);
    printf(CLR_GREEN "  ✓ Ward %d  [%s]  with %d beds created.\n" CLR_RESET,
           w.ward_no, w.ward_type, w.total_beds);
    pressEnterToContinue();
}

/* ============================================================
   admitToWard — place a patient in a ward bed and
                 update patient's ward_no in patients.dat
   ============================================================ */
void admitToWard(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_GREEN "\n  ADMIT PATIENT TO WARD\n" CLR_RESET);

    Ward buf[MAX_WARDS];
    int n = loadWards(buf, MAX_WARDS);
    if (n == 0) {
        printf(CLR_RED "  No wards exist. Please create a ward first.\n" CLR_RESET);
        pressEnterToContinue(); return;
    }

    /* Show available wards */
    printf(CLR_CYAN);
    printSeparator(60, '-');
    printf("  %-6s  %-20s  %-12s  %-10s\n","WardNo","Type","Beds","Available");
    printSeparator(60, '-');
    printf(CLR_RESET);
    for (int i = 0; i < n; i++) {
        int avail = buf[i].total_beds - buf[i].occupied_beds;
        printf("  %-6d  %-20s  %d/%-10d  %-10d\n",
               buf[i].ward_no, buf[i].ward_type, buf[i].occupied_beds,
               buf[i].total_beds, avail);
    }
    printf("\n");

    printf("  Enter Ward No   : "); fflush(stdout); int wno = inputInt(1, 999999);
    printf("  Enter Patient ID: "); fflush(stdout); int pid = inputInt(1, 999999);

    /* Validate patient */
    if (!patientExists(pid)) {
        printf(CLR_RED "  ✗ Patient ID %d does not exist.\n" CLR_RESET, pid);
        pressEnterToContinue(); return;
    }

    /* Find the ward in active list */
    int widx = -1;
    for (int i = 0; i < n; i++) if (buf[i].ward_no == wno) { widx = i; break; }
    if (widx < 0) {
        printf(CLR_RED "  ✗ Ward %d not found.\n" CLR_RESET, wno);
        pressEnterToContinue(); return;
    }

    Ward *w = &buf[widx];

    /* Capacity check */
    if (w->occupied_beds >= w->total_beds) {
        printf(CLR_RED "  ✗ Ward %d is FULL (%d/%d beds).\n" CLR_RESET,
               wno, w->occupied_beds, w->total_beds);
        pressEnterToContinue(); return;
    }

    /* Already-in-ward check */
    for (int i = 0; i < w->occupied_beds; i++) {
        if (w->patient_ids[i] == pid) {
            printf(CLR_YELLOW "  ! Patient %d is already in Ward %d.\n" CLR_RESET, pid, wno);
            pressEnterToContinue(); return;
        }
    }

    /* Admit: update ward record */
    w->patient_ids[w->occupied_beds++] = pid;
    saveAllWards(buf, n);

    /* ★ Update patient's ward_no — the critical integration point ★ */
    if (!updatePatientWardNo(pid, wno)) {
        printf(CLR_RED "  Warning: could not update patient ward number.\n" CLR_RESET);
    }

    Patient p;
    char pname[NAME_LEN] = "Unknown";
    if (getPatientById(pid, &p)) strncpy(pname, p.name, NAME_LEN - 1);

    printf(CLR_GREEN
           "\n  ╔══════════════════════════════════╗\n"
           "  ║  ✓ Patient Admitted!             ║\n"
           "  ╚══════════════════════════════════╝\n" CLR_RESET);
    printf("  Patient : %s (ID %d)\n", pname, pid);
    printf("  Ward    : %d  [%s]\n", wno, w->ward_type);
    printf("  Beds    : %d / %d occupied\n", w->occupied_beds, w->total_beds);
    pressEnterToContinue();
}

/* ============================================================
   dischargeFromWard — remove patient from ward bed and
                       clear patient's ward_no in patients.dat
   ============================================================ */
void dischargeFromWard(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_RED "\n  DISCHARGE PATIENT FROM WARD\n" CLR_RESET);

    Ward buf[MAX_WARDS];
    int n = loadWards(buf, MAX_WARDS);
    if (n == 0) {
        printf(CLR_RED "  No ward records.\n" CLR_RESET); pressEnterToContinue(); return;
    }

    printf("  Enter Ward No   : "); fflush(stdout); int wno = inputInt(1, 999999);
    printf("  Enter Patient ID: "); fflush(stdout); int pid = inputInt(1, 999999);

    int widx = -1;
    for (int i = 0; i < n; i++) if (buf[i].ward_no == wno) { widx = i; break; }
    if (widx < 0) {
        printf(CLR_RED "  Ward %d not found.\n" CLR_RESET, wno);
        pressEnterToContinue(); return;
    }

    Ward *w = &buf[widx];
    int pos = -1;
    for (int i = 0; i < w->occupied_beds; i++)
        if (w->patient_ids[i] == pid) { pos = i; break; }

    if (pos < 0) {
        printf(CLR_RED "  Patient %d not found in Ward %d.\n" CLR_RESET, pid, wno);
        pressEnterToContinue(); return;
    }

    /* Shift array left */
    for (int i = pos; i < w->occupied_beds - 1; i++)
        w->patient_ids[i] = w->patient_ids[i + 1];
    w->patient_ids[--w->occupied_beds] = 0;

    saveAllWards(buf, n);

    /* ★ Clear patient's ward_no in patients.dat ★ */
    updatePatientWardNo(pid, 0);

    printf(CLR_GREEN "  ✓ Patient %d discharged from Ward %d.\n" CLR_RESET, pid, wno);
    printf(CLR_YELLOW "  Beds now: %d/%d occupied.\n"
                      "  Reminder: Generate a final bill via Billing module.\n" CLR_RESET,
           w->occupied_beds, w->total_beds);
    pressEnterToContinue();
}

/* ============================================================
   viewWardStatus — detailed view of one or all wards showing
                    patient names if possible
   ============================================================ */
void viewWardStatus(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_CYAN "\n  WARD STATUS\n" CLR_RESET);
    printf("  Enter Ward No (0 = all wards): "); fflush(stdout);
    int wno = inputInt(0, 999999);

    Ward buf[MAX_WARDS];
    int n = loadWards(buf, MAX_WARDS);
    int found = 0;

    for (int i = 0; i < n; i++) {
        if (wno != 0 && buf[i].ward_no != wno) continue;
        Ward *w = &buf[i];
        int avail = w->total_beds - w->occupied_beds;
        float pct  = w->total_beds > 0
                     ? (float)w->occupied_beds / w->total_beds * 100.0f
                     : 0.0f;

        printf(CLR_CYAN); printSeparator(65, '='); printf(CLR_RESET);
        printf("  Ward No     : " CLR_BOLD "%d\n" CLR_RESET, w->ward_no);
        printf("  Type        : %s\n", w->ward_type);
        printf("  Capacity    : %d total | %d occupied | %d available\n",
               w->total_beds, w->occupied_beds, avail);
        printf("  Occupancy   : %.1f%%\n", pct);

        printf("  Patients    : ");
        if (w->occupied_beds == 0) {
            printf(CLR_GREEN "Empty\n" CLR_RESET);
        } else {
            printf("\n");
            for (int j = 0; j < w->occupied_beds; j++) {
                Patient p;
                if (getPatientById(w->patient_ids[j], &p))
                    printf("    Bed %2d: [%d] %s  (%s, %d yrs)\n",
                           j+1, p.patient_id, p.name, p.blood_group, p.age);
                else
                    printf("    Bed %2d: [ID %d] (record not found)\n",
                           j+1, w->patient_ids[j]);
            }
        }
        found = 1;
    }
    if (!found) printf(CLR_RED "  Ward %d not found.\n" CLR_RESET, wno);
    pressEnterToContinue();
}

/* ============================================================
   listAllWards — summary table
   ============================================================ */
void listAllWards(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_CYAN "\n  ALL WARDS — SUMMARY\n" CLR_RESET);
    Ward buf[MAX_WARDS];
    int n = loadWards(buf, MAX_WARDS);
    if (n == 0) { printf(CLR_YELLOW "  No ward records.\n" CLR_RESET); pressEnterToContinue(); return; }

    printf(CLR_CYAN);
    printSeparator(72, '=');
    printf("  %-8s  %-20s  %-6s  %-8s  %-8s  %-7s\n",
           "WardNo","Type","Total","Occupied","Available","Occ%");
    printSeparator(72, '-');
    printf(CLR_RESET);

    int grand_total = 0, grand_occ = 0;
    for (int i = 0; i < n; i++) {
        Ward *w = &buf[i];
        int avail = w->total_beds - w->occupied_beds;
        float pct  = w->total_beds > 0
                     ? (float)w->occupied_beds / w->total_beds * 100.0f : 0.0f;
        printf("  %-8d  %-20s  %-6d  %-8d  %-8d  %5.1f%%\n",
               w->ward_no, w->ward_type, w->total_beds,
               w->occupied_beds, avail, pct);
        grand_total += w->total_beds;
        grand_occ   += w->occupied_beds;
    }
    printSeparator(72, '=');
    float tot_pct = grand_total > 0 ? (float)grand_occ / grand_total * 100.0f : 0.0f;
    printf(CLR_YELLOW "  Total: %d wards | %d/%d beds occupied (%.1f%%)\n" CLR_RESET,
           n, grand_occ, grand_total, tot_pct);
    pressEnterToContinue();
}
