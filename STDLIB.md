# STDLIB.uasm

## Macros

### General

**SETBZ**

Set B to 0

**SETABZ**

Set A and B to 0

**SETB1**

Set B to 1

**CALL [u64: address]**

Add PC to call stack and return once subroutine is finished

**FJMP [u64: address]**

Set B to 0 and jump to address

### 64 Bit

**LDA64 [u64: address]**

Load 8 bytes of data from address into A using big endian

**STA64 [u64: address]**

Store 8 bytes of data from A into address using big endian

**LDB64 [u64: address]**

Load 8 bytes of data from address into B using big endian

**STB64 [u64: address]**

Store 8 bytes of data from B into address using big endian

**LDA64R [u64: address]**

Load 8 bytes of data from address into A using little endian

**STA64R [u64: address]**

Store 8 bytes of data from A into address using little endian

**LDB64R [u64: address]**

Load 8 bytes of data from address into B using little endian

**STB64R [u64: address]**

Store 8 bytes of data from B into address using little endian

### 32 Bit

### 16 Bit

**LDA16 [u64: address]**

Load 2 bytes of data from address into A using big endian

**STA16 [u64: address]**

Store 2 bytes of data from A into address using big endian

**LDB16 [u64: address]**

Load 2 bytes of data from address into B using big endian

**STB16 [u64: address]**

Store 2 bytes of data from B into address using big endian

### 8 Bit

**LDB [u8: offset] [u64: address]**

Load address into B

**STB [u64: address] [u8: offset]**

Store B into address
