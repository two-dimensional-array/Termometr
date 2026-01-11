#ifndef __SLEEP_CONTROL_H__
#define __SLEEP_CONTROL_H__

typedef enum
{
    SLEEP_CONTROL_WAKE_UP_CAUSE_UNDEFINED,
    SLEEP_CONTROL_WAKE_UP_CAUSE_TIMER,
} sleep_contol_wake_up_cause_t;

sleep_contol_wake_up_cause_t sleep_contol_init(void);
void sleep_contol_sleep(void);

#endif // __SLEEP_CONTROL_H__
