#include "cpu_features_macros.h"

#if defined(CPU_FEATURES_ARCH_AARCH64)
#include "cpuinfo_driver.h"
#include "cpuinfo_aarch64.h"
#include "arm64_neon.h"

// https://reviews.llvm.org/D53115
#define SYS_REG(op0, op1, crn, crm, op2)                       \
  (((op0 & 1) << 14) | ((op1 & 7) << 11) | ((crn & 15) << 7) | \
   ((crm & 15) << 3) | ((op2 & 7) << 0))

#define SYS_MIDR_EL1 SYS_REG(3, 0, 0, 0, 0)

void GetCpuInfo(PVOID user_buffer) {
  UINT64 midr = _ReadStatusReg(SYS_MIDR_EL1);
  memcpy((PUINT64)user_buffer, &midr, sizeof(UINT64));

}
#endif // defined(CPU_FEATURES_ARCH_AARCH64)
