#include "gc9a01.h"
#include "nrf_drv_spi.h"
#include "nrfx_spim.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "app_error.h"

#define ORIENTATION 2 // Set the display orientation 0,1,2,3

#define SPI 0 // SPI (1) or SPIM (0) mode

#define RESET_ON            nrf_gpio_pin_set(LCD_RES_Pin)
#define RESET_OFF           nrf_gpio_pin_clear(LCD_RES_Pin)
#define DC_ON               nrf_gpio_pin_set(LCD_DC_Pin)
#define DC_OFF              nrf_gpio_pin_clear(LCD_DC_Pin)
#define CS_ON               nrf_gpio_pin_set(LCD_CS_Pin)
#define CS_OFF              nrf_gpio_pin_clear(LCD_CS_Pin)

#if defined(SPI) && (SPI)
/***** SPI **************************************************************************/
#define SPI_INSTANCE 0                                                   // SPI instance index.
static const nrf_drv_spi_t lcd_spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE); // SPI instance.
static uint8_t m_tx_buf[3];                                              // TX buffer.
/************************************************************************************/
#else
/***** SPIM *************************************************************************/
#define SPIM_INSTANCE 0                                                 // SPI instance index.
static const nrfx_spim_t lcd_spim = NRFX_SPIM_INSTANCE(SPIM_INSTANCE); // SPI instance.
static uint8_t m_tx_buf[] = {0xff, 0xff, 0xff};                        // TX buffer.
nrfx_spim_xfer_desc_t xfer_desc;
/************************************************************************************/
#endif

/***** Structures *******************************************************************/
struct GC9A01_frame frame;     // coordinates(X(start,end), Y(start,end)) frame, from 0 to 239
struct GC9A01_draw_prop fonts; // conf using font
/************************************************************************************/

/***** Display write func ***********************************************************/
void GC9A01_write_command(uint8_t cmd) {
    DC_OFF;
    CS_OFF;
#if (!SPI)
    xfer_desc.p_tx_buffer = &cmd;      // for SPIM
    xfer_desc.tx_length = sizeof(cmd); // for SPIM
#endif
    GC9A01_spi_tx(&cmd, sizeof(cmd));
    CS_ON;
}

void GC9A01_write_data(uint8_t *data, size_t len) {
    DC_ON;
    CS_OFF;
#if (!SPI)
    xfer_desc.p_tx_buffer = data; // for SPIM
    xfer_desc.tx_length = len;    // for SPIM
#endif
    GC9A01_spi_tx(data, len);
    CS_ON;
}

static inline void GC9A01_write_byte(uint8_t val) {
    GC9A01_write_data(&val, sizeof(val));
}

void GC9A01_write(uint8_t *data, size_t len) {
    GC9A01_write_command(MEM_WR);
    GC9A01_write_data(data, len);
}

void GC9A01_write_continue(uint8_t *data, size_t len) {
    GC9A01_write_command(MEM_WR_CONT);
    GC9A01_write_data(data, len);
}
/************************************************************************************/

/***** Hardware and soft func *******************************************************/
void lcd_spi_init(void) {

#if defined(SPI) && (SPI)
    /* SPI */
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.miso_pin = LCD_MISO_PIN;
    spi_config.mosi_pin = LCD_MOSI_PIN;
    spi_config.sck_pin = LCD_SCK_PIN;
    spi_config.frequency = NRF_DRV_SPI_FREQ_8M;
    APP_ERROR_CHECK(nrf_drv_spi_init(&lcd_spi, &spi_config, NULL, NULL));
#else
    /* SPIM */
    nrfx_spim_config_t spim_config = NRFX_SPIM_DEFAULT_CONFIG;
    spim_config.miso_pin = LCD_MISO_PIN;
    spim_config.mosi_pin = LCD_MOSI_PIN;
    spim_config.sck_pin = LCD_SCK_PIN;
    spim_config.frequency = NRF_SPIM_FREQ_8M;
    APP_ERROR_CHECK(nrfx_spim_init(&lcd_spim, &spim_config, NULL, NULL));
#endif

    /* GPIO for display */
    nrf_gpio_cfg_output(LCD_RES_Pin);
    nrf_gpio_cfg_output(LCD_CS_Pin);
    nrf_gpio_cfg_output(LCD_DC_Pin);

    NRF_LOG_INFO("SPI example started.");
}

void GC9A01_spi_tx(uint8_t *data, size_t len) {
#if defined(SPI) && (SPI)
    nrf_drv_spi_transfer(&lcd_spi, data, len, 0, 0);
#else
    nrfx_spim_xfer(&lcd_spim, &xfer_desc, 0); // for SPIM
#endif
}

