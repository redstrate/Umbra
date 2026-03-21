// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use crate::exh::physis_EXH;
use crate::{ffi_from_c_string, ffi_to_c_string, physis_Buffer};
use physis::exd::EXD;
use physis::{Language, Platform, ReadableFile};
use std::os::raw::{c_char, c_uint};
use std::ptr::{null, null_mut};
use std::slice;

#[repr(C)]
#[allow(dead_code)]
pub enum physis_Field {
    String(*const c_char),
    Bool(bool),
    Int8(i8),
    UInt8(u8),
    Int16(i16),
    UInt16(u16),
    Int32(i32),
    UInt32(u32),
    Float32(f32),
    Int64(i64),
    UInt64(u64),
}

#[repr(C)]
pub struct physis_ExcelRow {
    pub subrow_id: u16,
    pub columns: *mut physis_Field,
}

impl Default for physis_ExcelRow {
    fn default() -> Self {
        Self {
            subrow_id: 0,
            columns: null_mut(),
        }
    }
}

#[repr(C)]
pub struct physis_ExcelEntry {
    pub row_id: u32,
    pub subrows: *mut physis_ExcelRow,
    pub subrow_count: c_uint,
}

impl Default for physis_ExcelEntry {
    fn default() -> Self {
        Self {
            row_id: 0,
            subrows: null_mut(),
            subrow_count: 0,
        }
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn physis_exd_calculate_filename(
    name: *const c_char,
    exh: &physis_EXH,
    language: Language,
    page: c_uint,
) -> *const c_char {
    unsafe {
        let Some(r_name) = ffi_from_c_string(name) else {
            return null();
        };

        ffi_to_c_string(&EXD::calculate_filename(
            &r_name,
            language,
            &(&(*exh.p_ptr).pages)[page as usize],
        ))
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn physis_exd_debug(
    platform: Platform,
    buffer: physis_Buffer,
) -> *const std::ffi::c_char {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(exd) = EXD::from_existing(platform, data) {
        ffi_to_c_string(&format!("{exd:#?}"))
    } else {
        null()
    }
}
