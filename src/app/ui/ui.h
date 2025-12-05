#ifndef UI_H_
#define UI_H_

typedef enum ui_colors_e {
    WHITE = 0,
    YELLOW,
    BLUE, 
    RED,
    SIZE_GUARD
} ui_colors_t;

int ui_init(void);
int ui_setColor(ui_colors_t state);

#endif /* UI_H_ */
