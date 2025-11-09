typedef enum ui_states_e { TAG, BEACON, ERROR } ui_states_t;

int ui_init(void);
int ui_setColor(ui_states_t state);
