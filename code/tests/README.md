# How to run tests
To run this test, enter the following in terminal inside a test directory.

- Compile the code to generate IR `output.ll`

    `../../mccomp ./tests/addition/addition.c`

- Compile driver code `driver.cpp` and IR `output.ll` together to generate executable `add`

    `clang++ driver.cpp output.ll -o add`

- Run the executable `add` to see test result

    `./add`

