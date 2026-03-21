// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use crate::{ffi_to_c_string, physis_Buffer};
use physis::ReadableFile;
use physis::exh::{ColumnDataType, EXH};
use physis::{Language, Platform};
use std::ffi::c_char;
use std::ptr::{null, null_mut};
use std::{mem, slice};

// TODO: re-use from Physis since their struct is also simple
#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_ColumnDefinition {
    data_type: ColumnDataType,
    offset: u16,
}

// TODO: re-use from Physis since their struct is also simple
#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_ExcelPage {
    start_id: u32,
    row_count: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_EXH {
    pub(crate) p_ptr: *mut EXH,
    page_count: u32,
    pages: *mut physis_ExcelPage,
    language_count: u32,
    languages: *mut Language,
    column_count: u32,
    row_count: u32,
    column_definitions: *mut physis_ColumnDefinition,
}

impl Default for physis_EXH {
    fn default() -> Self {
        Self {
            p_ptr: null_mut(),
            page_count: 0,
            pages: null_mut(),
            language_count: 0,
            languages: null_mut(),
            column_count: 0,
            row_count: 0,
            column_definitions: null_mut(),
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_exh_parse(platform: Platform, buffer: physis_Buffer) -> physis_EXH {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    let exh = EXH::from_existing(platform, data);
    if exh.is_none() {
        return physis_EXH::default();
    }

    let exh = Box::new(exh.unwrap());

    let mut c_languages: Vec<Language> = vec![];

    for lang in &exh.languages {
        c_languages.push(*lang);
    }

    let mut c_column_definitions: Vec<physis_ColumnDefinition> = vec![];

    for column in &exh.column_definitions {
        c_column_definitions.push(physis_ColumnDefinition {
            data_type: column.data_type,
            offset: column.offset,
        });
    }

    let mut c_pages: Vec<physis_ExcelPage> = vec![];

    for page in &exh.pages {
        c_pages.push(physis_ExcelPage {
            start_id: page.start_id,
            row_count: page.row_count,
        });
    }

    let page_len = exh.pages.len() as u32;
    let row_count = exh.header.row_count as u32;
    let column_count = exh.column_definitions.len() as u32;

    let repositories = physis_EXH {
        p_ptr: Box::leak(exh),
        page_count: page_len,
        language_count: c_languages.len() as u32,
        languages: c_languages.as_mut_ptr(),
        column_count,
        row_count,
        column_definitions: c_column_definitions.as_mut_ptr(),
        pages: c_pages.as_mut_ptr(),
    };

    mem::forget(c_languages);
    mem::forget(c_column_definitions);
    mem::forget(c_pages);

    repositories
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_exh_free(exh: &mut physis_EXH) {
    unsafe {
        drop(Box::from_raw(exh.p_ptr)); // TODO: free other things
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn physis_exh_debug(
    platform: Platform,
    buffer: physis_Buffer,
) -> *const c_char {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(pcb) = EXH::from_existing(platform, data) {
        ffi_to_c_string(&format!("{pcb:#?}"))
    } else {
        null()
    }
}
