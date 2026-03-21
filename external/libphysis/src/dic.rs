// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use crate::{ffi_to_c_string, physis_Buffer};
use physis::Platform;
use physis::ReadableFile;
use physis::dic::Dictionary;
use std::os::raw::c_char;
use std::ptr::null;
use std::{mem, slice};

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_Dictionary {
    num_words: i32,
    words: *const *const c_char,
}

impl Default for physis_Dictionary {
    fn default() -> Self {
        Self {
            num_words: 0,
            words: null(),
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_dictionary_parse(
    platform: Platform,
    buffer: physis_Buffer,
) -> physis_Dictionary {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(dic) = Dictionary::from_existing(platform, data) {
        let mut c_words = vec![];

        for word in &dic.words {
            c_words.push(ffi_to_c_string(word));
        }

        let mat = physis_Dictionary {
            num_words: c_words.len() as i32,
            words: c_words.as_ptr(),
        };

        mem::forget(c_words);

        mat
    } else {
        physis_Dictionary::default()
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn physis_dic_debug(
    platform: Platform,
    buffer: physis_Buffer,
) -> *const c_char {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(dic) = Dictionary::from_existing(platform, data) {
        ffi_to_c_string(&format!("{dic:#?}"))
    } else {
        null()
    }
}
