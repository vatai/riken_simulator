/* $Id$ */

/* @file
 * Emulation of the Tsunami CChip CSRs
 */

#include <deque>
#include <string>
#include <vector>

#include "base/trace.hh"
#include "cpu/exec_context.hh"
#include "dev/console.hh"
#include "dev/tsunami_cchip.hh"
#include "dev/tsunamireg.h"
#include "dev/tsunami.hh"
#include "cpu/intr_control.hh"
#include "mem/functional_mem/memory_control.hh"
#include "sim/builder.hh"
#include "sim/system.hh"

using namespace std;

TsunamiCChip::TsunamiCChip(const string &name, Tsunami *t, Addr a,
                           MemoryController *mmu)
    : FunctionalMemory(name), addr(a), tsunami(t)
{
    mmu->add_child(this, Range<Addr>(addr, addr + size));

    for(int i=0; i < Tsunami::Max_CPUs; i++) {
        dim[i] = 0;
        dir[i] = 0;
        dirInterrupting[i] = false;
    }

    drir = 0;
    misc = 0;
    RTCInterrupting = false;

    //Put back pointer in tsunami
    tsunami->cchip = this;
}

Fault
TsunamiCChip::read(MemReqPtr &req, uint8_t *data)
{
    DPRINTF(Tsunami, "read  va=%#x size=%d\n",
            req->vaddr, req->size);

    Addr daddr = (req->paddr - (addr & PA_IMPL_MASK)) >> 6;
    ExecContext *xc = req->xc;

    switch (req->size) {

      case sizeof(uint64_t):
          switch(daddr) {
              case TSDEV_CC_CSR:
                  *(uint64_t*)data = 0x0;
                  return No_Fault;
              case TSDEV_CC_MTR:
                  panic("TSDEV_CC_MTR not implemeted\n");
                   return No_Fault;
              case TSDEV_CC_MISC:
                *(uint64_t*)data = misc | (xc->cpu_id & 0x3);
                  return No_Fault;
              case TSDEV_CC_AAR0:
              case TSDEV_CC_AAR1:
              case TSDEV_CC_AAR2:
              case TSDEV_CC_AAR3:
                  panic("TSDEV_CC_AARx not implemeted\n");
                  return No_Fault;
              case TSDEV_CC_DIM0:
                  *(uint64_t*)data = dim[0];
                  return No_Fault;
              case TSDEV_CC_DIM1:
                  *(uint64_t*)data = dim[1];
                  return No_Fault;
              case TSDEV_CC_DIM2:
                  *(uint64_t*)data = dim[2];
                  return No_Fault;
              case TSDEV_CC_DIM3:
                  *(uint64_t*)data = dim[3];
                  return No_Fault;
              case TSDEV_CC_DIR0:
                  *(uint64_t*)data = dir[0];
                  return No_Fault;
              case TSDEV_CC_DIR1:
                  *(uint64_t*)data = dir[1];
                  return No_Fault;
              case TSDEV_CC_DIR2:
                  *(uint64_t*)data = dir[2];
                  return No_Fault;
              case TSDEV_CC_DIR3:
                  *(uint64_t*)data = dir[3];
                  return No_Fault;
              case TSDEV_CC_DRIR:
                  *(uint64_t*)data = drir;
                  return No_Fault;
              case TSDEV_CC_PRBEN:
                  panic("TSDEV_CC_PRBEN not implemented\n");
                  return No_Fault;
              case TSDEV_CC_IIC0:
              case TSDEV_CC_IIC1:
              case TSDEV_CC_IIC2:
              case TSDEV_CC_IIC3:
                  panic("TSDEV_CC_IICx not implemented\n");
                  return No_Fault;
              case TSDEV_CC_MPR0:
              case TSDEV_CC_MPR1:
              case TSDEV_CC_MPR2:
              case TSDEV_CC_MPR3:
                  panic("TSDEV_CC_MPRx not implemented\n");
                  return No_Fault;
              default:
                  panic("default in cchip read reached, accessing 0x%x\n");
           } // uint64_t

      break;
      case sizeof(uint32_t):
      case sizeof(uint16_t):
      case sizeof(uint8_t):
      default:
        panic("invalid access size(?) for tsunami register!\n");
    }
    DPRINTFN("Tsunami CChip ERROR: read  daddr=%#x size=%d\n", daddr, req->size);

    return No_Fault;
}

