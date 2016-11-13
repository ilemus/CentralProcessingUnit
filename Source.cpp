#include <iostream>
#include <string>
#include <fstream>

using namespace std;

// Memory bank of size 100 * 8 bits
char MAIN_MEMORY[100];
char PROGRAM_CODE[50];

char mProgramCounter;

const char R1 = 0xF0; // A
const char R2 = 0xF1; // B
const char R3 = 0xF2; // C
const char R4 = 0xF3; // D
const char R5 = 0xF4; // E

const char ADD = 0x00; // ADD R1 R2
const char SUB = 0x01; // SUB R1 R2
const char ADD_W_CARRY = 0x02; // ADDC R1 R2
const char INC = 0x03; // INC R1
const char LDD = 0x04; // LDD R1 MAIN_MEMORY[index]
const char LDD_IO = 0x05; // READ R1
const char STR = 0x06; // STR R1 MAIN_MEMORY[index]
const char SHL = 0x07; // SHL R1
const char SHR = 0x08; // SHR R1
const char AND = 0x09; // AND R1 R2
const char OR = 0x0A; // OR R1 R2
const char XOR = 0x0B; // XOR R1 R2
const char COMP = 0x0C; // COMP R1
const char BEQ = 0x0D; // BEQ
const char BIC = 0x0E; // BIC
const char END = 0x10; // END

string getOpName(char x) {
    switch (x) {
    case ADD:
        return "ADD";
    case SUB:
        return "SUB";
    case ADD_W_CARRY:
        return "ADDC";
    case INC:
        return "INC";
    case LDD:
        return "LDD";
    case LDD_IO:
        return "READ";
    case STR:
        return "STR";
    case SHL:
        return "SHL";
    case SHR:
        return "SHR";
    case AND:
        return "AND";
    case OR:
        return "OR";
    case XOR:
        return "XOR";
    case COMP:
        return "COMP";
    case BEQ:
        return "BEQ";
    case BIC:
        return "BIC";
    default:
        return "ADD";
    }
}

// Used to get IO from command line
char getChar() {
    char line[10];

    cin.getline(line, 10);

    return (char) atoi(line);
}

// Used to read Machine code from file
void loadCode() {
    ifstream in("MachineCode.mc");
    char line[10];
    int i = 0;

    if (in.is_open()) {
        while (in.getline(line, 10)) {
            PROGRAM_CODE[i] = (char) atoi(line);
            i++;

            if (i >= 49) {
                break;
            }
        }
    } else {
        cout << "NO FILE" << endl;
    }
}

class Register {
    // 8-bit register
    unsigned char mRegister;

public:
    // Load the register with value
    void loadRegister(char b) {
        mRegister = (unsigned char) b;
    }

    // Stores into main memory given a location index
    void storeRegister(int location) {
        MAIN_MEMORY[location] = mRegister;
    }

    // Shift to the left (Logical)
    void shiftLeft() {
        mRegister = (unsigned char)mRegister << 1;
    }

    // Shift to the right (Logical)
    void shiftRight() {
        mRegister = (unsigned char)mRegister >> 1;
    }

    // Used to return the value of the register
    char getValue() {
        return mRegister;
    }

    void copyRegister(Register a) {
        mRegister = (unsigned char) a.mRegister;
    }
};

Register *A = new Register();
Register *B = new Register();
Register *C = new Register();
Register *D = new Register();
Register *E = new Register();

Register* getRegister(char x) {
    switch (x) {
    case R1:
        return A;
    case R2:
        return B;
    case R3:
        return C;
    case R4:
        return D;
    case R5:
        return E;
    default:
        return A;
    }
}

class Stack {
public:
    Stack *p;
    char value;

    void push(char val) {
        Stack *q = new Stack();
        q->p = p;
        q->value = val;
        p = q;
    }

    char pop() {
        char ret;
        if (p != NULL) {
            ret = p->value;
            p = p->p;
        }
        return ret;
    }
};

Stack *alpha = new Stack();

class FlagRegister {
    bool C, Z, V, N;

public:
    void setCarry(bool x) {
        C = x;
    }

    // Returns C flag
    bool getCarry() {
        return C;
    }

    void clearCarry() {
        C = false;
    }


    void setZero(bool x) {
        Z = x;
    }

