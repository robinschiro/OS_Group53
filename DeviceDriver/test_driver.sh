#!/bin/bash

# Set shell attribute the exits when a command returns non-zero result.
set -e

# Source the functions of the assertion library.
source assert.sh

# Set constants
DEVICE_FILE_PATH=/dev/SampleCharDevice

echo "Removing the previous versions of module and device file..."
# Continue script even if the following two commands produce an error.
rmmod main 2>/dev/null || true
rm ${DEVICE_FILE_PATH} 2>/dev/null || true

echo "Building character device driver..."
make

echo "Installing the module..."
insmod main.ko

# Retrieve the major version of the module from the last line of the kernel module log file.
MAJOR_VERSION=$(dmesg | tail -1 | awk '{ print $NF }')

echo "Creating the device file..."
mknod ${DEVICE_FILE_PATH} c ${MAJOR_VERSION} 0

printf "\nTest Results:\n"

# Test 1: Basic Functionality
# Push some data into the character device buffer.
# Verify that popping each piece of data works correctly.
echo -n "onetwothree" > ${DEVICE_FILE_PATH}
assert "head -c 3 ${DEVICE_FILE_PATH}" "one"
assert "head -c 3 ${DEVICE_FILE_PATH}" "two"
assert "head -c 5 ${DEVICE_FILE_PATH}" "three"
echo -n "five" > ${DEVICE_FILE_PATH}
echo -n "six" > ${DEVICE_FILE_PATH}
echo -n "seven" > ${DEVICE_FILE_PATH}
assert "head -c 4 ${DEVICE_FILE_PATH}" "five"
assert "head -c 3 ${DEVICE_FILE_PATH}" "six"
assert "head -c 5 ${DEVICE_FILE_PATH}" "seven"
assert_end basic_functionality

# Test 2: Read Overflow
# Push some data into the character device buffer
# and attempt to read more bytes than are in the buffer.
# The read result should simply return the entire contents of the buffer.
echo -n "The quick brown fox jumps over the lazy dog" > ${DEVICE_FILE_PATH}
assert "head -c 500 ${DEVICE_FILE_PATH}" "The quick brown fox jumps over the lazy dog"
assert "head -c 100 ${DEVICE_FILE_PATH}" ""
assert_end read_overflow

# Test 3: Write Overflow
# Push an amount of data into the buffer that exceeds the maximum size (1024)
# and then read the buffer contents.
# Verify that only the first 1024 bytes of the input was written to the device.
SAMPLE_TEXT_INPUT="Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec cursus euismod ligula efficitur faucibus. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Quisque molestie libero interdum auctor condimentum. Nullam non enim libero. Fusce fermentum lacus ex, non vehicula urna laoreet ut. Aenean at velit odio. Donec blandit imperdiet nunc, et molestie mi tempor at. Mauris dapibus leo augue. In ex ex, interdum ornare auctor sit amet, rhoncus ut ipsum. Integer dictum est non ornare scelerisque. Duis faucibus nisi accumsan, ullamcorper risus et, tincidunt dolor. Fusce laoreet ex purus, eu mattis urna pharetra at. Aliquam nec fermentum eros. Vivamus ac sapien eu metus euismod varius vel sit amet mi. Donec consectetur, tortor quis tincidunt fringilla, ligula ante consectetur massa, eget semper nulla tellus sit amet sapien. Mauris eget risus laoreet lorem volutpat varius eu lacinia ante. Sed enim dolor, blandit sed pharetra et, hendrerit ac tellus. Nam sed sapien eget lectus condimentum semper eget non purus."
SAMPLE_TEXT_OUPUT="Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec cursus euismod ligula efficitur faucibus. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Quisque molestie libero interdum auctor condimentum. Nullam non enim libero. Fusce fermentum lacus ex, non vehicula urna laoreet ut. Aenean at velit odio. Donec blandit imperdiet nunc, et molestie mi tempor at. Mauris dapibus leo augue. In ex ex, interdum ornare auctor sit amet, rhoncus ut ipsum. Integer dictum est non ornare scelerisque. Duis faucibus nisi accumsan, ullamcorper risus et, tincidunt dolor. Fusce laoreet ex purus, eu mattis urna pharetra at. Aliquam nec fermentum eros. Vivamus ac sapien eu metus euismod varius vel sit amet mi. Donec consectetur, tortor quis tincidunt fringilla, ligula ante consectetur massa, eget semper nulla tellus sit amet sapien. Mauris eget risus laoreet lorem volutpat varius eu lacinia ante. Sed enim dolor, blandit sed pharetra et, hendrerit ac tellus. Nam sed sapien eget lectu"
echo -n ${SAMPLE_TEXT_INPUT} > ${DEVICE_FILE_PATH}
assert "cat ${DEVICE_FILE_PATH}" "${SAMPLE_TEXT_OUPUT}"
assert_end write_overflow

