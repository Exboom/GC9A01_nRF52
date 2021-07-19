#include "nrf_drv_spi.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include <string.h>
#include <stdint.h>
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "gc9a01.h"

int main(void)
{
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    lcd_spi_init();
    GC9A01_init();
    struct GC9A01_frame frame = {{0, 0}, {239, 239}};
    GC9A01_set_frame(frame);
    GC9A01_fonts_init();

    while (1)
    {
        uint8_t color[3];
        // Triangle
        color[0] = 0xFF;
        color[1] = 0xFF;
        for (int x = 0; x < 240; x++)
        {
            for (int y = 0; y < 240; y++)
            {
                if (x < y)
                {
                    color[2] = 0xFF;
                }
                else
                {
                    color[2] = 0x00;
                }
                if (x == 0 && y == 0)
                {
                    GC9A01_write(color, sizeof(color));
                }
                else
                {
                    GC9A01_write_continue(color, sizeof(color));
                }
            }
        }
        nrf_delay_ms(500);

        //raduga
        float frequency = 0.026;
        for (int x = 0; x < 240; x++)
        {
            color[0] = sin(frequency * x + 0) * 127 + 128;
            color[1] = sin(frequency * x + 2) * 127 + 128;
            color[2] = sin(frequency * x + 4) * 127 + 128;
            for (int y = 0; y < 240; y++)
            {
                if (x == 0 && y == 0)
                {
                    GC9A01_write(color, sizeof(color));
                }
                else
                {
                    GC9A01_write_continue(color, sizeof(color));
                }
            }
        }
        nrf_delay_ms(500);

        // Checkerboard
        for (int x = 0; x < 240; x++)
        {
            for (int y = 0; y < 240; y++)
            {
                if ((x / 10) % 2 == (y / 10) % 2)
                {
                    color[0] = 0xFF;
                    color[1] = 0xFF;
                    color[2] = 0xFF;
                }
                else
                {
                    color[0] = 0x00;
                    color[1] = 0x00;
                    color[2] = 0x00;
                }
                if (x == 0 && y == 0)
                {
                    GC9A01_write(color, sizeof(color));
                }
                else
                {
                    GC9A01_write_continue(color, sizeof(color));
                }
            }
        }
        nrf_delay_ms(500);

        // Swiss flag
        color[0] = 0xFF;
        for (int x = 0; x < 240; x++)
        {
            for (int y = 0; y < 240; y++)
            {
                if ((x >= 1 * 48 && x < 4 * 48 && y >= 2 * 48 && y < 3 * 48) ||
                    (x >= 2 * 48 && x < 3 * 48 && y >= 1 * 48 && y < 4 * 48))
                {
                    color[1] = 0xFF;
                    color[2] = 0xFF;
                }
                else
                {
                    color[1] = 0x00;
                    color[2] = 0x00;
                }
                if (x == 0 && y == 0)
                {
                    GC9A01_write(color, sizeof(color));
                }
                else
                {
                    GC9A01_write_continue(color, sizeof(color));
                }
            }
        }
        nrf_delay_ms(500);

        GC9A01_fill_rect(0, 0, GC9A01A_Width, GC9A01A_Height, WHITE);
        GC9A01_set_font(&Font24);
        GC9A01_fill_circle(50, 50, 50, YELLOW);
        GC9A01_draw_line(MAGENTA,220, 50, 150, 240);
        GC9A01_set_back_color(WHITE);
        GC9A01_draw_string(80,110, "HELLO");

        nrf_delay_ms(5000);

    }
}