    // Returns Z flag
    bool getZero() {
        return Z;
    }

    void clearZero() {
        Z = false;
    }

    void setOverflow(bool x) {
        V = x;
    }

    // Returns V flag
    bool getOverflow() {
        return V;
    }

    void clearOverflow() {
        V = false;
    }

    void setNegative(bool x) {
        N = x;
    }

    // Returns N flag
    bool getNegative() {
        return N;
    }

    void clearNegative() {
        N = false;
    }

    void clearFlags() {
        clearCarry();
        clearNegative();
        clearOverflow();
        clearZero();
    }
};

FlagRegister *flag = new FlagRegister();

class LogicUnit {
public:
    char AND(char a, char b) {
        return (char)a & b;
    }

    char OR(char a, char b) {
        return (char)a | b;
    }

    char XOR(char a, char b) {
        return (char)a ^ b;
    }

    char COMP(char a) {
        return (char)~a;
    }
} LU;

class ArithmeticUnit {
public:
    char ADD_W_CARRY(char a, char b) {
        // Temp register
        alpha->push(A->getValue());

        if (flag->getCarry()) {
            B->loadRegister(ADD(a, b) + 1);
        }
        else {
            B->loadRegister(ADD(a, b));
        }

        // Store result into temp var memory
        B->storeRegister(10);
        B->loadRegister(alpha->pop());

        return MAIN_MEMORY[10];
    }

    char INC(char a) {
        return a++;
    }

    char SUB(char a, char b) {
        alpha->push(A->getValue());
        alpha->push(B->getValue());
        alpha->push(C->getValue());

        A->loadRegister(a);

        // 1's compliment
        C->loadRegister(LU.COMP(b));
        // 2's compliment
        B->loadRegister(C->getValue() + 1);

        A->loadRegister(ADD(A->getValue(), B->getValue()));

        A->storeRegister(10);

        C->loadRegister(alpha->pop());
        B->loadRegister(alpha->pop());
        A->loadRegister(alpha->pop());

        return MAIN_MEMORY[10];
    }

    char ADD(char a, char b) {
        bool c_1 = false;
        bool carry = false;
        bool negative = false;
        bool zero = false;

        // Push register before executing addition
        alpha->push(A->getValue());
        alpha->push(B->getValue());
        alpha->push(C->getValue());
        alpha->push(D->getValue());
        alpha->push(E->getValue());

        // Load registers with high bits
        A->loadRegister(a & 0x80);
        B->loadRegister(b & 0x80);

        // Load registers with low bits
        C->loadRegister(a & 0x7F);
        D->loadRegister(b & 0x7F);

        // Add lower bits
        C->loadRegister(C->getValue() + D->getValue());

        // Get carry of c(n - 1)
        if ((C->getValue() & 0x80) == 0x80) {
            c_1 = true;
        }

        cout << "C_1 is " << c_1 << ", C->getValue()" << (int) C->getValue() << endl;

        // Push result so C can be used again
        alpha->push(C->getValue());

        C->loadRegister(C->getValue() & 0x80);

        cout << "A: " << (int)A->getValue() << ", B: " << (int)B->getValue() << ", C: " << (int)C->getValue() << endl;

        C->shiftRight();

        // Add higher bit
        A->shiftRight();
        B->shiftRight();

        cout << "A: " << (int)A->getValue() << ", B: " << (int)B->getValue() << ", C: " << (int)C->getValue() << endl;

        A->loadRegister(A->getValue() + B->getValue() + C->getValue());

        // Test if there is a carry out
        if ((A->getValue() & 0x80) == 0x80) {
            carry = true;
        }

        cout << "CARRY is " << carry << ", A VAL: " << (int)A->getValue() << endl;
        
        E->loadRegister(A->getValue());
        // Get rid of Carry
        E->shiftLeft();

        // Load and get rid of highest bit
        C->loadRegister(alpha->pop() & 0x7F);

        A->loadRegister(E->getValue() + C->getValue());

        // Memory for temp variables starts at 10
        A->storeRegister(10);
        
        // Set flags based on registers
        if ((A->getValue() & 0x80) == 0x80) {
            negative = true;
        }

        if (A->getValue() == 0) {
            zero = true;
        }

        E->loadRegister(alpha->pop());
        D->loadRegister(alpha->pop());
        C->loadRegister(alpha->pop());
        B->loadRegister(alpha->pop());
        A->loadRegister(alpha->pop());

        flag->setCarry(carry);
        flag->setNegative(negative);
        flag->setOverflow(carry ^ c_1);
        flag->setZero(zero);

        return MAIN_MEMORY[10];
    }
} AU;

