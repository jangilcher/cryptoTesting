# Dependencies

```bash
sudo apt install libssl-dev
sudo apt install libgmp-dev
sudo apt install libcrypto++-dev
```

Then, install libkeccak https://github.com/maandree/libkeccak
```bash
git clone https://github.com/maandree/libkeccak
cd libkeccak
make
sudo make install
```

# Running

Before running for the first time you have to ensure that supercop is initialized.
This can be done by running
```bash
./data-init
``` 
in the supercop directory.

The path to the supercop directory is read from the SUPERDIR environment variable.
To run tests on supercop on inputs of length x, x+1, ..., y:

```bash
make clean libs
bash export SUPERDIR=/path/to/SUPERCOP/; supercop.sh -l x -u y
```

By default the script will run on N-1 CPU cores, where N is the total number of cores on the device.
