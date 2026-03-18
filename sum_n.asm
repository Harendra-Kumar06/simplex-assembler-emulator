; Program: Sum of first n natural numbers

ldc 0x1000
a2sp

; n = 5
ldc 5
stl 0        ; SP[0] = n

; i = 1
ldc 1
stl 1        ; SP[1] = i

; sum = 0
ldc 0
stl 2        ; SP[2] = sum

loop:

; if i > n stop
ldl 1
ldl 0
sub
brlz addnum
br done

addnum:

; sum += i
ldl 2
ldl 1
add
stl 2

; i++
ldl 1
adc 1
stl 1

br loop

done:
HALT