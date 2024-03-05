QCC74xLOG
=============

Introduction
---------------

- QCC74xLOG is a multi-threaded logging library with simple porting and diverse functions
- It has two working modes: synchronous and asynchronous. If the buffer overflows in the asynchronous mode, the earliest record will be completely deleted
- Overall three parts, recorder, directed output, formatter
- A recorder can output in multiple directions, and can simultaneously output to buffers, IO peripherals, files, file size division, and file time division
- Directed to file output, supports setting the retention quantity, supports dividing by file size, and supports dividing by time
- Each directional output can independently set the formatter, output level, TAG label filtering, output mode, and color
- The formatter supports simple formats, user-defined formats, YAML format (under implementation), and CSV format (under planning)
- Six levels of log level control, FATAL, ERROR, WARNING, INFO, DEBUG, TRACE
- Supports level, TAG label, function, line number, file name, TICK number, TIME, and thread information output
- Supports level and TAG label filtering functions
- Unnecessary levels, functions, and labels can be trimmed to reduce code size

Configure QCC74xLOG Functions
------------------------------

If the user need to configure the related functions of QCC74xLOG, they need to add the corresponding code to the ``proj.conf`` file in the corresponding project directory, for example:

The following is a ``proj.conf`` configuration that does not support file output

.. code-block:: cmake
    :linenos:

    # Enable QCC74xLOG
    set(CONFIG_QCC74xLOG 1)
    # Enable parameter checking, can be disabled
    set(CONFIG_QCC74xLOG_DEBUG 1)

In addition, adding the following configuration in the `proj.conf` configuration can use the ``qcc74xlog_user.h`` configuration file. The default configuration file is ``qcc74xlog_default.h``

.. code-block:: cmake
    :linenos:

    # Enable QCC74xLOG_USER configuration file
    set(CONFIG_QCC74xLOG_USER 1)

Macros
------------

QCC74xLOG_CSI
^^^^^^^^^^^^^^^^^^^^

- Control Sequence Introducer
- For control terminal

.. code-block:: c
   :linenos:

    #define QCC74xLOG_CSI_START "\033["
    #define QCC74xLOG_CSI_CUU   "A"
    #define QCC74xLOG_CSI_CUD   "B"
    #define QCC74xLOG_CSI_CUF   "C"
    #define QCC74xLOG_CSI_CUB   "D"
    #define QCC74xLOG_CSI_CNL   "E"
    #define QCC74xLOG_CSI_CPL   "F"
    #define QCC74xLOG_CSI_CHA   "G"
    #define QCC74xLOG_CSI_CUP   "H"
    #define QCC74xLOG_CSI_ED    "J"
    #define QCC74xLOG_CSI_EL    "K"
    #define QCC74xLOG_CSI_SU    "S"
    #define QCC74xLOG_CSI_SD    "T"
    #define QCC74xLOG_CSI_SGR   "m"

QCC74xLOG_SGR
^^^^^^^^^^^^^^^^^^^^

- Select Graphic Rendition
- For text graphics

.. code-block:: c
   :linenos:

    #define QCC74xLOG_SGR_RESET      "0"
    #define QCC74xLOG_SGR_BOLD       "1"
    #define QCC74xLOG_SGR_FAINT      "2"
    #define QCC74xLOG_SGR_ITALICS    "3"
    #define QCC74xLOG_SGR_UNDERLINE  "4"
    #define QCC74xLOG_SGR_BLINKS     "5"
    #define QCC74xLOG_SGR_BLINKR     "6"
    #define QCC74xLOG_SGR_REVERSE    "7"
    #define QCC74xLOG_SGR_HIDE       "8"
    #define QCC74xLOG_SGR_STRIKE     "9"
    #define QCC74xLOG_SGR_NORMAL     "22"
    #define QCC74xLOG_SGR_FG_BLACK   "30"
    #define QCC74xLOG_SGR_FG_RED     "31"
    #define QCC74xLOG_SGR_FG_GREEN   "32"
    #define QCC74xLOG_SGR_FG_YELLOW  "33"
    #define QCC74xLOG_SGR_FG_BLUE    "34"
    #define QCC74xLOG_SGR_FG_MAGENTA "35"
    #define QCC74xLOG_SGR_FG_CYAN    "36"
    #define QCC74xLOG_SGR_FG_WHITE   "37"
    #define QCC74xLOG_SGR_BG_BLACK   "40"
    #define QCC74xLOG_SGR_BG_RED     "41"
    #define QCC74xLOG_SGR_BG_GREEN   "42"
    #define QCC74xLOG_SGR_BG_YELLOW  "43"
    #define QCC74xLOG_SGR_BG_BLUE    "44"
    #define QCC74xLOG_SGR_BG_MAGENTA "45"
    #define QCC74xLOG_SGR_BG_CYAN    "46"
    #define QCC74xLOG_SGR_BG_WHITE   "47"

