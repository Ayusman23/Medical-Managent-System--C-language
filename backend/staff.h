#ifndef STAFF_H
#define STAFF_H

#include "../config.h"

typedef struct {
    int   staff_id;
    char  name[NAME_LEN];
    char  role[ROLE_LEN];
    char  department[DEPT_LEN];
    char  phone[PHONE_LEN];
    float salary;
    int   is_active;
} Staff;

void addStaff(void);
void updateStaff(void);
void removeStaff(void);
void listStaff(void);
void searchStaff(void);

#endif /* STAFF_H */
