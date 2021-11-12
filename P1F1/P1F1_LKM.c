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
static unsigned int gpioButton1 = 26;  //Power on LED1 (pin 20)       
static unsigned int gpioButton2 = 19;  //Power off LED1 (pin 20)  
static unsigned int gpioButton3 = 13;  //Power on LED2 (pin 16)      
static unsigned int gpioButton4 = 21;  //Power off LED2 (pin 16)     

/* IRQ number for each button */
static unsigned int irqNumber1;              
static unsigned int irqNumber2;              
static unsigned int irqNumber3;              
static unsigned int irqNumber4;           

static unsigned int numberPresses1 = 0;      //Number of button1 presses
static unsigned int numberPresses2 = 0;      //Number of button2 presses
static unsigned int numberPresses3 = 0;      //Number of button3 presses
static unsigned int numberPresses4 = 0;      //Number of button4 presses

/// Function prototype for the custom IRQ handler function
static irq_handler_t  ebbgpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);

/**
* @brief The LKM initialization function
* @return returns 0 if successful
**/
static int __init ebbgpio_init(void){
   int result1 = 0;
   int result2 = 0;
   int result3 = 0;
   int result4 = 0;
   printk(KERN_INFO "ASO: Initializing the GPIO_TEST LKM\n");

   /* Check for unavailable pins for the GPIO (leds and buttons)*/
   if (!gpio_is_valid(gpioLED1) || !gpio_is_valid(gpioLED2)){
      printk(KERN_INFO "ASO: invalid LED GPIO\n");
      return -ENODEV;
   }
   if (!gpio_is_valid(gpioButton1) || !gpio_is_valid(gpioButton2) || !gpio_is_valid(gpioButton3) || !gpio_is_valid(gpioButton4)){
      printk(KERN_INFO "ASO: invalid button GPIO\n");
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
   gpio_set_debounce(gpioButton1, 200);
   gpio_export(gpioButton1, false);  

   gpio_request(gpioButton2, "sysfs");       
   gpio_direction_input(gpioButton2);      
   gpio_set_debounce(gpioButton2, 200);  
   gpio_export(gpioButton2, false);  

   gpio_request(gpioButton3, "sysfs");       
   gpio_direction_input(gpioButton3);
   gpio_set_debounce(gpioButton3, 200);
   gpio_export(gpioButton3, false);  

   gpio_request(gpioButton4, "sysfs");       
   gpio_direction_input(gpioButton4);
   gpio_set_debounce(gpioButton4, 200);        
   gpio_export(gpioButton4, false);       
			                                
   /* Mapping each GPIO button's number to a different IRQ number */
   irqNumber1 = gpio_to_irq(gpioButton1);
   printk(KERN_INFO "ASO: The button1 is mapped to IRQ: %d\n", irqNumber1);
   irqNumber2 = gpio_to_irq(gpioButton2);
   printk(KERN_INFO "ASO: The button2 is mapped to IRQ: %d\n", irqNumber2);
   irqNumber3 = gpio_to_irq(gpioButton3);
   printk(KERN_INFO "ASO: The button3 is mapped to IRQ: %d\n", irqNumber3);
   irqNumber4 = gpio_to_irq(gpioButton4);
   printk(KERN_INFO "ASO: The button4 is mapped to IRQ: %d\n", irqNumber4);

   // This next call requests an interrupt line for irqNumber1 (button 1)
   result1 = request_irq(irqNumber1, (irq_handler_t) ebbgpio_irq_handler, IRQF_TRIGGER_RISING, "ebb_gpio_handler1", NULL);

   // This next call requests an interrupt line for irqNumber2 (button 2)
   result2 = request_irq(irqNumber2, (irq_handler_t) ebbgpio_irq_handler, IRQF_TRIGGER_RISING, "ebb_gpio_handler2", NULL);  

   // This next call requests an interrupt line for irqNumber3 (button 3)
   result3 = request_irq(irqNumber3, (irq_handler_t) ebbgpio_irq_handler, IRQF_TRIGGER_RISING, "ebb_gpio_handler3", NULL);  
   
   // This next call requests an interrupt line for irqNumber4 (button 4)
   result4 = request_irq(irqNumber4, (irq_handler_t) ebbgpio_irq_handler, IRQF_TRIGGER_RISING, "ebb_gpio_handler4", NULL);                 

   return 0;
}

/** 
* @brief The LKM cleanup function
**/
static void __exit ebbgpio_exit(void){
   printk(KERN_INFO "ASO: Button 1 was pressed %d times!", numberPresses1);
   printk(KERN_INFO "ASO: Button 2 was pressed %d times!", numberPresses2);
   printk(KERN_INFO "ASO: Button 3 was pressed %d times!", numberPresses3);
   printk(KERN_INFO "ASO: Button 4 was pressed %d times!", numberPresses4);


   gpio_set_value(gpioLED1, 0);              // Turn the LED off, makes it clear the device was unloaded
   gpio_unexport(gpioLED1);                  // Unexport the LED GPIO
   gpio_free(gpioLED1);                      // Free the LED GPIO
   gpio_set_value(gpioLED2, 0);              // Turn the LED off, makes it clear the device was unloaded
   gpio_unexport(gpioLED2);                  // Unexport the LED GPIO
   gpio_free(gpioLED2);                      // Free the LED GPIO

   free_irq(irqNumber1, NULL);               // Free the IRQ1, no *dev_id required in this case
   free_irq(irqNumber2, NULL);               // Free the IRQ2, no *dev_id required in this case
   free_irq(irqNumber3, NULL);               // Free the IRQ3, no *dev_id required in this case
   free_irq(irqNumber4, NULL);               // Free the IRQ4, no *dev_id required in this case

   gpio_unexport(gpioButton1);                // Unexport the Button1 GPIO
   gpio_free(gpioButton1);                    // Free the Button1 GPIO
   gpio_unexport(gpioButton2);                // Unexport the Button2 GPIO
   gpio_free(gpioButton2);                    // Free the Button2 GPIO
   gpio_unexport(gpioButton3);                // Unexport the Button3 GPIO
   gpio_free(gpioButton3);                    // Free the Button3 GPIO
   gpio_unexport(gpioButton4);                // Unexport the Button4 GPIO
   gpio_free(gpioButton4);                    // Free the Button4 GPIO

   printk(KERN_INFO "ASO: Goodbye from the LKM!\n");
}

/**
*@brief The GPIO IRQ Handler function
**/
static irq_handler_t ebbgpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs){
   int err = 0;
   char *argv1[] = { "/home/pi/P1F1-ASO-LKM/P1F1/buttonScripts/button1Script.sh", NULL };
   char *argv2[] = { "/home/pi/P1F1-ASO-LKM/P1F1/buttonScripts/button2Script.sh", NULL };
   char *argv3[] = { "/home/pi/P1F1-ASO-LKM/P1F1/buttonScripts/button3Script.sh", NULL };
   char *argv4[] = { "/home/pi/P1F1-ASO-LKM/P1F1/buttonScripts/button4Script.sh", NULL };
   static char *envp[] = {"HOME=/", "TERM=linux", "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };

   if(irq == irqNumber1){
      gpio_set_value(gpioLED1, 1);  //Power on LED1 (pin 20)   

      err = call_usermodehelper( argv1[0], argv1, envp, UMH_NO_WAIT );

      printk(KERN_INFO "ASO: Interrupt! (button1 state is %d)\n", gpio_get_value(gpioButton1));
      numberPresses1++;
   }else if(irq == irqNumber2){
      gpio_set_value(gpioLED1, 0);  //Power off LED1 (pin 20)   

      err = call_usermodehelper( argv2[0], argv2, envp, UMH_NO_WAIT );

      printk(KERN_INFO "ASO: Interrupt! (button2 state is %d)\n", gpio_get_value(gpioButton2));
      numberPresses2++;
   }else if(irq == irqNumber3){
      gpio_set_value(gpioLED2, 1);  //Power on LED2 (pin 16)   

      err = call_usermodehelper( argv3[0], argv3, envp, UMH_NO_WAIT );

      printk(KERN_INFO "ASO: Interrupt! (button3 state is %d)\n", gpio_get_value(gpioButton3));
      numberPresses3++;
   }else if(irq == irqNumber4){
      gpio_set_value(gpioLED2, 0);  //Power off LED2 (pin 16)

      err = call_usermodehelper( argv4[0], argv4, envp, UMH_NO_WAIT );

      printk(KERN_INFO "ASO: Interrupt! (button4 state is %d)\n", gpio_get_value(gpioButton4));
      numberPresses4++;
   }
   
   if(err != 0){
      printk("ASO: ERROR on exec: %d", err);
   }
   return (irq_handler_t) IRQ_HANDLED;      // Announce that the IRQ has been handled correctly
}

module_init(ebbgpio_init);
module_exit(ebbgpio_exit);