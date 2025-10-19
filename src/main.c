#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <stdint.h>

LOG_MODULE_REGISTER(main); 

int main(void)
{
    printk("Zephyr beacon application!\n");

    uint32_t appCoutner = 0;
    while(1)
    {
        k_sleep(K_SECONDS(10));
        printk("Zephyr beacon application iteration= %d!\n", appCoutner++);
    }

    return 0;
}