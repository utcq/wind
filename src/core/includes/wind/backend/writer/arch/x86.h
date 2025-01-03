#include <wind/backend/writer/writer.h>
#ifndef WRITER_Ax86_64_H
#define WRITER_Ax86_64_H

#define GPREG64(name, id) const Reg name = {id, 8, Reg::GPR}
#define GPREG32(name, id) const Reg name = {id, 4, Reg::GPR}
#define GPREG16(name, id) const Reg name = {id, 2, Reg::GPR}
#define GPREG8(name, id) const Reg name = {id, 1, Reg::GPR}
#define SEGREG(name, id) const Reg name = {id, 8, Reg::SEG}

namespace x86 {
    namespace Gp {
        GPREG64(rax, 0);
        GPREG64(rcx, 1);
        GPREG64(rdx, 2);
        GPREG64(rbx, 3);
        GPREG64(rsp, 4);
        GPREG64(rbp, 5);
        GPREG64(rsi, 6);
        GPREG64(rdi, 7);
        GPREG64(r8, 8);
        GPREG64(r9, 9);
        GPREG64(r10, 10);
        GPREG64(r11, 11);
        GPREG64(r12, 12);
        GPREG64(r13, 13);
        GPREG64(r14, 14);
        GPREG64(r15, 15);

        GPREG32(eax, 0);
        GPREG32(ecx, 1);
        GPREG32(edx, 2);
        GPREG32(ebx, 3);
        GPREG32(esp, 4);
        GPREG32(ebp, 5);
        GPREG32(esi, 6);
        GPREG32(edi, 7);
        GPREG32(r8d, 8);
        GPREG32(r9d, 9);
        GPREG32(r10d, 10);
        GPREG32(r11d, 11);
        GPREG32(r12d, 12);
        GPREG32(r13d, 13);
        GPREG32(r14d, 14);
        GPREG32(r15d, 15);

        GPREG16(ax, 0);
        GPREG16(cx, 1);
        GPREG16(dx, 2);
        GPREG16(bx, 3);
        GPREG16(sp, 4);
        GPREG16(bp, 5);
        GPREG16(si, 6);
        GPREG16(di, 7);
        GPREG16(r8w, 8);
        GPREG16(r9w, 9);
        GPREG16(r10w, 10);
        GPREG16(r11w, 11);
        GPREG16(r12w, 12);
        GPREG16(r13w, 13);
        GPREG16(r14w, 14);
        GPREG16(r15w, 15);

        GPREG8(al, 0);
        GPREG8(cl, 1);
        GPREG8(dl, 2);
        GPREG8(bl, 3);
        GPREG8(ah, 4);
        GPREG8(ch, 5);
        GPREG8(dh, 6);
        GPREG8(bh, 7);
        GPREG8(r8b, 8);
        GPREG8(r9b, 9);
        GPREG8(r10b, 10);
        GPREG8(r11b, 11);
        GPREG8(r12b, 12);
        GPREG8(r13b, 13);
        GPREG8(r14b, 14);
        GPREG8(r15b, 15);
    }
    namespace Seg {
        SEGREG(es, 0);
        SEGREG(cs, 1);
        SEGREG(ss, 2);
        SEGREG(ds, 3);
        SEGREG(fs, 4);
        SEGREG(gs, 5);
    }
    using Mem = Mem;
}


// Declaration macros

#define INSTR_HANDLE(handler) \
    if (handler != 0) \
        this->Write("jo", handler);


#define A_IRR_INSTR(name, handler) void name(Reg dst, Reg src) { this->Write(#name, dst, src); INSTR_HANDLE(handler) } // IRR stands for "Instruction Register-Register"
#define A_IRM_INSTR(name, handler) void name(Reg dst, Mem src) { this->Write(#name, dst, src); INSTR_HANDLE(handler) } // IRM stands for "Instruction Register-Memory"
#define A_IMR_INSTR(name, handler) void name(Mem dst, Reg src) { this->Write(#name, dst, src); INSTR_HANDLE(handler) } // IMR stands for "Instruction Memory-Register"
#define A_IRI_INSTR(name, handler) void name(Reg dst, int64_t imm) { this->Write(#name, dst, imm); INSTR_HANDLE(handler) } // IRI stands for "Instruction Register-Immediate"
#define A_IMI_INSTR(name, handler) void name(Mem dst, int64_t imm) { this->Write(#name, dst, imm); INSTR_HANDLE(handler) } // IMI stands for "Instruction Memory-Immediate"
#define A_IIR_INSTR(name, handler) void name(int64_t imm, Reg src) { this->Write(#name, imm, src); INSTR_HANDLE(handler) } // IIR stands for "Instruction Immediate-Register"
#define A_IIM_INSTR(name, handler) void name(int64_t imm, Mem src) { this->Write(#name, imm, src); INSTR_HANDLE(handler) } // IIM stands for "Instruction Immediate-Memory"
#define A_FIVE_INSTR(name, handler) \
    A_IRR_INSTR(name, handler) \
    A_IRM_INSTR(name, handler) \
    A_IMR_INSTR(name, handler) \
    A_IRI_INSTR(name, handler) \
    A_IMI_INSTR(name, handler) // FIVE for (IRR IRM IMR)

