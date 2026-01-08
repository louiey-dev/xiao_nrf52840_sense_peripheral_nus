/********************************************************************

   Created     :   2024.01.29.

   Author      : louiey.dev@gmail.com

   Description :  command line interface command handler

   History     :  // louiey, 2024.01.29. Basic code implemented

*********************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "cli.h"
#include "cli_command.h"

#include "bsp.h"

#define CLI_VERSION_INFO "Xiao CLI Version 0.1"

static Bool cliCommandInterpreter(int command, int argc, char **argv);

extern BSP_ST g_Bsp;

/**************************** PRIVATE FUNCTION DEFINITIONS *******************/
static CliJte cliCommandTable[] =
    {
        {"init",
         NULL,
         "Initialize",
         CLI_CMD_INITIALISE,
         1,
         NULL,
         0,
         &cliCommandInterpreter},
        {"ver",
         NULL,
         "Get version info",
         CLI_CMD_VERSION,
         1,
         NULL,
         0,
         &cliCommandInterpreter},
        {"exit",
         NULL,
         "Quits the application",
         CLI_CMD_EXIT,
         1,
         NULL,
         0,
         &cliCommandInterpreter},
        /************ RTC *****************/
        {"rtc_init",
         NULL,
         "RTC initialize",
         CLI_CMD_RTC_INIT,
         1,
         NULL,
         0,
         &cliCommandInterpreter},
        {"rtc_set",
         "rtc_set year mon day wday hour min sec // 2024 1 24 3 15 23 45",
         "RTC set date time values",
         CLI_CMD_RTC_SET,
         8,
         NULL,
         0,
         &cliCommandInterpreter},
        {"rtc_get",
         NULL,
         "RTC date time value read",
         CLI_CMD_RTC_GET,
         1,
         NULL,
         0,
         &cliCommandInterpreter},
        /*
         */

        /** Periodic Task **/
        {"prd_set",
         "prd_set 1000 // set 1000ms periodic task",
         "Set PRD task period in ms",
         CLI_CMD_PRD_SET_TICK,
         2,
         NULL,
         0,
         &cliCommandInterpreter},
        {"prd_get",
         "prd_get",
         "Get PRD task period in ms",
         CLI_CMD_PRD_GET_TICK,
         1,
         NULL,
         0,
         &cliCommandInterpreter},

        /** LCD APIs **/
        {"lcd_init",
         NULL,
         "LCD Initialize",
         CLI_CMD_LCD_INIT,
         1,
         NULL,
         0,
         &cliCommandInterpreter},
        {"lcd_ch",
         "lcd_ch a // print ch",
         "LCD print ch which is given",
         CLI_CMD_LCD_WRITE_CHAR,
         2,
         NULL,
         0,
         &cliCommandInterpreter},
        {"lcd_str",
         "lcd_str 0 hello",
         "LCD prints string",
         CLI_CMD_LCD_WRITE_STR,
         3,
         NULL,
         0,
         &cliCommandInterpreter},
        {"lcd_xy",
         "lcd_xy 20 30",
         "LCD set position with given x/y",
         CLI_CMD_LCD_SET_X_Y,
         2,
         NULL,
         0,
         &cliCommandInterpreter},
        {"lcd_clear",
         NULL,
         "LCD clears all screen",
         CLI_CMD_LCD_CLEAR,
         1,
         NULL,
         0,
         &cliCommandInterpreter},
        {"lcd_page",
         NULL,
         "LCD clears given page",
         CLI_CMD_LCD_CLEAR_PAGE,
         2,
         NULL,
         0,
         &cliCommandInterpreter},
        {"lcd_str3",
         "lcd_str3 3 hello",
         "LCD prints string with x3 font",
         CLI_CMD_LCD_WRITE_STR_X3,
         3,
         NULL,
         0,
         &cliCommandInterpreter},
        ///////////////////////////////////////////////////
        {"pwm_init",
         NULL,
         "PWM initialize",
         CLI_CMD_PWM_INIT,
         1,
         NULL,
         0,
         &cliCommandInterpreter},
        {"pwm_duty",
         "pwm_duty 2048",
         "PWM set duty value",
         CLI_CMD_PWM_SET_DUTY,
         2,
         NULL,
         0,
         &cliCommandInterpreter},
        //////////////////////////////////////////////////////
        {"nvs_init",
         NULL,
         "NVS initialize",
         CLI_CMD_NVS_INIT,
         1,
         NULL,
         0,
         &cliCommandInterpreter},
         {"nvs_get",
         NULL,
         "NVS Read",
         CLI_CMD_NVS_GET,
         1,
         NULL,
         0,
         &cliCommandInterpreter},
         {"nvs_set",
         NULL,
         "NVS write info",
         CLI_CMD_NVS_SET,
         1,
         NULL,
         0,
         &cliCommandInterpreter},
         {"nvs_reset",
         NULL,
         "NVS reset so clear NVS",
         CLI_CMD_NVS_RESET,
         1,
         NULL,
         0,
         &cliCommandInterpreter},
};

