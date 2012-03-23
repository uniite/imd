//============ 
// OLED macros 
//============ 

#define ON  1 
#define OFF 0 

//================ 
// command support 
//================ 

#define int1 unsigned char
#define int8 unsigned char
#define int16 unsigned short
#define int32 unsigned int
#define int64 unsigned long long

#define MAX_COMMAND_SIZE    11          // max size of command (command+data) 
int8 oled_buffer[MAX_COMMAND_SIZE];    
int8 command_index=0; 

#define reset_data_buffer()     {oled_buffer[0]=0; command_index=0;} 
#define add_oled_data(data)     {oled_buffer[command_index++]=data;} 

#define issue_0_arg_oled_command(command)   {reset_data_buffer(); add_oled_data(command); write_oled_data();}

#define issue_1_arg_oled_command(command, arg1)     {reset_data_buffer(); add_oled_data(command); add_oled_data(arg1); write_oled_data();}

#define issue_2_arg_oled_command(command, arg1, arg2)   {reset_data_buffer(); add_oled_data(command); add_oled_data(arg1); add_oled_data(arg2); write_oled_data();} 

#define issue_3_arg_oled_command(command, arg1, arg2, arg3) {reset_data_buffer(); add_oled_data(command); add_oled_data(arg1); add_oled_data(arg2); add_oled_data(arg3); write_oled_data();}

#define issue_4_arg_oled_command(command, arg1, arg2, arg3, arg4)   {reset_data_buffer(); add_oled_data(command); add_oled_data(arg1); add_oled_data(arg2); add_oled_data(arg3); add_oled_data(arg4); write_oled_data();}

#define issue_5_arg_oled_command(command, arg1, arg2, arg3, arg4, arg5)       {reset_data_buffer(); add_oled_data(command); add_oled_data(arg1); add_oled_data(arg2); add_oled_data(arg3); add_oled_data(arg4); add_oled_data(arg5); write_oled_data();} 

#define issue_6_arg_oled_command(command, arg1, arg2, arg3, arg4, arg5, arg6)       {reset_data_buffer(); add_oled_data(command); add_oled_data(arg1); add_oled_data(arg2); add_oled_data(arg3); add_oled_data(arg4); add_oled_data(arg5); add_oled_data(arg6); write_oled_data();} 

#define issue_7_arg_oled_command(command, arg1, arg2, arg3, arg4, arg5, arg6, arg7) {reset_data_buffer(); add_oled_data(command); add_oled_data(arg1); add_oled_data(arg2); add_oled_data(arg3); add_oled_data(arg4); add_oled_data(arg5); add_oled_data(arg6); add_oled_data(arg7); write_oled_data();} 

#define issue_8_arg_oled_command(command, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) {reset_data_buffer(); add_oled_data(command); add_oled_data(arg1); add_oled_data(arg2); add_oled_data(arg3); add_oled_data(arg4); add_oled_data(arg5); add_oled_data(arg6); add_oled_data(arg7); add_oled_data(arg8); write_oled_data();}

int8 const bitMask8[]={ 
   0x0, 
   0x40, 
   0x20, 
   0x10, 
   0x8, 
   0x4, 
   0x2, 
   0x1 
}; 


// colors 
#define RED     0xff, 0x00, 0x00 
#define GREEN   0x00, 0xff, 0x00 
#define BLUE    0x00, 0x00, 0xff 

//================ 
// command defines 
//================ 

#define writeSingleByte(arg1)   send_8bit_serial_data(arg1) 

