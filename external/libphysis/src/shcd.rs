// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use crate::{ffi_to_c_string, physis_Buffer};
use physis::shcd::SHCD;
use physis::shcd::ShaderStage;
use physis::{Platform, ReadableFile};
use std::ffi::c_char;
use std::ptr::{null, null_mut};
use std::slice;

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_SHCD {
    stage: ShaderStage,
    len: u32,
    bytecode: *mut u8,
}

impl Default for physis_SHCD {
    fn default() -> Self {
        Self {
            stage: ShaderStage::Vertex,
            len: 0,
            bytecode: null_mut(),
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_shcd_parse(platform: Platform, buffer: physis_Buffer) -> physis_SHCD {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(shcd) = SHCD::from_existing(platform, data) {
        let mut c_bytecode = shcd.bytecode.clone();

        let shcd = physis_SHCD {
            stage: shcd.stage,
            len: c_bytecode.len() as u32,
            bytecode: c_bytecode.as_mut_ptr(),
        };

        std::mem::forget(c_bytecode);

        shcd
    } else {
        physis_SHCD::default()
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn physis_shcd_debug(
    platform: Platform,
    buffer: physis_Buffer,
) -> *const c_char {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(shcd) = SHCD::from_existing(platform, data) {
        ffi_to_c_string(&format!("{shcd:#?}"))
    } else {
        null()
    }
}
