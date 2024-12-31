# Variables
TARGET = tablet_mode
SRC = tablet_mode.c
INSTALL_PATH = /usr/local/bin
SERVICE_PATH = /etc/systemd/system
SERVICE_NAME = tablet-mode.service
SERVICE_FILE = $(SERVICE_PATH)/$(SERVICE_NAME)

# Default target: build, deploy, and start the service
.PHONY: all
all: deploy start

# Compile the binary
.PHONY: build
build:
	gcc -o $(TARGET) $(SRC) -lsystemd
	@echo "Build completed."

# Deploy the binary and service
.PHONY: deploy
deploy: build
	@if systemctl is-active --quiet $(SERVICE_NAME); then \
		echo "Stopping existing service..."; \
		sudo systemctl stop $(SERVICE_NAME); \
	fi
	@echo "Copying binary to $(INSTALL_PATH)..."
	sudo cp $(TARGET) $(INSTALL_PATH)/$(TARGET)
	@echo "Deploying service file..."
	@echo "[Unit]\nDescription=Custom Tablet Mode Switch\nAfter=multi-user.target\n\n[Service]\nExecStart=$(INSTALL_PATH)/$(TARGET)\nRestart=always\nUser=root\n\n[Install]\nWantedBy=multi-user.target" | sudo tee $(SERVICE_FILE) > /dev/null
	@echo "Reloading systemd daemon..."
	sudo systemctl daemon-reload
	@echo "Enabling service..."
	sudo systemctl enable $(SERVICE_NAME)
	@echo "Deployment completed."

# Start the service
.PHONY: start
start:
	@echo "Starting the service..."
	sudo systemctl start $(SERVICE_NAME)
	@echo "Service started."

# Stop the service
.PHONY: stop
stop:
	@echo "Stopping the service..."
	sudo systemctl stop $(SERVICE_NAME)
	@echo "Service stopped."

# Remove binary and service
.PHONY: clean
clean:
	@echo "Stopping service if active..."
	@if systemctl is-active --quiet $(SERVICE_NAME); then \
		sudo systemctl stop $(SERVICE_NAME); \
	fi
	@echo "Removing binary from $(INSTALL_PATH)..."
	sudo rm -f $(INSTALL_PATH)/$(TARGET)
	@echo "Removing service file..."
	sudo rm -f $(SERVICE_FILE)
	@echo "Reloading systemd daemon..."
	sudo systemctl daemon-reload
	@echo "Clean-up completed."

