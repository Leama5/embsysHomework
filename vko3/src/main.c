#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <stdlib.h>

/****************************
 * Remember to add line:
 * CONFIG_HEAP_MEM_POOL_SIZE=1024
 * to prj.conf
 ****************************/

 //Perustehtävät tehtynä tavoitteena tehdä myös lisää lisäominaisuuksia tulevina viikkoina

// Led pin configurations
static const struct gpio_dt_spec red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
//sininen ei käytössä
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
// Thread initializations
#define STACKSIZE 500
#define PRIORITY 5

void red_led_task(void *, void *, void*);
void yellow_led_task(void *, void *, void *);
void yellow_flash_task(void *, void *, void *);
void green_led_task(void *, void *, void *);

int button1=0;
int button2=0;
int button3=0;
int button4=0;


K_THREAD_DEFINE(red_thread,STACKSIZE,red_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(yellow_thread,STACKSIZE,yellow_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(yellow_flash_thread,STACKSIZE,yellow_flash_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(green_thread,STACKSIZE,green_led_task,NULL,NULL,NULL,PRIORITY,0,0);
// UART initialization
#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)
static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

//Condition Variables
K_MUTEX_DEFINE(red_mutex);
K_CONDVAR_DEFINE(red_signal);
K_MUTEX_DEFINE(yellow_mutex);
K_CONDVAR_DEFINE(yellow_signal);
K_MUTEX_DEFINE(green_mutex);
K_CONDVAR_DEFINE(green_signal);
K_MUTEX_DEFINE(release_mutex);
K_CONDVAR_DEFINE(release_signal);

// Create dispatcher FIFO buffer
K_FIFO_DEFINE(dispatcher_fifo);
K_FIFO_DEFINE(red_fifo);
K_FIFO_DEFINE(yellow_fifo);
K_FIFO_DEFINE(green_fifo);


// FIFO dispatcher data type
struct data_t {
	/*************************
	// Add fifo_reserved below
	*************************/
	void *fifo_reserved;
	char msg[20];
	int time;
};

void button_0_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("Button pressed\n");
	//ei tapahdu mitään tällä hetkellä

}
void button_1_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("1 Button pressed\n");
	//lähetetään signaali red valotaskille
	//k_condvar_broadcast(&red_signal);
	struct data_t *item = k_malloc(sizeof(struct data_t));
    if (item == NULL) {
        return;
	}
    item->time = 1000;

    k_fifo_put(&red_fifo, item);
}
void button_2_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("2 button pressed \n");
	//k_condvar_broadcast(&yellow_signal);
	struct data_t *item = k_malloc(sizeof(struct data_t));
    if (item == NULL) {
        return;
	}
    item->time = 1000;
	k_fifo_put(&yellow_fifo, item);

}
void button_3_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("3 Button pressed\n");
	//k_condvar_broadcast(&green_signal);
	struct data_t *item = k_malloc(sizeof(struct data_t));
    if (item == NULL) {
        return;
	}
    item->time = 1000;
	k_fifo_put(&green_fifo, item);
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
/********************
 * init UART
 */
