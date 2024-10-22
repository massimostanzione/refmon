INSTALL_PATH = /usr/local/bin
EXEC_NAME = refmon

all:
	gcc -o $(EXEC_NAME) user*

test:
	gcc -o $(EXEC_NAME) -DTEST user*

install:
	sudo rm -f $(INSTALL_PATH)/$(EXEC_NAME)
	sudo ln -sr $(EXEC_NAME) $(INSTALL_PATH)/$(EXEC_NAME)
	sudo chmod +x $(INSTALL_PATH)/$(EXEC_NAME)

clean:
	sudo rm -f $(EXEC_NAME) $(INSTALL_PATH)/$(EXEC_NAME)
