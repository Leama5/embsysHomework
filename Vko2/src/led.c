#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <inttypes.h>

//Tavoitteena 2 viikolta täydet pisteet!

// Led pin configurations
static const struct gpio_dt_spec red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec blue = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

#define BUTTON_0 DT_ALIAS(sw0)
#define BUTTON_1 DT_ALIAS(sw1)
#define BUTTON_2 DT_ALIAS(sw2)
#define BUTTON_3 DT_ALIAS(sw3)
#define BUTTON_4 DT_ALIAS(sw4)
// #define BUTTON_1 DT_ALIAS(sw1)
static const struct gpio_dt_spec button_0 = GPIO_DT_SPEC_GET_OR(BUTTON_0, gpios, {0});
static struct gpio_callback button_0_data;
static const struct gpio_dt_spec button_1 = GPIO_DT_SPEC_GET_OR(BUTTON_1, gpios, {0});
static struct gpio_callback button_1_data;
static const struct gpio_dt_spec button_2 = GPIO_DT_SPEC_GET_OR(BUTTON_2, gpios, {0});
static struct gpio_callback button_2_data;
static const struct gpio_dt_spec button_3 = GPIO_DT_SPEC_GET_OR(BUTTON_3, gpios, {0});
static struct gpio_callback button_3_data;
static const struct gpio_dt_spec button_4 = GPIO_DT_SPEC_GET_OR(BUTTON_4, gpios, {0});
static struct gpio_callback button_4_data;
// Red led thread initialization
#define STACKSIZE 500
#define PRIORITY 5
void red_led_task(void *, void *, void*);
void yellow_led_task(void *, void *, void *);
void yellow_flash_task(void *, void *, void *);
void green_led_task(void *, void *, void *);
int led_state=0;
int old_led_state=0;
int button1=0;
int button2=0;
int button3=0;
int button4=0;

K_THREAD_DEFINE(red_thread,STACKSIZE,red_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(yellow_thread,STACKSIZE,yellow_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(green_thread,STACKSIZE,green_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(yellow_flash_thread,STACKSIZE,yellow_flash_task,NULL,NULL,NULL,PRIORITY,0,0);

void button_0_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	if (led_state != 4){
		old_led_state=led_state;
		led_state=4;
	}
	else 
		led_state=old_led_state;
	printk("Button pressed\n");
}
void button_1_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("1 Button pressed\n");
	if (button1 == 0){
		gpio_pin_set_dt(&green,0);
		gpio_pin_set_dt(&red,1);
		button1=1;
	}
	else{
		gpio_pin_set_dt(&red,0);
		button1=0;
	}
		
	
}
void button_2_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("2 Button pressed\n");
	if (button2 == 0){
		gpio_pin_set_dt(&red,1);
		gpio_pin_set_dt(&green,1);
		button2=1;
	}
	else{
		gpio_pin_set_dt(&red,0);
		gpio_pin_set_dt(&green,0);
		button2=0;
	}
}
void button_3_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("3 Button pressed\n");
	if (button3 == 0){
		gpio_pin_set_dt(&red,0);
		gpio_pin_set_dt(&green,1);
		button3=1;
	}
	else{
		gpio_pin_set_dt(&green,0);
		button3=0;
	}
}
void button_4_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("4 Button pressed\n");
	if (button4 == 0){
		button4=1;
	}
	else
		button4=0;

}

// Initialize leds
int  init_led() {

	// Led pin initialization
	int ret = gpio_pin_configure_dt(&red, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error: Red led configure failed\n");		
		return ret;
	}
	// set red led off
	gpio_pin_set_dt(&red,0);

	ret = gpio_pin_configure_dt(&green, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error: Green led configure failed\n");		
		return ret;
	}
	// set green led off
	gpio_pin_set_dt(&green,0);

	ret = gpio_pin_configure_dt(&blue, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error: Blue led configure failed\n");		
		return ret;
	}
	// set blue led off
	gpio_pin_set_dt(&blue,0);
	

	printk("Leds initialized ok\n");
	
	return 0;
}


