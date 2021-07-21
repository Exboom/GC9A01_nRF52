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
#include "img.h"

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

        GC9A01_fill_rect(0, 0, GC9A01A_Width, GC9A01A_Height, WHITE);

        GC9A01_set_font(&Font24);
        GC9A01_fill_circle(50, 50, 50, YELLOW);
        GC9A01_draw_line(MAGENTA, 220, 50, 150, 240);
        GC9A01_set_back_color(WHITE);
        GC9A01_draw_string(80, 110, "Hello");
        GC9A01_draw_string(50, 145, "my friend");

        nrf_delay_ms(2500);
        GC9A01_fill_rect(0, 0, GC9A01A_Width, GC9A01A_Height, WHITE);

        // /* Img */
        frame.start.X = 10;
        frame.end.X = 230;
        frame.start.Y = 10;
        frame.end.Y = 230;

        GC9A01_set_frame(frame);

        for (size_t x = 0; x < 48399; x = x + 219) {
            for (size_t y = 0; y < 220; y++) {

                color[2] = (uint8_t)((img[x + y] & 0x1F) << 3);   // blue
                color[1] = (uint8_t)((img[x + y] & 0x7E0) >> 3);  // green
                color[0] = (uint8_t)((img[x + y] & 0xF800) >> 8); // red

                if (x == 0 && y == 0) {
                    GC9A01_write(color, sizeof(color));
                }
                else {
                    GC9A01_write_continue(color, sizeof(color));
                }
            }
        }
        nrf_delay_ms(5000);
    }
}
