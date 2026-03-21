// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use std::ffi::{CStr, CString};
use std::fs::read;
use std::mem;
use std::os::raw::c_char;
use std::ptr::{null, null_mut};

/// Convert from a C string to a proper Rust string
fn ffi_from_c_string(ptr: *const c_char) -> Option<String> {
    unsafe {
        if let Ok(str) = CStr::from_ptr(ptr as *mut i8).to_str() {
            Some(str.to_string())
        } else {
            None
        }
    }
}

/// Convert from a Rust string to a C string
fn ffi_to_c_string(s: &String) -> *const c_char {
    let s = CString::new(s.as_bytes());

    match s {
        Ok(x) => x.into_raw(),
        Err(_) => null(),
    }
}

/// Convert from a C vector (which is just a pointer and a length) to a Rust Vec
fn ffi_to_vec<T>(ptr: *mut T, count: u32) -> Vec<T> {
    unsafe { Vec::from_raw_parts(ptr, count as usize, count as usize) }
}

/// Free a C string
fn ffi_free_string(ptr: *const c_char) {
    unsafe {
        let str = CString::from_raw(ptr as *mut i8);
        drop(str);
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_get_physis_version() -> *const c_char {
    ffi_to_c_string(&physis::PHYSIS_VERSION.to_string())
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_get_libphysis_version() -> *const c_char {
    ffi_to_c_string(&env!("CARGO_PKG_VERSION").to_string())
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_Buffer {
    size: u32,
    data: *mut u8,
}

impl Default for physis_Buffer {
    fn default() -> Self {
        Self {
            size: 0,
            data: null_mut(),
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_read_file(path: *const c_char) -> physis_Buffer {
    let Some(r_path) = ffi_from_c_string(path) else {
        return physis_Buffer::default();
    };

    let Ok(mut f) = read(r_path) else {
        return physis_Buffer::default();
    };

    let buf = physis_Buffer {
        size: f.len() as u32,
        data: f.as_mut_ptr(),
    };

    mem::forget(f);

    buf
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_free_file(buffer: &physis_Buffer) {
    let bytes = ffi_to_vec(buffer.data, buffer.size);
    drop(bytes);
}

mod bootdata;

mod model;

mod race;

mod exl;

mod equipment;

mod blowfish;

mod exh;

mod exd;

mod skeleton;

mod tex;

mod mtrl;

mod shpk;

mod cmp;

mod chardat;

mod cfg;

mod pbd;

mod tera;

mod dic;

mod index;

mod existing_dirs;

mod shcd;

mod execlookup;

mod patchlist;

mod layer;

mod patch;

mod hwc;

mod resource;

mod lgb;

mod sgb;

mod lvb;

mod scn;

mod tmb;

mod uld;

mod cutb;

mod avfx;

mod pcb;

mod scd;

mod lcb;

mod uwb;

mod svb;

mod stm;

mod envb;

mod essb;

mod amb;

mod obsb;

mod pap;