Fault
TsunamiCChip::write(MemReqPtr &req, const uint8_t *data)
{
    DPRINTF(Tsunami, "write - va=%#x size=%d \n",
            req->vaddr, req->size);

    Addr daddr = (req->paddr - (addr & PA_IMPL_MASK)) >> 6;
    uint64_t olddim;

    switch (req->size) {

      case sizeof(uint64_t):
          switch(daddr) {
              case TSDEV_CC_CSR:
                  panic("TSDEV_CC_CSR write\n");
                  return No_Fault;
              case TSDEV_CC_MTR:
                  panic("TSDEV_CC_MTR write not implemented\n");
                   return No_Fault;
              case TSDEV_CC_MISC:
                //If it is the seventh bit, clear the RTC interrupt
                if ((*(uint64_t*) data) & (1<<4)) {
                    RTCInterrupting = false;
                    tsunami->intrctrl->clear(0, TheISA::INTLEVEL_IRQ2, 0);
                    DPRINTF(Tsunami, "clearing rtc interrupt\n");
                    misc &= ~(1<<4);
                } else panic("TSDEV_CC_MISC write not implemented\n");
                  return No_Fault;
              case TSDEV_CC_AAR0:
              case TSDEV_CC_AAR1:
              case TSDEV_CC_AAR2:
              case TSDEV_CC_AAR3:
                  panic("TSDEV_CC_AARx write not implemeted\n");
                  return No_Fault;
              case TSDEV_CC_DIM0:
              case TSDEV_CC_DIM1:
              case TSDEV_CC_DIM2:
              case TSDEV_CC_DIM3:
                  int number;
                  if(daddr == TSDEV_CC_DIM0)
                      number = 0;
                  else if(daddr == TSDEV_CC_DIM1)
                      number = 1;
                  else if(daddr == TSDEV_CC_DIM2)
                      number = 2;
                  else
                      number = 3;

                  olddim = dim[number];
                  dim[number] = *(uint64_t*)data;
                  dir[number] = dim[number] & drir;
                  uint64_t bitvector;
                  for(int x = 0; x < 64; x++)
                  {
                      bitvector = 1 << x;
                      // Figure out which bits have changed
                      if ((dim[number] & bitvector) != (olddim & bitvector))
                      {
                          // The bit is now set and it wasn't before (set)
                          if((dim[number] & bitvector) && (dir[number] & bitvector))
                          {
                              tsunami->intrctrl->post(number, TheISA::INTLEVEL_IRQ1, x);
                              DPRINTF(Tsunami, "posting dir interrupt to cpu 0\n");
                          }
                          else if (!(dir[number] & bitvector))
                          {
                              // The bit was set and now its now clear and
                              // we were interrupting on that bit before
                              tsunami->intrctrl->clear(number, TheISA::INTLEVEL_IRQ1, x);
                              DPRINTF(Tsunami, "dim write resulting in clear"
                                      "dir interrupt to cpu 0\n");

                          }


                      }
                  }
                  return No_Fault;
              case TSDEV_CC_DIR0:
              case TSDEV_CC_DIR1:
              case TSDEV_CC_DIR2:
              case TSDEV_CC_DIR3:
                  panic("TSDEV_CC_DIR write not implemented\n");
              case TSDEV_CC_DRIR:
                  panic("TSDEV_CC_DRIR write not implemented\n");
              case TSDEV_CC_PRBEN:
                  panic("TSDEV_CC_PRBEN write not implemented\n");
              case TSDEV_CC_IIC0:
              case TSDEV_CC_IIC1:
              case TSDEV_CC_IIC2:
              case TSDEV_CC_IIC3:
                  panic("TSDEV_CC_IICx write not implemented\n");
              case TSDEV_CC_MPR0:
              case TSDEV_CC_MPR1:
              case TSDEV_CC_MPR2:
              case TSDEV_CC_MPR3:
                  panic("TSDEV_CC_MPRx write not implemented\n");
              default:
                  panic("default in cchip read reached, accessing 0x%x\n");
          }

      break;
      case sizeof(uint32_t):
      case sizeof(uint16_t):
      case sizeof(uint8_t):
      default:
        panic("invalid access size(?) for tsunami register!\n");
    }

    DPRINTFN("Tsunami ERROR: write daddr=%#x size=%d\n", daddr, req->size);

    return No_Fault;
}

