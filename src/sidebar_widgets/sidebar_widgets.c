#include <pebble.h>
#include <math.h>
#include "settings.h"
#include "weather.h"
#include "languages.h"
#include "util.h"
#include "sidebar_widgets.h"

bool SidebarWidgets_useCompactMode = false;
int SidebarWidgets_xOffset;

// sidebar icons
GDrawCommandImage* dateImage;
GDrawCommandImage* disconnectImage;
GDrawCommandImage* batteryImage;
GDrawCommandImage* batteryChargeImage;

// fonts
GFont smSidebarFont;
GFont mdSidebarFont;
GFont lgSidebarFont;
GFont currentSidebarFont;
GFont batteryFont;

// the date, time and weather strings
char currentDayName[8];
char currentDayNum[8];
char currentMonth[8];
char currentWeekNum[8];
char currentSecondsNum[8];
char altClock[8];
char currentHours[8];
char currentMinutes[8];
char currentDayOfYearNum[8];

// the widgets
SidebarWidget batteryMeterWidget;
int BatteryMeter_getHeight();
void BatteryMeter_draw(GContext* ctx, int yPosition);

SidebarWidget emptyWidget;
int EmptyWidget_getHeight();
void EmptyWidget_draw(GContext* ctx, int yPosition);

SidebarWidget dateWidget;
int DateWidget_getHeight();
void DateWidget_draw(GContext* ctx, int yPosition);

SidebarWidget currentWeatherWidget;
int CurrentWeather_getHeight();
void CurrentWeather_draw(GContext* ctx, int yPosition);

SidebarWidget weatherForecastWidget;
int WeatherForecast_getHeight();
void WeatherForecast_draw(GContext* ctx, int yPosition);

SidebarWidget btDisconnectWidget;
int BTDisconnect_getHeight();
void BTDisconnect_draw(GContext* ctx, int yPosition);

SidebarWidget weekNumberWidget;
int WeekNumber_getHeight();
void WeekNumber_draw(GContext* ctx, int yPosition);

SidebarWidget secondsWidget;
int Seconds_getHeight();
void Seconds_draw(GContext* ctx, int yPosition);

SidebarWidget altTimeWidget;
int AltTime_getHeight();
void AltTime_draw(GContext* ctx, int yPosition);

SidebarWidget timeWidget;
int Time_getHeight();
void Time_draw(GContext* ctx, int yPosition);

SidebarWidget dayNumberWidget;
int DayNumber_getHeight();
void DayNumber_draw(GContext* ctx, int yPosition);

#ifdef PBL_HEALTH
  GDrawCommandImage* sleepImage;
  GDrawCommandImage* stepsImage;

  SidebarWidget healthWidget;
  int Health_getHeight();
  void Health_draw(GContext* ctx, int yPosition);
  void Sleep_draw(GContext* ctx, int yPosition);
  void Steps_draw(GContext* ctx, int yPosition);
#endif

void SidebarWidgets_init() {
  // load fonts
  smSidebarFont = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  mdSidebarFont = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  lgSidebarFont = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);

  // load the sidebar graphics
  dateImage = gdraw_command_image_create_with_resource(RESOURCE_ID_DATE_BG);
  disconnectImage = gdraw_command_image_create_with_resource(RESOURCE_ID_DISCONNECTED);
  batteryImage = gdraw_command_image_create_with_resource(RESOURCE_ID_BATTERY_BG);
  batteryChargeImage = gdraw_command_image_create_with_resource(RESOURCE_ID_BATTERY_CHARGE);

  #ifdef PBL_HEALTH
    sleepImage = gdraw_command_image_create_with_resource(RESOURCE_ID_HEALTH_SLEEP);
    stepsImage = gdraw_command_image_create_with_resource(RESOURCE_ID_HEALTH_STEPS);
  #endif

  // set up widgets' function pointers correctly
  batteryMeterWidget.getHeight = BatteryMeter_getHeight;
  batteryMeterWidget.draw      = BatteryMeter_draw;

  emptyWidget.getHeight = EmptyWidget_getHeight;
  emptyWidget.draw      = EmptyWidget_draw;

  dateWidget.getHeight = DateWidget_getHeight;
  dateWidget.draw      = DateWidget_draw;

  currentWeatherWidget.getHeight = CurrentWeather_getHeight;
  currentWeatherWidget.draw      = CurrentWeather_draw;

  weatherForecastWidget.getHeight = WeatherForecast_getHeight;
  weatherForecastWidget.draw      = WeatherForecast_draw;

  btDisconnectWidget.getHeight = BTDisconnect_getHeight;
  btDisconnectWidget.draw      = BTDisconnect_draw;

  weekNumberWidget.getHeight = WeekNumber_getHeight;
  weekNumberWidget.draw      = WeekNumber_draw;

  secondsWidget.getHeight = Seconds_getHeight;
  secondsWidget.draw      = Seconds_draw;

  altTimeWidget.getHeight = AltTime_getHeight;
  altTimeWidget.draw      = AltTime_draw;

  timeWidget.getHeight = Time_getHeight;
  timeWidget.draw      = Time_draw;

  dayNumberWidget.getHeight = DayNumber_getHeight;
  dayNumberWidget.draw      = DayNumber_draw;

  #ifdef PBL_HEALTH
    healthWidget.getHeight = Health_getHeight;
    healthWidget.draw = Health_draw;
  #endif

}

