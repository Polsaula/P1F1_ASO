#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>


/*META INFORMATION*/
MODULE_AUTHOR("Pol Saula Lloveras");
MODULE_DESCRIPTION("Registers a device nr. and implement some callback functions");

static int device_file_major_number = 0;
static const char device_name[] = "ASO";

/* Buffer for data */
static char buffer[255]; 
static int buffer_pointer;

/*
* @brief Read data out of the buffer
*/
static ssize_t driver_read(struct file *File, char *user_buffer, size_t count, loff_t *offs){
    int to_copy, not_copied, delta;

    /* Get amount of data to copy */
    to_copy = min(count, buffer_pointer);

    /* Copy data to user */
    not_copied = copy_to_user(user_buffer, buffer, to_copy);

    /* Calculate data */
    delta = to_copy - not_copied;

    return delta;
}

/*
* @brief Write data to the buffer
*/
static ssize_t driver_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs){
    int to_copy, not_copied, delta;

    /* Get amount of data to copy */
    to_copy = min(count, sizeof(buffer));

    /* Copy data to user */
    not_copied = copy_from_user(buffer, user_buffer, to_copy);
    buffer_pointer = to_copy;

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
    return 0;
}

/*
 * @brief Unregister device number 
 */
void unregister_device(void) {
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