QCC74xLOG_COLOR
^^^^^^^^^^^^^^^^^^^^

- A range of colors for configuration use

.. code-block:: c
   :linenos:

    #define QCC74xLOG_COLOR_START QCC74xLOG_CSI_START
    #define QCC74xLOG_COLOR_END   QCC74xLOG_CSI_SGR
    #define QCC74xLOG_CLOLR_SEP   ";"
    #define QCC74xLOG_COLOR_DEFAULT
    #define QCC74xLOG_COLOR_RESET QCC74xLOG_SGR_RESET QCC74xLOG_CLOLR_SEP
    #define QCC74xLOG_COLOR_FG_NONE
    #define QCC74xLOG_COLOR_FG_BLACK   QCC74xLOG_SGR_FG_BLACK QCC74xLOG_CLOLR_SEP
    #define QCC74xLOG_COLOR_FG_RED     QCC74xLOG_SGR_FG_RED QCC74xLOG_CLOLR_SEP
    #define QCC74xLOG_COLOR_FG_GREEN   QCC74xLOG_SGR_FG_GREEN QCC74xLOG_CLOLR_SEP
    #define QCC74xLOG_COLOR_FG_YELLOW  QCC74xLOG_SGR_FG_YELLOW QCC74xLOG_CLOLR_SEP
    #define QCC74xLOG_COLOR_FG_BLUE    QCC74xLOG_SGR_FG_BLUE QCC74xLOG_CLOLR_SEP
    #define QCC74xLOG_COLOR_FG_MAGENTA QCC74xLOG_SGR_FG_MAGENTA QCC74xLOG_CLOLR_SEP
    #define QCC74xLOG_COLOR_FG_CYAN    QCC74xLOG_SGR_FG_CYAN QCC74xLOG_CLOLR_SEP
    #define QCC74xLOG_COLOR_FG_WHITE   QCC74xLOG_SGR_FG_WHITE QCC74xLOG_CLOLR_SEP
    #define QCC74xLOG_COLOR_BG_NONE
    #define QCC74xLOG_COLOR_BG_BLACK   QCC74xLOG_SGR_BG_BLACK QCC74xLOG_CLOLR_SEP
    #define QCC74xLOG_COLOR_BG_RED     QCC74xLOG_SGR_BG_RED QCC74xLOG_CLOLR_SEP
    #define QCC74xLOG_COLOR_BG_GREEN   QCC74xLOG_SGR_BG_GREEN QCC74xLOG_CLOLR_SEP
    #define QCC74xLOG_COLOR_BG_YELLOW  QCC74xLOG_SGR_BG_YELLOW QCC74xLOG_CLOLR_SEP
    #define QCC74xLOG_COLOR_BG_BLUE    QCC74xLOG_SGR_BG_BLUE QCC74xLOG_CLOLR_SEP
    #define QCC74xLOG_COLOR_BG_MAGENTA QCC74xLOG_SGR_BG_MAGENTA QCC74xLOG_CLOLR_SEP
    #define QCC74xLOG_COLOR_BG_CYAN    QCC74xLOG_SGR_BG_CYAN QCC74xLOG_CLOLR_SEP
    #define QCC74xLOG_COLOR_BG_WHITE   QCC74xLOG_SGR_BG_WHITE QCC74xLOG_CLOLR_SEP

QCC74xLOG_COLOR_CONTROL
^^^^^^^^^^^^^^^^^^^^^^^^^

- Default configured LOG colors for each level