void SidebarWidgets_deinit() {
  gdraw_command_image_destroy(dateImage);
  gdraw_command_image_destroy(disconnectImage);
  gdraw_command_image_destroy(batteryImage);
  gdraw_command_image_destroy(batteryChargeImage);

  #ifdef PBL_HEALTH
    gdraw_command_image_destroy(stepsImage);
    gdraw_command_image_destroy(sleepImage);
  #endif
}

void SidebarWidgets_updateFonts() {
  if(globalSettings.useLargeFonts) {
    currentSidebarFont = lgSidebarFont;
    batteryFont = lgSidebarFont;
  } else {
    currentSidebarFont = mdSidebarFont;
    batteryFont = smSidebarFont;
  }
}

// c can't do true modulus on negative numbers, apparently
// from http://stackoverflow.com/questions/11720656/modulo-operation-with-negative-numbers
int mod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}

void SidebarWidgets_updateTime(struct tm* timeInfo) {
  // set all the date strings
  strftime(currentDayNum,  3, "%e", timeInfo);
  strftime(currentWeekNum, 3, "%V", timeInfo);

  strftime(currentDayOfYearNum, 8, "%j", timeInfo);
  int today = atoi(currentDayOfYearNum);
  snprintf(currentDayOfYearNum, sizeof(currentDayOfYearNum), "%d", today);
    
  // set the seconds string
  strftime(currentSecondsNum, 4, ":%S", timeInfo);

  // set the current time strings
  if(clock_is_24h_style()) {
    strftime(currentHours, 3, "%H", timeInfo);
  } else {
    strftime(currentHours, 3, "%I", timeInfo);
  }
  if(!globalSettings.showLeadingZero && currentHours[0] == '0') {
    currentHours[0] = ' ';
  }
  strftime(currentMinutes, 3, "%M", timeInfo);

  // set the alternate time zone string
  int hour = timeInfo->tm_hour;

  // apply the configured offset value
  hour += globalSettings.altclockOffset;

  // format it
  if(clock_is_24h_style()) {
    hour = mod(hour, 24);
  } else {
    hour = mod(hour, 12);

    if(hour == 0) {
      hour = 12;
    }
  }

  if(globalSettings.showLeadingZero && hour < 10) {
    snprintf(altClock, sizeof(altClock), "0%i", hour);
  } else {
    snprintf(altClock, sizeof(altClock), "%i", hour);
  }

  strncpy(currentDayName, dayNames[globalSettings.languageId][timeInfo->tm_wday], sizeof(currentDayName));
  strncpy(currentMonth, monthNames[globalSettings.languageId][timeInfo->tm_mon], sizeof(currentMonth));

  // remove padding on date num, if needed
  if(currentDayNum[0] == ' ') {
    currentDayNum[0] = currentDayNum[1];
    currentDayNum[1] = '\0';
  }
}

