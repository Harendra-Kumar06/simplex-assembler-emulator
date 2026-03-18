; GCD of two numbers using Euclidean Algorithm (Subtraction)
; regA: first number (n1)
; regB: second number (n2)

        ldc n1
        ldnl 0          ; Load n1 into A
        ldc n2
        ldnl 0          ; Load n2 into B (A=n2, B=n1)

loop:
        ; Check if A == B
        stl 0           ; Temporarily store A (n2) at [SP]
        ldl 0           ; A = n2, B = n1
        sub             ; A = n1 - n2
        brz done        ; If n1 - n2 == 0, then n1 == n2, GCD found

        ; Check if n1 < n2 (brlz uses regA)
        ; Currently A = n1 - n2
        brlz n2Greater ; If n1 - n2 < 0, then n2 is bigger

n1Greater:
        ; n1 = n1 - n2
        ldc n1
        ldnl 0          ; A = n1
        ldc n2
        ldnl 0          ; A = n2, B = n1
        sub             ; A = n1 - n2
        ldc n1
        stnl 0          ; Store result back in n1
        
        ; Reload for next iteration
        ldc n1
        ldnl 0
        ldc n2
        ldnl 0          ; A = n2, B = n1
        br loop

n2Greater:
        ; n2 = n2 - n1
        ldc n2
        ldnl 0          ; A = n2
        ldc n1
        ldnl 0          ; A = n1, B = n2
        sub             ; A = n2 - n1
        ldc n2
        stnl 0          ; Store result back in n2
        
        ; Reload for next iteration
        ldc n1
        ldnl 0
        ldc n2
        ldnl 0          ; A = n2, B = n1
        br loop

done:
        ldc n1
        ldnl 0          ; Load the result (GCD) into A
        ldc result
        stnl 0          ; Store in result memory
        HALT

; Data Section
n1:     data 24         ; First number
n2:     data 18         ; Second number
result: data 0          ; Expected GCD: 6 (0x6)