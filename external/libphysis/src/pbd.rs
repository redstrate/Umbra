// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use crate::{ffi_to_c_string, physis_Buffer};
use physis::Platform;
use physis::ReadableFile;
use physis::pbd::PreBoneDeformer;
use std::os::raw::c_char;
use std::ptr::{null, null_mut};
use std::{mem, slice};

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_PBD {
    p_ptr: *mut PreBoneDeformer,
}

impl Default for physis_PBD {
    fn default() -> Self {
        Self { p_ptr: null_mut() }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_pbd_parse(platform: Platform, buffer: physis_Buffer) -> physis_PBD {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(pbd) = PreBoneDeformer::from_existing(platform, data) {
        physis_PBD {
            p_ptr: Box::leak(Box::new(pbd)),
        }
    } else {
        physis_PBD::default()
    }
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_PreBoneDeformBone {
    name: *const c_char,
    deform: [f32; 12],
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_PreBoneDeformMatrices {
    num_bones: i32,
    bones: *mut physis_PreBoneDeformBone,
}

impl Default for physis_PreBoneDeformMatrices {
    fn default() -> Self {
        Self {
            num_bones: 0,
            bones: null_mut(),
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_pbd_get_deform_matrix(
    pbd: physis_PBD,
    from_body_id: u16,
    to_body_id: u16,
) -> physis_PreBoneDeformMatrices {
    unsafe {
        if let Some(prebd) = (*pbd.p_ptr).get_deform_matrices(from_body_id, to_body_id) {
            let mut c_bones = vec![];

            for bone in &prebd.bones {
                c_bones.push(physis_PreBoneDeformBone {
                    name: ffi_to_c_string(&bone.name),
                    deform: bone.deform,
                });
            }

            let mat = physis_PreBoneDeformMatrices {
                num_bones: c_bones.len() as i32,
                bones: c_bones.as_mut_ptr(),
            };

            mem::forget(c_bones);

            mat
        } else {
            physis_PreBoneDeformMatrices::default()
        }
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn physis_pbd_debug(
    platform: Platform,
    buffer: physis_Buffer,
) -> *const c_char {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(pbd) = PreBoneDeformer::from_existing(platform, data) {
        ffi_to_c_string(&format!("{pbd:#?}"))
    } else {
        null()
    }
}