/* Sidebar Widget Selection */
SidebarWidget getSidebarWidgetByType(SidebarWidgetType type) {
  switch(type) {
    case BATTERY_METER:
      return batteryMeterWidget;
      break;
    case BLUETOOTH_DISCONNECT:
      return btDisconnectWidget;
      break;
    case DATE:
      return dateWidget;
      break;
    case ALT_TIME_ZONE:
      return altTimeWidget;
      break;
    case TIME:
      return timeWidget;
      break;
    case SECONDS:
      return secondsWidget;
      break;
    case WEATHER_CURRENT:
      return currentWeatherWidget;
      break;
    case WEATHER_FORECAST_TODAY:
      return weatherForecastWidget;
      break;
    case WEEK_NUMBER:
      return weekNumberWidget;
      break;
    #ifdef PBL_HEALTH
      case HEALTH:
        return healthWidget;
        break;
    #endif
    case DAY_NUMBER:
      return dayNumberWidget;
      break;
    default:
      return emptyWidget;
      break;
  }
}

/********** functions for the empty widget **********/
int EmptyWidget_getHeight() {
  return 0;
}

void EmptyWidget_draw(GContext* ctx, int yPosition) {
  return;
}

/********** functions for the battery meter widget **********/

int BatteryMeter_getHeight() {
  BatteryChargeState chargeState = battery_state_service_peek();

  if(chargeState.is_charging || !globalSettings.showBatteryPct) {
    return 14; // graphic only height
  } else {
    return (globalSettings.useLargeFonts) ? 33 : 27; // heights with text
  }
}

void BatteryMeter_draw(GContext* ctx, int yPosition) {

  BatteryChargeState chargeState = battery_state_service_peek();

  graphics_context_set_text_color(ctx, globalSettings.sidebarTextColor);

  char batteryString[6];
  int batteryPositionY = yPosition - 5; // correct for vertical empty space on battery icon

  if (batteryImage) {
    gdraw_command_image_recolor(batteryImage, globalSettings.iconFillColor, globalSettings.iconStrokeColor);
    gdraw_command_image_draw(ctx, batteryImage, GPoint(3 + SidebarWidgets_xOffset, batteryPositionY));
  }

  if(chargeState.is_charging) {
    if(batteryChargeImage) {
      // the charge "bolt" icon uses inverted colors
      gdraw_command_image_recolor(batteryChargeImage, globalSettings.iconStrokeColor, globalSettings.iconFillColor);
      gdraw_command_image_draw(ctx, batteryChargeImage, GPoint(3 + SidebarWidgets_xOffset, batteryPositionY));
    }
  } else {

    int width = roundf(18 * chargeState.charge_percent / 100.0f);

    graphics_context_set_fill_color(ctx, globalSettings.iconStrokeColor);

    if(chargeState.charge_percent <= 20) {
      graphics_context_set_fill_color(ctx, GColorRed);
    }

    graphics_fill_rect(ctx, GRect(6 + SidebarWidgets_xOffset, 8 + batteryPositionY, width, 8), 0, GCornerNone);
  }

  // never show battery % while charging, because of this issue:
  // https://github.com/freakified/TimeStylePebble/issues/11
  if(globalSettings.showBatteryPct && !chargeState.is_charging) {
    if(!globalSettings.useLargeFonts) {
      snprintf(batteryString, sizeof(batteryString), "%d%%", chargeState.charge_percent);

      graphics_draw_text(ctx,
                         batteryString,
                         batteryFont,
                         GRect(-4 + SidebarWidgets_xOffset, 18 + batteryPositionY, 38, 20),
                         GTextOverflowModeFill,
                         GTextAlignmentCenter,
                         NULL);
    } else {
      snprintf(batteryString, sizeof(batteryString), "%d", chargeState.charge_percent);

      graphics_draw_text(ctx,
                         batteryString,
                         batteryFont,
                         GRect(-4 + SidebarWidgets_xOffset, 14 + batteryPositionY, 38, 20),
                         GTextOverflowModeFill,
                         GTextAlignmentCenter,
                         NULL);
    }
  }
}

/********** current date widget **********/

int DateWidget_getHeight() {
  if(globalSettings.useLargeFonts) {
    return (SidebarWidgets_useCompactMode) ? 42 : 62;
  } else  {
    return (SidebarWidgets_useCompactMode) ? 41 : 58;
  }
}

