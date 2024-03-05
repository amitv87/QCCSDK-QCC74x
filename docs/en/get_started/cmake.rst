Usage of CMake Framework
================================

This section mainly introduces how to use the CMake framework. qcc74x SDK encapsulates the following function interfaces for users, which basically meet the needs of common scenarios.

.. list-table::
    :widths: 40 60
    :header-rows: 1

    * - Function
      - Description
    * - sdk_generate_library
      - Generate a library. If the function has no formal parameters, the name of the directory where the current library is located will be used
    * - sdk_library_add_sources
      - Add source files to the library
    * - sdk_library_add_sources_ifdef
      - Add source files for the library (satisfying the if condition)
    * - sdk_add_include_directories
      - Add header file path
    * - sdk_add_include_directories_ifdef
      - Add header file path (satisfies if condition)
    * - sdk_add_compile_definitions
      - Add macro definition without -D
    * - sdk_add_compile_definitions_ifdef
      - Add macro definition (satisfy if condition)
    * - sdk_add_compile_options
      - Add compile options
    * - sdk_add_compile_options_ifdef
      - Add compilation options (satisfy if condition)
    * - sdk_add_link_options
      - Add link options
    * - sdk_add_link_options_ifdef
      - Add link options (satisfies if condition)
    * - sdk_add_link_libraries
      - Add static library
    * - sdk_add_link_libraries_ifdef
      - Add static library (satisfies if condition)
    * - sdk_add_subdirectory_ifdef
      - Compile cmakelist in the subdirectory (satisfies the if condition)
    * - sdk_add_static_library
      - Add external static library
    * - sdk_set_linker_script
      - Set up link script
    * - sdk_set_main_file
      - Set the file where the main function is located
    * - project
      - Project compilation
    * - target_source(app PRIVATE xxx)
      - Add source files to the app library. When users need to add some source files themselves but do not want to create a cmakelist and compile them into a library separately, this item can be used

New Project
----------------

Users can copy a project, such as ``helloworld``, and modify the ``SDK_DEMO_PATH`` variable to the path you use to create a new project.

Add Components to the Project
--------------------------------------

If you need to compile related components, such as FATFS and LVGL, you need to add the component enablement in the ``proj.conf`` file, for example:

.. code-block:: c

    set(CONFIG_FATFS 1)
    set(CONFIG_LVGL 1)

Enable Conditional Compilation
--------------------------------------

User-defined cmake conditional compilation items (**The if syntax of cmake is used**), or using sdk functions ending with ``ifdef``, are enabled in the same way as above
