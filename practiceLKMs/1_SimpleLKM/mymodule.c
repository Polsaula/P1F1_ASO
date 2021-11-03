#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>


/*META INFORMATION*/
MODULE_AUTHOR("Pol Saula Lloveras");
MODULE_DESCRIPTION("A hello world LKM");

/*
 * @brief This function is called when the module is loaded into de kernel
 */
static int ModuleInit(void){
	printk("Hello Kernel\n");
	return 0;
}

/*
 * @brief This function is called when the module is removed from the kernel
 */
static void ModuleExit(void){
	printk("Goodbye kernel\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);

