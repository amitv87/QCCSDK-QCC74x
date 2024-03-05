Usage of bugkiller
=======================

1. Exception Output Information

   Exceptions will print the following information:

   - Exception Entry
   - mcause: Indicates the cause of the exception.
   - mepc: Address where the exception occurred.
   - mtval: Value of the exception, which is related to the type of the exception.

   .. figure:: img/coredump1.png
      :align: center
      :scale: 100%

2. Coredump Log File Format

   The line with the word QCC74x COREDUMP marks the beginning of the coredump. The content of the coredump is composed of segments. Each segment is composed of a DATA DEBIN line and an END line, which contains crc check information, segment start information, segment length information, and segment name.

3. The Process of Parsing Coredump (Format Check)

   .. code-block:: text

      dump_log=/mnt/ybwang/066-mcu-0926.log
      eval `file -i $dump_log |sed "s/ /\n/g" |grep charset`
      iconv -c -f $charset -t utf8 $dump_log >new.log
      dos2unix new.log
      file new.log

      # please manually delete ascii control character
      perl -ne 'print "$. $_" if /[\x00-\x09\x0b-\x0c\x0e-\x1f\x7f-\xff]/' new.log |hexdump

   .. code-block:: c

      /tmp/bugkiller_dump -bin bin.log -log Serial-COM1720230810_111222890.log.bak

   The bugkiller_dump command can parse coredump log files and convert them into binary format. The command has two parameters, bin specifies the output file, and log specifies the input log file.

4. Launch Bugkiller Using Coredump Binary

   .. code-block:: c

      /tmp/bugkiller -d bin.log -e qcc743_demo_sdiowifi.elf

   Input the binary coredump and elf files into bugkiller, start the gdb server and riscv simulator, the default port of gdb server is 6000

5. Start gdb client and Connect to server

   .. code-block:: c

      riscv64-unknown-elf-gdb -se qcc743_demo_sdiowifi.elf

   Use info registers to observe the current context and find that you need to restore to the context where the exception occurred

   .. figure:: img/coredump5.png
      :align: center
      :scale: 100%

6. Debug

   Run the following command:

   .. code-block:: text

      set $gp=&__global_pointer$
      set $sp=0xe0000000+0xfffff
      set $originPC = *pxCurrentTCB->pxTopOfStack
      set *pxCurrentTCB->pxTopOfStack = 0
      call (void (*)(void))processed_source()
      set *pxCurrentTCB->pxTopOfStack = $originPC
      set $pc=$originPC-4
      bt

7. Switch to Another Process Context

   In the previous step, we restored the context of the abnormal task. Sometimes we may need to restore the context of other tasks. Use the following method:

   This method receives 1 parameter, the parameter type is list, and prints the name and tcb address of the task in the list.

   .. code-block:: text

      define task_list
        set $item = $arg0.xListEnd.pxPrevious
        while ($item != &$arg0.xListEnd)
          p $item.pvOwner
          p ((TCB_t *)$item.pvOwner)->pcTaskName
          set $item = $item.pxPrevious
        end
      end

   .. figure:: img/coredump7.png
      :align: center
      :scale: 100%

   The set command sets pxCurrentTCB (set pxCurrentTCB = 0x2302240f0), and then re-performs the context recovery operation (previous step).

   Using the bt command, it can be seen that the context has been switched to the ez_mcu_ircut_ta process.

8. Traverse All Processes on the os

   .. code-block:: text

      # READY status:
      define ready_task_list
        set $i = 0
        while ($i < 32)
          task_list pxReadyTasksLists[$i]
          set $i = $i + 1
        end
      end
      ready_task_list

      # Delay status:
      task_list xDelayedTaskList1
      task_list xDelayedTaskList2

      # PendingReady status:
      task_list xPendingReadyList

      # Suspend status:
      task_list xSuspendedTaskList

9. gdb Debugging Command

   The p command, set command, bt command, and define command are introduced above.

   In addition to these, the following commands are also supported:

   The f command (guaranteed `f 0` before `set $sp`!!!) switches the current stack frame. After the switch, local variables can be accessed, such as p wait_time. This local variable can only be accessed after switching the stack frame.

   ptype command can print the type of variables.

   .. figure:: img/coredump10.png
      :align: center
      :scale: 100%

   With the disassemble command, users can view the instructions of the function.
