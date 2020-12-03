1. Merge kernel_gpio_key_patch to your kernel.
	#cd kernel
	#make

2. Build gpio key driver:
	-put gpio_key_driver and kernel in same path.
	-If you want to change GPIO number, modify gpio_key_driver.c. In current sample, it uses PM_GPIO4.
	-make driver in gpio_key_driver folder.It generate gpio_key_driver.ko
		#cd gpio_key_driver
		#make

3. copy gpio_ker_driver to your target board and insert driver.
	/customer#insert gpio_key_driver.ko
   After driver inserted, event0 will show in /dev/input folder in your taget board.

4. Press key button on target board, you will see some log as bellow.
	"gpio_keys_gpio_report_event: code = 7 state=1	"
	"gpio_keys_gpio_report_event: code = 7 state=0  "

5. Use key_test.c in user space to monitor key event.


    