void GC9A01_init(void) {
    
    CS_ON;
    nrf_delay_ms(5);
    RESET_OFF;
    nrf_delay_ms(10);
    RESET_ON;
    nrf_delay_ms(120);
    
    /* Initial Sequence */ 
    
    GC9A01_write_command(0xEF);
    
    GC9A01_write_command(0xEB);
    GC9A01_write_byte(0x14);
    
    GC9A01_write_command(0xFE);
    GC9A01_write_command(0xEF);
    
    GC9A01_write_command(0xEB);
    GC9A01_write_byte(0x14);
    
    GC9A01_write_command(0x84);
    GC9A01_write_byte(0x40);
    
    GC9A01_write_command(0x85);
    GC9A01_write_byte(0xFF);
    
    GC9A01_write_command(0x86);
    GC9A01_write_byte(0xFF);
    
    GC9A01_write_command(0x87);
    GC9A01_write_byte(0xFF);
    
    GC9A01_write_command(0x88);
    GC9A01_write_byte(0x0A);
    
    GC9A01_write_command(0x89);
    GC9A01_write_byte(0x21);
    
    GC9A01_write_command(0x8A);
    GC9A01_write_byte(0x00);
    
    GC9A01_write_command(0x8B);
    GC9A01_write_byte(0x80);
    
    GC9A01_write_command(0x8C);
    GC9A01_write_byte(0x01);
    
    GC9A01_write_command(0x8D);
    GC9A01_write_byte(0x01);
    
    GC9A01_write_command(0x8E);
    GC9A01_write_byte(0xFF);
    
    GC9A01_write_command(0x8F);
    GC9A01_write_byte(0xFF);
    
    
    GC9A01_write_command(0xB6);
    GC9A01_write_byte(0x00);
    GC9A01_write_byte(0x20); // used 0x00
    
    GC9A01_write_command(0x36);
    
#if ORIENTATION == 0
    GC9A01_write_byte(0x18);
#elif ORIENTATION == 1
    GC9A01_write_byte(0x28);
#elif ORIENTATION == 2
    GC9A01_write_byte(0x08); //0x48
#else
    GC9A01_write_byte(0x88);
#endif
    
    GC9A01_write_command(COLOR_MODE);
    GC9A01_write_byte(COLOR_MODE__18_BIT);
    
    GC9A01_write_command(0x90);
    GC9A01_write_byte(0x08);
    GC9A01_write_byte(0x08);
    GC9A01_write_byte(0x08);
    GC9A01_write_byte(0x08);
    
    GC9A01_write_command(0xBD);
    GC9A01_write_byte(0x06);
    
    GC9A01_write_command(0xBC);
    GC9A01_write_byte(0x00);
    
    GC9A01_write_command(0xFF);
    GC9A01_write_byte(0x60);
    GC9A01_write_byte(0x01);
    GC9A01_write_byte(0x04);
    
    GC9A01_write_command(0xC3);
    GC9A01_write_byte(0x13);
    GC9A01_write_command(0xC4);
    GC9A01_write_byte(0x13);
    
    GC9A01_write_command(0xC9);
    GC9A01_write_byte(0x22);
    
    GC9A01_write_command(0xBE);
    GC9A01_write_byte(0x11);
    
    GC9A01_write_command(0xE1);
    GC9A01_write_byte(0x10);
    GC9A01_write_byte(0x0E);
    
    GC9A01_write_command(0xDF);
    GC9A01_write_byte(0x21);
    GC9A01_write_byte(0x0c);
    GC9A01_write_byte(0x02);
    
    GC9A01_write_command(0xF0);
    GC9A01_write_byte(0x45);
    GC9A01_write_byte(0x09);
    GC9A01_write_byte(0x08);
    GC9A01_write_byte(0x08);
    GC9A01_write_byte(0x26);
    GC9A01_write_byte(0x2A);
    
    GC9A01_write_command(0xF1);
    GC9A01_write_byte(0x43);
    GC9A01_write_byte(0x70);
    GC9A01_write_byte(0x72);
    GC9A01_write_byte(0x36);
    GC9A01_write_byte(0x37);
    GC9A01_write_byte(0x6F);
    
    GC9A01_write_command(0xF2);
    GC9A01_write_byte(0x45);
    GC9A01_write_byte(0x09);
    GC9A01_write_byte(0x08);
    GC9A01_write_byte(0x08);
    GC9A01_write_byte(0x26);
    GC9A01_write_byte(0x2A);
    
    GC9A01_write_command(0xF3);
    GC9A01_write_byte(0x43);
    GC9A01_write_byte(0x70);
    GC9A01_write_byte(0x72);
    GC9A01_write_byte(0x36);
    GC9A01_write_byte(0x37);
    GC9A01_write_byte(0x6F);
    
    GC9A01_write_command(0xED);
    GC9A01_write_byte(0x1B);
    GC9A01_write_byte(0x0B);
    
    GC9A01_write_command(0xAE);
    GC9A01_write_byte(0x77);
    
    GC9A01_write_command(0xCD);
    GC9A01_write_byte(0x63);
    
    GC9A01_write_command(0x70);
    GC9A01_write_byte(0x07);
    GC9A01_write_byte(0x07);
    GC9A01_write_byte(0x04);
    GC9A01_write_byte(0x0E);
    GC9A01_write_byte(0x0F);
    GC9A01_write_byte(0x09);
    GC9A01_write_byte(0x07);
    GC9A01_write_byte(0x08);
    GC9A01_write_byte(0x03);
    
    GC9A01_write_command(0xE8);
    GC9A01_write_byte(0x34);
    
    GC9A01_write_command(0x62);
    GC9A01_write_byte(0x18);
    GC9A01_write_byte(0x0D);
    GC9A01_write_byte(0x71);
    GC9A01_write_byte(0xED);
    GC9A01_write_byte(0x70);
    GC9A01_write_byte(0x70);
    GC9A01_write_byte(0x18);
    GC9A01_write_byte(0x0F);
    GC9A01_write_byte(0x71);
    GC9A01_write_byte(0xEF);
    GC9A01_write_byte(0x70);
    GC9A01_write_byte(0x70);
    
    GC9A01_write_command(0x63);
    GC9A01_write_byte(0x18);
    GC9A01_write_byte(0x11);
    GC9A01_write_byte(0x71);
    GC9A01_write_byte(0xF1);
    GC9A01_write_byte(0x70);
    GC9A01_write_byte(0x70);
    GC9A01_write_byte(0x18);
    GC9A01_write_byte(0x13);
    GC9A01_write_byte(0x71);
    GC9A01_write_byte(0xF3);
    GC9A01_write_byte(0x70);
    GC9A01_write_byte(0x70);
    
    GC9A01_write_command(0x64);
    GC9A01_write_byte(0x28);
    GC9A01_write_byte(0x29);
    GC9A01_write_byte(0xF1);
    GC9A01_write_byte(0x01);
    GC9A01_write_byte(0xF1);
    GC9A01_write_byte(0x00);
    GC9A01_write_byte(0x07);
    
    GC9A01_write_command(0x66);
    GC9A01_write_byte(0x3C);
    GC9A01_write_byte(0x00);
    GC9A01_write_byte(0xCD);
    GC9A01_write_byte(0x67);
    GC9A01_write_byte(0x45);
    GC9A01_write_byte(0x45);
    GC9A01_write_byte(0x10);
    GC9A01_write_byte(0x00);
    GC9A01_write_byte(0x00);
    GC9A01_write_byte(0x00);
    
    GC9A01_write_command(0x67);
    GC9A01_write_byte(0x00);
    GC9A01_write_byte(0x3C);
    GC9A01_write_byte(0x00);
    GC9A01_write_byte(0x00);
    GC9A01_write_byte(0x00);
    GC9A01_write_byte(0x01);
    GC9A01_write_byte(0x54);
    GC9A01_write_byte(0x10);
    GC9A01_write_byte(0x32);
    GC9A01_write_byte(0x98);
    
    GC9A01_write_command(0x74);
    GC9A01_write_byte(0x10);
    GC9A01_write_byte(0x85);
    GC9A01_write_byte(0x80);
    GC9A01_write_byte(0x00);
    GC9A01_write_byte(0x00);
    GC9A01_write_byte(0x4E);
    GC9A01_write_byte(0x00);
    
    GC9A01_write_command(0x98);
    GC9A01_write_byte(0x3e);
    GC9A01_write_byte(0x07);
    
    GC9A01_write_command(0x35);
    GC9A01_write_command(0x21);
    
    GC9A01_write_command(0x11);
    nrf_delay_ms(120);
    GC9A01_write_command(0x29);
    nrf_delay_ms(20);
    
}

