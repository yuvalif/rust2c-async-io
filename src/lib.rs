use std::ffi::{CStr, CString};
use std::fs::File;
use std::io::Read;
use std::os::raw::c_char;
use tokio::fs::File as AsyncFile;
use tokio::io::AsyncReadExt;
use tokio::runtime::Runtime;

pub async fn calculate_md5_hash(
    file_path: &str,
) -> Result<String, Box<dyn std::error::Error + Send + Sync>> {
    let mut file = AsyncFile::open(file_path).await?;
    let mut context = md5::Context::new();
    let mut buffer = vec![0u8; 4 * 1024 * 1024]; // 4MB buffer

    loop {
        let bytes_read = file.read(&mut buffer).await?;
        if bytes_read == 0 {
            break; // End of file
        }

        context.consume(&buffer[..bytes_read]);

        // Yield control to allow other tasks to run
        tokio::task::yield_now().await;
    }

    let digest = context.compute();
    Ok(format!("{digest:x}"))
}

pub fn calculate_md5_hash_sync(
    file_path: &str,
) -> Result<String, Box<dyn std::error::Error + Send + Sync>> {
    let mut file = File::open(file_path)?;
    let mut context = md5::Context::new();
    let mut buffer = vec![0u8; 4 * 1024 * 1024]; // 4MB buffer

    loop {
        let bytes_read = file.read(&mut buffer)?;
        if bytes_read == 0 {
            break; // End of file
        }

        context.consume(&buffer[..bytes_read]);
    }

    let digest = context.compute();
    Ok(format!("{digest:x}"))
}

/// # Safety
///
/// This function is unsafe because it dereferences a raw pointer.
/// The caller must ensure that `file_path` is a valid, null-terminated string pointer.
#[no_mangle]
pub unsafe extern "C" fn calculate_md5_hash_sync_c(file_path: *const c_char) -> *mut c_char {
    if file_path.is_null() {
        return std::ptr::null_mut();
    }

    let c_str = CStr::from_ptr(file_path);
    let file_path_str = match c_str.to_str() {
        Ok(s) => s,
        Err(_) => return std::ptr::null_mut(),
    };

    match calculate_md5_hash_sync(file_path_str) {
        Ok(hash) => match CString::new(hash) {
            Ok(c_string) => c_string.into_raw(),
            Err(_) => std::ptr::null_mut(),
        },
        Err(_) => std::ptr::null_mut(),
    }
}

/// # Safety
///
/// This function is unsafe because it dereferences a raw pointer.
/// The caller must ensure that `s` is either NULL or a valid pointer previously returned
/// by one of this library's functions.
#[no_mangle]
pub unsafe extern "C" fn free_string(s: *mut c_char) {
    if !s.is_null() {
        let _ = CString::from_raw(s);
    }
}

// Opaque runtime handle type
pub struct RuntimeHandle {
    runtime: Runtime,
}

// Callback type for async operations
type AsyncCallback = extern "C" fn(*mut c_char, *mut std::ffi::c_void);

#[no_mangle]
pub extern "C" fn create_runtime() -> *mut RuntimeHandle {
    match Runtime::new() {
        Ok(runtime) => {
            let handle = RuntimeHandle { runtime };
            Box::into_raw(Box::new(handle))
        }
        Err(_) => std::ptr::null_mut(),
    }
}

/// # Safety
///
/// This function is unsafe because it dereferences a raw pointer.
/// The caller must ensure that `runtime_handle` is either NULL or a valid pointer
/// previously returned by `create_runtime`.
#[no_mangle]
pub unsafe extern "C" fn free_runtime(runtime_handle: *mut RuntimeHandle) {
    if !runtime_handle.is_null() {
        let _ = Box::from_raw(runtime_handle);
    }
}

/// # Safety
///
/// This function is unsafe because it dereferences raw pointers.
/// The caller must ensure that:
/// - `runtime_handle` is a valid pointer returned by `create_runtime`
/// - `file_path` is a valid, null-terminated string pointer
/// - `callback` is a valid function pointer
/// - `user_data` can be safely passed to the callback
#[no_mangle]
pub unsafe extern "C" fn calculate_md5_hash_c(
    runtime_handle: *mut RuntimeHandle,
    file_path: *const c_char,
    callback: AsyncCallback,
    user_data: *mut std::ffi::c_void,
) {
    if runtime_handle.is_null() || file_path.is_null() {
        callback(std::ptr::null_mut(), user_data);
        return;
    }

    let runtime = &(*runtime_handle).runtime;

    let c_str = CStr::from_ptr(file_path);
    let file_path_str = match c_str.to_str() {
        Ok(s) => s.to_string(),
        Err(_) => {
            callback(std::ptr::null_mut(), user_data);
            return;
        }
    };

    //let callback_fn = callback;
    let user_data_ptr = user_data as usize; // Convert to usize for Send

    // Spawn the async task on the Tokio runtime
    runtime.spawn(async move {
        let result = calculate_md5_hash(&file_path_str).await;
        let hash_ptr = match result {
            Ok(hash) => match CString::new(hash) {
                Ok(c_string) => c_string.into_raw(),
                Err(_) => std::ptr::null_mut(),
            },
            Err(_) => std::ptr::null_mut(),
        };

        // Convert back to raw pointer and call callback
        let user_data_restored = user_data_ptr as *mut std::ffi::c_void;
        callback(hash_ptr, user_data_restored);
    });
}

