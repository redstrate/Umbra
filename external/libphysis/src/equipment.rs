// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use crate::{ffi_from_c_string, ffi_to_c_string};
use physis::equipment::{
    CharacterCategory, Slot, build_character_path, build_ear_material_path, build_equipment_path,
    build_face_material_path, build_gear_material_path, build_hair_material_path,
    build_skin_material_path, build_tail_material_path, get_slot_abbreviation, get_slot_from_id,
};
use physis::race::{Gender, Race, Tribe};
use std::os::raw::c_char;
use std::ptr::null;

#[unsafe(no_mangle)]
pub extern "C" fn physis_slot_from_id(slot_id: i32) -> Slot {
    get_slot_from_id(slot_id)
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_get_slot_name(slot: Slot) -> *const c_char {
    // TODO: no need to dynamically allocate a new string
    if let Some(abbr) = get_slot_abbreviation(slot) {
        ffi_to_c_string(&abbr.to_string())
    } else {
        null()
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_build_equipment_path(
    model_id: i32,
    race: Race,
    tribe: Tribe,
    gender: Gender,
    slot: Slot,
) -> *const c_char {
    ffi_to_c_string(&build_equipment_path(model_id, race, tribe, gender, slot))
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_build_character_path(
    category: CharacterCategory,
    body_ver: i32,
    race: Race,
    tribe: Tribe,
    gender: Gender,
) -> *const c_char {
    ffi_to_c_string(&build_character_path(
        category, body_ver, race, tribe, gender,
    ))
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_build_skin_material_path(
    race_code: i32,
    body_code: i32,
    material_name: *const c_char,
) -> *const c_char {
    let Some(r_material_name) = ffi_from_c_string(material_name) else {
        return null();
    };

    ffi_to_c_string(&build_skin_material_path(
        race_code,
        body_code,
        &r_material_name,
    ))
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_build_gear_material_path(
    gear_id: i32,
    gear_version: i32,
    material_name: *const c_char,
) -> *const c_char {
    let Some(r_material_name) = ffi_from_c_string(material_name) else {
        return null();
    };

    ffi_to_c_string(&build_gear_material_path(
        gear_id,
        gear_version,
        &r_material_name,
    ))
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_build_face_material_path(
    race_code: i32,
    face_code: i32,
    material_name: *const c_char,
) -> *const c_char {
    let Some(r_material_name) = ffi_from_c_string(material_name) else {
        return null();
    };

    ffi_to_c_string(&build_face_material_path(
        race_code,
        face_code,
        &r_material_name,
    ))
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_build_hair_material_path(
    race_code: i32,
    hair_code: i32,
    material_name: *const c_char,
) -> *const c_char {
    let Some(r_material_name) = ffi_from_c_string(material_name) else {
        return null();
    };

    ffi_to_c_string(&build_hair_material_path(
        race_code,
        hair_code,
        &r_material_name,
    ))
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_build_ear_material_path(
    race_code: i32,
    ear_code: i32,
    material_name: *const c_char,
) -> *const c_char {
    let Some(r_material_name) = ffi_from_c_string(material_name) else {
        return null();
    };

    ffi_to_c_string(&build_ear_material_path(
        race_code,
        ear_code,
        &r_material_name,
    ))
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_build_tail_material_path(
    race_code: i32,
    tail_code: i32,
    material_name: *const c_char,
) -> *const c_char {
    let Some(r_material_name) = ffi_from_c_string(material_name) else {
        return null();
    };

    ffi_to_c_string(&build_tail_material_path(
        race_code,
        tail_code,
        &r_material_name,
    ))
}