void GC9A01A_sleep_mode(uint8_t Mode) {
	if (Mode)
		GC9A01_write_command(SLPIN);
	else
		GC9A01_write_command(SLPOUT);

	nrf_delay_ms(500);
}
/************************************************************************************/

/***** Display picture func *********************************************************/
void GC9A01_set_frame(struct GC9A01_frame frame) {

    uint8_t data[4];
    
    GC9A01_write_command(COL_ADDR_SET);
    data[0] = (frame.start.X >> 8) & 0xFF;
    data[1] = frame.start.X & 0xFF;
    data[2] = (frame.end.X >> 8) & 0xFF;
    data[3] = frame.end.X & 0xFF;
    GC9A01_write_data(data, sizeof(data));

    GC9A01_write_command(ROW_ADDR_SET);
    data[0] = (frame.start.Y >> 8) & 0xFF;
    data[1] = frame.start.Y & 0xFF;
    data[2] = (frame.end.Y >> 8) & 0xFF;
    data[3] = frame.end.Y & 0xFF;
    GC9A01_write_data(data, sizeof(data));
    
}

void GC9A01_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h,
		uint16_t color) {

	if ((x + w) > GC9A01A_Width)
		w = GC9A01A_Width - x;
	if ((y + h) > GC9A01A_Height)
		h = GC9A01A_Height - y;

    uint8_t color_new[3];
    color_new[2] = (uint8_t)((color & 0x1F) << 3);   // blue
    color_new[1] = (uint8_t)((color & 0x7E0) >> 3);  // green
    color_new[0] = (uint8_t)((color & 0xF800) >> 8); // red

    frame.start.X = x;
    frame.start.Y = y;
    frame.end.X = w;
    frame.end.Y = h;
    GC9A01_set_frame(frame);

    for (uint16_t row = 0; row < h+1; row++) {
		for (uint16_t col = 0; col < w+1; col++) {		
            if (row == 0 && col == 0) {
                GC9A01_write(color_new, sizeof(color_new));
            } else {  
                GC9A01_write_continue(color_new, sizeof(color_new));
            }
        }
    }
 
    frame.start.X = 0;
    frame.start.Y = 0;
    frame.end.X = 239;
    frame.end.Y = 239;
    GC9A01_set_frame(frame);
}