void
TsunamiCChip::postDRIR(uint32_t interrupt)
{
    uint64_t bitvector = 0x1 << interrupt;
    drir |= bitvector;
    for(int i=0; i < Tsunami::Max_CPUs; i++) {
        dir[i] = dim[i] & drir;
        if (dim[i] & bitvector) {
                tsunami->intrctrl->post(i, TheISA::INTLEVEL_IRQ1, interrupt);
                DPRINTF(Tsunami, "posting dir interrupt to cpu %d,"
                        "interrupt %d\n",i, interrupt);
        }
    }
}

void
TsunamiCChip::clearDRIR(uint32_t interrupt)
{
    uint64_t bitvector = 0x1 << interrupt;
    if (drir & bitvector)
    {
        drir &= ~bitvector;
        for(int i=0; i < Tsunami::Max_CPUs; i++) {
            if (dir[i] & bitvector) {
                tsunami->intrctrl->clear(i, TheISA::INTLEVEL_IRQ1, interrupt);
                DPRINTF(Tsunami, "clearing dir interrupt to cpu %d,"
                    "interrupt %d\n",i, interrupt);

            }
            dir[i] = dim[i] & drir;
        }
    }
    else
        DPRINTF(Tsunami, "Spurrious clear? interrupt %d\n", interrupt);
}

void
TsunamiCChip::serialize(std::ostream &os)
{
    SERIALIZE_ARRAY(dim, Tsunami::Max_CPUs);
    SERIALIZE_ARRAY(dir, Tsunami::Max_CPUs);
    SERIALIZE_ARRAY(dirInterrupting, Tsunami::Max_CPUs);
    SERIALIZE_SCALAR(drir);
    SERIALIZE_SCALAR(misc);
    SERIALIZE_SCALAR(RTCInterrupting);
}

void
TsunamiCChip::unserialize(Checkpoint *cp, const std::string &section)
{
    UNSERIALIZE_ARRAY(dim, Tsunami::Max_CPUs);
    UNSERIALIZE_ARRAY(dir, Tsunami::Max_CPUs);
    UNSERIALIZE_ARRAY(dirInterrupting, Tsunami::Max_CPUs);
    UNSERIALIZE_SCALAR(drir);
    UNSERIALIZE_SCALAR(misc);
    UNSERIALIZE_SCALAR(RTCInterrupting);
}

BEGIN_DECLARE_SIM_OBJECT_PARAMS(TsunamiCChip)

    SimObjectParam<Tsunami *> tsunami;
    SimObjectParam<MemoryController *> mmu;
    Param<Addr> addr;

END_DECLARE_SIM_OBJECT_PARAMS(TsunamiCChip)

BEGIN_INIT_SIM_OBJECT_PARAMS(TsunamiCChip)

    INIT_PARAM(tsunami, "Tsunami"),
    INIT_PARAM(mmu, "Memory Controller"),
    INIT_PARAM(addr, "Device Address")

END_INIT_SIM_OBJECT_PARAMS(TsunamiCChip)

CREATE_SIM_OBJECT(TsunamiCChip)
{
    return new TsunamiCChip(getInstanceName(), tsunami, addr, mmu);
}

REGISTER_SIM_OBJECT("TsunamiCChip", TsunamiCChip)
