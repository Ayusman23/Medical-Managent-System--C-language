#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../config.h"
#include "billing.h"
#include "patient.h"
#include "utils.h"

/* ============================================================
   Internal helpers
   ============================================================ */
static int loadBills(Bill *arr, int maxn) {
    FILE *f = fopen(DB_BILLING, "rb");
    if (!f) return 0;
    int n = 0;
    Bill b;
    while (n < maxn && fread(&b, sizeof(Bill), 1, f) == 1)
        if (b.is_active) arr[n++] = b;
    fclose(f);
    return n;
}

static int loadAllRawBills(Bill *arr, int maxn) {
    FILE *f = fopen(DB_BILLING, "rb");
    if (!f) return 0;
    int n = 0;
    while (n < maxn && fread(&arr[n], sizeof(Bill), 1, f) == 1) n++;
    fclose(f);
    return n;
}

static void saveRawBills(Bill *arr, int n) {
    FILE *f = fopen(DB_BILLING, "wb");
    if (!f) { perror("Cannot write billing.dat"); return; }
    fwrite(arr, sizeof(Bill), n, f);
    fclose(f);
}

static void appendBill(const Bill *b) {
    FILE *f = fopen(DB_BILLING, "ab");
    if (!f) { perror("Cannot open billing.dat"); return; }
    fwrite(b, sizeof(Bill), 1, f);
    fclose(f);
}

static void printBillHeader(void) {
    printf(CLR_CYAN);
    printSeparator(100, '=');
    printf("  %-6s  %-10s  %-20s  %-10s  %-10s  %-10s  %-10s  %-8s\n",
           "BillID","PatientID","PatientName","Room($)","Med($)","Doc($)","Total($)","Status");
    printSeparator(100, '-');
    printf(CLR_RESET);
}

static void printBillRow(const Bill *b) {
    Patient p;
    char pname[NAME_LEN] = "-";
    if (getPatientById(b->patient_id, &p)) strncpy(pname, p.name, 19);
    pname[19] = '\0';

    const char *status_str = b->paid_status ? CLR_GREEN "PAID" CLR_RESET : CLR_RED "UNPAID" CLR_RESET;

    printf("  %-6d  %-10d  %-20s  %-10.2f  %-10.2f  %-10.2f  %-10.2f  %s\n",
           b->bill_id, b->patient_id, pname, b->room_charges,
           b->medicine_charges, b->doctor_fee, b->total, status_str);
}

static void printInvoice(const Bill *b) {
    Patient p;
    char pname[NAME_LEN] = "Unknown", pphone[PHONE_LEN] = "-", paddr[ADDR_LEN] = "-";
    if (getPatientById(b->patient_id, &p)) {
        strncpy(pname, p.name, NAME_LEN - 1);
        strncpy(pphone, p.phone, PHONE_LEN - 1);
        strncpy(paddr, p.address, ADDR_LEN - 1);
    }

    printf(CLR_BOLD CLR_CYAN);
    printSeparator(60, '=');
    printf("                  HOSPITAL INVOICE                 \n");
    printSeparator(60, '=');
    printf(CLR_RESET);
    printf("  Bill ID      : %d\n",    b->bill_id);
    printf("  Date         : %s\n",    b->bill_date);
    printf("  Patient ID   : %d\n",    b->patient_id);
    printf("  Patient Name : %s\n",    pname);
    printf("  Contact      : %s | %s\n", pphone, paddr);
    printSeparator(60, '-');
    printf("  1. Room Charges        : $%10.2f\n", b->room_charges);
    printf("  2. Medicine & Pharmacy : $%10.2f\n", b->medicine_charges);
    printf("  3. Doctor & OP Fees    : $%10.2f\n", b->doctor_fee);
    printSeparator(60, '-');
    printf(CLR_BOLD "  TOTAL AMOUNT           : $%10.2f\n" CLR_RESET, b->total);
    printf("\n  Payment Status : %s\n", b->paid_status ? CLR_GREEN "PAID" CLR_RESET : CLR_RED "UNPAID - DUE" CLR_RESET);
    printSeparator(60, '=');
}

/* ============================================================
   generateBill  - includes smart suggestions based on patient state
   ============================================================ */