// Button initialization
int init_button() {
	static const struct gpio_dt_spec buttons[] = {
    button_0,
    button_1,
    button_2,
    button_3,
    button_4
	};
	int ret;

	for (int i=0; i<5; i++){
		if (!gpio_is_ready_dt(&buttons[i])) {
			printk("Error: button %d is not ready\n", i);
			return -1;
		}
		ret = gpio_pin_configure_dt(&buttons[i], GPIO_INPUT);
		if (ret != 0) {
				printk("Error: failed to configure interrupt for button %d\n", i);
				return -1;
		}
		ret = gpio_pin_interrupt_configure_dt(&buttons[i], GPIO_INT_EDGE_TO_ACTIVE);
		if (ret != 0) {
			printk("Error: failed to configure interrupt for button %d\n",i);
			return -1;
		}
	}

	gpio_init_callback(&button_0_data, button_0_handler, BIT(button_0.pin));
	gpio_add_callback(button_0.port, &button_0_data);
	printk("Set up button 0 ok\n");
	gpio_init_callback(&button_1_data, button_1_handler, BIT(button_1.pin));
	gpio_add_callback(button_1.port, &button_1_data);
	printk("Set up button 1 ok\n");
	gpio_init_callback(&button_2_data, button_2_handler, BIT(button_2.pin));
	gpio_add_callback(button_2.port, &button_2_data);
	printk("Set up button 2 ok\n");
	gpio_init_callback(&button_3_data, button_3_handler, BIT(button_3.pin));
	gpio_add_callback(button_3.port, &button_3_data);
	printk("Set up button 3 ok\n");
	gpio_init_callback(&button_4_data, button_4_handler, BIT(button_4.pin));
	gpio_add_callback(button_4.port, &button_4_data);
	printk("Set up button 4 ok\n");
	return 0;
}

int main(void)
{
	init_led();
    int ret = init_button();
	if (ret < 0) {
		return 0;
	}

	while (1) {
		k_msleep(10); // sleep 10ms
	}

	return 0;
}

// Task to handle red led
void red_led_task(void *, void *, void*) {
	
	printk("Red led thread started\n");
	while (true) {
		if (led_state==0){
			// 1. set led on 
			gpio_pin_set_dt(&red,1);
			printk("Red on\n");
			// 2. sleep for 2 seconds
			k_sleep(K_SECONDS(1));
			// 3. set led off
			if (led_state != 4){
			gpio_pin_set_dt(&red,0);
			printk("Red off\n");
			led_state=1;
			}
		}
		k_msleep(100);
	}
}
// Task to handle yellow led
void yellow_led_task(void *, void *, void*) {
	
	printk("Yellow led thread started\n");
	while (true) {
		if (led_state==1){
			gpio_pin_set_dt(&red,1);
			gpio_pin_set_dt(&green,1);
			printk("Yellow on\n");
			// 2. sleep for 2 seconds
			k_sleep(K_SECONDS(1));
			if (led_state != 4){
			// 3. set led off
			gpio_pin_set_dt(&red,0);
			gpio_pin_set_dt(&green,0);
			printk("Yellow off\n");
			led_state=2;
			}
		}
		k_msleep(100);
	}
}
// Task to handle green led
void green_led_task(void *, void *, void*) {
	
	printk("Green led thread started\n");
	while (true) {
		if (led_state==2){
			// 1. set led on 
			gpio_pin_set_dt(&green,1);
			printk("Green on\n");
			// 2. sleep for 2 seconds
			k_sleep(K_SECONDS(1));
			if (led_state != 4){
			// 3. set led off
			gpio_pin_set_dt(&green,0);
			printk("Green off\n");
			led_state = 0;
			}
		}
		k_msleep(100);
	}
}
void yellow_flash_task(void *, void *, void *){
	while (true){
		if (button4 ==1){
		gpio_pin_set_dt(&red,1);
		gpio_pin_set_dt(&green,1);
		k_sleep(K_SECONDS(1));
		gpio_pin_set_dt(&red,0);
		gpio_pin_set_dt(&green,0);
		k_sleep(K_SECONDS(1));
		}
		else{
			k_msleep(10);
		}
	}
}


