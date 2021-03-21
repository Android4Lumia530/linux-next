// SPDX-License-Identifier: GPL-2.0

//! File operations.
//!
//! C header: [`include/linux/fs.h`](../../../../include/linux/fs.h)

use core::convert::{TryFrom, TryInto};
use core::{marker, mem, ptr};

use alloc::boxed::Box;
use alloc::sync::Arc;

use crate::bindings;
use crate::c_types;
use crate::error::{Error, KernelResult};
use crate::user_ptr::{UserSlicePtr, UserSlicePtrReader, UserSlicePtrWriter};

/// Wraps the kernel's `struct file`.
pub struct File {
    ptr: *const bindings::file,
}

impl File {
    unsafe fn from_ptr(ptr: *const bindings::file) -> File {
        File { ptr }
    }

    /// Returns the current seek/cursor/pointer position (`struct file::f_pos`).
    pub fn pos(&self) -> u64 {
        unsafe { (*self.ptr).f_pos as u64 }
    }
}

/// Equivalent to [`std::io::SeekFrom`].
///
/// [`std::io::SeekFrom`]: https://doc.rust-lang.org/std/io/enum.SeekFrom.html
pub enum SeekFrom {
    /// Equivalent to C's `SEEK_SET`.
    Start(u64),

    /// Equivalent to C's `SEEK_END`.
    End(i64),

    /// Equivalent to C's `SEEK_CUR`.
    Current(i64),
}

fn from_kernel_result<T>(r: KernelResult<T>) -> T
where
    T: TryFrom<c_types::c_int>,
    T::Error: core::fmt::Debug,
{
    match r {
        Ok(v) => v,
        Err(e) => T::try_from(e.to_kernel_errno()).unwrap(),
    }
}

macro_rules! from_kernel_result {
    ($($tt:tt)*) => {{
        from_kernel_result((|| {
            $($tt)*
        })())
    }};
}

unsafe extern "C" fn open_callback<T: FileOperations>(
    _inode: *mut bindings::inode,
    file: *mut bindings::file,
) -> c_types::c_int {
    from_kernel_result! {
        let ptr = T::open()?.into_pointer();
        (*file).private_data = ptr as *mut c_types::c_void;
        Ok(0)
    }
}

unsafe extern "C" fn read_callback<T: FileOperations>(
    file: *mut bindings::file,
    buf: *mut c_types::c_char,
    len: c_types::c_size_t,
    offset: *mut bindings::loff_t,
) -> c_types::c_ssize_t {
    from_kernel_result! {
        let mut data = UserSlicePtr::new(buf as *mut c_types::c_void, len)?.writer();
        let f = &*((*file).private_data as *const T);
        // No `FMODE_UNSIGNED_OFFSET` support, so `offset` must be in [0, 2^63).
        // See discussion in https://github.com/fishinabarrel/linux-kernel-module-rust/pull/113
        T::read(f, &File::from_ptr(file), &mut data, (*offset).try_into()?)?;
        let written = len - data.len();
        (*offset) += bindings::loff_t::try_from(written).unwrap();
        Ok(written.try_into().unwrap())
    }
}

unsafe extern "C" fn write_callback<T: FileOperations>(
    file: *mut bindings::file,
    buf: *const c_types::c_char,
    len: c_types::c_size_t,
    offset: *mut bindings::loff_t,
) -> c_types::c_ssize_t {
    from_kernel_result! {
        let mut data = UserSlicePtr::new(buf as *mut c_types::c_void, len)?.reader();
        let f = &*((*file).private_data as *const T);
        // No `FMODE_UNSIGNED_OFFSET` support, so `offset` must be in [0, 2^63).
        // See discussion in https://github.com/fishinabarrel/linux-kernel-module-rust/pull/113
        T::write(f, &mut data, (*offset).try_into()?)?;
        let read = len - data.len();
        (*offset) += bindings::loff_t::try_from(read).unwrap();
        Ok(read.try_into().unwrap())
    }
}

unsafe extern "C" fn release_callback<T: FileOperations>(
    _inode: *mut bindings::inode,
    file: *mut bindings::file,
) -> c_types::c_int {
    let ptr = mem::replace(&mut (*file).private_data, ptr::null_mut());
    T::release(T::Wrapper::from_pointer(ptr as _), &File::from_ptr(file));
    0
}

unsafe extern "C" fn llseek_callback<T: FileOperations>(
    file: *mut bindings::file,
    offset: bindings::loff_t,
    whence: c_types::c_int,
) -> bindings::loff_t {
    from_kernel_result! {
        let off = match whence as u32 {
            bindings::SEEK_SET => SeekFrom::Start(offset.try_into()?),
            bindings::SEEK_CUR => SeekFrom::Current(offset),
            bindings::SEEK_END => SeekFrom::End(offset),
            _ => return Err(Error::EINVAL),
        };
        let f = &*((*file).private_data as *const T);
        let off = T::seek(f, &File::from_ptr(file), off)?;
        Ok(off as bindings::loff_t)
    }
}

unsafe extern "C" fn fsync_callback<T: FileOperations>(
    file: *mut bindings::file,
    start: bindings::loff_t,
    end: bindings::loff_t,
    datasync: c_types::c_int,
) -> c_types::c_int {
    from_kernel_result! {
        let start = start.try_into()?;
        let end = end.try_into()?;
        let datasync = datasync != 0;
        let f = &*((*file).private_data as *const T);
        let res = T::fsync(f, &File::from_ptr(file), start, end, datasync)?;
        Ok(res.try_into().unwrap())
    }
}

pub(crate) struct FileOperationsVtable<T>(marker::PhantomData<T>);

