#include <ui/ui.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ui);




int ui_init(void)
{
    LOG_INF("Initialized UI");
    return 0;
}