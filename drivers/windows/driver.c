#include <ntddk.h>
#include "cpuinfo_driver.h"
#include "cpu_features_driver_macros.h"

void CpuFeaturesUnload(PDRIVER_OBJECT driver_object) {
  KdPrint(("cpu_features driver unload\n"));

  UNICODE_STRING symbolic_link_name = RTL_CONSTANT_STRING(CPU_FEATURES_DRIVER_SYMBOLIK_LINK_NAME);

  // Delete symbolic link.
  const NTSTATUS delete_symbolic_link_status =
      IoDeleteSymbolicLink(&symbolic_link_name);
  if (!NT_SUCCESS(delete_symbolic_link_status)) {
    KdPrint(
        ("Failed to delete symbolink link cpu_features_arm64. Status: "
         "(0x%08X)\n",
         delete_symbolic_link_status));
  }
  IoDeleteDevice(driver_object->DeviceObject);
}

NTSTATUS CpuFeaturesDispatchCreateClose(PDEVICE_OBJECT driver_object, PIRP irp) {
  UNREFERENCED_PARAMETER(driver_object);
  UNREFERENCED_PARAMETER(irp);
  return STATUS_SUCCESS;
}

NTSTATUS CompleteIrp(PIRP irp, NTSTATUS status, ULONG_PTR info) {
  irp->IoStatus.Status = status;
  irp->IoStatus.Information = info;
  IoCompleteRequest(irp, 0);
  return status;
}

NTSTATUS CpuFeaturesDispatchRead(PDEVICE_OBJECT device_object, PIRP irp) {
  UNREFERENCED_PARAMETER(device_object);
  const PIO_STACK_LOCATION pIoStackLocation = IoGetCurrentIrpStackLocation(irp);
  const ULONG request_length = pIoStackLocation->Parameters.Read.Length;
  if (request_length == 0) {
    return CompleteIrp(irp, STATUS_INVALID_BUFFER_SIZE, 0);
  }

  const PVOID user_buffer = MmGetSystemAddressForMdlSafe(irp->MdlAddress, NormalPagePriority);
  if (!user_buffer) {
    return CompleteIrp(irp, STATUS_INVALID_BUFFER_SIZE, 0);
  }

  GetCpuInfo(user_buffer);

  return CompleteIrp(irp, STATUS_SUCCESS, request_length);
}

UINT64 ExtractBitRange(const UINT64 reg, const UINT64 msb, const UINT64 lsb) {
  const UINT64 bits = msb - lsb + 1ULL;
  const UINT64 mask = (1ULL << bits) - 1ULL;
  return reg >> lsb & mask;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver_object,
                     PUNICODE_STRING registry_path) {
  UNREFERENCED_PARAMETER(registry_path);

  // Set an unload routine.
  driver_object->DriverUnload = CpuFeaturesUnload;

  // Set dispatch routines the driver support.
  driver_object->MajorFunction[IRP_MJ_CREATE] = CpuFeaturesDispatchCreateClose;
  driver_object->MajorFunction[IRP_MJ_CLOSE] = CpuFeaturesDispatchCreateClose;
  driver_object->MajorFunction[IRP_MJ_READ] = CpuFeaturesDispatchRead;

  UNICODE_STRING dev_name = RTL_CONSTANT_STRING(CPU_FEATURES_DRIVER_DEVICE_NAME);
  UNICODE_STRING sym_link = RTL_CONSTANT_STRING(CPU_FEATURES_DRIVER_SYMBOLIK_LINK_NAME);
  PDEVICE_OBJECT device_object = NULL;
  NTSTATUS status = STATUS_SUCCESS;
  BOOLEAN sym_link_created = FALSE;

// just midr output
//#if defined(_M_ARM64)
//  KdPrint(("cpu_features aarch64 working!!!\n"));
//  UINT64 value;
//  GetCpuInfo(&value);
//  KdPrint(("%d\n", value));
//  UINT64 impl = ExtractBitRange(value, 31, 24);
//  UINT64 variant = ExtractBitRange(value, 23, 20);
//  UINT64 part = ExtractBitRange(value, 15, 4);
//  UINT64 revision = ExtractBitRange(value, 3, 0);
//  KdPrint(("part:	   %d\n", part));
//  KdPrint(("variant:     %d\n", variant));
//  KdPrint(("revision:    %d\n", revision));
//  KdPrint(("implementer: %d\n", impl));
//#endif

  do {
    status = IoCreateDevice(driver_object, 0, &dev_name, FILE_DEVICE_UNKNOWN, 0,
                            FALSE, &device_object);
    if (!NT_SUCCESS(status)) {
      KdPrint(("failed to create device (0x%08X)\n", status));
      break;
    }
    device_object->Flags |= DO_DIRECT_IO;

    status = IoCreateSymbolicLink(&sym_link, &dev_name);
    if (!NT_SUCCESS(status)) {
      KdPrint(("failed to create symbolic link (0x%08X)\n", status));
      break;
    }
    sym_link_created = TRUE;

  } while (FALSE);

  if (!NT_SUCCESS(status)) {
    if (sym_link_created) IoDeleteSymbolicLink(&sym_link);
    if (device_object) IoDeleteDevice(device_object);
  }

  return status;
}