.. code-block:: c
   :linenos:

    #ifndef QCC74xLOG_COLOR_FATAL
    #define QCC74xLOG_COLOR_FATAL QCC74xLOG_COLOR_FG_MAGENTA QCC74xLOG_COLOR_BG_NONE QCC74xLOG_SGR_BLINKS
    #endif

    #ifndef QCC74xLOG_COLOR_ERROR
    #define QCC74xLOG_COLOR_ERROR QCC74xLOG_COLOR_FG_RED QCC74xLOG_COLOR_BG_NONE QCC74xLOG_SGR_NORMAL
    #endif

    #ifndef QCC74xLOG_COLOR_WARN
    #define QCC74xLOG_COLOR_WARN QCC74xLOG_COLOR_FG_YELLOW QCC74xLOG_COLOR_BG_NONE QCC74xLOG_SGR_NORMAL
    #endif

    #ifndef QCC74xLOG_COLOR_INFO
    #define QCC74xLOG_COLOR_INFO QCC74xLOG_COLOR_FG_NONE QCC74xLOG_COLOR_BG_NONE QCC74xLOG_SGR_RESET
    #endif

    #ifndef QCC74xLOG_COLOR_DEBUG
    #define QCC74xLOG_COLOR_DEBUG QCC74xLOG_COLOR_FG_WHITE QCC74xLOG_COLOR_BG_NONE QCC74xLOG_SGR_NORMAL
    #endif

    #ifndef QCC74xLOG_COLOR_TRACE
    #define QCC74xLOG_COLOR_TRACE QCC74xLOG_COLOR_FG_WHITE QCC74xLOG_COLOR_BG_NONE QCC74xLOG_SGR_FAINT
    #endif

QCC74xLOG_LEVEL_STRING
^^^^^^^^^^^^^^^^^^^^^^^^^^^

- Default configuration prompt information for each level

.. code-block:: c
   :linenos:

    #ifndef QCC74xLOG_LEVEL_FATAL_STRING
    #define QCC74xLOG_LEVEL_FATAL_STRING "FATL"
    #endif

    #ifndef QCC74xLOG_LEVEL_ERROR_STRING
    #define QCC74xLOG_LEVEL_ERROR_STRING "ERRO"
    #endif

    #ifndef QCC74xLOG_LEVEL_WARN_STRING
    #define QCC74xLOG_LEVEL_WARN_STRING "WARN"
    #endif

    #ifndef QCC74xLOG_LEVEL_INFO_STRING
    #define QCC74xLOG_LEVEL_INFO_STRING "INFO"
    #endif

    #ifndef QCC74xLOG_LEVEL_DEBUG_STRING
    #define QCC74xLOG_LEVEL_DEBUG_STRING "DBUG"
    #endif

    #ifndef QCC74xLOG_LEVEL_TRACE_STRING
    #define QCC74xLOG_LEVEL_TRACE_STRING "TRAC"
    #endif

QCC74xLOG_LEVEL
^^^^^^^^^^^^^^^^^^^^

- LOG level for configuring recorder and direct

.. code-block:: c
   :linenos:

    #define QCC74xLOG_LEVEL_FATAL 0x00 /*!< level fatal, create a panic */
    #define QCC74xLOG_LEVEL_ERROR 0x01 /*!< level error                 */
    #define QCC74xLOG_LEVEL_WARN  0x02 /*!< level warning               */
    #define QCC74xLOG_LEVEL_INFO  0x03 /*!< level information           */
    #define QCC74xLOG_LEVEL_DEBUG 0x04 /*!< level debug                 */
    #define QCC74xLOG_LEVEL_TRACE 0x05 /*!< level trace information     */


QCC74xLOG_FLAG
^^^^^^^^^^^^^^^^^^^^

- Functions for configuring recorder and direct

.. code-block:: c
   :linenos:

    #define QCC74xLOG_FLAG_LEVEL  ((uint8_t)0x01) /*!< supported print level     */
    #define QCC74xLOG_FLAG_TAG    ((uint8_t)0x02) /*!< supported record tag      */
    #define QCC74xLOG_FLAG_FUNC   ((uint8_t)0x04) /*!< supported record function */
    #define QCC74xLOG_FLAG_LINE   ((uint8_t)0x08) /*!< supported record line     */
    #define QCC74xLOG_FLAG_FILE   ((uint8_t)0x10) /*!< supported record file     */
    #define QCC74xLOG_FLAG_CLK    ((uint8_t)0x20) /*!< supported record clock    */
    #define QCC74xLOG_FLAG_TIME   ((uint8_t)0x40) /*!< supported record time     */
    #define QCC74xLOG_FLAG_THREAD ((uint8_t)0x80) /*!< supported record thread   */


