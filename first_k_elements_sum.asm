; Sum of first k elements of an array

ldc 0x1000
a2sp

; k = 3
ldc 3
stl 0          ; SP[0] = k

; pointer = array
ldc array
stl 1          ; SP[1] = pointer

; sum = 0
ldc 0
stl 2          ; SP[2] = sum

loop:
ldl 0
brz done       ; if k == 0 stop

; load array element
ldl 1
ldnl 0
ldl 2
add
stl 2          ; sum += array[i]

; pointer++
ldl 1
adc 1
stl 1

; k--
ldl 0
adc -1
stl 0

br loop

done:
HALT


array:
data 54
data 12
data 89
data 3
data 22