void cliCommandsInitialise(void)
{
  cliRegisterTable(pTableCli,
                   "Cli commands",
                   sizeof(cliCommandTable) / sizeof(cliCommandTable[0]),
                   cliCommandTable);
}

static Bool cliCommandInterpreter(int command, int argc, char **argv)
{
  Bool result = TRUE; // Assume that we are going to process the command

  int year, mon, day, wday, hour, min, sec;
  uint8_t u8;
  uint16_t u16;
  uint32_t u32;

  // printf("cliCommandInterpreter cmd %d argc %d argv %s\n", command, argc, *argv);

  switch (command)
  {
  case CLI_CMD_INITIALISE:
    break;

  case CLI_CMD_VERSION:
    CLI_PRINT("CLI Version %s", CLI_VERSION_INFO);
    break;

  case CLI_CMD_EXIT:
    // Quit the program
    cliExit();
    break;

#if 1
  /********************************************************/
  //  RTC Command
  case CLI_CMD_RTC_INIT:
    break;

  case CLI_CMD_RTC_SET:
#if 1
    RTC_TIME_ST date = {0};

    year = atoi(argv[1]);
    mon = atoi(argv[2]);
    day = atoi(argv[3]);
    wday = atoi(argv[4]);
    hour = atoi(argv[5]);
    min = atoi(argv[6]);
    sec = atoi(argv[7]);

    date.year = year;
    date.mon = mon;
    date.weekday = wday;
    date.day = day;
    date.hour = hour;
    date.min = min;
    date.sec = sec;

    bsp_rtc_set_time(&date);
#endif

    CLI_PRINT("RTC Set %04d-%02d-%02d, %02d:%02d:%02d\n", year, mon, day, hour, min, sec);
    break;

  case CLI_CMD_RTC_GET:
#if 1
    RTC_TIME_ST gdate = {0};

    bsp_rtc_get_time(&gdate);
    CLI_PRINT("RTC Get 20%02d-%02d-%02d, %02d:%02d:%02d\n", gdate.year, gdate.mon, gdate.day, gdate.hour, gdate.min, gdate.sec);
#endif
    break;
#endif
#if 1
  /********************************************************/
  //  PRD Command
  case CLI_CMD_PRD_SET_TICK:
    g_Bsp.prdTick = atoi(argv[1]);
    CLI_PRINT("Periodic tick set to %d ms", g_Bsp.prdTick);
    break;

  case CLI_CMD_PRD_GET_TICK:

    break;
#endif
#if 0
  /********************************************************/
  /* LCD APIs */
  case CLI_CMD_LCD_INIT:
    bsp_lcd_ssd1306_init();
    CLI_PRINT("LCD init");
    break;

  case CLI_CMD_LCD_WRITE_CHAR:

    break;

  case CLI_CMD_LCD_WRITE_STR:
    u8 = atoi(argv[1]);
    u16 = strlen(argv[2]);
    bsp_lcd_ssd1306_write_text_page(u8, argv[2], u16, 0);
    CLI_PRINT("LCD write string...%s, %d", argv[2], u16);
    break;

  case CLI_CMD_LCD_SET_X_Y:

    break;

  case CLI_CMD_LCD_CLEAR:
    bsp_lcd_ssd1306_clear();
    CLI_PRINT("LCD Clear");
    break;

  case CLI_CMD_LCD_CLEAR_PAGE:
    u8 = atoi(argv[1]);
    bsp_lcd_ssd1306_page_clear(u8);
    CLI_PRINT("LCD page %d clear", u8);
    break;

  case CLI_CMD_LCD_WRITE_STR_X3:
    u8 = atoi(argv[1]);
    u16 = strlen(argv[2]);
    bsp_lcd_ssd1306_write_text_page_x3(u8, argv[2], u16, 0);
    CLI_PRINT("LCD write string...%s, %d", argv[2], u16);
    break;
#endif
  /********************************************************/
  /* PWM APIs */
  case CLI_CMD_PWM_INIT:
    // bsp_buzzer_pwm_init();
    CLI_PRINT("Buzzer pwm initialize");
    break;

  case CLI_CMD_PWM_SET_DUTY:
    u32 = (uint32_t)atoi(argv[1]);
    // bsp_buzzer_pwm_set_duty(u32);
    CLI_PRINT("Buzzer duty set to %d\n", (int)u32);
    break;
  /********************************************************/

  case CLI_CMD_NVS_INIT:
    bsp_nvs_init();
    CLI_PRINT("NVS initialize");
    break;

  case CLI_CMD_NVS_GET:
    bsp_nvs_read(&g_Bsp.nvs);
    CLI_PRINT("NVS read");
    break;

  case CLI_CMD_NVS_SET:
    bsp_nvs_write(&g_Bsp.nvs);
    CLI_PRINT("NVS write");
    break;

  case CLI_CMD_NVS_RESET:
    bsp_nvs_reset();
    CLI_PRINT("NVS reset");
    break;

  default:
    // Unknown command! We should never get here...
    CLI_PRINT("Unknown command, %d\n", command);
    result = FALSE;
    break;
  }

  return result;
}
