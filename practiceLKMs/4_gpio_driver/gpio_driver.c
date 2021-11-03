#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>



/*META INFORMATION*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pol Saula Lloveras");
MODULE_DESCRIPTION("A simple gpio driver for setting a LED and reading a button");

static int device_file_major_number = 0;
static const char device_name[] = "ASO";

/*
* @brief Read data out of the buffer
*/
static ssize_t driver_read(struct file *File, char *user_buffer, size_t count, loff_t *offs){
    int to_copy, not_copied, delta;
    char tmp[3] = " \n";

    /* Get amount of data to copy */
    to_copy = min(count, sizeof(tmp));

    /* Read value of button */
    printk("Value of the button: %d\n", gpio_get_value(17));
    tmp[0] = gpio_get_value(17) + '0'; //Get value from pin

    /* Copy data to user */
    not_copied = copy_to_user(user_buffer, &tmp, to_copy);

    /* Calculate data */
    delta = to_copy - not_copied;

    return delta;
}

/*
* @brief Write data to the buffer
*/
static ssize_t driver_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs){
    int to_copy, not_copied, delta;
    char value;

    /* Get amount of data to copy */
    to_copy = min(count, sizeof(value));

    /* Copy data to user */
    not_copied = copy_from_user(&value, user_buffer, to_copy);

    /* Setting the LED */
    switch(value){
        case '0':
            gpio_set_value(4, 0);
            break;
        case '1':
            gpio_set_value(4, 1);
            break;
        default:
            printk("Invalid input! %c\n", value);
            break;
    }
    
    /* Calculate data */
    delta = to_copy - not_copied;

    return delta;
}

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
	.release = driver_close,
    .read = driver_read,
    .write = driver_write
};
/*
 * @brief Register device number 
 */
int register_device(void) {
    int result = 0;
    printk(KERN_NOTICE "ASO: register_device() is called\n");
    result = register_chrdev(0, device_name, &fops);
    if (result < 0) {
        printk( KERN_WARNING "ASO: can\'t register character device with error code = %i\n", result);
        return result;
    }
    device_file_major_number = result;
    printk(KERN_NOTICE "ASO: Device File: %i\n", device_file_major_number);

    /* GPIO 4 init*/
    if(gpio_request(4, "rpi-gpio-4")){
        printk("Can not allocate GPIO 4\n");
    }

    /* Set GPIO 4 direction*/
    if(gpio_direction_output(4, 0)){
        printk("Can not set GPIO 4 to output\n");
    }

    /* GPIO 27 init*/
    if(gpio_request(27, "rpi-gpio-27")){
        printk("Can not allocate GPIO 27\n");
    }

    /* Set GPIO 27 direction*/
    if(gpio_direction_input(27)){
        printk("Can not set GPIO 27 to input\n");
    }



    return 0;
}

/*
 * @brief Unregister device number 
 */
void unregister_device(void) {
    gpio_set_value(4, 0);
    gpio_free(17);
    gpio_free(4);
    printk(KERN_NOTICE "ASO: unregister_device() is called\n");
    if (device_file_major_number != 0) {
        unregister_chrdev(device_file_major_number, device_name);
    }
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

