BUILD_DIR ?= build
BUILD_TYPE ?= Release
QT_PREFIX ?= $(shell brew --prefix qt 2>/dev/null)
CMAKE_PREFIX_PATH ?= $(QT_PREFIX)

.PHONY: all configure cmake build test run clean distclean help

all: build

configure cmake:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DCMAKE_PREFIX_PATH="$(CMAKE_PREFIX_PATH)"

build: configure
	cmake --build $(BUILD_DIR)

test: build
	./$(BUILD_DIR)/gomoku_tests

run: build
	./$(BUILD_DIR)/gomoku

clean:
	cmake --build $(BUILD_DIR) --target clean

distclean:
	rm -rf $(BUILD_DIR)

help:
	@echo "Targets:"
	@echo "  make cmake       Configure CMake into $(BUILD_DIR)"
	@echo "  make build       Configure and build"
	@echo "  make test        Build and run gomoku_tests"
	@echo "  make run         Build and run the Qt game"
	@echo "  make clean       Clean build artifacts inside $(BUILD_DIR)"
	@echo "  make distclean   Remove $(BUILD_DIR)"
	@echo ""
	@echo "Variables:"
	@echo "  BUILD_DIR=build"
	@echo "  BUILD_TYPE=Release"
	@echo "  QT_PREFIX=$$(brew --prefix qt)"
