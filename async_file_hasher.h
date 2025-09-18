#ifndef ASYNC_FILE_HASHER_H
#define ASYNC_FILE_HASHER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Calculate MD5 hash of a file synchronously
 * @param file_path Path to the file
 * @return MD5 hash as a string (must be freed with free_string), or NULL on error
 * @note This function is unsafe - file_path must be a valid pointer
 */
char* calculate_md5_hash_sync_c(const char* file_path);

/**
 * Free a string returned by the library
 * @param s String to free
 * @note This function is unsafe - s must be a valid pointer or NULL
 */
void free_string(char* s);

/**
 * Callback type for async MD5 calculation
 * @param hash MD5 hash result (or NULL on error)
 * @param user_data User data passed to the async function
 */
typedef void (*async_callback_t)(char* hash, void* user_data);

/**
 * Opaque runtime handle for managing Tokio runtime
 */
typedef struct RuntimeHandle RuntimeHandle;

/**
 * Create a new Tokio runtime
 * @return Runtime handle, or NULL on failure
 */
RuntimeHandle* create_runtime(void);

/**
 * Free a Tokio runtime
 * @param runtime_handle Runtime handle to free
 * @note This function is unsafe - runtime_handle must be a valid pointer or NULL
 */
void free_runtime(RuntimeHandle* runtime_handle);

/**
 * Calculate MD5 hash of a file asynchronously
 * @param runtime_handle Runtime handle for async execution
 * @param file_path Path to the file
 * @param callback Callback function to call when done
 * @param user_data User data to pass to callback
 * @note This function is unsafe - runtime_handle and file_path must be valid pointers
 */
void calculate_md5_hash_c(RuntimeHandle* runtime_handle, const char* file_path, async_callback_t callback, void* user_data);

#ifdef __cplusplus
}
#endif

#endif // ASYNC_FILE_HASHER_H
