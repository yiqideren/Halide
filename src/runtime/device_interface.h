#ifndef HALIDE_DEVICE_INTERFACE_H
#define HALIDE_DEVICE_INTERFACE_H

extern "C" {

struct halide_device_interface_t {
    // These next two methods are used to reference count the runtime code
    // these function pointers point to. They should always be initialized
    // to halide_use_jit_module and halide_release_jit_module and Halide's JIT
    // arranges for this to reference count the container for the code. In AOT
    // compilation, these are empty functions which do nothing.
    void (*use_module)();
    void (*release_module)();
    int (*device_malloc)(void *user_context, struct halide_buffer_t *buf);
    int (*device_free)(void *user_context, struct halide_buffer_t *buf);
    int (*device_sync)(void *user_context, struct halide_buffer_t *buf);
    int (*device_release)(void *user_context);
    int (*copy_to_host)(void *user_context, struct halide_buffer_t *buf);
    int (*copy_to_device)(void *user_context, struct halide_buffer_t *buf);
    int (*device_and_host_malloc)(void *user_context, struct halide_buffer_t *buf);
    int (*device_and_host_free)(void *user_context, struct halide_buffer_t *buf);
};

extern WEAK int halide_default_device_and_host_malloc(void *user_context, struct halide_buffer_t *buf,
                                                      const struct halide_device_interface_t *device_interface);
extern WEAK int halide_default_device_and_host_free(void *user_context, struct halide_buffer_t *buf,
                                                    const struct halide_device_interface_t *device_interface);

}

#endif // HALIDE_DEVICE_INTERFACE_H
