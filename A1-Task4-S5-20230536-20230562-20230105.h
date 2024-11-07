#ifndef VOLEMACHINE_H
#define VOLEMACHINE_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <bitset>
#include <cmath>
#include <bits/stdc++.h>

// Forward declarations
class Memory;
class Register;
class Alu;
class CU;
class CPU;
class Machine;
class MainUI;

// Register Class
class Register {
private:
    std::string registers[16];
    const int size = 16;

public:
    Register();
    std::string read(int address) const;
    void write(int address, std::string value);
    void display();
    void clear();
};

// ALU Class
class Alu {
public:
    bool isValid(std::string hexValue);
    int hexToDec(std::string hexValue);
    std::string decToHex(int decimalValue);
    void addIntegers(int idx1, int idx2, int resIdx, Register& reg);
    void addFloats(int idx1, int idx2, int resIdx, Register& reg);
    void execute(int type, int idx1, int idx2, int resIdx, Register& reg);

private:
    uint8_t float_to_custom_8bit(float num);
    std::string custom_8bit_to_hex(uint8_t customFloat);
};

// Memory Class
class Memory {
private:
    std::string memory[256];
    const int size = 256;

public:
    Memory();
    std::string read(int address) const;
    void write(int address, std::string value);
    void writeFromFile(const std::string& filename);
    void display() const;
    void clear();
    int getSize() const;
};

// Decoded Instruction Struct
struct DecodedInstruction {
    char opcode;
    int R, S, T;
    int address;
    int immediate;
};

// Control Unit Class
class CU {
public:
    CU();
    void load(int idxReg, int intMem, Register& reg, Memory& mem);
    void loadImmediate(int idxReg, int val, Register& reg);
    void store(int idxReg, int idxMem, Register& reg, Memory& mem);
    void move(int sourceReg, int destReg, Register& reg);
    void jump(int idxReg, int& PC, Register& reg);
    void halt();
    void addIntegers(int idx1, int idx2, int resIdx, Register& reg);
    void addFloats(int idx1, int idx2, int resIdx, Register& reg);

private:
    Alu alu;
};

// CPU Class
class CPU {
public:
    int programCounter;
    std::string instructionRegister;
    Register registers;
    Alu alu;
    CU controlUnit;

    void displayState(Memory& mem);

public:
    CPU(int startAddress = 0x0A);
    std::string fetch(int address, const Memory &memory);
    void runNextStep(Memory& mem);
    void setProgramCounter(int pc);
    int getProgramCounter() const;
    void setInstructionRegister(const std::string& instruction);
    std::string getInstructionRegister() const;
    std::string getRegister(int index) const;

private:
    DecodedInstruction decode();
    void execute(DecodedInstruction& instruction, Memory& mem);
    int hexToDec(const std::string& hexValue) const;
};

// Machine Class
class Machine {
private:
    CPU cpu;
    Memory memory;

public:
    Machine();
    bool loadFile(const std::string& filename);
    void printStateStepByStep();
    void printWholeState();
};

// MainUI Class
class MainUI {
private:
    Machine machine;

public:
    void displayMenu();
    void handleUserChoice(int choice);
    void start();
};

#endif // VOLEMACHINE_H

