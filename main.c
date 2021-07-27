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
// #include "gc9a01.h"
#include "img.h"

/* For 3D */

#define MAX_OBJ 12
int bgMode=3;
int object=6;
int bfCull=1;
int orient=0;
int polyMode=0;

#include "pat2.h"
#include "pat7.h"
#include "pat8.h"
#include "gfx3d.h"

unsigned int msMin=1000, msMax=0, stats=1, optim=0;

/**********/

static void draw_arrow(int16_t angle, uint8_t lineLen,
		uint16_t color) {
    /* old */
    int16_t old_angel = (angle - 1) * 6 - 90;
    float old_angleRad = (float)old_angel * 3.14159265 / 180;
    int start_old_x = cos(old_angleRad) * 50 + 120;
    int start_old_y = sin(old_angleRad) * 50 + 120;
    int end_old_x = cos(old_angleRad) * lineLen + 120;
    int end_old_y = sin(old_angleRad) * lineLen + 120;
    GC9A01_draw_line(WHITE, start_old_x, start_old_y, end_old_x, end_old_y);
    
    /* new */
    angle = angle * 6 - 90;
    float angleRad = (float)angle * 3.14159265 / 180;
    int start_x = cos(angleRad) * 50 + 120;
    int start_y = sin(angleRad) * 50 + 120;
    int end_x = cos(angleRad) * lineLen + 120;
    int end_y = sin(angleRad) * lineLen + 120;
    GC9A01_draw_line(color, start_x, start_y, end_x, end_y);
}

static void number_update() {
    GC9A01_set_font(&Font16);
    GC9A01_draw_string(165, 30, "1");
    GC9A01_draw_string(200, 63, "2");
    GC9A01_set_font(&Font24);
    GC9A01_draw_string(207, 110, "3");
    GC9A01_set_font(&Font16);
    GC9A01_draw_string(200, 160, "4");
    GC9A01_draw_string(165, 193, "5");
    GC9A01_set_font(&Font24);
    GC9A01_draw_string(111, 200, "6");
    GC9A01_set_font(&Font16);
    GC9A01_draw_string(65, 193, "7");
    GC9A01_draw_string(32, 160, "8");
    GC9A01_set_font(&Font24);
    GC9A01_draw_string(17, 110, "9");
    GC9A01_set_font(&Font16);
    GC9A01_draw_string(32, 63, "10");
    GC9A01_draw_string(65, 30, "11");
    GC9A01_set_font(&Font24);
    GC9A01_draw_string(105, 14, "12");
}

int main(void) {
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

        /* Text and geomentry */
        {
            GC9A01_fill_rect(0, 0, GC9A01A_Width, GC9A01A_Height, WHITE);
            GC9A01_set_font(&Font24);
            GC9A01_fill_circle(50, 50, 50, YELLOW);
            GC9A01_draw_line(MAGENTA, 220, 50, 150, 240);
            GC9A01_set_back_color(WHITE);
            GC9A01_draw_string(80, 110, "Hello");
            GC9A01_draw_string(50, 145, "my friend");
            nrf_delay_ms(2500);
            GC9A01_fill_rect(0, 0, GC9A01A_Width, GC9A01A_Height, WHITE);
        }

        /* Clock */
        {
            uint8_t radius1 = 119;
            for (uint16_t angle = 0; angle <= 360; angle += 6) {
                uint8_t riskSize;
                if (!(angle % 90))
                    riskSize = 13;
                else if (!(angle % 30))
                    riskSize = 10;
                else
                    riskSize = 6;

                uint8_t radius2 = radius1 - riskSize;
                float angleRad = (float)angle * 3.14159265 / 180;
                int x1 = cos(angleRad) * radius1 + 120;
                int y1 = sin(angleRad) * radius1 + 120;
                int x2 = cos(angleRad) * radius2 + 120;
                int y2 = sin(angleRad) * radius2 + 120;
                GC9A01_draw_line(BLACK, x1, y1, x2, y2);
            }
            number_update();
            GC9A01_draw_circle(119, 119, 47, BLACK);
            for (size_t i = 0; i < 60; i++) {
                GC9A01_printf(78, 110, "10:%02d", 0 + i);
                draw_arrow((size_t)50, 70, BLACK);
                draw_arrow(i, 86, BLACK);
                nrf_delay_ms(1000);
            }
            GC9A01_fill_rect(0, 0, GC9A01A_Width, GC9A01A_Height, WHITE);
        }

        /* Image */
        {
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

        /* 3D */
        GC9A01_fill_rect(0, 0, GC9A01A_Width, GC9A01A_Height, BLACK);
        int i = 0;

        while (i < 25000)
        {
            polyMode = 0;
            orient = 0;
            bfCull = 1;
            lightShade = 0;

            numVerts = numVertsCubes;
            verts = (int16_t *)vertsCubes;
            numPolys = numQuadsCubes;
            polys = (uint8_t *)quadsCubes;
            polyColors = (uint16_t *)colsCubes;
            bfCull = 0;
            render3D(polyMode);
            i++;
        }
    }
}
