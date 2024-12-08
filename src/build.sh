#!/bin/bash

# Check if a filename was provided as an argument
if [ $# -eq 0 ]; then
  echo "Usage: $0 <filename>"
  exit 1
fi

# Get the filename from the argument
FILENAME=$1

# Check if the file exists
if [ ! -f "$FILENAME" ]; then
  echo "Error: File '$FILENAME' not found."
  exit 1
fi

# Extract the base name without the extension
BASENAME=$(basename "$FILENAME" .c)

# Compile the file
gcc -Wall -fsanitize=address,undefined -o "$BASENAME" "$FILENAME"

# Check if the compilation was successful
if [ $? -eq 0 ]; then
  echo "Compilation successful. Executable created: $BASENAME"
else
  echo "Compilation failed."
fi
