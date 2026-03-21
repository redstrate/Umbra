// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use crate::{ffi_to_c_string, physis_Buffer};
use physis::Platform;
use physis::ReadableFile;
use physis::mtrl::Constant;
use physis::mtrl::Material;
use physis::mtrl::Sampler;
use physis::mtrl::ShaderKey;
use physis::mtrl::{ColorTable, DawntrailColorTableRow, LegacyColorTableRow};
use std::os::raw::c_char;
use std::ptr::{null, null_mut};
use std::{mem, slice};

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_LegacyColorTable {
    num_rows: u32,
    rows: *mut LegacyColorTableRow,
}

impl Default for physis_LegacyColorTable {
    fn default() -> Self {
        Self {
            num_rows: 0,
            rows: null_mut(),
        }
    }
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_DawntrailColorTable {
    num_rows: u32,
    rows: *mut DawntrailColorTableRow,
}

impl Default for physis_DawntrailColorTable {
    fn default() -> Self {
        Self {
            num_rows: 0,
            rows: null_mut(),
        }
    }
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_Material {
    shpk_name: *const c_char,
    num_textures: u32,
    textures: *mut *const c_char,
    num_shader_keys: u32,
    shader_keys: *mut ShaderKey,
    num_constants: u32,
    constants: *mut Constant,
    num_samplers: u32,
    samplers: *mut Sampler,
    legacy_color_table: physis_LegacyColorTable,
    dawntrail_color_table: physis_DawntrailColorTable,
}

impl Default for physis_Material {
    fn default() -> Self {
        Self {
            shpk_name: null(),
            num_textures: 0,
            textures: null_mut(),
            num_shader_keys: 0,
            shader_keys: null_mut(),
            num_constants: 0,
            constants: null_mut(),
            num_samplers: 0,
            samplers: null_mut(),
            legacy_color_table: Default::default(),
            dawntrail_color_table: Default::default(),
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_material_parse(
    platform: Platform,
    buffer: physis_Buffer,
) -> physis_Material {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(material) = Material::from_existing(platform, data) {
        let mut c_strings = vec![];

        for tex in &material.texture_paths {
            c_strings.push(ffi_to_c_string(tex));
        }

        let mut shader_keys = material.shader_keys.clone();
        let mut constants = material.constants.clone();
        let mut samplers = material.samplers.clone();
        let mut legacy_rows = vec![];
        let mut dawntrail_rows = vec![];

        let legacy_color_table = match &material.color_table {
            Some(ColorTable::LegacyColorTable(data)) => {
                legacy_rows.clone_from(&data.rows);

                physis_LegacyColorTable {
                    num_rows: legacy_rows.len() as u32,
                    rows: if legacy_rows.is_empty() {
                        null_mut()
                    } else {
                        legacy_rows.as_mut_ptr()
                    },
                }
            }
            _ => physis_LegacyColorTable::default(),
        };

        let dawntrail_color_table = match &material.color_table {
            Some(ColorTable::DawntrailColorTable(data)) => {
                dawntrail_rows.clone_from(&data.rows);

                physis_DawntrailColorTable {
                    num_rows: dawntrail_rows.len() as u32,
                    rows: if dawntrail_rows.is_empty() {
                        null_mut()
                    } else {
                        dawntrail_rows.as_mut_ptr()
                    },
                }
            }
            _ => physis_DawntrailColorTable::default(),
        };

        let mat = physis_Material {
            shpk_name: ffi_to_c_string(&material.shader_package_name),
            num_textures: c_strings.len() as u32,
            textures: c_strings.as_mut_ptr(),
            num_shader_keys: shader_keys.len() as u32,
            shader_keys: shader_keys.as_mut_ptr(),
            num_constants: constants.len() as u32,
            constants: constants.as_mut_ptr(),
            num_samplers: samplers.len() as u32,
            samplers: samplers.as_mut_ptr(),
            legacy_color_table,
            dawntrail_color_table,
        };

        mem::forget(c_strings);
        mem::forget(shader_keys);
        mem::forget(constants);
        mem::forget(samplers);
        mem::forget(legacy_rows);
        mem::forget(dawntrail_rows);

        mat
    } else {
        physis_Material::default()
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn physis_mtrl_debug(
    platform: Platform,
    buffer: physis_Buffer,
) -> *const c_char {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(mtrl) = Material::from_existing(platform, data) {
        ffi_to_c_string(&format!("{mtrl:#?}"))
    } else {
        null()
    }
}