string getRegisterName(char x) {
    switch (x) {
    case R1:
        return "R1";
    case R2:
        return "R2";
    case R3:
        return "R3";
    case R4:
        return "R4";
    case R5:
        return "R5";
    default:
        return "R1";
    }
}

void printCommand(string opname, string r1, string r2) {
    system("cls");
    printf("***************************************\n");
    printf("%s %s %s\n", opname.c_str(), r1.c_str(), r2.c_str());
}

void printCommand(string opname, string r1) {
    system("cls");
    printf("***************************************\n");
    printf("%s %s\n", opname.c_str(), r1.c_str());
    if (opname == "READ") {
        cout << "Enter char value: ";
    }
}

void printRegisters() {
    // Draw Header
    printf("_______________________________________\n");
    printf("R1 %d\tR2 %d\tR3 %d\tR4 %d\tR5 %d\n",
        (int)A->getValue(), (int)B->getValue(), (int)C->getValue(), (int)D->getValue(), (int)E->getValue());
    printf("_______________________________________\n");
    printf("C %s Z %s V %s N %s\n",
        flag->getCarry() ? "true" : "false", 
        flag->getZero() ? "true" : "false",
        flag->getOverflow() ? "true" : "false",
        flag->getNegative() ? "true" : "false");
    printf("***************************************\n");
}

