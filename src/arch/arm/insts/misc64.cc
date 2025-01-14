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

#include "arch/arm/insts/misc64.hh"
#include "cpu/exec_context.hh"

std::string
RegRegImmImmOp64::generateDisassembly(Addr pc, const SymbolTable *symtab) const
{
    std::stringstream ss;
    printMnemonic(ss, "", false);
    printIntReg(ss, dest);
    ss << ", ";
    printIntReg(ss, op1);
    ccprintf(ss, ", #%d, #%d", imm1, imm2);
    return ss.str();
}

std::string
RegRegRegImmOp64::generateDisassembly(
    Addr pc, const SymbolTable *symtab) const
{
    std::stringstream ss;
    printMnemonic(ss, "", false);
    printIntReg(ss, dest);
    ss << ", ";
    printIntReg(ss, op1);
    ss << ", ";
    printIntReg(ss, op2);
    ccprintf(ss, ", #%d", imm);
    return ss.str();
}

std::string
UnknownOp64::generateDisassembly(Addr pc, const SymbolTable *symtab) const
{
    return csprintf("%-10s (inst %#08x)", "unknown", machInst);
}

std::string
MiscRegRegImmOp64::generateDisassembly(
    Addr pc, const SymbolTable *symtab) const
{
    std::stringstream ss;
    printMnemonic(ss);
    printMiscReg(ss, dest);
    ss << ", ";
    printIntReg(ss, op1);
    return ss.str();
}

std::string
RegMiscRegImmOp64::generateDisassembly(
    Addr pc, const SymbolTable *symtab) const
{
    std::stringstream ss;
    printMnemonic(ss);
    printIntReg(ss, dest);
    ss << ", ";
    printMiscReg(ss, op1);
    return ss.str();
}

DummyOp::DummyOp(ExtMachInst machInst,IntRegIndex _dest,
                 IntRegIndex _op1, OpClass __opClass):
    DataX1RegOp("DummyOp", machInst, __opClass, _dest, _op1)
{
    _numSrcRegs = 0;
    _numDestRegs = 0;
    _numFPDestRegs = 0;
    _numIntDestRegs = 0;
    _numCCDestRegs = 0;
    _numVecDestRegs = 0;
    _srcRegIdx[_numSrcRegs++] = RegId(IntRegClass, op1);
    _srcRegIdx[_numSrcRegs++] = RegId(IntRegClass, dest);
    _destRegIdx[_numDestRegs++] = RegId(IntRegClass, dest);
    _numIntDestRegs++;
    flags[IsInteger] = true;
    flags[IsMicroop] = true;
}

Fault DummyOp::execute(ExecContext *xc, Trace::InstRecord *traceData) const
{
    uint64_t val = xc->readIntRegOperand(this, 1);
    xc->setIntRegOperand(this, 0, val);
    return NoFault;
}

void
DummyOp::advancePC(PCState &pcState) const
{
    if (flags[IsLastMicroop]) {
        pcState.uEnd();
    } else if (flags[IsMicroop]) {
        pcState.uAdvance();
    } else {
        pcState.advance();
    }
}