impl<T: FileOperations> FileOperationsVtable<T> {
    pub(crate) const VTABLE: bindings::file_operations = bindings::file_operations {
        open: Some(open_callback::<T>),
        release: Some(release_callback::<T>),
        read: if T::TO_USE.read {
            Some(read_callback::<T>)
        } else {
            None
        },
        write: if T::TO_USE.write {
            Some(write_callback::<T>)
        } else {
            None
        },
        llseek: if T::TO_USE.seek {
            Some(llseek_callback::<T>)
        } else {
            None
        },

        check_flags: None,
        compat_ioctl: None,
        copy_file_range: None,
        fallocate: None,
        fadvise: None,
        fasync: None,
        flock: None,
        flush: None,
        fsync: if T::TO_USE.fsync {
            Some(fsync_callback::<T>)
        } else {
            None
        },
        get_unmapped_area: None,
        iterate: None,
        iterate_shared: None,
        iopoll: None,
        lock: None,
        mmap: None,
        mmap_supported_flags: 0,
        owner: ptr::null_mut(),
        poll: None,
        read_iter: None,
        remap_file_range: None,
        sendpage: None,
        setlease: None,
        show_fdinfo: None,
        splice_read: None,
        splice_write: None,
        unlocked_ioctl: None,
        write_iter: None,
    };
}

/// Represents which fields of [`struct file_operations`] should be populated with pointers.
pub struct ToUse {
    /// The `read` field of [`struct file_operations`].
    pub read: bool,

    /// The `write` field of [`struct file_operations`].
    pub write: bool,

    /// The `llseek` field of [`struct file_operations`].
    pub seek: bool,

    /// The `fsync` field of [`struct file_operations`].
    pub fsync: bool,
}

/// A constant version where all values are to set to `false`, that is, all supported fields will
/// be set to null pointers.
pub const USE_NONE: ToUse = ToUse {
    read: false,
    write: false,
    seek: false,
    fsync: false,
};

/// Defines the [`FileOperations::TO_USE`] field based on a list of fields to be populated.
#[macro_export]
macro_rules! declare_file_operations {
    () => {
        const TO_USE: $crate::file_operations::ToUse = $crate::file_operations::USE_NONE;
    };
    ($($i:ident),+) => {
        const TO_USE: kernel::file_operations::ToUse =
            $crate::file_operations::ToUse {
                $($i: true),+ ,
                ..$crate::file_operations::USE_NONE
            };
    };
}

/// Corresponds to the kernel's `struct file_operations`.
///
/// You implement this trait whenever you would create a `struct file_operations`.
///
/// File descriptors may be used from multiple threads/processes concurrently, so your type must be
/// [`Sync`].
pub trait FileOperations: Sync + Sized {
    /// The methods to use to populate [`struct file_operations`].
    const TO_USE: ToUse;

    /// The pointer type that will be used to hold ourselves.
    type Wrapper: PointerWrapper<Self>;

    /// Creates a new instance of this file.
    ///
    /// Corresponds to the `open` function pointer in `struct file_operations`.
    fn open() -> KernelResult<Self::Wrapper>;

    /// Cleans up after the last reference to the file goes away.
    ///
    /// Note that the object is moved, so it will be freed automatically unless the implementation
    /// moves it elsewhere.
    ///
    /// Corresponds to the `release` function pointer in `struct file_operations`.
    fn release(_obj: Self::Wrapper, _file: &File) {}

    /// Reads data from this file to userspace.
    ///
    /// Corresponds to the `read` function pointer in `struct file_operations`.
    fn read(&self, _file: &File, _data: &mut UserSlicePtrWriter, _offset: u64) -> KernelResult<()> {
        Err(Error::EINVAL)
    }

    /// Writes data from userspace to this file.
    ///
    /// Corresponds to the `write` function pointer in `struct file_operations`.
    fn write(&self, _data: &mut UserSlicePtrReader, _offset: u64) -> KernelResult<isize> {
        Err(Error::EINVAL)
    }

    /// Changes the position of the file.
    ///
    /// Corresponds to the `llseek` function pointer in `struct file_operations`.
    fn seek(&self, _file: &File, _offset: SeekFrom) -> KernelResult<u64> {
        Err(Error::EINVAL)
    }

    /// Syncs pending changes to this file.
    ///
    /// Corresponds to the `fsync` function pointer in `struct file_operations`.
    fn fsync(&self, _file: &File, _start: u64, _end: u64, _datasync: bool) -> KernelResult<u32> {
        Err(Error::EINVAL)
    }
}

/// Used to convert an object into a raw pointer that represents it.
///
/// It can eventually be converted back into the object. This is used to store objects as pointers
/// in kernel data structures, for example, an implementation of [`FileOperations`] in `struct
/// file::private_data`.
pub trait PointerWrapper<T> {
    /// Returns the raw pointer.
    fn into_pointer(self) -> *const T;

    /// Returns the instance back from the raw pointer.
    ///
    /// # Safety
    ///
    /// The passed pointer must come from a previous call to [`PointerWrapper::into_pointer()`].
    unsafe fn from_pointer(ptr: *const T) -> Self;
}

impl<T> PointerWrapper<T> for Box<T> {
    fn into_pointer(self) -> *const T {
        Box::into_raw(self)
    }

    unsafe fn from_pointer(ptr: *const T) -> Self {
        Box::<T>::from_raw(ptr as _)
    }
}

impl<T> PointerWrapper<T> for Arc<T> {
    fn into_pointer(self) -> *const T {
        Arc::into_raw(self)
    }

    unsafe fn from_pointer(ptr: *const T) -> Self {
        Arc::<T>::from_raw(ptr)
    }
}
