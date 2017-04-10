
all:
	@echo "\033[1;33m>>> Building DRIVER\033[0m"
	cd driver; make -C $(PWD)/driver
	@echo "\033[1;33m>>> Building API library\033[0m"
	cd api; make -C $(PWD)/api
	@echo "\033[1;33m>>> Building TEST\033[0m"
	cd test; make -C $(PWD)/test

clean:
	@echo "\033[1;33m>>> Cleaning DRIVER directory\033[0m"
	cd driver; make -C $(PWD)/driver clean
	@echo "\033[1;33m>>> Cleaning API directory\033[0m"
	cd api; make -C $(PWD)/api clean
	@echo "\033[1;33m>>> Cleaning TEST directory\033[0m"
	cd test; make -C $(PWD)/test clean