void CPU(string opname, int x) {
    string r1, r2;
    char inx;
    int m1, m2;

    switch (x) {
    case ADD:
        m1 = PROGRAM_CODE[mProgramCounter];
        r1 = getRegisterName(PROGRAM_CODE[mProgramCounter]);
        mProgramCounter++;
        m2 = PROGRAM_CODE[mProgramCounter];
        r2 = getRegisterName(PROGRAM_CODE[mProgramCounter]);
        mProgramCounter++;
        printCommand(opname, r1, r2);
        getRegister(m1)->loadRegister(AU.ADD(getRegister(m1)->getValue(), getRegister(m2)->getValue()));
        printRegisters();
        break;
    case SUB:
        m1 = PROGRAM_CODE[mProgramCounter];
        r1 = getRegisterName(PROGRAM_CODE[mProgramCounter]);
        mProgramCounter++;
        m2 = PROGRAM_CODE[mProgramCounter];
        r2 = getRegisterName(PROGRAM_CODE[mProgramCounter]);
        mProgramCounter++;
        printCommand(opname, r1, r2);
        getRegister(m1)->loadRegister(AU.SUB(getRegister(m1)->getValue(), getRegister(m2)->getValue()));
        printRegisters();
        break;
    case ADD_W_CARRY:
        m1 = PROGRAM_CODE[mProgramCounter];
        r1 = getRegisterName(PROGRAM_CODE[mProgramCounter]);
        mProgramCounter++;
        m2 = PROGRAM_CODE[mProgramCounter];
        r2 = getRegisterName(PROGRAM_CODE[mProgramCounter]);
        mProgramCounter++;
        printCommand(opname, r1, r2);
        getRegister(m1)->loadRegister(AU.ADD_W_CARRY(getRegister(m1)->getValue(), getRegister(m2)->getValue()));
        printRegisters();
        break;
    case INC:
        m1 = PROGRAM_CODE[mProgramCounter];
        r1 = getRegisterName(PROGRAM_CODE[mProgramCounter]);
        mProgramCounter++;
        printCommand(opname, r1);
        getRegister(m1)->loadRegister(AU.INC(getRegister(m1)->getValue()));
        printRegisters();
        break;
    case LDD:
        m1 = PROGRAM_CODE[mProgramCounter];
        r1 = getRegisterName(PROGRAM_CODE[mProgramCounter]);
        mProgramCounter++;
        inx = PROGRAM_CODE[mProgramCounter];
        r2 = "MEM[";
        r2.append(to_string((int)inx)).append("]");
        mProgramCounter++;
        printCommand(opname, r1, r2);
        getRegister(m1)->loadRegister(MAIN_MEMORY[x]);
        printRegisters();
        break;
    case LDD_IO:
        m1 = PROGRAM_CODE[mProgramCounter];
        r1 = getRegisterName(PROGRAM_CODE[mProgramCounter]);
        mProgramCounter++;
        printCommand(opname, r1);
        getRegister(m1)->loadRegister(getChar());
        printRegisters();
        break;
    case STR:
        m1 = PROGRAM_CODE[mProgramCounter];
        r1 = getRegisterName(PROGRAM_CODE[mProgramCounter]);
        mProgramCounter++;
        inx = PROGRAM_CODE[mProgramCounter];
        r2 = "MEM[";
        r2.append(to_string((int)inx)).append("]");
        mProgramCounter++;
        printCommand(opname, r2, r1);
        getRegister(m1)->storeRegister(MAIN_MEMORY[(int)PROGRAM_CODE[mProgramCounter]]);
        printRegisters();
        break;
    case SHL:
        m1 = PROGRAM_CODE[mProgramCounter];
        r1 = getRegisterName(PROGRAM_CODE[mProgramCounter]);
        mProgramCounter++;
        printCommand(opname, r1);
        getRegister(m1)->shiftLeft();
        printRegisters();
        break;
    case SHR:
        m1 = PROGRAM_CODE[mProgramCounter];
        r1 = getRegisterName(PROGRAM_CODE[mProgramCounter]);
        mProgramCounter++;
        printCommand(opname, r1);
        getRegister(m1)->shiftRight();
        printRegisters();
        break;
    case AND:
        m1 = PROGRAM_CODE[mProgramCounter];
        r1 = getRegisterName(PROGRAM_CODE[mProgramCounter]);
        mProgramCounter++;
        m2 = PROGRAM_CODE[mProgramCounter];
        r2 = getRegisterName(PROGRAM_CODE[mProgramCounter]);
        mProgramCounter++;
        printCommand(opname, r1, r2);
        getRegister(m1)->loadRegister(LU.AND(getRegister(m1)->getValue(), getRegister(m2)->getValue()));
        printRegisters();
        break;
    case OR:
        m1 = PROGRAM_CODE[mProgramCounter];
        r1 = getRegisterName(PROGRAM_CODE[mProgramCounter]);
        mProgramCounter++;
        m2 = PROGRAM_CODE[mProgramCounter];
        r2 = getRegisterName(PROGRAM_CODE[mProgramCounter]);
        mProgramCounter++;
        printCommand(opname, r1, r2);
        getRegister(m1)->loadRegister(LU.OR(getRegister(m1)->getValue(), getRegister(m2)->getValue()));
        printRegisters();
        break;
    case XOR:
        m1 = PROGRAM_CODE[mProgramCounter];
        r1 = getRegisterName(PROGRAM_CODE[mProgramCounter]);
        mProgramCounter++;
        m2 = PROGRAM_CODE[mProgramCounter];
        r2 = getRegisterName(PROGRAM_CODE[mProgramCounter]);
        mProgramCounter++;
        printCommand(opname, r1, r2);
        getRegister(m1)->loadRegister(LU.XOR(getRegister(m1)->getValue(), getRegister(m2)->getValue()));
        printRegisters();
        break;
    case COMP:
        m1 = PROGRAM_CODE[mProgramCounter];
        r1 = getRegisterName(PROGRAM_CODE[mProgramCounter]);
        mProgramCounter++;
        printCommand(opname, r1);
        getRegister(m1)->loadRegister(LU.COMP(getRegister(m1)->getValue()));
        printRegisters();
        break;
    case BEQ:
        if (flag->getZero()) {
            mProgramCounter = PROGRAM_CODE[mProgramCounter];
            break;
        }
        else {
            mProgramCounter++;
            break;
        }
    case BIC:
        if (flag->getCarry()) {
            mProgramCounter = PROGRAM_CODE[mProgramCounter];
            break;
        }
        else {
            mProgramCounter++;
            break;
        }
    default:
        break;
    }
    cin.get();
}

int main() {
    string opname;
    flag->clearFlags();
    loadCode();

    while (true) {
        char x = PROGRAM_CODE[mProgramCounter];
        mProgramCounter++;
        opname = getOpName(x);

        if (x == END) {
            break;
        }

        CPU(opname, x);
    }

    return 0;
}