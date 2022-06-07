## MD5
### Usage
g++ sorgente -o nome_output
./nome_output


### Paddings
There are three types of padding (the index can be 0,1 or 2 and it is the parameter of the initializer of the MD5 class).
- 0: complete padding (0x80, zeros and length)
- 1: only 0x80 and zeros (at least one byte of padding is added)
- 2: simple padding: only 0x80 and zeros, but it is not required if the message length is a multiple of 64 bytes (512 bits)

### Examples
In the > main function there is an example of the usage. 

### Remark
update(msg1);
update(msg2);

is the same of 

update(msg1 + msg2);

meaning that two updates give the same result of a single update with the message composed by the concatenation of the messages.