void generateBill(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_GREEN "\n  ╔══════════════════════════════════╗\n");
    printf("  ║     GENERATE BILL                ║\n");
    printf("  ╚══════════════════════════════════╝\n" CLR_RESET);

    Bill b;
    memset(&b, 0, sizeof(Bill));

    {
        FILE *f = fopen(DB_BILLING, "rb");
        int max_id = 0;
        if (f) {
            Bill tmp;
            while (fread(&tmp, sizeof(Bill), 1, f) == 1)
                if (tmp.bill_id > max_id) max_id = tmp.bill_id;
            fclose(f);
        }
        b.bill_id = max_id + 1;
    }

    printf("  Patient ID        : "); fflush(stdout); b.patient_id = inputInt(1, 999999);
    Patient p;
    if (!getPatientById(b.patient_id, &p)) {
        printf(CLR_RED "  ✗ Patient %d does not exist.\n" CLR_RESET, b.patient_id);
        pressEnterToContinue(); return;
    }

    printf(CLR_CYAN "  Billing for: %s\n" CLR_RESET, p.name);
    if (p.ward_no > 0) {
        printf(CLR_YELLOW "  [INFO] Patient is currently admitted in Ward %d since %s.\n" CLR_RESET,
               p.ward_no, p.admitted_date);
    }

    printf("  Room Charges ($)  : "); fflush(stdout); b.room_charges      = inputFloat(0, 1000000);
    printf("  Medicine Charges  : "); fflush(stdout); b.medicine_charges  = inputFloat(0, 1000000);
    printf("  Doctor Fee ($)    : "); fflush(stdout); b.doctor_fee        = inputFloat(0, 1000000);

    b.total = b.room_charges + b.medicine_charges + b.doctor_fee;
    b.paid_status = 0;
    getCurrentDate(b.bill_date);
    b.is_active = 1;

    appendBill(&b);
    printf(CLR_GREEN "\n  ✓ Bill #%d generated successfully.\n" CLR_RESET, b.bill_id);
    printInvoice(&b);
    pressEnterToContinue();
}

void viewBill(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_CYAN "\n  VIEW BILL\n" CLR_RESET);
    printf("  Enter Bill ID: "); fflush(stdout);
    int id = inputInt(1, 999999);

    Bill buf[MAX_BILLS];
    int n = loadBills(buf, MAX_BILLS);
    int found = 0;
    for (int i = 0; i < n; i++) {
        if (buf[i].bill_id == id) { printInvoice(&buf[i]); found = 1; break; }
    }
    if (!found) printf(CLR_RED "  Bill ID %d not found.\n" CLR_RESET, id);
    pressEnterToContinue();
}

void markPaid(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_YELLOW "\n  MARK BILL AS PAID\n" CLR_RESET);
    printf("  Enter Bill ID: "); fflush(stdout);
    int id = inputInt(1, 999999);

    Bill all[MAX_BILLS]; int total = loadAllRawBills(all, MAX_BILLS);

    int found = 0;
    for (int i = 0; i < total; i++) {
        if (all[i].bill_id == id && all[i].is_active) {
            if (all[i].paid_status) {
                printf(CLR_YELLOW "  Bill #%d is already marked as PAID.\n" CLR_RESET, id);
            } else {
                all[i].paid_status = 1;
                printf(CLR_GREEN "  ✓ Payment received for Bill #%d.\n" CLR_RESET, id);
            }
            found = 1; break;
        }
    }
    if (!found) {
        printf(CLR_RED "  Bill ID %d not found.\n" CLR_RESET, id);
    } else {
        saveRawBills(all, total);
    }

    pressEnterToContinue();
}

void listAllBills(void) {
    CLEAR_SCREEN();
    printf(CLR_BOLD CLR_CYAN "\n  ALL BILLS\n" CLR_RESET);
    Bill buf[MAX_BILLS];
    int n = loadBills(buf, MAX_BILLS);
    if (n == 0) { printf(CLR_YELLOW "  No billing records found.\n" CLR_RESET); pressEnterToContinue(); return; }

    printBillHeader();
    float grand_unpaid = 0, grand_paid = 0;
    for (int i = 0; i < n; i++) {
        printBillRow(&buf[i]);
        if (buf[i].paid_status) grand_paid += buf[i].total;
        else                    grand_unpaid += buf[i].total;
    }
    printSeparator(100, '=');
    printf(CLR_BOLD "  Total Paid: " CLR_GREEN "$%.2f" CLR_RESET CLR_BOLD
                    "  |  Total Due (Unpaid): " CLR_RED "$%.2f\n" CLR_RESET,
           grand_paid, grand_unpaid);
    pressEnterToContinue();
}
