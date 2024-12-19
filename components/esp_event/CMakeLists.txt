sdk_generate_library()
sdk_library_add_sources(
default_event_loop.c
esp_event.c
esp_event_private.c
)

sdk_add_include_directories(include)
sdk_add_private_include_directories(private_include)
