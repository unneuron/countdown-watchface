#include <pebble.h>
#include <time.h>
#include "main.h"

// We have to render a main window
static Window *s_main_window;
// The Time layer (text)
static TextLayer *s_time_layer;
// The Background Layer -> Display an image/icon
static BitmapLayer *s_background_layer;
// The Bitmap used to load our image -> then display it as background
static GBitmap *s_background_bitmap;
// The Text layer used for countdown
static TextLayer *s_countdown_layer;

/* START EVENT SETTINGS */
// Result of concatenation should be like: Christmas in XX days
static char PRE_TEXT[] = "Christmas in ";  // used before the number of days
static char POST_TEXT[] = " days";         // used after the number of days
// Setup Your Future Event Here
static int EVENT_YEAR = 2016; // Year.
static int EVENT_MONTH = 12; // 1 (January) - 12 (December)
static int EVENT_DAY = 25;   // Day number 25 - Christmas
static int EVENT_HOUR = 0;   // Start of the day (00:00)
static int EVENT_MINUTE = 0; // minute 0
/* END EVENT SETTINGS */



/* START of Utilities functions */
/*
  This code is derived from PDPCLIB, the public domain C runtime
  library by Paul Edwards. http://pdos.sourceforge.net/
  This code is released to the public domain.
*/
/* scalar date routines    --    public domain by Ray Gardner
** These will work over the range 1-01-01 thru 14699-12-31
** The functions written by Ray are isleap, months_to_days,
** years_to_days, ymd_to_scalar, scalar_to_ymd.
** modified slightly by Paul Edwards
*/
static int isleap (unsigned yr) {
  return yr % 400 == 0 || (yr % 4 == 0 && yr % 100 != 0);
}

static unsigned months_to_days (unsigned month) {
  return (month * 3057 - 3007) / 100;
}

static unsigned years_to_days (unsigned yr) {
  return yr * 365L + yr / 4 - yr / 100 + yr / 400;
}

static long ymd_to_scalar (unsigned yr, unsigned mo, unsigned day) {
  long scalar;

  scalar = day + months_to_days(mo);
  if (mo > 2) // adjust if past February
    scalar -= isleap(yr) ? 1 : 2;
  yr--;
  scalar += years_to_days(yr);
  return scalar;
}

time_t p_mktime (struct tm *timeptr) {
  time_t tt;

  if ((timeptr->tm_year < 70) || (timeptr->tm_year > 120)) {
    tt = (time_t)-1;
  } else {
    tt = ymd_to_scalar(timeptr->tm_year + 1900,
                       timeptr->tm_mon + 1,
                       timeptr->tm_mday)
      - ymd_to_scalar(1970, 1, 1);
    tt = tt * 24 + timeptr->tm_hour;
    tt = tt * 60 + timeptr->tm_min;
    tt = tt * 60 + timeptr->tm_sec;
  }
  return tt;
}
/* END of Utilities functions */


// Function used to update the countdown
static void calculate_countdown() {
	time_t t = time(NULL);
	struct tm *now = localtime(&t);
	static char countText[] = "";
	

	// Set the current time
	time_t seconds_now = p_mktime(now);
	
	now->tm_year = EVENT_YEAR - 1900;
	now->tm_mon = EVENT_MONTH - 1;
	now->tm_mday = EVENT_DAY;
	now->tm_hour = EVENT_HOUR;
	now->tm_min = EVENT_MINUTE;
	now->tm_sec = 0;
	
	time_t seconds_event = p_mktime(now);
	
	// Determine the time difference
	int difference = ((((seconds_event - seconds_now) / 60) / 60) / 24);
	
	if(difference < 0) {
		difference = 0;
	}
	
	// Set the countdown display
	snprintf(countText, 200, "%d", difference);
  
  char *result = malloc(strlen(PRE_TEXT) + strlen(countText) + strlen(POST_TEXT) + 1);
  strcpy(result, PRE_TEXT);
  strcat(result, countText);
  strcat(result, POST_TEXT);
	
	text_layer_set_text(s_countdown_layer, result);
}

// Function used to update the current time
static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}

// Tick Handler -> event called on every tick: used to update Time and CountDown
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  calculate_countdown();
}

// UI. Everything we load is here. Texts, layers, graphics etc.
static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Create GBitmap
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CHRISTMAS_TREE);
  
  // Create BitmapLayer to display the GBitmap
  s_background_layer = bitmap_layer_create(bounds);
  
  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_compositing_mode(s_background_layer, GCompOpSet);
  bitmap_layer_set_alignment(s_background_layer, GAlignTop);
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));

  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  
  // Add days remaining layer
	s_countdown_layer = text_layer_create(GRect(0, 130, bounds.size.w, 23));
  text_layer_set_text(s_countdown_layer, "");
	text_layer_set_text_color(s_countdown_layer, GColorBlack);
	text_layer_set_background_color(s_countdown_layer, GColorClear);
	text_layer_set_text_alignment(s_countdown_layer, GTextAlignmentCenter);
	text_layer_set_font(s_countdown_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_countdown_layer));


  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
}

// UI. Everything we created earlier, we destroy here.
static void main_window_unload(Window *window) {
  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);
  
  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
  
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  
  // Destroy CountDown
  text_layer_destroy(s_countdown_layer);
}

// CORE. Application init. Create the window, handlers, subscribe to timer sercice...
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  // Set Background color
  // window_set_background_color(s_main_window, GColorBlack);
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Make sure the time is displayed from the start
  update_time();
}

// CORE. Destroy the window
static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

// MAIN. This is our main function that is being executed at runtime.
// This is where the application starts.
int main(void) {
  init();
  app_event_loop();
  deinit();
}