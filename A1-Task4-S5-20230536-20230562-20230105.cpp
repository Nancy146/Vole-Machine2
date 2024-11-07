#include "volemachine.h"

// Register Class Implementation
Register::Register() {
    clear();
}

std::string Register::read(int address) const {
    if (address < 0 || address >= size) {
        std::cout << "ERROR: The address is out of range" << std::endl;
        return "00";
    }
    return registers[address];
}

void Register::write(int address, std::string value) {
    if (address < 0 || address >= size) {
        std::cout << "ERROR: The address is out of range" << std::endl;
        return;
    }
    registers[address] = value;
}

void Register::display() {
    std::cout << "Registers:" << std::endl;
    for (int i = 0; i < size; i++) {
        std::cout << "R" << i << ": " << registers[i] << " ";
        if (i % 4 == 3) std::cout << std::endl;
    }
}

void Register::clear() {
    for (int i = 0; i < size; i++) {
        registers[i] = "00";
    }
}

// ALU Class Implementation
bool Alu::isValid(std::string hexValue) {
    for (char ch : hexValue) {
        if (!((ch >= '0' && ch <= '9') ||
              (ch >= 'A' && ch <= 'F') ||
              (ch >= 'a' && ch <= 'f'))) {
            return false;
        }
    }
    return true;
}

int Alu::hexToDec(std::string hexValue) {
    int result = 0;
    for (char ch : hexValue) {
        result *= 16;
        if (ch >= '0' && ch <= '9') {
            result += ch - '0';
        } else if (ch >= 'A' && ch <= 'F') {
            result += 10 + (ch - 'A');
        } else {
            result += 10 + (ch - 'a');
        }
    }
    return result;
}

std::string Alu::decToHex(int decimalValue) {
    std::string res = "";
    if (decimalValue == 0) {
        res = "0";
    }
    while (decimalValue > 0) {
        int remainder = decimalValue % 16;
        char hexaValue;
        if (remainder <= 9) {
            hexaValue = '0' + remainder;
        } else {
            hexaValue = 'A' + (remainder - 10);
        }
        res = hexaValue + res;
        decimalValue /= 16;
    }
    return res;
}

void Alu::addIntegers(int idx1, int idx2, int resIdx, Register& reg) {
    std::string hexVal1 = reg.read(idx1);
    std::string hexVal2 = reg.read(idx2);

    if (!(isValid(hexVal1) && isValid(hexVal2))) {
        std::cout << "Invalid hexadecimal number." << std::endl;
        return;
    }

    int decSum = hexToDec(hexVal1) + hexToDec(hexVal2);
    std::string hexSum = decToHex(decSum);
    reg.write(resIdx, hexSum);
}

void Alu::addFloats(int idx1, int idx2, int resIdx, Register& reg) {
    std::string hexVal1 = reg.read(idx1);
    std::string hexVal2 = reg.read(idx2);

    int decimalVal1 = hexToDec(hexVal1);
    int decimalVal2 = hexToDec(hexVal2);

    std::bitset<8> binaryVal1(decimalVal1);
    std::bitset<8> binaryVal2(decimalVal2);

    int sign1 = binaryVal1[7];
    int exp1 = (binaryVal1.to_ulong() >> 4) & 0b111;
    int mantissa1 = (binaryVal1.to_ulong() & 0b1111);

    int sign2 = binaryVal2[7];
    int exp2 = (binaryVal2.to_ulong() >> 4) & 0b111;
    int mantissa2 = (binaryVal2.to_ulong() & 0b1111);

    int unbiasedExp1 = exp1 - 4;
    int unbiasedExp2 = exp2 - 4;

    float normalizedMantissa1 = mantissa1 / 16.0f;
    float normalizedMantissa2 = mantissa2 / 16.0f;

    float float1 = pow(2, unbiasedExp1) * normalizedMantissa1 * (sign1 == 1 ? -1 : 1);
    float float2 = pow(2, unbiasedExp2) * normalizedMantissa2 * (sign2 == 1 ? -1 : 1);

    std::cout << "Float Value 1: " << float1 << std::endl;
    std::cout << "Float Value 2: " << float2 << std::endl;

    float floatRes = float1 + float2;
    std::cout << "Result of Addition: " << floatRes << std::endl;

    if (floatRes == 0) {
        reg.write(resIdx, "00");
        std::cout << "Result is zero. Writing 00 to register." << std::endl;
        return;
    }

    uint8_t customFloatRes = float_to_custom_8bit(floatRes);
    std::string hexResult = custom_8bit_to_hex(customFloatRes);
    reg.write(resIdx, hexResult);
    std::cout << "Result in custom 8-bit format (hex): " << hexResult << std::endl;
}

