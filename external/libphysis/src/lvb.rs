// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use crate::scn::{physis_ScnSection, to_c_section};
use crate::{ffi_to_c_string, physis_Buffer};
use physis::Platform;
use physis::ReadableFile;
use physis::lvb::Lvb;
use std::ffi::c_char;
use std::ptr::null;
use std::slice;

#[repr(C)]
pub struct physis_Lvb {
    section_count: u32,
    sections: *const physis_ScnSection,
}

impl Default for physis_Lvb {
    fn default() -> Self {
        Self {
            section_count: 0,
            sections: null(),
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_lvb_parse(platform: Platform, buffer: physis_Buffer) -> physis_Lvb {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(lvb) = Lvb::from_existing(platform, data) {
        let mut c_sections = Vec::new();

        for section in &lvb.sections {
            c_sections.push(to_c_section(section))
        }

        let lvb = physis_Lvb {
            section_count: c_sections.len() as u32,
            sections: c_sections.as_ptr(),
        };

        std::mem::forget(c_sections);

        lvb
    } else {
        physis_Lvb::default()
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn physis_lvb_debug(
    platform: Platform,
    buffer: physis_Buffer,
) -> *const c_char {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(lvb) = Lvb::from_existing(platform, data) {
        ffi_to_c_string(&format!("{lvb:#?}"))
    } else {
        null()
    }
}