int init_uart(void) {
	// UART initialization
	if (!device_is_ready(uart_dev)) {
		return 1;
	} 
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

/********************
 * Main task
 */
int main(void)
{
	int ret = init_uart();
	if (ret != 0) {
		printk("UART initialization failed!\n");
		return ret;
	}
	init_led();
	init_button();
	return 0;
}

/********************
 * UART task
 */
static void uart_task(void *unused1, void *unused2, void *unused3)
{
	// Received character from UART
	char rc;
	// Message from UART
	char uart_msg[20];
	int uart_msg_cnt = 0;
	memset(uart_msg,0,20);
	while (true) {
		// Ask UART if data available
		if (uart_poll_in(uart_dev,&rc) == 0) {
			//printk("Received: %c\n",rc);
			// If character is not newline, add to UART message buffer
			if (rc != '\n') {
				uart_msg[uart_msg_cnt] = rc;
				uart_msg_cnt++;
			// Character is newline, copy dispatcher data and put to FIFO buffer
			} else {
				printk("UART msg: %s \n", uart_msg);
                
				struct data_t *buf = k_malloc(sizeof(struct data_t));
				if (buf == NULL) {
					return;
				}
				// Copy UART message to dispatcher data
				snprintf(buf->msg, 20, "%s", uart_msg);

				// You need to:
				// Put dispatcher data to FIFO buffer
				k_fifo_put(&dispatcher_fifo, buf);
				// Clear UART receive buffer
				uart_msg_cnt = 0;
				memset(uart_msg,0,20);

			}
		}
		k_msleep(10);
	}
	return;
}

/********************
 * Dispatcher task
 */
static void dispatcher_task(void *unused1, void *unused2, void *unused3)
{
	while (true) {
		// Receive dispatcher data from uart_task fifo
		struct data_t *rec_item = k_fifo_get(&dispatcher_fifo, K_FOREVER);
		char sequence[20];
		snprintf(sequence, sizeof(sequence), "%s", rec_item->msg);
		char color = sequence[0];
		int time = atoi(sequence + 2);
		rec_item->time = time;
		printk("Data: %c %d\n", color, time);
		printk("Dispatcher: %s\n", sequence);
		//int cnt=0;
		//tulostetaan merkki kerrallaan
		//while (sequence[cnt] != 0){
			if(color == 'R'){
				printk("Red");
				k_fifo_put(&red_fifo,rec_item);
				//lähetetään signaali red valotaskille
				//k_condvar_broadcast(&red_signal);
				//k_condvar_wait(&release_signal, &release_mutex, K_FOREVER);
		
			}
			else if (color == 'Y'){
				printk("Yellow");
				k_fifo_put(&yellow_fifo,rec_item);
				//k_condvar_broadcast(&yellow_signal);
				//k_condvar_wait(&release_signal, &release_mutex, K_FOREVER);
		
			}
			else if (color == 'G'){
				printk("Green");
				k_fifo_put(&green_fifo,rec_item);
				//k_condvar_broadcast(&green_signal);
				//k_condvar_wait(&release_signal, &release_mutex, K_FOREVER);
			}
			//cnt++;

		//}
	}
}
		// You need to:
        // Parse color and time from the fifo data
        // Example
        //    char color = sequence[0];
        //    int time = atoi(sequence+2);
		//    printk("Data: %c %d\n", color, time);
        // Send the parsed color information to tasks using fifo
        // Use release signal to control sequence or k_yield

// Task to handle red led
void red_led_task(void *, void *, void*) {
	
	printk("Red led thread started\n");
	while (true) {
		struct data_t *item =k_fifo_get(&red_fifo,K_FOREVER);
		int time=item->time;
		gpio_pin_set_dt(&red,1);
		printk("Red on\n");
		k_sleep(K_MSEC(time));
		gpio_pin_set_dt(&red,0);
		printk("red off \n");
		k_free(item);
	}
	
}
// Task to handle yellow led
void yellow_led_task(void *, void *, void*) {
	
	printk("Yellow led thread started\n");
	while (true) {
		struct data_t *item =k_fifo_get(&yellow_fifo,K_FOREVER);
		int time=item->time;
		gpio_pin_set_dt(&red,1);
		gpio_pin_set_dt(&green,1);
		printk("Yellow on\n");
		k_sleep(K_MSEC(time));
		gpio_pin_set_dt(&red,0);
		gpio_pin_set_dt(&green,0);
		printk("Yellow off \n");
		k_free(item);
	}
}
// Task to handle green led
void green_led_task(void *, void *, void*) {
	
	printk("Green led thread started\n");
	while (true) {
		struct data_t *item =k_fifo_get(&green_fifo,K_FOREVER);
		int time=item->time;
		gpio_pin_set_dt(&green,1);
		printk("Green on\n");
		k_sleep(K_MSEC(time));
		gpio_pin_set_dt(&green,0);
		printk("Green off \n");
		k_free(item);
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

K_THREAD_DEFINE(dis_thread,STACKSIZE,dispatcher_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(uart_thread,STACKSIZE,uart_task,NULL,NULL,NULL,PRIORITY,0,0);