void Alu::execute(int type, int idx1, int idx2, int resIdx, Register& reg) {
    if (type == 0) {
        addIntegers(idx1, idx2, resIdx, reg);
    } else if (type == 1) {
        addFloats(idx1, idx2, resIdx, reg);
    } else {
        std::cout << "Invalid operation type." << std::endl;
    }
}

uint8_t Alu::float_to_custom_8bit(float num) {
    uint8_t result = 0;
    int signBit = 0;
    int exponent = 0;
    uint8_t mantissa = 0;

    if (num < 0) {
        signBit = 1;
        num = -num;
    }

    int exp = 0;
    if (num != 0) {
        while (num >= 1.0) {
            num /= 2.0;
            exp++;
        }
        while (num < 0.5) {
            num *= 2.0;
            exp--;
        }
    }

    exponent = exp + 4;
    if (exponent < 0 || exponent > 7) {
        std::cout << "Error: Exponent out of range for 8-bit format.\n";
        return 0;
    }

    num *= 16;
    mantissa = (uint8_t)num;

    result |= (signBit << 7);
    result |= (exponent << 4);
    result |= (mantissa & 0x0F);

    return result;
}

std::string Alu::custom_8bit_to_hex(uint8_t customFloat) {
    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0') << (int)customFloat;
    return ss.str();
}

// Memory Class Implementation
Memory::Memory() {
    clear();
}

std::string Memory::read(int address) const {
    if (address < 0 || address >= size) {
        std::cout << "ERROR: Invalid memory address." << std::endl;
        return "";
    }
    return memory[address];
}

void Memory::write(int address, std::string value) {
    if (address < 0 || address >= size) {
        std::cout << "ERROR: Invalid memory address." << std::endl;
        return;
    }
    memory[address] = value;
}

void Memory::writeFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        int address = 0;
        std::string value;
        while (file >> value) {
            write(address++, value);
        }
        file.close();
    } else {
        std::cout << "ERROR: Could not open the file." << std::endl;
    }
}

void Memory::display() const {
    std::cout << "Memory:" << std::endl;
    for (int i = 0; i < size; ++i) {
        std::cout << "Memory[" << i << "] = " << memory[i] << std::endl;
    }
}

void Memory::clear() {
    for (int i = 0; i < size; ++i) {
        memory[i] = "00";
    }
}

int Memory::getSize() const {
    return size;
}

// CU Class Implementation
CU::CU() {}

void CU::load(int idxReg, int intMem, Register& reg, Memory& mem) {
    std::string value = mem.read(intMem);
    reg.write(idxReg, value);
}

void CU::loadImmediate(int idxReg, int val, Register& reg) {
    reg.write(idxReg, std::to_string(val));
}

void CU::store(int idxReg, int idxMem, Register& reg, Memory& mem) {
    std::string value = reg.read(idxReg);
    mem.write(idxMem, value);
}

void CU::move(int sourceReg, int destReg, Register& reg) {
    std::string value = reg.read(sourceReg);
    reg.write(destReg, value);
}

void CU::jump(int idxReg, int& PC, Register& reg) {
    if (reg.read(idxReg) == reg.read(0)) {
        int address = stoi(reg.read(idxReg), nullptr, 16);
        PC = address;
    }
}

void CU::halt() {
    std::cout << "Machine halted." << std::endl;
}

