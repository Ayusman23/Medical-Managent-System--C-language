#ifndef WARD_H
#define WARD_H

#include "../config.h"

typedef struct {
    int  ward_no;
    char ward_type[WTYPE_LEN];    /* General / ICU / Pediatric / Maternity etc. */
    int  total_beds;
    int  occupied_beds;
    int  patient_ids[MAX_BEDS_PER_WARD];
    int  is_active;
} Ward;

void addWard(void);
void admitToWard(void);
void dischargeFromWard(void);
void viewWardStatus(void);
void listAllWards(void);

#endif /* WARD_H */