#define clear_window(arg1, arg2, arg3, arg4)    issue_4_arg_oled_command(0x8e, arg1, arg2, arg3, arg4) 
#define copy(arg1, arg2, arg3, arg4, arg5, arg6)   issue_6_arg_oled_command(0x8a, arg1, arg2, arg3, arg4, arg5, arg6) 
#define dim_window(arg1, arg2, arg3, arg4)  issue_4_arg_oled_command(0x8c, arg1, arg2, arg3, arg4) 
#define draw_circle(arg1, arg2, arg3, arg4, arg5, arg6, arg7)   issue_7_arg_oled_command(0x86, arg1, arg2, arg3, arg4, arg5, arg6, arg7) 
#define draw_line(arg1, arg2, arg3, arg4, arg5, arg6)   issue_6_arg_oled_command(0x83, arg1, arg2, arg3, arg4, arg5, arg6) 
#define draw_rect(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)   issue_8_arg_oled_command(0x84, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) 
#define fill_enable(arg1)                       issue_1_arg_oled_command(0x92, arg1) 
#define front_clock_divider(arg1)       issue_1_arg_oled_command(0xb3, arg1) 
#define horiz_scroll(arg1, arg2, arg3, arg4, arg5)  issue_5_arg_oled_command(0x96, arg1, arg2, arg3, arg4, arg5) 
#define lookup_table_gray_scale(arg1, arg2, arg3, arg4, ar5, arg6, arg7) issue_7_arg_oled_command(0xb8, arg1, arg2, arg3, arg4, arg5, arg6, arg7) 
#define master_config(arg1)             issue_1_arg_oled_command(0xad, arg1) 
#define nop()                           issue_0_arg_oled_command(0xe3) 
#define power_saving_mode(arg1)         issue_1_arg_oled_command(0xb0, arg1) 
#define read_ram_command()              issue_0_arg_oled_command(0x5d) 
#define set_col_address(start, end)     issue_2_arg_oled_command(0x15, start, end) 
#define set_current(arg1, arg2, arg3)   issue_3_arg_oled_command(0xc1, arg1, arg2, arg3) 
#define set_display_all_off()           issue_0_arg_oled_command(0xa4) 
#define set_display_all_on()            issue_0_arg_oled_command(0xa5) 
#define set_display_inverse()           issue_0_arg_oled_command(0xa7) 
#define set_display_offset(arg1)        issue_1_arg_oled_command(0xa2, arg1) 
#define set_display_reset_normal()      issue_0_arg_oled_command(0xa6) 
#define set_display_start_line(arg1)    issue_1_arg_oled_command(0xa1, arg1) 
#define set_master_contrast(arg1)       issue_1_arg_oled_command(0xc7, arg1) 
#define set_mux_ratio(arg1)             issue_1_arg_oled_command(0xca, arg1) 
#define set_precharge_voltage(arg1, arg2, arg3)   issue_3_arg_oled_command(0xbb, arg1, arg2, arg3) 
#define set_remap(arg1)                 issue_1_arg_oled_command(0xa0, arg1) 
#define set_reset(arg1)                 issue_1_arg_oled_command(0xb1, arg1) 
#define set_row_address(start, end)     issue_2_arg_oled_command(0x75, start, end) 
#define set_sleep_off()                 issue_0_arg_oled_command(0xaf) 
#define set_sleep_on()                  issue_0_arg_oled_command(0xae) 
#define set_vcomh(arg1)                 issue_1_arg_oled_command(0xbe, arg1) 
#define start_moving()                  issue_0_arg_oled_command(0x9f) 
#define stop_moving()                   issue_0_arg_oled_command(0x9e) 
#define use_built_in_lut()              issue_0_arg_oled_command(0xb9) 
#define write_ram_command()             issue_0_arg_oled_command(0x5c)
#define clear_screen()                  clear_window(0, 0, 130, 130)

// protos 

void write_oled_data(void); 

#define LCD_DC  42
#define LCD_RES 45
#define LCD_CS  43

#define screenwidth 130
#define screenheight 130

// Basic 18-bit Colors
#define oled_red        63, 0, 0
#define oled_green      0, 26, 0
#define oled_lime       0, 63, 0
#define oled_blue       0, 0, 63
#define oled_black      0, 0, 0   
#define oled_grey       24, 24, 24   
#define oled_white      63, 63, 63
#define oled_yellow     63, 63, 0
#define oled_ltblue     0, 0, 63
#define oled_pink       63, 0, 63
#define oled_ltblue     0, 0, 63
#define oled_orange     63, 19, 0
#define oled_purple     26, 0, 26

// Background color
#define oled_bg oled_black
