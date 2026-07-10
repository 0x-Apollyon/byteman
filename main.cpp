#include <cstdint>
#include <iostream>
#include <bitset>
#include <stdint.h>
#include <cstdio>

class Register{
    private:
        uint8_t value;
    public:
        Register(){
            this->value = 0;
        }

        Register(int val){
            this->value = val;
        }

        int get_val(){
            return this->value;
        }

        void operator=(int val){
            this->value = val;
        }
};

class ALU{
    private:
        Register A;
        Register B;

        std::bitset<4> flags; //psw register
        //Z, N, C, V
        enum FlagIndex { Z = 0, N = 1, C = 2, V = 3 };

    public:

        void flag_set(uint16_t result_16, uint8_t a, uint8_t b, bool is_arithmetic, bool is_add = true) {
            this->flags.reset(); 
        
            if (result_16 > 255 && is_arithmetic) {
                this->flags[C] = 1;
            }

            uint8_t result_8 = result_16 & 0xFF;

            if (result_8 == 0) {
                this->flags[Z] = 1;
            } 
        
            if ((result_8 & (1 << 7)) != 0) {
                this->flags[N] = 1;
            }

            if (is_arithmetic){
                if (((a ^ result_8) & (b ^ result_8) & (1 << 7)) != 0 && is_add) {
                    this->flags[V] = 1;
                } else if ((((a ^ result_8) & (a ^ b) & (1 << 7)) != 0) && !is_add) {
                    this->flags[V] = 1;
                }
            }
            
        }

        //00100100
        void add(){
            uint16_t val = A.get_val() + B.get_val();
            flag_set(val, A.get_val(), B.get_val(), true);
            A = val & 0xFF; 
        }

        /*Register multiply(Register a, Register b){
            uint16_t val = a.get_val() * b.get_val();
            flag_set(val, a.get_val(), b.get_val(), true);
            return Register(val & 0xFF); 
        }*/

        //01010100
        void bitwise_and() {
            uint8_t val = A.get_val() & B.get_val();
            flag_set(val, A.get_val(), B.get_val(), false);
            A = val;
        }

        //01000100
        void bitwise_or() {
            uint8_t val = A.get_val() | B.get_val();
            flag_set(val, A.get_val(), B.get_val(), false);
            A = val;
        }

        //01100100
        void bitwise_xor() {
            uint8_t val = A.get_val() ^ B.get_val();
            flag_set(val, A.get_val(), B.get_val(), false);
            A = val;
        }

        //11110100
        void bitwise_not() {
            uint8_t val = ~A.get_val();
            flag_set(val, A.get_val(), A.get_val(), false);
            A = val;
        }

        //00110100
        void addc() {
            uint8_t carry_val = this->flags[C];
            uint16_t val = this->A.get_val() + this->B.get_val() + carry_val;
            flag_set(val, this->A.get_val(), this->B.get_val(), true);
            A = val & 0xFF;
        }

        void subb() {
            uint8_t a_val = this->A.get_val();
            uint8_t b_val = this->B.get_val();
            uint8_t c_val = this->flags[C];

            uint16_t val = a_val - b_val - c_val;
        
            flag_set(val , a_val , b_val , true, false);
        
            this->A = Register(val & 0xFF);
        }

        //10100100
        void mul() {
            uint16_t val = this->A.get_val() * this->B.get_val();
            
            uint8_t low_byte = val & 0xFF;
            uint8_t high_byte = (val >> 8) & 0xFF;

            this->flags[C] = 0;

            if (high_byte != 0){
                this->flags[V] = 1;
            }

            if (low_byte == 0){
                this->flags[Z] = 1;
            }

            if ((low_byte & (1 << 7)) != 0) {
                this->flags[N] = 1;
            }
        
            this->A = low_byte;
            this->B = high_byte;
        }

        //10000100
        void div() {
            uint8_t a_val = this->A.get_val();
            uint8_t b_val = this->B.get_val();

            this->flags.reset(); 

            if (b_val == 0){
                this->A = 0;
                this->B = 0;
                this->flags[V] = 1;
                this->flags[Z] = 1;
            } else {
                uint8_t quo = a_val/b_val;
                uint8_t rem = a_val%b_val;

                this->A = quo;
                this->B = rem;

                if (quo == 0){
                    this->flags[Z] = 1;
                }

                if ((quo & (1 << 7)) != 0) {
                    this->flags[N] = 1;
                }
            }
        }   

        //00110011
        void rlc() {
            uint8_t a_val = this->A.get_val();
            uint8_t old_carry = this->flags[C];

            uint8_t msb = (a_val & 0x80) >> 7;

            uint8_t new_a = a_val << 1;
            new_a = new_a | old_carry;

            this->flags[C] = msb;

            if (new_a == 0){
                this->flags[Z] = 1;
            }

            if ((new_a & (1 << 7)) != 0) {
                    this->flags[N] = 1;
            }
            this->A = new_a;
        }

        void execute_instruction(std::string opcode_str, std::string arg1_str, std::string arg2_str, std::string flags_file) {
            uint8_t val_1 = static_cast<uint8_t>(std::bitset<8>(arg1_str).to_ulong());
            uint8_t val_2 = static_cast<uint8_t>(std::bitset<8>(arg2_str).to_ulong());
            
            flags = std::bitset<4>(flags_file);
            this->A = val_1;
            this->B = val_2;

            if (opcode_str == "00100100") {
                add();
            } else if (opcode_str == "00110100") {
                addc();
            } else if (opcode_str == "10010100") {
                subb();
            } else if (opcode_str == "10100100") {
                mul();
            } else if (opcode_str == "10000100") {
                div();
            } else if (opcode_str == "01010100") {
                bitwise_and();
            } else if (opcode_str == "01000100") {
                bitwise_or();
            } else if (opcode_str == "01100100") {
                bitwise_xor();
            } else if (opcode_str == "11110100") {
                bitwise_not();
            } else if (opcode_str == "00110011") {
                rlc();
            } else {
                return; //treat unknown opcodes as NOPs
            }   
        }

        std::string get_state_string() {
            std::string out = "";
            out = out + std::bitset<8>(this->A.get_val()).to_string() + " ";
            out = out + std::bitset<8>(this->B.get_val()).to_string() + " ";
            out = out + this->flags.to_string();
            return out;
        }
};

int main(){
    freopen("instructions.in", "r", stdin);
    freopen("output.out", "w", stdout);

    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    ALU cpu;

    std::string opcode, a_str, b_str, flag_str;

    while (std::cin >> opcode >> a_str >> b_str >> flag_str) {
        cpu.execute_instruction(opcode, a_str, b_str, flag_str);
        std::cout << cpu.get_state_string() << '\n';
    }
}