void GC9A01_fonts_init(void) {
    Font8.Height = 8;
    Font8.Width = 5;
    Font12.Height = 12;
    Font12.Width = 7;
    Font16.Height = 16;
    Font16.Width = 11;
    Font20.Height = 20;
    Font20.Width = 14;
    Font24.Height = 24;
    Font24.Width = 17;
    fonts.BackColor = BLACK;
    fonts.TextColor = GREEN;
    fonts.pFont = &Font16;
}

void GC9A01_set_text_color(uint16_t color) {
    fonts.TextColor = color;
}

void GC9A01_set_back_color(uint16_t color) {
    fonts.BackColor = color;
}

void GC9A01_set_font(sFONT *pFonts) {
    fonts.pFont = pFonts;
}

void GC9A01_draw_pixel(int x, int y, uint16_t color) {
    if ((x < 0) || (y < 0) || (x >= GC9A01A_Width) || (y >= GC9A01A_Height))
        return;
    frame.start.X = x;
    frame.end.X = x;
    frame.start.Y = y;
    frame.end.Y = y;
    GC9A01_set_frame(frame);
    m_tx_buf[2] = (uint8_t)((color & 0x1F) << 3);    // blue
    m_tx_buf[1] = (uint8_t)((color & 0x7E0) >> 3);   // green
    m_tx_buf[0] = (uint8_t)((color & 0xF800) >> 8);  // red
    GC9A01_write(m_tx_buf, sizeof(m_tx_buf));
    frame.start.X = 0;
    frame.end.X = 239;
    frame.start.Y = 0;
    frame.end.Y = 239;
    GC9A01_set_frame(frame);
}

void GC9A01_draw_line(uint16_t color, uint16_t x1, uint16_t y1,
                      uint16_t x2, uint16_t y2) {
    int steep = abs(y2 - y1) > abs(x2 - x1);
    if (steep)
    {
        swap(x1, y1);
        swap(x2, y2);
    }
    if (x1 > x2)
    {
        swap(x1, x2);
        swap(y1, y2);
    }
    int dx, dy;
    dx = x2 - x1;
    dy = abs(y2 - y1);
    int err = dx / 2;
    int ystep;
    if (y1 < y2)
        ystep = 1;
    else
        ystep = -1;
    for (; x1 <= x2; x1++)
    {
        if (steep)
            GC9A01_draw_pixel(y1, x1, color);
        else
            GC9A01_draw_pixel(x1, y1, color);
        err -= dy;
        if (err < 0)
        {
            y1 += ystep;
            err += dx;
        }
    }
}

