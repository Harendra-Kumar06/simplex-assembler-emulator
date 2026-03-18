; bubble.asm - Bubble Sort in SIMPLEX
; Initialize Stack Pointer
ldc 0x1000
a2sp

; Set up loop counters
ldc 5       ; N = 5 (Array size)
stl 0       ; SP[0] = Outer loop counter (i)

outerLoop:
ldl 0
adc -1
stl 0       ; i = i - 1
ldl 0       ; FIX: Reload 'i' into A for the branch check!
brz done    ; if i == 0, sorting is done

ldl 0
stl 1       ; SP[1] = Inner loop counter (j)
ldc array   ; Base address of array
stl 2       ; SP[2] = Current element pointer

innerLoop:
; LOAD array[j] into SP[3]
ldl 2       ; A = address
ldnl 0      ; A = memory[address]
stl 3       ; SP[3] = array[j]

; LOAD array[j+1] into SP[4]
ldl 2
adc 1       ; A = address + 1
ldnl 0      ; A = memory[address+1]
stl 4       ; SP[4] = array[j+1]

; COMPARE array[j] and array[j+1]
ldl 4       ; A = array[j+1]
ldl 3       ; B = array[j+1], A = array[j]
sub         ; A = B - A  ==> array[j+1] - array[j]
brlz swap   ; If array[j+1] - array[j] < 0, do swap!
br noSwap

swap:
; memory[address] = array[j+1]
ldl 4       ; A = array[j+1]
ldl 2       ; B = array[j+1], A = address
stnl 0      ; memory[address] = B

; memory[address+1] = array[j]
ldl 3       ; A = array[j]
ldl 2       ; B = array[j], A = address
adc 1       ; A = address + 1
stnl 0      ; memory[address+1] = B

noSwap:
ldl 2
adc 1
stl 2       ; Move pointer to next element

ldl 1
adc -1
stl 1       ; j = j - 1
ldl 1       ; FIX: Reload 'j' into A for the branch check!
brz outerLoop ; if j == 0, go to next outer pass
br innerLoop

done:
HALT

array: 
data 54    ; 0x36
data 12    ; 0x0C
data 89    ; 0x59
data 3     ; 0x03
data 22    ; 0x16