#define B_N_INSTR(name) void name() { this->Write(#name); } // N stands for "No"
#define B_IR_INSTR(name) void name(Reg dst) { this->Write(#name, dst); } // IR stands for "Instruction Register"
#define B_IM_INSTR(name) void name(Mem dst) { this->Write(#name, dst); } // IM stands for "Instruction Memory"
#define B_IL_INSTR(name) void name(std::string label) { this->Write(#name, label); } // IL stands for "Instruction Label"
#define B_TRIPLE_INSTR(name) \
    B_IR_INSTR(name) \
    B_IM_INSTR(name) \
    B_IL_INSTR(name) // TRIPLE for (IR IM IL)


#define C_SEVEN_INSTR(name) \
    A_IRR_INSTR(name, 0) \
    A_IRM_INSTR(name, 0) \
    A_IMR_INSTR(name, 0) \
    A_IRI_INSTR(name, 0) \
    A_IMI_INSTR(name, 0) \
    A_IIR_INSTR(name, 0) \
    A_IIM_INSTR(name, 0) // SEVEN for (IRR IRM IMR IRI IMI IIR IIM)
    
// ----------------------------


class Ax86_64 : public WindWriter {
private:
    // Resolve regs
    std::string ResolveGpr(Reg &reg);
    std::string ResolveSeg(Reg &reg);
    std::string ResolveReg(Reg &reg) override;
    std::string ResolveMem(Mem &reg) override;
    std::string ResolveWord(uint16_t size) override;

public:
    Ax86_64() {}
    
    A_FIVE_INSTR(mov, 0)
    A_FIVE_INSTR(lea, 0)
    A_FIVE_INSTR(add, "__WDH_sum_overflow")
    A_FIVE_INSTR(sub, "__WDH_sub_overflow")
    A_FIVE_INSTR(shr, 0)
    A_FIVE_INSTR(shl, 0)
    A_FIVE_INSTR(imul, "__WDH_mul_overflow")

    A_FIVE_INSTR(movzx, 0)

    B_TRIPLE_INSTR(jmp)
    B_TRIPLE_INSTR(call)
    B_TRIPLE_INSTR(push)
    B_TRIPLE_INSTR(pop)
    B_TRIPLE_INSTR(je)
    B_TRIPLE_INSTR(jne)
    B_TRIPLE_INSTR(jg)
    B_TRIPLE_INSTR(jge)
    B_TRIPLE_INSTR(jl)
    B_TRIPLE_INSTR(jle)
    B_TRIPLE_INSTR(ja)
    B_TRIPLE_INSTR(jae)
    B_TRIPLE_INSTR(jb)
    B_TRIPLE_INSTR(jbe)
    B_TRIPLE_INSTR(jo)
    B_TRIPLE_INSTR(jno)
    B_TRIPLE_INSTR(js)
    B_TRIPLE_INSTR(jns)
    B_TRIPLE_INSTR(jp)
    B_TRIPLE_INSTR(jnp)
    B_TRIPLE_INSTR(sete)
    B_TRIPLE_INSTR(setne)
    B_TRIPLE_INSTR(setg)
    B_TRIPLE_INSTR(setge)
    B_TRIPLE_INSTR(setl)
    B_TRIPLE_INSTR(setle)
    B_TRIPLE_INSTR(seta)
    B_TRIPLE_INSTR(setae)
    B_TRIPLE_INSTR(setb)
    B_TRIPLE_INSTR(setbe)

    B_N_INSTR(leave)
    B_N_INSTR(ret)
    B_N_INSTR(rdtsc)
    B_N_INSTR(rdtscp)

    C_SEVEN_INSTR(cmp)
    C_SEVEN_INSTR(test)




    inline void and_(Reg dst, Reg src) { this->Write("and", dst, src); }
    inline void and_(Reg dst, int32_t imm) { this->Write("and", dst, imm); }
    inline void and_(Mem mem, Reg src) { this->Write("and", mem, src); }
    inline void and_(Mem mem, int32_t imm) { this->Write("and", mem, imm); }
    inline void and_(Reg dst, Mem mem) { this->Write("and", dst, mem); }

    inline void or_(Reg dst, Reg src) { this->Write("or", dst, src); }
    inline void or_(Reg dst, int32_t imm) { this->Write("or", dst, imm); }
    inline void or_(Mem mem, Reg src) { this->Write("or", mem, src); }
    inline void or_(Mem mem, int32_t imm) { this->Write("or", mem, imm); }
    inline void or_(Reg dst, Mem mem) { this->Write("or", dst, mem); }

    inline void xor_(Reg dst, Reg src) { this->Write("xor", dst, src); }
    inline void xor_(Reg dst, int32_t imm) { this->Write("xor", dst, imm); }
    inline void xor_(Mem mem, Reg src) { this->Write("xor", mem, src); }
    inline void xor_(Mem mem, int32_t imm) { this->Write("xor", mem, imm); }
    inline void xor_(Reg dst, Mem mem) { this->Write("xor", dst, mem); }
};

#endif