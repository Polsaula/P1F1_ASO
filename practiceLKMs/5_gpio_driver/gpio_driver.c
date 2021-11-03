#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/uaccess.h>


/*META INFORMATION*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pol Saula Lloveras");
MODULE_DESCRIPTION("A simple gpio driver for setting a LED and reading a button");

static int device_file_major_number = 0;
static const char device_name[] = "ASO";

static unsigned int gpioLED = 4;       //Hard coding the LED gpio (GPIO4)
static unsigned int gpioButton = 27;   //Hard coding the button gpio (GPIO27)
static unsigned int irqNumber;          //Used to share the IRQ number within this file
static bool ledOn = 0;                  //Is the LED on or off? Used to invert its state (off by default)

/// Function prototype for the custom IRQ handler function -- see below for the implementation
static irq_handler_t  ebbgpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);

/*
* @brief This function is called when the device file is opened
*/
static int driver_open(struct inode *device_file, struct file *instance){
	printk("dev_nr - open was called!\n");
	return 0;
}

/*
* @brief This function is called when the device file is closed
*/
static int driver_close(struct inode *device_file, struct file *instance){
	printk("dev_nr - close was called!\n");
	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = driver_open,
	.release = driver_close
};

/*
 * @brief Register device number 
 */
int register_device(void) {
    int result = 0;
    int request = 0;
    printk(KERN_NOTICE "ASO: register_device() is called\n");
    result = register_chrdev(0, device_name, &fops);
    if (result < 0) {
        printk( KERN_WARNING "ASO: can\'t register character device with error code = %i\n", result);
        return result;
    }
    device_file_major_number = result;
    printk(KERN_NOTICE "ASO: Device File: %i\n", device_file_major_number);

    if (!gpio_is_valid(gpioLED)){
      printk(KERN_INFO "GPIO_TEST: invalid LED GPIO\n");
      return -ENODEV;
    }

    // Going to set up the LED. It is a GPIO in output mode and will be on by default
    ledOn = true;
    gpio_request(gpioLED, "sysfs");          // gpioLED is hardcoded to 4, request it
    gpio_direction_output(gpioLED, ledOn);   // Set the gpio to be in output mode and on
    gpio_export(gpioLED, false);             // Causes gpio4 to appear in /sys/class/gpio

    gpio_request(gpioButton, "sysfs");       // Set up the gpioButton
    gpio_direction_input(gpioButton);        // Set the button GPIO to be an input
    gpio_set_debounce(gpioButton, 200);      // Debounce the button with a delay of 200ms
    gpio_export(gpioButton, false);          // Causes gpio27 to appear in /sys/class/gpio

    // Perform a quick test to see that the button is working as expected on LKM load
    printk(KERN_INFO "GPIO_TEST: The button state is currently: %d\n", gpio_get_value(gpioButton));
 
    // GPIO numbers and IRQ numbers are not the same! This function performs the mapping for us
    irqNumber = gpio_to_irq(gpioButton);
    printk(KERN_INFO "GPIO_TEST: The button is mapped to IRQ: %d\n", irqNumber);
    
    // This next call requests an interrupt line
    request = request_irq(irqNumber,                 // The interrupt number requested
                            (irq_handler_t) ebbgpio_irq_handler, // The pointer to the handler function below
                            IRQF_TRIGGER_RISING,    // Interrupt on rising edge (button press, not release)
                            "ebb_gpio_handler",     // Used in /proc/interrupts to identify the owner
                            NULL);                  // The *dev_id for shared interrupt lines, NULL is okay
    
    printk(KERN_INFO "GPIO_TEST: The interrupt request result is: %d\n", request);
    return request;
}

/*
 * @brief Unregister device number 
 */
void unregister_device(void) {
    gpio_set_value(gpioLED, 0);              // Turn the LED off, makes it clear the device was unloaded
    gpio_unexport(gpioLED);                  // Unexport the LED GPIO
    free_irq(irqNumber, NULL);               // Free the IRQ number, no *dev_id required in this case
    gpio_unexport(gpioButton);               // Unexport the Button GPIO
    gpio_free(gpioLED);                      // Free the LED GPIO
    gpio_free(gpioButton);                   // Free the Button GPIO

    printk(KERN_NOTICE "ASO: unregister_device() is called\n");
    if (device_file_major_number != 0) {
        unregister_chrdev(device_file_major_number, device_name);
    }
}

static irq_handler_t ebbgpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs){
    ledOn = !ledOn;                          // Invert the LED state on each button press
    gpio_set_value(gpioLED, ledOn);          // Set the physical LED accordingly
    printk(KERN_INFO "GPIO_TEST: Interrupt! (button state is %d)\n", gpio_get_value(gpioButton));
    return (irq_handler_t) IRQ_HANDLED;      // Announce that the IRQ has been handled correctly
}

/*
 * @brief This function is called when the module is loaded into de kernel
 */
static int ModuleInit(void){
	printk("Hello Kernel\n");
	register_device();
	return 0;
}

/*
 * @brief This function is called when the module is removed from the kernel
 */
static void ModuleExit(void){
	printk("Goodbye kernel\n");
	unregister_device();
}

module_init(ModuleInit);
module_exit(ModuleExit);

