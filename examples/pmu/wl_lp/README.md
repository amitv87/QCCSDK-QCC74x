## Test mode and testing methods：

### Deep sleep ：

1.Reset board 2.Enter the command "tickless" in the command line.

### DTIM:

1.Reset board

2.Connect wifi, and got ip.

DTIM = 1: Enter the command "tickless 1" in the command line.

DTIM = 3: Enter the command "tickless 3" in the command line.

DTIM = 10: Enter the command "tickless 10" in the command line.

### Benchmark：

| Mode       | uA  |
| :--------- | :-- |
| Deep sleep | 67  |
| DTIM = 1   | 810 |
| DTIM = 3   | 319 |
| DTIM = 10  | 154 |