QCC74xLOG_MODE
^^^^^^^^^^^^^^^^^^^^

- Mode used to configure the recorder

.. code-block:: c
   :linenos:

    #define QCC74xLOG_MODE_SYNC  ((uint8_t)0x00)
    #define QCC74xLOG_MODE_ASYNC ((uint8_t)0x01)

QCC74xLOG_COMMAND
^^^^^^^^^^^^^^^^^^^^

- Configuration command, used for the second parameter of qcc74xlog_control

.. code-block:: c
   :linenos:

    #define QCC74xLOG_CMD_FLAG           ((uint32_t)0x01)
    #define QCC74xLOG_CMD_LEVEL          ((uint32_t)0x02)
    #define QCC74xLOG_CMD_QUEUE_POOL     ((uint32_t)0x03)
    #define QCC74xLOG_CMD_QUEUE_SIZE     ((uint32_t)0x04)
    #define QCC74xLOG_CMD_QUEUE_RST      ((uint32_t)0x05)
    #define QCC74xLOG_CMD_ENTER_CRITICAL ((uint32_t)0x06)
    #define QCC74xLOG_CMD_EXIT_CRITICAL  ((uint32_t)0x07)
    #define QCC74xLOG_CMD_FLUSH_NOTICE   ((uint32_t)0x08)
    #define QCC74xLOG_CMD_MODE           ((uint32_t)0x09)

QCC74xLOG_DIRECT_COMMAND
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- Configuration command, used for the second parameter of qcc74xlog_direct_control

.. code-block:: c
   :linenos:

    #define QCC74xLOG_DIRECT_CMD_ILLEGAL ((uint32_t)0x00)
    #define QCC74xLOG_DIRECT_CMD_LEVEL   ((uint32_t)0x02)
    #define QCC74xLOG_DIRECT_CMD_LOCK    ((uint32_t)0x06)
    #define QCC74xLOG_DIRECT_CMD_UNLOCK  ((uint32_t)0x07)
    #define QCC74xLOG_DIRECT_CMD_COLOR   ((uint32_t)0x0A)

QCC74xLOG_DIRECT_TYPE
^^^^^^^^^^^^^^^^^^^^^^^^^^^

- The direct type to be created, used in the second parameter of qcc74xlog_direct_create

.. code-block:: c
   :linenos:

    #define QCC74xLOG_DIRECT_TYPE_BUFFER    ((uint8_t)0x01)
    #define QCC74xLOG_DIRECT_TYPE_STREAM    ((uint8_t)0x02)
    #define QCC74xLOG_DIRECT_TYPE_FILE      ((uint8_t)0x03)
    #define QCC74xLOG_DIRECT_TYPE_FILE_TIME ((uint8_t)0x04)
    #define QCC74xLOG_DIRECT_TYPE_FILE_SIZE ((uint8_t)0x05)

QCC74xLOG_DIRECT_COLOR
^^^^^^^^^^^^^^^^^^^^^^^^^

- Whether to enable color output, used in the third parameter of qcc74xlog_direct_create

.. code-block:: c
   :linenos:

    #define QCC74xLOG_DIRECT_COLOR_DISABLE ((uint8_t)0)
    #define QCC74xLOG_DIRECT_COLOR_ENABLE  ((uint8_t)1)

QCC74xLOG_LAYOUT_TYPE
^^^^^^^^^^^^^^^^^^^^^^^^^^

- The layout type to be created, used in the second parameter of qcc74xlog_layout_create

.. code-block:: c
   :linenos:

    #define QCC74xLOG_LAYOUT_TYPE_SIMPLE ((uint8_t)0)
    #define QCC74xLOG_LAYOUT_TYPE_FORMAT ((uint8_t)1)
    #define QCC74xLOG_LAYOUT_TYPE_YAML   ((uint8_t)2)

Port Functions
------------------------

Port the interface functions to be implemented

qcc74xlog_clock
^^^^^^^^^^^^^^^^^^^^

Get the current cpu clock number

.. code-block:: c
   :linenos:

    uint64_t qcc74xlog_clock(void);

qcc74xlog_time
^^^^^^^^^^^^^^^^^^^^

Get the current UTC timestamp

.. code-block:: c
   :linenos:

    uint32_t qcc74xlog_time(void);

