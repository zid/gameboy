#ifndef LCD_H
#define LCD_H
int lcd_cycle(void);
int lcd_get_line(void);
void lcd_write_control(unsigned char);
void lcd_write_scroll_x(unsigned char);
void lcd_write_scroll_y(unsigned char);
#endif
