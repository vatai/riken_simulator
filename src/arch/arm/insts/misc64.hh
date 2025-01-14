/*
 * Copyright (c) 2011-2013,2017 ARM Limited
 * All rights reserved
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Gabe Black
 */

#ifndef __ARCH_ARM_INSTS_MISC64_HH__
#define __ARCH_ARM_INSTS_MISC64_HH__

#include "arch/arm/insts/data64.hh"
#include "arch/arm/insts/pred_inst.hh"
#include "arch/arm/insts/static_inst.hh"

class RegRegImmImmOp64 : public ArmStaticInst
{
  protected:
    IntRegIndex dest;
    IntRegIndex op1;
    uint64_t imm1;
    uint64_t imm2;

    RegRegImmImmOp64(const char *mnem, ExtMachInst _machInst,
                     OpClass __opClass, IntRegIndex _dest, IntRegIndex _op1,
                     uint64_t _imm1, uint64_t _imm2) :
        ArmStaticInst(mnem, _machInst, __opClass),
        dest(_dest), op1(_op1), imm1(_imm1), imm2(_imm2)
    {}

    std::string generateDisassembly(Addr pc, const SymbolTable *symtab) const;
};

class RegRegRegImmOp64 : public ArmStaticInst
{
  protected:
    IntRegIndex dest;
    IntRegIndex op1;
    IntRegIndex op2;
    uint64_t imm;

    RegRegRegImmOp64(const char *mnem, ExtMachInst _machInst,
                     OpClass __opClass, IntRegIndex _dest, IntRegIndex _op1,
                     IntRegIndex _op2, uint64_t _imm) :
        ArmStaticInst(mnem, _machInst, __opClass),
        dest(_dest), op1(_op1), op2(_op2), imm(_imm)
    {}

    std::string generateDisassembly(Addr pc, const SymbolTable *symtab) const;
};

class RegRegImmImmOp64micro: public RegRegImmImmOp64
{
public:
    RegRegImmImmOp64micro(const char *mnem, ExtMachInst _machInst,
                     OpClass __opClass, IntRegIndex _dest, IntRegIndex _op1,
                     uint64_t _imm1, uint64_t _imm2) :
        RegRegImmImmOp64(mnem, _machInst, __opClass, _dest, _op1, _imm1,_imm2)
        {
            flags[IsMicroop] = true;
        };
    void
    advancePC(PCState &pcState) const
    {
        if (flags[IsLastMicroop]) {
            pcState.uEnd();
        } else if (flags[IsMicroop]) {
            pcState.uAdvance();
        } else {
            pcState.advance();
        }
    }
};

class DummyOp:public DataX1RegOp
{
public:
    DummyOp(ExtMachInst machInst,IntRegIndex _dest,
            IntRegIndex _op1, OpClass __opClass);
    Fault execute(ExecContext *xc, Trace::InstRecord *traceData)const;
    void advancePC(PCState &pcState) const;
};

template <class Op, OpClass oc1, OpClass oc2, OpClass oc3>
class WrapMacroOp: public PredMacroOp
{
  public:
    WrapMacroOp(ExtMachInst machInst, IntRegIndex _dest, IntRegIndex _op1,
                uint64_t _imm1, uint64_t _imm2):
        PredMacroOp("WrapOp", machInst, No_OpClass)
        {
            numMicroops = 4;
            microOps = new StaticInstPtr[numMicroops];
            microOps[0] = new DummyOp(machInst, _dest, _op1, oc1);
            microOps[1] = new DummyOp(machInst, _dest, _op1, oc2);
            microOps[2] = new DummyOp(machInst, _dest, _op1, oc3);
            microOps[3] = new Op(machInst, _dest, _op1, _imm1, _imm2);
            microOps[0]->setFirstMicroop();
            microOps[numMicroops-1]->setLastMicroop();
            for (int i = 0; i < numMicroops -1; i++){
                microOps[i]->setDelayedCommit();
            }

        }
    Fault execute(ExecContext *xc, Trace::InstRecord *traceData) const{
        panic("Should not executed");
        return NoFault;
    }
};

class UnknownOp64 : public ArmStaticInst
{
  protected:

    UnknownOp64(const char *mnem, ExtMachInst _machInst, OpClass __opClass) :
        ArmStaticInst(mnem, _machInst, __opClass)
    {}

    std::string generateDisassembly(Addr pc, const SymbolTable *symtab) const;
};

class MiscRegRegImmOp64 : public ArmStaticInst
{
  protected:
    MiscRegIndex dest;
    IntRegIndex op1;
    uint32_t imm;

    MiscRegRegImmOp64(const char *mnem, ExtMachInst _machInst,
                      OpClass __opClass, MiscRegIndex _dest,
                      IntRegIndex _op1, uint32_t _imm) :
        ArmStaticInst(mnem, _machInst, __opClass),
        dest(_dest), op1(_op1), imm(_imm)
    {}

    std::string generateDisassembly(Addr pc, const SymbolTable *symtab) const;
};

class RegMiscRegImmOp64 : public ArmStaticInst
{
  protected:
    IntRegIndex dest;
    MiscRegIndex op1;
    uint32_t imm;

    RegMiscRegImmOp64(const char *mnem, ExtMachInst _machInst,
                      OpClass __opClass, IntRegIndex _dest,
                      MiscRegIndex _op1, uint32_t _imm) :
        ArmStaticInst(mnem, _machInst, __opClass),
        dest(_dest), op1(_op1), imm(_imm)
    {}

    std::string generateDisassembly(Addr pc, const SymbolTable *symtab) const;
};

#endif