qcc74xlog_thread
^^^^^^^^^^^^^^^^^^^^

Get the current thread name

.. code-block:: c
   :linenos:

    char *qcc74xlog_thread(void);

Global Functions
-----------------

qcc74xlog_global_filter
^^^^^^^^^^^^^^^^^^^^^^^^^^

Used to globally switch the tag filter, which will affect all recorders and directs.

.. code-block:: c
   :linenos:

    int qcc74xlog_global_filter(void *tag_string, uint8_t enable);

.. list-table::
    :widths: 10 10
    :header-rows: 1

    * - parameter
      - description
    * - tag_string
      - pointer to label string
    * - enable
      - whether to enable

.. code-block:: c
   :linenos:

    qcc74xlog_global_filter("YOURTAG", true);
    qcc74xlog_global_filter("YOURTAG", false);


Recorder Functions
----------------------

The recorder is responsible for collecting logs and has four states: illegal, ready, running, and suspend.
Logs can be collected in the running state, and configured in the ready and suspend states.
Except for level configuration operations, other configuration operations must be under ready and suspend.

qcc74xlog_create
^^^^^^^^^^^^^^^^^^^^

- To create a recorder, you need to define a qcc74xlog_t structure and pass its pointer in, and define a memory array for swapping.
- Returns 0 on success, -1 on failure

.. code-block:: c
   :linenos:

    int qcc74xlog_create(qcc74xlog_t *log, void *pool, uint16_t size, uint8_t mode);

.. list-table::
    :widths: 10 10
    :header-rows: 1

    * - parameter
      - description
    * - log
      - recorder pointer
    * - pool
      - array for buffering
    * - size
      - user buffered array size
    * - mode
      - QCC74xLOG_MODE

.. code-block:: c
   :linenos:

    #include "qcc74xlog.h"

    #define EXAMPLE_LOG_POOL_SIZE 4096

    qcc74xlog_t example_recorder;
    static uint32_t example_pool[EXAMPLE_LOG_POOL_SIZE / 4];

    /*!< Create a logger, configure the memory pool, memory pool size, and the mode is synchronization */
    if (0 != qcc74xlog_create((void *)&example_recorder, example_pool, EXAMPLE_LOG_POOL_SIZE, QCC74xLOG_MODE_SYNC)) {
        printf("qcc74xlog_create faild\r\n");
    }

qcc74xlog_delete
^^^^^^^^^^^^^^^^^^^^

- Delete a recorder
- In ready and suspended states
- Thread safety
- Returns 0 on success, -1 on failure

.. code-block:: c
   :linenos:

    int qcc74xlog_delete(qcc74xlog_t *log);

.. list-table::
    :widths: 10 10
    :header-rows: 1

    * - parameter
      - description
    * - log
      - recorder pointer

qcc74xlog_append
^^^^^^^^^^^^^^^^^^^^

- Add a direct to this recorder. Multiple directs can be added, but a direct can only be added to one recorder.
- In ready and suspended states
- Thread safety
- Returns 0 on success, -1 on failure

.. code-block:: c
   :linenos:

    int qcc74xlog_append(qcc74xlog_t *log, qcc74xlog_direct_t *direct);

.. list-table::
    :widths: 10 10
    :header-rows: 1

    * - parameter
      - description
    * - log
      - recorder pointer
    * - direct
      - direct pointer

qcc74xlog_remove
^^^^^^^^^^^^^^^^^^^^

- Remove a direct from the recorder
- In ready and suspended states
- Thread safety
- Returns 0 on success, -1 on failure

.. code-block:: c
   :linenos:

    int qcc74xlog_remove(qcc74xlog_t *log, qcc74xlog_direct_t *direct);

.. list-table::
    :widths: 10 10
    :header-rows: 1

    * - parameter
      - description
    * - log
      - recorder pointer
    * - direct
      - direct pointer

qcc74xlog_suspend
^^^^^^^^^^^^^^^^^^^^

- Suspend a recorder
- In ready, running, suspend state
- Thread safety
- Returns 0 on success, -1 on failure

.. code-block:: c
   :linenos:

    int qcc74xlog_suspend(qcc74xlog_t *log);

.. list-table::
    :widths: 10 10
    :header-rows: 1

    * - parameter
      - description
    * - log
      - recorder pointer

