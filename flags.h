//$Id: flags.h,v 1.1 2003-03-23 19:53:46 peter Exp $
enum bit_FLAGS{
 flag_SERIAL_PORT_UP,
 flag_SPI
 };

extern u16 massiv_flags[2];

static inline void set_flag(u16 flag){
u16 st0=store_and_disable_ints();
massiv_flags[(flag&0xFFF0)>>4]|=1<<(flag&0x0F);
enable_int(st0);
}
//захват семафора
static inline u8 capture_flag(u16 flag){
u16 st0=store_and_disable_ints();
if (massiv_flags[(flag&0xFFF0)>>4]&(1<<(flag&0x0F))){ //уже захвачен
 enable_int(st0);
 return 0;
 }
massiv_flags[(flag&0xFFF0)>>4]|=1<<(flag&0x0F);
enable_int(st0);
return 1;
}
static inline void clear_flag(u16 flag){
u16 st0=store_and_disable_ints();
massiv_flags[(flag&0xFFF0)>>4]&=~(1<<(flag&0x0F));
enable_int(st0);
}
static inline u16 check_flag(u16 flag){
return (massiv_flags[(flag&0xFFF0)>>4]&(~(1<<(flag&0x0F))) );
}

static inline void  serial_port_worked_up(){
 set_flag(flag_SERIAL_PORT_UP);
}
static inline u16 is_serial_port_worked(){
 return check_flag(flag_SERIAL_PORT_UP);
}
static inline void  serial_port_down(){
 clear_flag(flag_SERIAL_PORT_UP);
}
static inline u8  spi_worked_up(){
 return capture_flag(flag_SPI);
}
u16 is_spi_worked();
//static inline u16 is_spi_worked(){
// return check_flag(flag_SPI);
//}
static inline void  spi_release(){
 clear_flag(flag_SPI);
}