void CU::addIntegers(int idx1, int idx2, int resIdx, Register& reg) {
    alu.addIntegers(idx1, idx2, resIdx, reg);
}

void CU::addFloats(int idx1, int idx2, int resIdx, Register& reg) {
    alu.addFloats(idx1, idx2, resIdx, reg);
}

// CPU Class Implementation
CPU::CPU(int startAddress) : programCounter(startAddress) {}

std::string CPU::fetch(int address, const Memory &memory) {
    return memory.read(address);
}

void CPU::runNextStep(Memory& mem) {
    instructionRegister = fetch(programCounter, mem);
    programCounter++;

    DecodedInstruction decoded = decode();
    execute(decoded, mem);
}

void CPU::setProgramCounter(int pc) {
    programCounter = pc;
}

int CPU::getProgramCounter() const {
    return programCounter;
}

void CPU::setInstructionRegister(const std::string& instruction) {
    instructionRegister = instruction;
}

std::string CPU::getInstructionRegister() const {
    return instructionRegister;
}

std::string CPU::getRegister(int index) const {
    return registers.read(index);
}

DecodedInstruction CPU::decode() {
    DecodedInstruction decoded;

    decoded.opcode = instructionRegister[0];
    decoded.R = instructionRegister[1] - '0';
    decoded.S = instructionRegister[2] - '0';
    decoded.T = instructionRegister[3] - '0';
    decoded.address = hexToDec(instructionRegister.substr(4, 2));
    decoded.immediate = hexToDec(instructionRegister.substr(6, 2));

    return decoded;
}

void CPU::execute(DecodedInstruction& instruction, Memory& mem) {
    switch (instruction.opcode) {
        case 'L': {
            controlUnit.load(instruction.R, instruction.address, registers, mem);
            break;
        }
        case 'S': {
            controlUnit.store(instruction.R, instruction.address, registers, mem);
            break;
        }
        case 'A': {
            controlUnit.addIntegers(instruction.R, instruction.S, instruction.T, registers);
            break;
        }
        case 'J': {
            controlUnit.jump(instruction.R, programCounter, registers);
            break;
        }
        case 'H': {
            controlUnit.halt();
            break;
        }
        default:
            std::cout << "Invalid instruction." << std::endl;
    }
}

int CPU::hexToDec(const std::string& hexValue) const {
    int result = 0;
    for (char ch : hexValue) {
        result *= 16;
        if (ch >= '0' && ch <= '9') {
            result += ch - '0';
        } else if (ch >= 'A' && ch <= 'F') {
            result += 10 + (ch - 'A');
        }
    }
    return result;
}

// Machine Class Implementation
Machine::Machine() : cpu(), memory() {}

bool Machine::loadFile(const std::string& filename) {
    memory.writeFromFile(filename);
    return true;
}

void Machine::printStateStepByStep() {
    cpu.runNextStep(memory);
    cpu.registers.display();
    memory.display();
}

void Machine::printWholeState() {
    cpu.registers.display();
    memory.display();
}

// MainUI Class Implementation
void MainUI::displayMenu() {
    std::cout << "Machine Simulator - Menu:" << std::endl;
    std::cout << "1. Load file" << std::endl;
    std::cout << "2. Step-by-step execution" << std::endl;
    std::cout << "3. Print current state" << std::endl;
    std::cout << "4. Exit" << std::endl;
}

void MainUI::handleUserChoice(int choice) {
    switch (choice) {
        case 1: {
            std::string filename;
            std::cout << "Enter file name: ";
            std::cin >> filename;
            machine.loadFile(filename);
            break;
        }
        case 2: {
            machine.printStateStepByStep();
            break;
        }
        case 3: {
            machine.printWholeState();
            break;
        }
        case 4: {
            std::cout << "Exiting program." << std::endl;
            exit(0);
            break;
        }
        default:
            std::cout << "Invalid choice." << std::endl;
    }
}

void MainUI::start() {
    while (true) {
        displayMenu();
        int choice;
        std::cout << "Enter choice: ";
        std::cin >> choice;
        handleUserChoice(choice);
    }
}