qcc74xlog_resume
^^^^^^^^^^^^^^^^^^^^

- Restore a recorder
- In ready, running, suspend state
- Thread safety
- Returns 0 on success, -1 on failure

.. code-block:: c
   :linenos:

    int qcc74xlog_resume(qcc74xlog_t *log);

.. list-table::
    :widths: 10 10
    :header-rows: 1

    * - parameter
      - description
    * - log
      - recorder pointer

qcc74xlog_control
^^^^^^^^^^^^^^^^^^^^

- Configure a recorder
- In the ready and suspended states; when the command is QCC74xLOG_CMD_LEVEL, it can be in the running state
- Thread safety
- Returns 0 on success, -1 on failure

.. code-block:: c
   :linenos:

    int qcc74xlog_control(qcc74xlog_t *log, uint32_t command, uint32_t param);

.. list-table::
    :widths: 10 10
    :header-rows: 1

    * - parameter
      - description
    * - log
      - recorder pointer
    * - command
      - configuration commands
    * - param
      - configuration parameters

``command`` ``param`` can fill in the following parameters:

.. list-table::
    :header-rows: 1

    * - command
      - param
      - description
    * - QCC74xLOG_CMD_FLAG
      - QCC74xLOG_FLAG
      - Configure recorder recording function
    * - QCC74xLOG_CMD_LEVEL
      - QCC74xLOG_LEVEL
      - Configure the level of recorder recording
    * - QCC74xLOG_CMD_QUEUE_POOL
      - Memory pool address
      - Configure the memory pool address used for buffering
    * - QCC74xLOG_CMD_QUEUE_SIZE
      - Memory pool size (byte)
      - Configure the memory pool size used for buffering
    * - QCC74xLOG_CMD_QUEUE_RST
      - NULL
      - Reset the buffer and clear all contents of the buffer
    * - QCC74xLOG_CMD_ENTER_CRITICAL
      - int (* enter_critical)(void) function pointer
      - Configure the function that enters the critical section
    * - QCC74xLOG_CMD_EXIT_CRITICAL
      - int (* exit_critical)(void) function pointer
      - Configure the function to exit the critical section
    * - QCC74xLOG_CMD_FLUSH_NOTICE
      - int (* flush_notice)(void) function pointer
      - Configure the clear buffer prompt function for multi-threaded flush thread work
    * - QCC74xLOG_CMD_MODE
      - QCC74xLOG_MODE
      - Configure recorder working mode


qcc74xlog_filter
^^^^^^^^^^^^^^^^^^^^

- Configure the tag filter and configure whether to enable the log output corresponding to the tag_string.
- In ready, suspended, running state
- Thread safety
- Returns 0 on success, -1 on failure

.. code-block:: c
   :linenos:

    int qcc74xlog_filter(qcc74xlog_t *log, void *tag_string, uint8_t enable);

.. list-table::
    :widths: 10 10
    :header-rows: 1

    * - parameter
      - description
    * - log
      - recorder pointer
    * - tag_string
      - tag string
    * - enable
      - whether to enable

qcc74xlog
^^^^^^^^^^^^^^^^^^^^

- Add log information to the recorder, sync mode will directly call qcc74xlog_flush, async mode will call the configured flush_notice function
- in running state
- Thread safety
- Returns 0 on success, -1 on failure

.. code-block:: c
   :linenos:

    int qcc74xlog(void *log, uint8_t level, void *tag, const char *const file, const char *const func, const long line, const char *format, ...);

.. list-table::
    :widths: 10 10
    :header-rows: 1

    * - parameter
      - description
    * - log
      - recorder pointer
    * - level
      - The level of this log
    * - tag
      - The tag structure pointer of this log
    * - file
      - The file name string pointer of this log
    * - func
      - The function name string pointer of this log
    * - line
      - The line number of this log
    * - format
      - The format string of this log
    * - ...
      - Variable length parameter list of this log

qcc74xlog_flush
^^^^^^^^^^^^^^^^^^^^

- Output all log information stored in the queue to all directs configured by the recorder
- in running state
- Thread safety
- Returns 0 on success, -1 on failure

.. code-block:: c
   :linenos:

    int qcc74xlog_flush(void *log);

.. list-table::
    :widths: 10 10
    :header-rows: 1

    * - parameter
      - description
    * - log
      - recorder pointer


