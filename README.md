# SIMPLEX Two-Pass Assembler & Emulator

## Overview

This project implements a **Two-Pass Assembler** and a **Machine Emulator** for a custom SIMPLEX instruction set architecture.

Developed as part of the **CS2206 Computer Architecture course (IIT Patna)**, the project demonstrates low-level system design including instruction encoding, symbol resolution, and execution simulation.

---

## Features

### Two-Pass Assembler (`asm.cpp`)

* Implements **two-pass assembly process**:

  * **Pass 1**: Syntax analysis, comment removal, label validation, and symbol table creation
  * **Pass 2**: Operand resolution, PC-relative addressing, and machine code generation
* Supports:

  * Labels & forward references
  * Decimal, Hex (`0x`), and Octal numbers
  * PC-relative branching (`call`, `br`, `brz`, `brlz`)
* Error Detection:

  * Duplicate labels
  * Invalid label syntax
  * Undefined symbols
  * Invalid mnemonics
  * Missing / extra operands
* Output Files:

  * `.o` → Object file (machine code)
  * `.lst` → Listing file (address + instruction + hex code)
  * `.log` → Error / warning logs

---

### Emulator (`emu.cpp`)

* Simulates SIMPLEX machine execution
* Architecture:

  * Registers: `A`, `B`, `PC`, `SP`
  * Memory: 10,000-word memory model
* Features:

  * Instruction execution engine
  * Execution trace logging (`-t`)
  * Memory dump before (`-b`) and after (`-a`) execution
  * Instruction set display (`-ISA`)
* Safety:

  * Stack overflow detection
  * Infinite loop detection (branch-to-self)
  * Invalid opcode handling

---

## Instruction Set (SIMPLEX ISA)

The SIMPLEX machine follows a **stack-based architecture** with two primary registers (`A`, `B`) and control registers (`PC`, `SP`). Each instruction consists of:

* **8-bit Opcode**
* **24-bit Operand (signed)**

---

### Data & Constant Instructions

| Instruction  | Description                                |
| ------------ | ------------------------------------------ |
| `ldc value`  | Load constant into accumulator (A)         |
| `adc value`  | Add constant to accumulator                |
| `data value` | Reserve memory initialized with value      |
| `SET value`  | Assign value to label (pseudo-instruction) |

---

### Memory Instructions

| Instruction   | Description                             |
| ------------- | --------------------------------------- |
| `ldl offset`  | Load from local memory (SP + offset)    |
| `stl offset`  | Store to local memory (SP + offset)     |
| `ldnl offset` | Load from non-local memory (A + offset) |
| `stnl offset` | Store to non-local memory (A + offset)  |

---

### Arithmetic Instructions

| Instruction | Description |
| ----------- | ----------- |
| `add`       | A = B + A   |
| `sub`       | A = B - A   |

---

### Bitwise Instructions

| Instruction | Description          |
| ----------- | -------------------- |
| `shl`       | Shift left (B << A)  |
| `shr`       | Shift right (B >> A) |

---

### Stack & Register Operations

| Instruction | Description                        |
| ----------- | ---------------------------------- |
| `adj value` | Adjust stack pointer (SP += value) |
| `a2sp`      | Transfer A → SP                    |
| `sp2a`      | Transfer SP → A                    |

---

### Control Flow Instructions

| Instruction   | Description                       |
| ------------- | --------------------------------- |
| `call offset` | Call procedure (PC-relative jump) |
| `return`      | Return from procedure             |
| `br offset`   | Unconditional branch              |
| `brz offset`  | Branch if A == 0                  |
| `brlz offset` | Branch if A < 0                   |

---

### System Instruction

| Instruction | Description            |
| ----------- | ---------------------- |
| `HALT`      | Stop program execution |

---

### Notes

* Branch instructions use **PC-relative addressing**
* Labels are resolved during assembly
* Emulator automatically handles instruction execution and PC updates




---

## Author

**Harendra Kumar**
B.Tech CSE, IIT Patna

