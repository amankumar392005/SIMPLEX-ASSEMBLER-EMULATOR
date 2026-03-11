# SIMPLEX-ASSEMBLER-EMULATOR

## Introduction

This project implements a Two-Pass Assembler and Emulator for the SIMPLEX Instruction Set Architecture using C++.
The main objective of this project is to understand how assembly programs are translated into machine code and how a processor executes these instructions.

The assembler reads a SIMPLEX assembly program, processes labels and instructions, detects errors, and generates a binary object file along with a listing file. The emulator then loads the object file and simulates the execution of the program on a SIMPLEX machine.

This project was developed as part of the CS2206 Systems Programming Mini Project.

---

## Features

* Implementation of a Two-Pass Assembler
* Symbol table for label definition and resolution
* Detection of common assembly errors:

  * duplicate label definitions
  * undefined labels
  * invalid mnemonics
  * incorrect operands
* Generation of:

  * Binary object file
  * Human-readable listing file
* Implementation of a SIMPLEX machine emulator
* Test assembly programs for validation
* Bubble sort implementation in SIMPLEX assembly

---

## SIMPLEX Machine Architecture

The SIMPLEX machine uses four 32-bit registers:

* A – accumulator
* B – secondary register
* PC – program counter
* SP – stack pointer

Each instruction consists of:

* 8-bit opcode
* 24-bit signed operand

Instructions may have either no operand or a single operand.

---


## Project Structure

simplex-assembler-emulator-cpp
│
├── assembler/       -> Source code for the two-pass assembler
├── emulator/        -> Source code for the SIMPLEX emulator
├── programs/        -> Sample assembly programs
│   ├── sample.asm   -> Example SIMPLEX assembly program
│   └── bubblesort.asm -> Bubble sort implementation in SIMPLEX assembly
├── output/          -> Generated object and listing files
├── docs/            -> Project documentation
└── README.md        -> Project description


---

## Compilation

Compile the assembler and emulator using g++.

g++ Assembler.cpp -o assembler
g++ Emulator.cpp -o emulator

---

## Running the Assembler

./assembler sample.asm

This command generates the following files:

sample.obj   -> binary object file
sample.lst   -> assembler listing file

---

## Running the Emulator

./emulator sample.obj

The emulator loads the object file into memory and executes instructions sequentially until the HALT instruction is encountered.

---

## Learning Outcomes

Through this project I gained practical understanding of:

* assembler design and implementation
* symbol tables and label resolution
* instruction encoding and machine code generation
* low-level program execution using an emulator
* systems programming concepts

---

## Author

Aman Kumar
Second Year Undergraduate
Indian Institute of Technology Patna