void DateWidget_draw(GContext* ctx, int yPosition) {
  graphics_context_set_text_color(ctx, globalSettings.sidebarTextColor);

  // compensate for extra space that appears on the top of the date widget
  yPosition -= (globalSettings.useLargeFonts) ? 10 : 7;

  // first draw the day name
  graphics_draw_text(ctx,
                     currentDayName,
                     currentSidebarFont,
                     GRect(-5 + SidebarWidgets_xOffset, yPosition, 40, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);

  // next, draw the date background
  // (an image in normal mode, a rectangle in large font mode)
  if(!globalSettings.useLargeFonts) {
    if(dateImage) {
      gdraw_command_image_recolor(dateImage, globalSettings.iconFillColor, globalSettings.iconStrokeColor);
      gdraw_command_image_draw(ctx, dateImage, GPoint(3 + SidebarWidgets_xOffset, yPosition + 23));
    }
  } else {
    graphics_context_set_fill_color(ctx, globalSettings.iconStrokeColor);
    graphics_fill_rect(ctx, GRect(2 + SidebarWidgets_xOffset, yPosition + 30, 26, 22), 2, GCornersAll);

    graphics_context_set_fill_color(ctx, globalSettings.iconFillColor);
    graphics_fill_rect(ctx, GRect(4 + SidebarWidgets_xOffset, yPosition + 32, 22, 18), 0, GCornersAll);
  }

  // next, draw the date number
  graphics_context_set_text_color(ctx, globalSettings.iconStrokeColor);

  int yOffset = 0;
  yOffset = globalSettings.useLargeFonts ? 24 : 26;

  graphics_draw_text(ctx,
                     currentDayNum,
                     currentSidebarFont,
                     GRect(0 + SidebarWidgets_xOffset, yPosition + yOffset, 30, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);


   // switch back to normal color for the rest
  graphics_context_set_text_color(ctx, globalSettings.sidebarTextColor);

  // don't draw the month if we're in compact mode
  if(!SidebarWidgets_useCompactMode) {
    yOffset = globalSettings.useLargeFonts ? 48 : 47;

    graphics_draw_text(ctx,
                       currentMonth,
                       currentSidebarFont,
                       GRect(0 + SidebarWidgets_xOffset, yPosition + yOffset, 30, 20),
                       GTextOverflowModeFill,
                       GTextAlignmentCenter,
                       NULL);
  }


}

/********** current weather widget **********/

int CurrentWeather_getHeight() {
  if(globalSettings.useLargeFonts) {
    return 44;
  } else {
    return 42;
  }
}

void CurrentWeather_draw(GContext* ctx, int yPosition) {
  graphics_context_set_text_color(ctx, globalSettings.sidebarTextColor);

  if (Weather_currentWeatherIcon) {
    gdraw_command_image_recolor(Weather_currentWeatherIcon, globalSettings.iconFillColor, globalSettings.iconStrokeColor);

    gdraw_command_image_draw(ctx, Weather_currentWeatherIcon, GPoint(3 + SidebarWidgets_xOffset, yPosition));
  }

  // draw weather data only if it has been set
  if(Weather_weatherInfo.currentTemp != INT32_MIN) {

    int currentTemp = Weather_weatherInfo.currentTemp;

    if(!globalSettings.useMetric) {
      currentTemp = roundf(currentTemp * 1.8f + 32);
    }

    char tempString[8];

    // in large font mode, omit the degree symbol and move the text
    if(!globalSettings.useLargeFonts) {
      snprintf(tempString, sizeof(tempString), " %d°", currentTemp);

      graphics_draw_text(ctx,
                         tempString,
                         currentSidebarFont,
                         GRect(-5 + SidebarWidgets_xOffset, yPosition + 24, 38, 20),
                         GTextOverflowModeFill,
                         GTextAlignmentCenter,
                         NULL);
    } else {
      snprintf(tempString, sizeof(tempString), " %d", currentTemp);

      graphics_draw_text(ctx,
                         tempString,
                         currentSidebarFont,
                         GRect(-5 + SidebarWidgets_xOffset, yPosition + 20, 35, 20),
                         GTextOverflowModeFill,
                         GTextAlignmentCenter,
                         NULL);
    }
  } else {
    // if the weather data isn't set, draw a loading indication
    graphics_draw_text(ctx,
                       "...",
                       currentSidebarFont,
                       GRect(-5 + SidebarWidgets_xOffset, yPosition, 38, 20),
                       GTextOverflowModeFill,
                       GTextAlignmentCenter,
                       NULL);
  }
}

/***** Bluetooth Disconnection Widget *****/

int BTDisconnect_getHeight() {
  return 22;
}

void BTDisconnect_draw(GContext* ctx, int yPosition) {
  if(disconnectImage) {
    gdraw_command_image_recolor(disconnectImage, globalSettings.iconFillColor, globalSettings.iconStrokeColor);


    gdraw_command_image_draw(ctx, disconnectImage, GPoint(3 + SidebarWidgets_xOffset, yPosition));
  }
}

/***** Week Number Widget *****/

int WeekNumber_getHeight() {
  return (globalSettings.useLargeFonts) ? 29 : 26;
}

void WeekNumber_draw(GContext* ctx, int yPosition) {
  graphics_context_set_text_color(ctx, globalSettings.sidebarTextColor);

  // note that it draws "above" the y position to correct for
  // the vertical padding
  graphics_draw_text(ctx,
                     wordForWeek[globalSettings.languageId],
                     smSidebarFont,
                     GRect(-4 + SidebarWidgets_xOffset, yPosition - 4, 38, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);

  if(!globalSettings.useLargeFonts) {
    graphics_draw_text(ctx,
                       currentWeekNum,
                       mdSidebarFont,
                       GRect(0 + SidebarWidgets_xOffset, yPosition + 9, 30, 20),
                       GTextOverflowModeFill,
                       GTextAlignmentCenter,
                       NULL);
  } else {
    graphics_draw_text(ctx,
                       currentWeekNum,
                       lgSidebarFont,
                       GRect(0 + SidebarWidgets_xOffset, yPosition + 6, 30, 20),
                       GTextOverflowModeFill,
                       GTextAlignmentCenter,
                       NULL);
  }
}

/***** Seconds Widget *****/

int Seconds_getHeight() {
  return 14;
}

void Seconds_draw(GContext* ctx, int yPosition) {
  graphics_context_set_text_color(ctx, globalSettings.sidebarTextColor);

  graphics_draw_text(ctx,
                     currentSecondsNum,
                     lgSidebarFont,
                     GRect(0 + SidebarWidgets_xOffset, yPosition - 10, 30, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);
}

/***** Time Widget *****/

int Time_getHeight() {
  return 31;
}

void Time_draw(GContext* ctx, int yPosition) {
  graphics_context_set_text_color(ctx, globalSettings.sidebarTextColor);

  graphics_draw_text(ctx,
                     currentHours,
                     lgSidebarFont,
                     GRect(0 + SidebarWidgets_xOffset, yPosition - 10, 25, 14),
                     GTextOverflowModeFill,
                     GTextAlignmentRight,
                     NULL);

  graphics_draw_text(ctx,
                     currentMinutes,
                     lgSidebarFont,
                     GRect(0 + SidebarWidgets_xOffset, yPosition + 7, 25, 14),
                     GTextOverflowModeFill,
                     GTextAlignmentRight,
                     NULL);
}

/***** Weather Forecast Widget *****/

int WeatherForecast_getHeight() {
  if(globalSettings.useLargeFonts) {
    return 63;
  } else {
    return 60;
  }
}

void WeatherForecast_draw(GContext* ctx, int yPosition) {
  graphics_context_set_text_color(ctx, globalSettings.sidebarTextColor);

  if(Weather_forecastWeatherIcon) {
    gdraw_command_image_recolor(Weather_forecastWeatherIcon, globalSettings.iconFillColor, globalSettings.iconStrokeColor);

    gdraw_command_image_draw(ctx, Weather_forecastWeatherIcon, GPoint(3 + SidebarWidgets_xOffset, yPosition));
  }

  // draw weather data only if it has been set
  if(Weather_weatherForecast.highTemp != INT32_MIN) {

    int highTemp = Weather_weatherForecast.highTemp;
    int lowTemp  = Weather_weatherForecast.lowTemp;

    if(!globalSettings.useMetric) {
      highTemp = roundf(highTemp * 1.8f + 32);
      lowTemp  = roundf(lowTemp * 1.8f + 32);
    }

    char tempString[8];

    graphics_context_set_fill_color(ctx, globalSettings.sidebarTextColor);

    // in large font mode, omit the degree symbol and move the text
    if(!globalSettings.useLargeFonts) {
      snprintf(tempString, sizeof(tempString), " %d°", highTemp);

      graphics_draw_text(ctx,
                         tempString,
                         currentSidebarFont,
                         GRect(-5 + SidebarWidgets_xOffset, yPosition + 24, 38, 20),
                         GTextOverflowModeFill,
                         GTextAlignmentCenter,
                         NULL);

      graphics_fill_rect(ctx, GRect(3 + SidebarWidgets_xOffset, 8 + yPosition + 37, 24, 1), 0, GCornerNone);

      snprintf(tempString, sizeof(tempString), " %d°", lowTemp);

      graphics_draw_text(ctx,
                         tempString,
                         currentSidebarFont,
                         GRect(-5 + SidebarWidgets_xOffset, yPosition + 42, 38, 20),
                         GTextOverflowModeFill,
                         GTextAlignmentCenter,
                         NULL);
    } else {
      snprintf(tempString, sizeof(tempString), "%d", highTemp);

      graphics_draw_text(ctx,
                         tempString,
                         currentSidebarFont,
                         GRect(0 + SidebarWidgets_xOffset, yPosition + 20, 30, 20),
                         GTextOverflowModeFill,
                         GTextAlignmentCenter,
                         NULL);

      graphics_fill_rect(ctx, GRect(3 + SidebarWidgets_xOffset, 8 + yPosition + 38, 24, 1), 0, GCornerNone);

      snprintf(tempString, sizeof(tempString), "%d", lowTemp);

      graphics_draw_text(ctx,
                         tempString,
                         currentSidebarFont,
                         GRect(0 + SidebarWidgets_xOffset, yPosition + 39, 30, 20),
                         GTextOverflowModeFill,
                         GTextAlignmentCenter,
                         NULL);
    }
  } else {
    // if the weather data isn't set, draw a loading indication
    graphics_draw_text(ctx,
                       "...",
                       currentSidebarFont,
                       GRect(-5 + SidebarWidgets_xOffset, yPosition, 38, 20),
                       GTextOverflowModeFill,
                       GTextAlignmentCenter,
                       NULL);
  }
}

/***** Alternate Time Zone Widget *****/

int AltTime_getHeight() {
  return (globalSettings.useLargeFonts) ? 29 : 26;
}

void AltTime_draw(GContext* ctx, int yPosition) {
  graphics_context_set_text_color(ctx, globalSettings.sidebarTextColor);

  graphics_draw_text(ctx,
                     globalSettings.altclockName,
                     smSidebarFont,
                     GRect(0 + SidebarWidgets_xOffset, yPosition - 5, 30, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);

  int yMod = (globalSettings.useLargeFonts) ? 5 : 8;

  graphics_draw_text(ctx,
                     altClock,
                     currentSidebarFont,
                     GRect(-1 + SidebarWidgets_xOffset, yPosition + yMod, 30, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);
}

/***** Health Widget *****/

#ifdef PBL_HEALTH

bool Health_use_sleep_mode() {
  uint32_t current_activities = health_service_peek_current_activities();
  bool sleeping = current_activities & HealthActivitySleep || current_activities & HealthActivityRestfulSleep;

  if(sleeping) {
    return true;
  } else {
    // check if they just woke up (ie have they been asleep in the last 5m?)
    time_t end = time(NULL);
    time_t start = end - SECONDS_PER_MINUTE * 5;

    if(health_service_is_activity_in_range(HealthActivitySleep | HealthActivityRestfulSleep, start, end)) {
      return true;
    } else {
      return false;
    }
  }

  return sleeping;
}

int Health_getHeight() {
  if(Health_use_sleep_mode()) {
    return 44;
  } else {
    return 32;
  }
}

void Health_draw(GContext* ctx, int yPosition) {
  // check if we're showing the sleep data or step data

  // is the user asleep?
  bool sleep_mode = Health_use_sleep_mode();

  if(sleep_mode) {
    Sleep_draw(ctx, yPosition);
  } else {
    Steps_draw(ctx, yPosition);
  }
}

void Sleep_draw(GContext* ctx, int yPosition) {
  if(sleepImage) {
    gdraw_command_image_recolor(sleepImage, globalSettings.iconFillColor, globalSettings.iconStrokeColor);
    gdraw_command_image_draw(ctx, sleepImage, GPoint(3 + SidebarWidgets_xOffset, yPosition - 7));
  }

  // get sleep in seconds
  int sleep_seconds;

  if(globalSettings.healthUseRestfulSleep) {
    sleep_seconds = (int)health_service_sum_today(HealthMetricSleepSeconds);
  } else {
    sleep_seconds = (int)health_service_sum_today(HealthMetricSleepRestfulSeconds);
  }

  // convert to hours/minutes
  int sleep_minutes = sleep_seconds / 60;
  int sleep_hours   = sleep_minutes / 60;

  // find minutes remainder
  sleep_minutes %= 60;

  char sleep_text[4];

  snprintf(sleep_text, sizeof(sleep_text), "%ih", sleep_hours);

  graphics_context_set_text_color(ctx, globalSettings.sidebarTextColor);
  graphics_draw_text(ctx,
                     sleep_text,
                     mdSidebarFont,
                     GRect(-2 + SidebarWidgets_xOffset, yPosition + 14, 34, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);

  snprintf(sleep_text, sizeof(sleep_text), "%im", sleep_minutes);

  graphics_draw_text(ctx,
                     sleep_text,
                     smSidebarFont,
                     GRect(-2 + SidebarWidgets_xOffset, yPosition + 30, 34, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);

}

void Steps_draw(GContext* ctx, int yPosition) {

  if(stepsImage) {
    gdraw_command_image_recolor(stepsImage, globalSettings.iconFillColor, globalSettings.iconStrokeColor);
    gdraw_command_image_draw(ctx, stepsImage, GPoint(3 + SidebarWidgets_xOffset, yPosition - 7));
  }

  char steps_text[8];

  if(globalSettings.healthUseDistance) {
    int meters = (int)health_service_sum_today(HealthMetricWalkedDistanceMeters);

    // format distance string
    if(globalSettings.useMetric) {
      if(meters < 1000) {
        snprintf(steps_text, sizeof(steps_text), "%im", meters);
      } else {
        meters /= 1000; // convert to km
        snprintf(steps_text, sizeof(steps_text), "%ikm", meters);
      }
    } else {
      int miles_tenths = meters * 10 / 1609 % 10;
      int miles_whole  = (int)roundf(meters / 1609.0f);

      if(miles_whole > 0) {
        snprintf(steps_text, sizeof(steps_text), "%imi", miles_whole);
      } else {
        snprintf(steps_text, sizeof(steps_text), "%c%imi", globalSettings.decimalSeparator, miles_tenths);
      }
    }
  } else {
    int steps = (int)health_service_sum_today(HealthMetricStepCount);

    // format step string
    if(steps < 1000) {
      snprintf(steps_text, sizeof(steps_text), "%i", steps);
    } else {
      int steps_thousands = steps / 1000;
      int steps_hundreds  = steps / 100 % 10;

      if (steps < 10000) {
        snprintf(steps_text, sizeof(steps_text), "%i%c%ik", steps_thousands, globalSettings.decimalSeparator, steps_hundreds);
      } else {
        snprintf(steps_text, sizeof(steps_text), "%ik", steps_thousands);
      }
    }
  }


  graphics_context_set_text_color(ctx, globalSettings.sidebarTextColor);

  graphics_draw_text(ctx,
                     steps_text,
                     mdSidebarFont,
                     GRect(-2 + SidebarWidgets_xOffset, yPosition + 13, 34, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);
}

#endif

/***** Day Number Widget *****/

int DayNumber_getHeight() {
  return (globalSettings.useLargeFonts) ? 29 : 26;
}

void DayNumber_draw(GContext* ctx, int yPosition) {
  graphics_context_set_text_color(ctx, globalSettings.sidebarTextColor);

  // note that it draws "above" the y position to correct for
  // the vertical padding
  graphics_draw_text(ctx,
                     wordForDay[globalSettings.languageId],
                     smSidebarFont,
                     GRect(-4 + SidebarWidgets_xOffset, yPosition - 4, 38, 20),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);
  int yOffset = 0;
  yOffset = globalSettings.useLargeFonts ? 9 : 6;
  graphics_draw_text(ctx,
                       currentDayOfYearNum,
                       mdSidebarFont,
                       GRect(0 + SidebarWidgets_xOffset, yPosition + yOffset, 30, 20),
                       GTextOverflowModeFill,
                       GTextAlignmentCenter,
                       NULL);
}