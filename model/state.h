#ifndef REFMON_STATE_H
#define REFMON_STATE_H

#define REFMON_STATE_STRING_OFF "OFF"
#define REFMON_STATE_STRING_ON "ON"
#define REFMON_STATE_STRING_RECOFF "REC-OFF"
#define REFMON_STATE_STRING_RECON "REC-ON"

#define SPIN_STATE_LOCK spin_lock(&spinlock_state);
#define SPIN_STATE_UNLOCK spin_unlock(&spinlock_state);

enum refmon_state { OFF, ON, REC_OFF, REC_ON };

extern enum refmon_state string_to_state(char *state);
extern char *state_to_string(enum refmon_state state);
int change_state(char *state);

extern int is_state_valid(char *state);
extern int is_state_on(int state);

#endif //REFMON_STATE_H