void GC9A01_draw_circle(uint16_t x0, uint16_t y0, int r, uint16_t color) {
    int f = 1 - r;
    int ddF_x = 1;
    int ddF_y = -2 * r;
    int x = 0;
    int y = r;
    GC9A01_draw_pixel(x0, y0 + r, color);
    GC9A01_draw_pixel(x0, y0 - r, color);
    GC9A01_draw_pixel(x0 + r, y0, color);
    GC9A01_draw_pixel(x0 - r, y0, color);
    while (x < y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        GC9A01_draw_pixel(x0 + x, y0 + y, color);
        GC9A01_draw_pixel(x0 - x, y0 + y, color);
        GC9A01_draw_pixel(x0 + x, y0 - y, color);
        GC9A01_draw_pixel(x0 - x, y0 - y, color);
        GC9A01_draw_pixel(x0 + y, y0 + x, color);
        GC9A01_draw_pixel(x0 - y, y0 + x, color);
        GC9A01_draw_pixel(x0 + y, y0 - x, color);
        GC9A01_draw_pixel(x0 - y, y0 - x, color);
    }
}

void GC9A01_fill_circle(int16_t x, int16_t y, int16_t radius,
		uint16_t color) {

    for (uint8_t curX = (x - radius); curX <= (x + radius); curX++)
    {
        for (uint8_t curY = (y - radius); curY <= (y + radius); curY++)
        {
            if ((pow(x-curX, 2) + pow(y-curY, 2)) <= pow(radius, 2))
            {
                GC9A01_draw_pixel(curX, curY, color);
            }
        }
    }

    // int x = 0;
	// int y = radius;
	// int delta = 1 - 2 * radius;
	// int error = 0;

	// while (y >= 0) {
	// 	GC9A01_draw_line(x0 + x, y0 - y, x0 + x, y0 + y, color);
	// 	GC9A01_draw_line(x0 - x, y0 - y, x0 - x, y0 + y, color);
	// 	error = 2 * (delta + y) - 1;

	// 	if (delta < 0 && error <= 0) {
	// 		++x;
	// 		delta += 2 * x + 1;
	// 		continue;
	// 	}

	// 	error = 2 * (delta - x) - 1;

	// 	if (delta > 0 && error > 0) {
	// 		--y;
	// 		delta += 1 - 2 * y;
	// 		continue;
	// 	}

	// 	++x;
	// 	delta += 2 * (x - y);
	// 	--y;
	// }
}

void GC9A01_draw_char(uint16_t x, uint16_t y, uint8_t c) {
    uint32_t i = 0, j = 0;
    uint16_t height, width;
    uint8_t offset;
    uint8_t *c_t;
    uint8_t *pchar;
    uint32_t line = 0;
    height = fonts.pFont->Height;
    width = fonts.pFont->Width;
    offset = 8 * ((width + 7) / 8) - width;
    c_t = (uint8_t *)&(fonts.pFont->table[(c - ' ') * fonts.pFont->Height * ((fonts.pFont->Width + 7) / 8)]);
    for (i = 0; i < height; i++)
    {
        pchar = ((uint8_t *)c_t + (width + 7) / 8 * i);
        switch (((width + 7) / 8))
        {
        case 1:
            line = pchar[0];
            break;
        case 2:
            line = (pchar[0] << 8) | pchar[1];
            break;
        case 3:
        default:
            line = (pchar[0] << 16) | (pchar[1] << 8) | pchar[2];
            break;
        }
        for (j = 0; j < width; j++)
        {
            if (line & (1 << (width - j + offset - 1)))
            {
                GC9A01_draw_pixel((x + j), y, fonts.TextColor);
            }
            else
            {
                GC9A01_draw_pixel((x + j), y, fonts.BackColor);
                // continue;
            }
        }
        y++;
    }
}

void GC9A01_draw_string(uint16_t x,uint16_t y, char *str) {
    while (*str)
    {
        GC9A01_draw_char(x, y, str[0]);
        x += fonts.pFont->Width;
        (void)*str++;
    }
}

void GC9A01_printf(int16_t X, int16_t Y, const char *args, ...) {
	char StrBuff[100];

	va_list ap;
	va_start(ap, args);
	vsnprintf(StrBuff, sizeof(StrBuff), args, ap);
	va_end(ap);
	GC9A01_draw_string(X, Y, StrBuff);
}
/************************************************************************************/
