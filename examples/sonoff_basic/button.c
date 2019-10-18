#include <string.h>
#include <esplibs/libmain.h>
#include "button.h"

button_t *buttons = NULL;

static button_t *button_find_by_gpio(const uint8_t gpio_num) {
    button_t *button = buttons;
    while (button && button->gpio_num != gpio_num)
        button = button->next;

    return button;
}
/*
void button_intr_callback(uint8_t gpio) {
    button_t *button = button_find_by_gpio(gpio);
    if (!button)
        return;    
     uint32_t now = xTaskGetTickCountFromISR();
    if ((now - button->last_event_time)*portTICK_PERIOD_MS < button->debounce_time) {
        // debounce time, ignore events
        return;
    }
    button->last_event_time = now;
    if (gpio_read(button->gpio_num) == button->pressed_value) {
        // Record when the button is pressed down.
        button->last_press_time = now;
    } else {
        // The button is released. Handle the use cases.
        if ((now - button->last_press_time) * portTICK_PERIOD_MS > button->long_press_time) {
            button->callback(button->gpio_num, button_event_long_press);
        } else {
            button->callback(button->gpio_num, button_event_single_press);
        }
    }	 
}	*/
	 
void read_button_task(void *_args) 
{
	button_t *button = button_find_by_gpio(button_gpio);
	printf("read button task start!\n");
	while (1) 
	{		
		if (gpio_read(button->gpio_num) == button->pressed_value)	//button is pressed
		{	if(button->last_press_time <  button->long_press_time) button->last_press_time ++;
			if(button->last_press_time > button->debounce_time)
			{	button->once_press = 1;			
			}
		}
		else	
		{	if(button->once_press == 1)
			{	if(button->last_press_time >= button->long_press_time)
				{	button->callback(button->gpio_num, button_event_long_press);			
				}
				else button->callback(button->gpio_num, button_event_single_press);
			}
			button->once_press = 0;	
			button->last_press_time = 0;
		}
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
	printf("read button task delete!\n");
}
	 
int button_create(const uint8_t gpio_num, bool pressed_value, uint16_t long_press_time, button_callback_fn callback) {
    button_t *button = button_find_by_gpio(gpio_num);
    if (button)
        return -1;

    button = malloc(sizeof(button_t));
    memset(button, 0, sizeof(*button));
    button->gpio_num = gpio_num;
    button->callback = callback;
    button->pressed_value = pressed_value;
    button->once_press = 0;

    // times in milliseconds
    button->debounce_time = 5;
    button->long_press_time = long_press_time;

    uint32_t now = xTaskGetTickCountFromISR();
    button->last_event_time = 0;
    button->last_press_time = 0;

    button->next = buttons;
    buttons = button;

    gpio_set_pullup(button->gpio_num, true, true);
//    gpio_set_interrupt(button->gpio_num, GPIO_INTTYPE_EDGE_ANY, button_intr_callback);

    return 0;
}


void button_delete(const uint8_t gpio_num) {
    if (!buttons)
        return;

    button_t *button = NULL;
    if (buttons->gpio_num == gpio_num) {
        button = buttons;
        buttons = buttons->next;
    } else {
        button_t *b = buttons;
        while (b->next) {
            if (b->next->gpio_num == gpio_num) {
                button = b->next;
                b->next = b->next->next;
                break;
            }
        }
    }

    if (button) {
        gpio_set_interrupt(gpio_num, GPIO_INTTYPE_EDGE_ANY, NULL);
    }
}

