// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use crate::physis_Buffer;
use physis::Platform;
use physis::ReadableFile;
use physis::hwc::Hwc;
use std::mem;
use std::ptr::null;
use std::slice;

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_HWC {
    rgba: *const u8,
}

impl Default for physis_HWC {
    fn default() -> Self {
        Self { rgba: null() }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_hwc_parse(platform: Platform, buffer: physis_Buffer) -> physis_HWC {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(hwc) = Hwc::from_existing(platform, data) {
        let c_hwc = physis_HWC {
            rgba: hwc.rgba.as_ptr(),
        };

        mem::forget(hwc.rgba);

        c_hwc
    } else {
        physis_HWC::default()
    }
}
