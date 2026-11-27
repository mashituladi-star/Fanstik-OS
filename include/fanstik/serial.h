#ifndef FANSTIK_SERIAL_H
#define FANSTIK_SERIAL_H
void serial_init(void);
void serial_write_char(char value);
void serial_write_string(const char *value);
#endif
