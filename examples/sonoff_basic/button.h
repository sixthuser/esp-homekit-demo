#pragma once

#define	 button_gpio 0

typedef enum {
    button_event_single_press,
    button_event_long_press,
} button_event_t;


typedef void (*button_callback_fn)(uint8_t gpio_num, button_event_t event);
/** 
    Starts monitoring the given GPIO pin for the pressed value. Events are recieved through the callback.

    @param gpio_num The GPIO pin that should be monitored
    @param pressed_value The expected value when the button is pressed. For buttons connected to ground this is 0/false, for other buttons this might be 1/true.
    @param long_press_time The duration that should be recognized as a long press, in miliseconds.
    @param callback The callback that is called when an "button" event occurs.
    @return A negative integer if this method fails.
*/

typedef struct _button {
    uint8_t gpio_num;
    button_callback_fn callback;

    uint16_t debounce_time;
    uint16_t long_press_time;

    bool pressed_value;
    bool once_press;

    uint32_t last_press_time;
    uint32_t last_event_time;

    struct _button *next;
} button_t;

int button_create(uint8_t gpio_num, bool pressed_value, uint16_t long_press_time, button_callback_fn callback);

void read_button_task(void *_args) ;

/** 
    Removes the given GPIO pin from monitoring.

    @param gpio_num The GPIO pin that should be removed from monitoring
*/
void button_destroy(uint8_t gpio_num);

void read_button(void *pvParameters);
