Usage of Linker Script
=======================
The sdk uses the bsp/board/qcc743dk/qcc743_flash.ld file as the linker script, which defines linker-related information such as memory layout and symbol addresses. Here are the main parts of this script file:

1. The OUTPUT_ARCH("riscv") directive configures the target architecture as RISC-V, which is crucial information for the linker to determine the target system architecture.

2. ENTRY(\_\_start) specifies the entry point of the program as \_\_start. The entry point is the first function or code segment that starts executing when the program runs.

3. It defines the stack size, minimum heap size, minimum PSRAM heap size, and other constants and variables.

4. It defines information about various memory regions, including their starting addresses and lengths. Some of these regions include:

   - fw_header_memory: Region for storing firmware header data.
   - xip_memory: Region for storing code (not modified during execution).
   - ram_psram: Region for RAM and PSRAM.
   - itcm_memory and dtcm_memory: Tightly Coupled Memory (TCM) region for storing instructions and data.
   - nocache_ram_memory: RAM region without caching.
   - ram_memory: Regular RAM region.
   - ram_wifi: RAM region for the WiFi module.

5. It defines the contents and placement addresses of various program sections. For example:

   - fw_header: Firmware header data section.
   - init: Initialization code section.
   - rftlv.tool: rftlv section.
   - RAM_DATA: Data section.
   - text: Code section of the application.
   - bss: Uninitialized global variable section.
   - heap: Heap section.

6. It defines the top of the stack, stack limits, and addresses of related symbols to identify the start and end positions of different memory regions for referencing and manipulating them in the program. For example:

   - \_\_StackTop: Top of the stack, indicating the end position of the stack.
   - \_sp_main: Main stack pointer.
   - \_\_freertos_irq_stack_top: Top of the FreeRTOS interrupt stack.

7. It performs validity checks using ASSERT to ensure that the sizes of different memory regions do not exceed their allocated memory space. If the size of a memory region exceeds the limit, the linking process will fail, allowing early detection of memory allocation issues.

8. For WiFi RAM, it defines the starting address, size, and other symbols for the WiFi BSS segment and heap.
