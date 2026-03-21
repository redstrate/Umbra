// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use crate::{ffi_to_c_string, physis_Buffer};
use physis::Platform;
use physis::ReadableFile;
use physis::exl::EXL;
use std::os::raw::c_char;
use std::ptr::null;
use std::{mem, slice};

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_EXL {
    version: i32,
    entry_count: i32,
    entry_keys: *const *const c_char,
    entry_values: *const i32,
}

impl Default for physis_EXL {
    fn default() -> Self {
        Self {
            version: 0,
            entry_count: 0,
            entry_keys: null(),
            entry_values: null(),
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_exl_parse(platform: Platform, buffer: physis_Buffer) -> physis_EXL {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(exl) = EXL::from_existing(platform, data) {
        let mut c_keys = vec![];
        let mut c_values = vec![];

        for (key, value) in &exl.entries {
            c_keys.push(ffi_to_c_string(key));
            c_values.push(*value);
        }

        let mat = physis_EXL {
            version: exl.version,
            entry_count: c_keys.len() as i32,
            entry_keys: c_keys.as_mut_ptr(),
            entry_values: c_values.as_mut_ptr(),
        };

        mem::forget(c_keys);
        mem::forget(c_values);

        mat
    } else {
        physis_EXL::default()
    }
}
