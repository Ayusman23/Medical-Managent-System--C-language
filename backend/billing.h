#ifndef BILLING_H
#define BILLING_H

#include "../config.h"

typedef struct {
    int   bill_id;
    int   patient_id;
    float room_charges;
    float medicine_charges;
    float doctor_fee;
    float total;
    int   paid_status;   /* 0=Unpaid, 1=Paid */
    char  bill_date[DATE_LEN];
    int   is_active;
} Bill;

void generateBill(void);
void viewBill(void);
void markPaid(void);
void listAllBills(void);

#endif /* BILLING_H */
