#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>                 // Required for the GPIO functions
#include <linux/interrupt.h>            // Required for the IRQ code

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pol Saula Lloveras");
MODULE_DESCRIPTION("Driver capable of switching on/off 2 LEDs with 4 buttons");

/* Hard coding the LEDs gpio */
static unsigned int gpioLED1 = 20;       
static unsigned int gpioLED2 = 16; 

/* Hard coding the buttons gpio */ 
static unsigned int gpioButton1 = 26;       
static unsigned int gpioButton2 = 19;       
static unsigned int gpioButton3 = 13;       
static unsigned int gpioButton4 = 21;       

/* IRQ number for each button */
static unsigned int irqNumber1;              
static unsigned int irqNumber2;              
static unsigned int irqNumber3;              
static unsigned int irqNumber4;           

static unsigned int numberPresses = 0;      //Number of button presses

/// Function prototype for the custom IRQ handler function
static irq_handler_t  ebbgpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);

/**
* @brief The LKM initialization function
* @return returns 0 if successful
**/
static int __init ebbgpio_init(void){
   int result = 0;
   printk(KERN_INFO "GPIO_TEST: Initializing the GPIO_TEST LKM\n");

   /* Check for unavailable pins for the GPIO (leds and buttons)*/
   if (!gpio_is_valid(gpioLED1) || !gpio_is_valid(gpioLED2)){
      printk(KERN_INFO "GPIO_TEST: invalid LED GPIO\n");
      return -ENODEV;
   }
   if (!gpio_is_valid(gpioButton1) || !gpio_is_valid(gpioButton2) || !gpio_is_valid(gpioButton3) || !gpio_is_valid(gpioButton4)){
      printk(KERN_INFO "GPIO_TEST: invalid button GPIO\n");
      return -ENODEV;
   }

   /* Setting up the LEDs as outputs by default (direction can't be changed)*/
   gpio_request(gpioLED1, "sysfs");          
   gpio_direction_output(gpioLED1, 0);   
   gpio_export(gpioLED1, false);  

   gpio_request(gpioLED2, "sysfs");          
   gpio_direction_output(gpioLED2, 0);   
   gpio_export(gpioLED2, false);      
   
   /* Setting up the buttons as inputs by default (direction can't be changed) */
   gpio_request(gpioButton1, "sysfs");       
   gpio_direction_input(gpioButton1);        
   gpio_set_debounce(gpioButton1, 200);      // Debounce the button with a delay of 200ms
   gpio_export(gpioButton1, false);  

   gpio_request(gpioButton2, "sysfs");       
   gpio_direction_input(gpioButton2);        
   gpio_set_debounce(gpioButton2, 200);      // Debounce the button with a delay of 200ms
   gpio_export(gpioButton2, false);  

   gpio_request(gpioButton3, "sysfs");       
   gpio_direction_input(gpioButton3);        
   gpio_set_debounce(gpioButton3, 200);      // Debounce the button with a delay of 200ms
   gpio_export(gpioButton3, false);  

   gpio_request(gpioButton4, "sysfs");       
   gpio_direction_input(gpioButton4);        
   gpio_set_debounce(gpioButton4, 200);      // Debounce the button with a delay of 200ms
   gpio_export(gpioButton4, false);       
			                                
   /* Mapping each GPIO button's number to a different IRQ number */
   irqNumber1 = gpio_to_irq(gpioButton1);
   printk(KERN_INFO "GPIO_TEST: The button1 is mapped to IRQ: %d\n", irqNumber1);
   irqNumber2 = gpio_to_irq(gpioButton2);
   printk(KERN_INFO "GPIO_TEST: The button2 is mapped to IRQ: %d\n", irqNumber2);
   irqNumber3 = gpio_to_irq(gpioButton3);
   printk(KERN_INFO "GPIO_TEST: The button3 is mapped to IRQ: %d\n", irqNumber3);
   irqNumber4 = gpio_to_irq(gpioButton4);
   printk(KERN_INFO "GPIO_TEST: The button4 is mapped to IRQ: %d\n", irqNumber4);

   // This next call requests an interrupt line
   result = request_irq(irqNumber,             // The interrupt number requested
                        (irq_handler_t) ebbgpio_irq_handler, // The pointer to the handler function below
                        IRQF_TRIGGER_RISING,   // Interrupt on rising edge (button press, not release)
                        "ebb_gpio_handler",    // Used in /proc/interrupts to identify the owner
                        NULL);                 // The *dev_id for shared interrupt lines, NULL is okay

   printk(KERN_INFO "GPIO_TEST: The interrupt request result is: %d\n", result);
   return result;
}

/** 
* @brief The LKM cleanup function
**/
static void __exit ebbgpio_exit(void){
   printk(KERN_INFO "GPIO_TEST: The button state is currently: %d\n", gpio_get_value(gpioButton));
   printk(KERN_INFO "GPIO_TEST: The button was pressed %d times\n", numberPresses);
   gpio_set_value(gpioLED, 0);              // Turn the LED off, makes it clear the device was unloaded
   gpio_unexport(gpioLED);                  // Unexport the LED GPIO
   free_irq(irqNumber, NULL);               // Free the IRQ number, no *dev_id required in this case
   gpio_unexport(gpioButton);               // Unexport the Button GPIO
   gpio_free(gpioLED);                      // Free the LED GPIO
   gpio_free(gpioButton);                   // Free the Button GPIO
   printk(KERN_INFO "GPIO_TEST: Goodbye from the LKM!\n");
}

/** 
*@brief The GPIO IRQ Handler function
**/
static irq_handler_t ebbgpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs){
   ledOn = !ledOn;                          // Invert the LED state on each button press
   gpio_set_value(gpioLED, ledOn);          // Set the physical LED accordingly
   printk(KERN_INFO "GPIO_TEST: Interrupt! (button state is %d)\n", gpio_get_value(gpioButton));
   numberPresses++;                         // Global counter, will be outputted when the module is unloaded
   return (irq_handler_t) IRQ_HANDLED;      // Announce that the IRQ has been handled correctly
}

module_init(ebbgpio_init);
module_exit(ebbgpio_exit);